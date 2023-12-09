#include <ps4.h>

struct get_auth_req {
	const char *file;
	unsigned char buf[136];
};

struct get_auth_args {
	void *handler;
	struct get_auth_req *req;
};

typedef int (*kern_get_self_auth_info_t) (struct thread *, const char *, int, unsigned char *);
typedef int (*copyout_t) (const void *, void *, unsigned);

static int get_auth(struct thread *td, struct get_auth_args *args) {
	void *kernel_base;
	kern_get_self_auth_info_t kern_get_self_auth_info;
	copyout_t copyout;
	unsigned char info[136];
	int error;

	// get kernel functions
	kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K900_XFAST_SYSCALL];
	kern_get_self_auth_info = (kern_get_self_auth_info_t)(kernel_base + 0x1697B0);
	copyout = (copyout_t)(kernel_base + K900_COPYOUT);

	// get auth
	error = kern_get_self_auth_info(td, args->req->file, 0, info);

	if (error == 0) {
		error = copyout(info, args->req->buf, 136);
	}

	return error;
}

int _main(struct thread *td) {
    int fd, write_result;
    struct stat st;
    char *buf, *next;
    size_t i;
    struct get_auth_req req;

    initKernel();
    initLibc();
    jailbreak();
    mmap_patch();
    initSysUtil();

    // open list file
    fd = open("/mnt/usb0/dump-list.txt", O_RDONLY, 0);
    if (fd < 0) {
        printf_notification("Cannot open dump-list.txt on the USB drive.");
        return 0;
    }

    if (fstat(fd, &st) < 0) {
        printf_notification("Cannot determine the size of dump-list.txt on the USB drive.");
        close(fd);
        return 0;
    }

    // read list file
    buf = malloc(st.st_size + 1);
    if (!buf) {
        printf_notification("Failed to allocate memory.");
        close(fd);
        return 0;
    }

    i = 0;
    while (i < (size_t)st.st_size) {
        ssize_t n = read(fd, buf + i, st.st_size - i);
        if (n < 0) {
            printf_notification("Cannot read dump-list.txt on the USB drive.");
            free(buf);
            close(fd);
            return 0;
        } else if (n == 0) {
            break;
        }
        i += n;
    }

    buf[st.st_size] = '\0';
    close(fd);

    // parse list file
    next = buf;
    for (;;) {
        char *p;

        if (!*next) {
            break;
        }

        p = strchr(next, ':');
        if (!p) {
            printf_notification("dump-list.txt on the USB drive has invalid format.");
            free(buf);
            return 0;
        }
        *p = '\0';

        req.file = next;
        if (kexec(get_auth, &req) < 0) {
            printf_notification("Failed to dump auth info for\n%s", next);
            free(buf);
            return 0;
        }

        next = p + 1;
        p = strchr(next, '\n');
        if (p) {
            if (p[-1] == '\r') {
                p[-1] = '\0';
            }
            *p = '\0';
        }

        fd = open(next, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (fd < 0) {
            printf_notification("Cannot create\n%s", next);
            free(buf);
            return 0;
        }

        write_result = write(fd, req.buf, 136);
        if (write_result < 0 || write_result != 136) {
            printf_notification("Failed to write auth info.");
            close(fd);
            free(buf);
            return 0;
        }
        close(fd);

        if (!p) { // if there's no newline, end the loop
            break;
        }

        next = p + 1;
    }

    free(buf);
    printf_notification("Auth info dump complete!");

    return 0;
}
