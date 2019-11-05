# Operating System: Project 2
Get familiar with Linux system. Remember to run as root if you don't have permission when running any commands below
**Prerequisites**: Linux kernel 3.13.0-32

## Assignment 1: Linux kernel module, device file system, interact between user and kernel space
Folder: `random`
Module source code: `random.c`
Sample output in Terminal: `log`
Testing source code: `test.c`
### Install
Firstly, run `make` command to compile source code and create a kernel module.
```
>>> cd random
>>> make
```
If compile successfully, a `.ko` file should appear. 
Use `modinfo` to check the module's info
```
>>> modinfo random.ko
```

Secondly, insert or remove the module to/from Linux kernel with commands: `insmod`, `rmmod`
Use `lsmod` to see our current modules.
```
>>> insmod random.ko
>>> lsmod | grep random
```

### Test
Compile test.c and check the results. You can check `log` file to take a look at the result of our sample `dmesg` command.
```
>>> gcc test.c -o test
>>> ./test
>>> dmesg
``` 

## Assignment 2: Hook a system call
Folder: `hook`
Hooking source code: `hook.c`
Sample output in Terminal: `log`
Testing source code: `test.c`
### Install
Firstly, run `make` command to compile source code and create a hooking module.
```
>>> cd hook
>>> make
```
If compile successfully, a `.ko` file should appear. 
Use `modinfo` to check module's info
```
>>> modinfo hook.ko
```

Secondly, insert or remove the module to Linux kernel with commands: `insmod`, `rmmod`
Use `lsmod` to see our current modules.
```
>>> insmod hook.ko
>>> lsmod | grep hook
```

### Test
Compile test.c and check the results. You can check `log` file to take a look at the result of our sample `dmesg` command.
This process will print a string to `hookprofile.txt`. Run `dmesg` to see what happened inside kernel space.
```
>>> gcc test.c -o test
>>> ./test
>>> dmesg
``` 

For example, we can see the notification of writing syscall from our hooking module.
```
[ 3408.399612] HOOK: [765][cupsd]: write 237 bytes to /etc/cups/subscriptions.conf.N
[ 3408.399711] HOOK open syscall

[ 3414.760017] HOOK: [3353][test]: open file /home/tienphat/Documents/OS/hookprofile.txt
[ 3414.760096] HOOK: [3353][test]: write 76 bytes to /home/tienphat/Documents/OS/hookprofile.txt
```
