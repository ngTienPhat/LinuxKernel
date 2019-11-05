#include <linux/module.h>
#include <linux/random.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/device.h>

#define DRIVER_AUTHOR "Phat T. Nguyen & Le Xuan Cuong"
#define DRIVER_DESC "assignment 1 - random module"
#define DEVICE_NAME "RandomGenerator"
#define CLASS_NAME "RandomGeneratorClass"
#define LICENSE "GPL"
#define MAX_RAND 8000
#define MIN_RAND 10

static dev_t file_numbers;
static struct class* randGenerator=NULL;
static struct cdev charDevice;

static int device_open(struct inode*, struct file*);
static int device_close(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);

static struct file_operations fileOps={
    .open = device_open,
    .release = device_close,
    .read = device_read,
};

static int __init createModule(void){
    printk(KERN_INFO "[%s]: created", DEVICE_NAME);
    if(alloc_chrdev_region(&file_numbers, 0, 1, DEVICE_NAME) < 0){
        printk(KERN_ALERT "[%s]: cannot register device file numbers", DEVICE_NAME);
        return -1;
    }
    printk(KERN_INFO "[%s]: register device file's numbers successfully - (Major, Minor): (%d, %d)\n",DEVICE_NAME, MAJOR(file_numbers), MINOR(file_numbers));
    
    randGenerator = class_create(THIS_MODULE, CLASS_NAME);
    if (!randGenerator){
        unregister_chrdev_region(file_numbers, 1);
        printk(KERN_ALERT "[%s]: cannot create device class", DEVICE_NAME);
        return -1;
    }
	
    if (!device_create(randGenerator, NULL, file_numbers, NULL, DEVICE_NAME)){
        class_destroy(randGenerator);
        unregister_chrdev_region(file_numbers, 1);
        printk(KERN_ALERT "[%s]: cannot create device numbers", DEVICE_NAME);
        return -1;
    }

    cdev_init(&charDevice, &fileOps);
    if (cdev_add(&charDevice, file_numbers, 1) == -1){
        device_destroy(randGenerator, file_numbers);
        class_destroy(randGenerator);
        unregister_chrdev_region(file_numbers, 1);
        return -1;
    }
    return 0;
}

static void __exit removeModule(void){
    cdev_del(&charDevice);
    device_destroy(randGenerator, file_numbers);
    class_destroy(randGenerator);
    unregister_chrdev_region(file_numbers, 1);
    printk(KERN_INFO "[%s]: remove module", DEVICE_NAME);
}

static ssize_t device_read(struct file* f, char __user *buf, size_t len, loff_t *off){
    int randNumber = 0;
    get_random_bytes(&randNumber, sizeof(randNumber));
    randNumber = randNumber%(MAX_RAND - MIN_RAND) + MIN_RAND;
    
    printk("[%s]: random number: %d\n", DEVICE_NAME, randNumber);
    copy_to_user(buf, &randNumber, sizeof(randNumber));

    return 0;
}

static int device_open(struct inode* i, struct file* f){
    printk(KERN_INFO "[%s]: Open", DEVICE_NAME);
    return 0;
}

static int device_close(struct inode* i, struct file* f){
    printk(KERN_INFO "[%s]: Close", DEVICE_NAME);
    return 0;
}

module_init(createModule);
module_exit(removeModule);

MODULE_LICENSE(LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
