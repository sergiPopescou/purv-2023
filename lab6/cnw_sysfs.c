#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/sysfs.h> 
#include <linux/kobject.h>              // struct kobject
#include <linux/err.h>

#define R 10
#define K 10

dev_t dev = 0;
static struct class *dev_class;
static struct cdev cnw_cdev;

struct kobject* kobj_ref;       // represents kernel object (device), shows up as a directory in sysfs file system

volatile static int generacija;
volatile static int ukupno_zivih;
volatile static int rodjenih;
volatile static int umrlih;

/*
** Function Prototypes
*/
static int      __init cnw_driver_init(void);
static void     __exit cnw_driver_exit(void);
 
/*************** Driver functions **********************/
static int      cnw_open(struct inode *inode, struct file *file);
static int      cnw_release(struct inode *inode, struct file *file);
static ssize_t  cnw_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  cnw_write(struct file *filp, const char *buf, size_t len, loff_t * off);
 
/*************** Sysfs functions **********************/
static ssize_t gen_show(struct kobject* kobj, struct kobj_attribute* attr, char* buff);
static ssize_t gen_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buff, size_t count);

static ssize_t zivih_show(struct kobject* kobj, struct kobj_attribute* attr, char* buff);
static ssize_t zivih_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buff, size_t count);

static ssize_t rodj_show(struct kobject* kobj, struct kobj_attribute* attr, char* buff);
static ssize_t rodj_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buff, size_t count);

static ssize_t umrl_show(struct kobject* kobj, struct kobj_attribute* attr, char* buff);
static ssize_t umrl_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buff, size_t count);

/*
** filling kobj_attribute structure (each variable of this structure type is going to be represented as file in sysfs) using macro _ATTR
** gen_attr, uk_ziv_attr, rodj_attr, umrl_attr represents a file that is to be created
** generacija, ukupno_zivih, rodjenih, umrlih is the value that corresponds to the attribute, there is always ONE value per file(attribute) !
*/
struct kobj_attribute gen_attr = __ATTR(generacija, 0660, gen_show, gen_store);
struct kobj_attribute uk_ziv_attr = __ATTR(ukupno_zivih, 0660, zivih_show, zivih_store);
struct kobj_attribute rodj_attr = __ATTR(rodjenih, 0660, rodj_show, rodj_store);
struct kobj_attribute umrl_attr = __ATTR(umrlih, 0660, umrl_show, umrl_store);


/*
** File operation structure
*/
static struct file_operations fops = 
{
        .owner          = THIS_MODULE,
        .read           = cnw_read,
        .write          = cnw_write,
        .open           = cnw_open,
        .release        = cnw_release,
};


/*
** This function will be called whew we read the gen_attr file
*/
static ssize_t gen_show(struct kobject* kobj, struct kobj_attribute* attr, char* buff){
    pr_info("Procitana generacija\n");
    return sprintf(buff, "%d", generacija);
}


/*
** This function will be called whew we write the gen_attr file
*/
static ssize_t gen_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buff, size_t count){
    sscanf(buff, "%d", &generacija);
    pr_info("Upisana generacija\n");
    return count;
}


/*
** This function will be called whew we read the uk_ziv_attr file
*/
static ssize_t zivih_show(struct kobject* kobj, struct kobj_attribute* attr, char* buff){
    pr_info("Procitane zive celije\n");
    return sprintf(buff, "%d", ukupno_zivih);
}

/*
** This function will be called whew we write the uk_ziv_attr file
*/
static ssize_t zivih_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buff, size_t count){
    sscanf(buff, "%d", &ukupno_zivih);
    pr_info("Upisane zive celije\n");
    return count;
}


/*
** This function will be called whew we read the rodj_attr file
*/
static ssize_t rodj_show(struct kobject* kobj, struct kobj_attribute* attr, char* buff){
    pr_info("Procitane rodjene celije\n");
    return sprintf(buff, "%d", rodjenih);
}

/*
** This function will be called whew we write the rodj_attr file
*/
static ssize_t rodj_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buff, size_t count){
    pr_info("Upisane rodjene celije\n");
    sscanf(buff, "%d", &rodjenih);
    return count;
}


/*
** This function will be called whew we read the umrl_attr file
*/
static ssize_t umrl_show(struct kobject* kobj, struct kobj_attribute* attr, char* buff){
    pr_info("Procitane umrle celije\n");
    return sprintf(buff, "%d", umrlih);
}

/*
** This function will be called whew we write the umrl_attr file
*/
static ssize_t umrl_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buff, size_t count){
    pr_info("Upisane rodjene celije\n");
    sscanf(buff, "%d", &umrlih);
    return count;
}



/*
** This function will be called when we open the Device file
*/ 
static int cnw_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}

/*
** This function will be called when we close the Device file
*/ 
static int cnw_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}
 
/*
** This function will be called when we read the Device file
*/
static ssize_t cnw_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read function\n");
        return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t cnw_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write Function\n");
        return len;
}

/*
** Module init function
*/
static int __init cnw_driver_init(void){

    /* Allocating Major number */
    if((alloc_chrdev_region(&dev, 0, 1, "cnw_dev")) < 0){
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major %d Minor %d\n", MAJOR(dev), MINOR(dev));

    /* Creating cdev structure */
    cdev_init(&cnw_cdev, &fops);

    /* Adding char device to the system */
    if((cdev_add(&cnw_cdev, dev, 1) < 0)){
        pr_info("Cannot add device to the system\n");
        goto r_class;
    }

    /* Creating struct class */
    if(IS_ERR(dev_class = class_create(THIS_MODULE, "cnw_class"))){
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    /* Creating the device */
    if(IS_ERR(device_create(dev_class, NULL, dev, NULL, "cnw_device"))){
        pr_err("Cannot create the device\n");
        goto r_device;
    }

    /* Creating a directory in sys/kernel*/
    kobj_ref = kobject_create_and_add("cnw_sysfs", kernel_kobj);


    /* Creating sysfs file for generacija */
    if(sysfs_create_file(kobj_ref, &gen_attr.attr)){
        pr_err("Cannot create sysfs file\n");
        goto r_sysfs;
    }

    /* Creating sysfs file for ukupno_zivih */
    if(sysfs_create_file(kobj_ref, &uk_ziv_attr.attr)){
        pr_err("Cannot create sysfs file\n");
        goto r_sysfs;
    }
    
    /* Creating sysfs file for rodjenih */
    if(sysfs_create_file(kobj_ref, &rodj_attr.attr)){
        pr_err("Cannot create sysfs file\n");
        goto r_sysfs;
    }
    
    /* Creating sysfs file for umrlih */
    if(sysfs_create_file(kobj_ref, &umrl_attr.attr)){
        pr_err("Cannot create sysfs file\n");
        goto r_sysfs;
    }


    pr_info("Device driver inserted\n");
    return 0;

r_sysfs:
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &gen_attr.attr);
    sysfs_remove_file(kernel_kobj, &uk_ziv_attr.attr);
    sysfs_remove_file(kernel_kobj, &rodj_attr.attr);
    sysfs_remove_file(kernel_kobj, &umrl_attr.attr);

r_device:
    class_destroy(dev_class);

r_class:
    unregister_chrdev_region(dev, 1);
    cdev_del(&cnw_cdev);
    return -1;

}


/*
** Module exit function
*/
static void __exit cnw_driver_exit(void)
{
        kobject_put(kobj_ref); 
        sysfs_remove_file(kernel_kobj, &gen_attr.attr);
        sysfs_remove_file(kernel_kobj, &uk_ziv_attr.attr);
        sysfs_remove_file(kernel_kobj, &rodj_attr.attr);
        sysfs_remove_file(kernel_kobj, &umrl_attr.attr);
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&cnw_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}

module_init(cnw_driver_init);
module_exit(cnw_driver_exit);

MODULE_LICENSE("GPL");
