#include <linux/init.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/fdtable.h>

#define MAX_WRITE_FILE_CWD 512

// Set module info
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phat T. Nguyen & Le Xuan Cuong");
MODULE_DESCRIPTION("Hook system call: open, write");


unsigned long ** syscall_table; // Global variable stores syscall table
asmlinkage int(*origin_open)(const char __user * , int, mode_t);
asmlinkage int(*origin_write)(unsigned int, const char __user * , size_t);

char cur_write_file[MAX_WRITE_FILE_CWD]; //used for hook_write: just printk info of file that have just been opened

//////////////////////////////////////////////////////////////////////////////////
// HOOK AREA

asmlinkage int hook_open(const char __user * pathname, int flags, mode_t mode) {
	// hook: print info to dmesg
    printk(KERN_INFO "HOOK open syscall\n");
    printk(KERN_INFO "HOOK: [%d][%s]: open file %s\n", current->pid, current->comm, pathname);
    strcpy(cur_write_file, pathname);
    // call original syscall
    return (*origin_open)(pathname, flags, mode);
}

asmlinkage int hook_write(unsigned int fd, const char __user * buf, size_t count) {
	// find pathname given file_descriptor
	struct files_struct *files = current->files;
	char* pathname;
	char* tmp;
	struct file *file;
	struct path *path;

	spin_lock(&files->file_lock);
	
	// check whether fd has an open file
	file = fcheck_files(files, fd);
	if (!file) {
    	spin_unlock(&files->file_lock);
    	return -ENOENT;
	}
	path = &file->f_path;
	path_get(path);
	spin_unlock(&files->file_lock);

	tmp = (char*)__get_free_page(GFP_KERNEL);

	if (!tmp) {
		path_put(path);
		return -ENOMEM;
	}

	pathname = (char*)d_path(path, tmp, PAGE_SIZE);
	path_put(path);

	if (IS_ERR(pathname)) {
		free_page((unsigned long)tmp);
		return PTR_ERR(pathname);
	}

	// Call original sys_write and Print info to dmesg
    int written_bytes = (*origin_write)(fd, buf, count);

    // Just printk files have just opened, avoid calling sys_write recursively
	if (strcmp(pathname, cur_write_file) == 0){
        printk(KERN_INFO "HOOK: [%d][%s]: write %d bytes to %s\n", current->pid, current->comm, written_bytes, pathname);
    }
	
	free_page((unsigned long)tmp); 
	// return 
    return written_bytes;
}


//////////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS: MANIPULATE SYSCALL TABLE

// Make syscall table readable
static void enable_write(void) {
    write_cr0(read_cr0() & (~0x10000));
}

// Make syscall table unreadable
static void disable_write(void) {
    write_cr0(read_cr0() | 0x10000);
}

// Get syscall table
static void get_syscall_table(void) {
    unsigned long int offset = PAGE_OFFSET;
    while (offset < ULLONG_MAX) {
        unsigned long **tmp_syscall_table = (unsigned long**) offset;
        
        // found system syscall table
        if (tmp_syscall_table[__NR_close] == (unsigned long*) sys_close) {
            syscall_table = tmp_syscall_table;
            return;
        }
        offset += sizeof(void *);
    }
    syscall_table = NULL;
}


//////////////////////////////////////////////////////////////////////////////////
// SET MODULE
static int init_hook_module(void) {
    printk(KERN_INFO "init HOOK module\n");
    
    //get syscall table
    get_syscall_table();
    // or use hard-code:
    // syscall_table = (void*)___
    
    if (syscall_table == NULL) {
        printk(KERN_ERR "cannot find syscall table\n");
        return -1;
    }
    
    // backup original syscall
    origin_open = (void *)syscall_table[__NR_open];
    origin_write = (void *)syscall_table[__NR_write];
    
    // make syscall table readable
    enable_write();
    
    // override syscall by our hooking functions
    syscall_table[__NR_open] = (unsigned long*)hook_open;
    syscall_table[__NR_write] = (unsigned long*)hook_write;
    
    // disable 
    disable_write();
    return 0;
}


static void exit_hook_module(void) {
    printk(KERN_INFO "exit HOOK module\n");
    
    if (syscall_table != NULL) {
        enable_write();
        // retrieve syscall
        syscall_table[__NR_open] = (unsigned long*)origin_open;
        syscall_table[__NR_write] = (unsigned long*)origin_write;
        disable_write();
    }
}

///////////////////////////////////////////////////////////////////////////////////
module_init(init_hook_module);
module_exit(exit_hook_module);
