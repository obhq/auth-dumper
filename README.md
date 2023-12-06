# Auth Info Dumper

This is a PS4 payload to dump [auth info](https://www.psdevwiki.com/ps4/Auth_Info) from a ELF, SELF, or eboot.bin file. This payload only works on firmware 9.00!

## How to use

1. Download .bin from [Github Actions](test) or build it yourself!
2. Setup your prefered payload loader, for Payload Guest, put the payload on the root of your usb under a folder named `payloads` or on your PS4 under `/data/payloads`
3. On the root of your USB, make a file named `dump-list.txt`
4. This payload can dump multiple auth-infos on one execution, so open `dump-list.txt` and format each line like this: `/readauthinfofrom/folder/file.self:/exportauthinfoto/file.bin` An example is below:
```
/system/sys/SceSysCore.elf:/data/dumpedSysCore.bin
/system/sys/SceVdecProxy.elf:/data/dumpedVdecProxy.bin
```
The program will dump SceSysCore, then SceVdecProxy. You can use the list above as a test as any PS4 with 9.00 will contain these files!
5. Plug the USB into your PS4, then run the payload.
6. Enjoy your newly dumped files!

- Note, if you're trying to dump a game's eboot.bin and encounter an error, try dumping the auth info from `app0-patch0-union` instead of just `app0`.

## Building

### Prerequisites

- Linux or WSL2
- [PS4 Payload SDK](https://github.com/Scene-Collective/ps4-payload-sdk)

### How to build

1. `cd` to extracted folder of SDK and run `sudo install.sh` to install the SDK.
2. `cd` to extracted folder of auth-dumper and run `make`
3. A compiled .bin file will be at the root of the auth-dumper folder
4. Enjoy!

## License

MIT
