#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
 
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev stat_cdev;

char alive_cells[4] = "0";
char born_cells[4] = "0";
char died_cells[4] = "0";
char generations[10] = "0";
static int guard = 1;

static int __init stat_driver_init(void);
static void __exit stat_driver_exit(void);
/*************** Driver Functions **********************/
static int stat_open(struct inode *inode, struct file *file);
static int stat_release(struct inode *inode, struct file *file);
static ssize_t stat_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t stat_write(struct file *filp, const char *buf, size_t len, loff_t * off);
 
/***************** Procfs Functions *******************/
static int open_proc(struct inode *inode, struct file *file);
static int release_proc(struct inode *inode, struct file *file);
static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset);
static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t * off);
 
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = stat_read,
        .write          = stat_write,
        .open           = stat_open,
        .release        = stat_release,
};
 
static struct proc_ops proc_fops = {
        .proc_open = open_proc,
        .proc_read = read_proc,
        .proc_write = write_proc,
        .proc_release = release_proc
};

int input_validity_check(char* input){
    int i = 0;
    int delim_cnt = 0;
    while(i < strlen(input) && input[i] != '#'){
        if((input[i] < '0' || input[i] > '9') && input[i] != ',')
            return 0;
        if(input[i] == ',')
            delim_cnt++;
        i++;
    }
    if(delim_cnt != 3)
        return 0;
    return 1;
}
 
int valid_number_check(char* number){
    if(number[0] == '0' && (strlen(number) != 1))
        return 0;
    else
        return 1;
}

static int open_proc(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Opened conway statistic file.....\t");
    return 0;
}
 
static int release_proc(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Closed conway statistic file.....\n");
    return 0;
}
 
static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset)
{ 
    if(guard){
        guard = 0;
    }
    else{
        guard = 1;
        return 0;
    }
    char data_out[100] = "Cells alive: ";
    strcat(data_out, alive_cells);
    strcat(data_out, "\nCells born: ");
    strcat(data_out, born_cells);
    strcat(data_out, "\nCells died: ");
    strcat(data_out, died_cells);
    strcat(data_out, "\nGeneration: ");
    strcat(data_out, generations);
    strcat(data_out, "\n\0");
    if(copy_to_user(buffer,data_out,strlen(data_out))){
        pr_err("ERROR: Read proc error.\n");
        return -1;
    }
    pr_info("Reading conway statistics...\n");
    return strlen(data_out);
}
 
static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    int i,j;
    char data_in[20], tmp_alive[4], tmp_born[4], tmp_died[4], tmp_gen[10];
    if(copy_from_user(data_in,buff,len)){
        pr_err("ERROR: Write proc error.\n");
        return -1;
    }
    if(!input_validity_check(data_in)){
        pr_err("ERROR: Bad input to proc file.");
        return -1;
    }
    i=j=0;
    while(data_in[i] != ','){
        tmp_alive[j] = data_in[i];
        i++;
        j++;
    }
    tmp_alive[j] = '\0';
    if(!valid_number_check(tmp_alive) || j == 0){
        pr_err("ERROR: Bad input to proc file.");
        return -1;
    }
    j=0;
    i++;
    while(data_in[i] != ','){
        tmp_born[j] = data_in[i];
        i++;
        j++;
    }
    tmp_born[j] = '\0';
    if(!valid_number_check(tmp_born) || j == 0){
        pr_err("ERROR: Bad input to proc file.");
        return -1;
    }
    j=0;
    i++;
    while(data_in[i] != ','){
        tmp_died[j] = data_in[i];
        i++;
        j++;
    }
    tmp_died[j] = '\0';
    if(!valid_number_check(tmp_died) || j == 0){
        pr_err("ERROR: Bad input to proc file.");
        return -1;
    }
    j=0;
    i++;
    while(data_in[i] != '#'){
        tmp_gen[j] = data_in[i];
        i++;
        j++;
    }
    tmp_gen[j] = '\0';
    if(!valid_number_check(tmp_gen) || j == 0){
        pr_err("ERROR: Bad input to proc file.");
        return -1;
    }
    strncpy(alive_cells,tmp_alive,sizeof(tmp_alive));
    strncpy(born_cells,tmp_born,sizeof(tmp_born));
    strncpy(died_cells,tmp_died,sizeof(tmp_died));
    strncpy(generations,tmp_gen,sizeof(tmp_gen));
    printk(KERN_INFO "Writing stats info...\n");
    return len;
}
 
static int stat_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Driver opened...!!!\n");
        return 0;
}
 
static int stat_release(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Driver closed...!!!\n");
        return 0;
}
 
static ssize_t stat_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Read function\n");
        return 0;
}
static ssize_t stat_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Write function\n");
        return 0;
}
 
 
 
static int __init stat_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "stat_Dev")) <0){
                printk(KERN_INFO "Cannot allocate major number\n");
                return -1;
        }
        printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&stat_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&stat_cdev,dev,1)) < 0){
            printk(KERN_INFO "Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"stat_class")) == NULL){
            printk(KERN_INFO "Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"stat_device")) == NULL){
            printk(KERN_INFO "Cannot create the Statistic Device \n");
            goto r_device;
        }
 
        /*Creating Proc entry*/
        proc_create("conway_stat_proc",0666,NULL,&proc_fops);
 
        printk(KERN_INFO "Statistic Device Driver Insert...Done!!!\n");
    return 0;
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}
 
void __exit stat_driver_exit(void)
{
        remove_proc_entry("conway_stat_proc",NULL);
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&stat_cdev);
        unregister_chrdev_region(dev, 1);
		printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}
 
module_init(stat_driver_init);
module_exit(stat_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luka Vidic");
MODULE_DESCRIPTION("Character driver to hold statistics for conway game of life program.");
MODULE_VERSION("1.0");
