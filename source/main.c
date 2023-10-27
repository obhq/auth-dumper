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
	struct get_auth_req req;
	int fd;

	initKernel();
	initLibc();
	jailbreak();
	mmap_patch();
	initSysUtil();

	// get auth info
	req.file = "/system/sys/SceSysCore.elf";

	if (kexec(get_auth, &req) < 0) {
		printf_notification("Failed to dump auth info.");
		return 0;
	}

	// write to usb drive
	fd = open("/mnt/usb0/auth-info.bin", O_WRONLY | O_CREAT | O_TRUNC, 0777);

	if (fd < 0) {
		printf_notification("Cannot create auth-info.bin on the USB drive.");
		return 0;
	}

	write(fd, req.buf, 136);
	close(fd);

	printf_notification("Auth info dumped!");

	return 0;
}
