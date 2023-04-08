#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/err.h>

#define R 3
#define K 3

#define UPISI_STANJE _IOW('a', 'a', struct celija_komsije*)
#define PROCITAJ_STANJE _IOR('a', 'b', int*)

dev_t dev;
static struct class* dev_class;
static struct cdev cnw_cdev;

int is_written = 0;

struct celija_komsije {
	int celija;
	int komsije[R*K-1];
	int dim_komsije;
};

struct celija_komsije ck;
int novo_stanje = 0;


/*
** Function prototypes
*/

static int 		__init cnw_driver_init(void);
static void 	__exit cnw_driver_exit(void);
static int      cnw_open(struct inode *inode, struct file *file);
static int 		cnw_release(struct inode* inode, struct file* file);
static ssize_t 	cnw_read(struct file* filp, char __user* buf, size_t len, loff_t* off);
static ssize_t  cnw_write(struct file *filp, const char __user *buf, size_t len, loff_t * off);
static long 	cnw_ioctl(struct file* file, unsigned int cmd, unsigned long arg);

/* File operation structure */
static struct file_operations fops = {
	.owner 			= THIS_MODULE,
	.read 			= cnw_read,
	.write 			= cnw_write,
	.open 			= cnw_open,
	.unlocked_ioctl	= cnw_ioctl,
	.release		= cnw_release,
};


/*
** This function is called when Device file is closed
** , it just prints out message, just like other functions, besides cnw_ioctl
*/
static int cnw_open(struct inode* inode, struct file* file){
	pr_info("File device open..\n");
	return 0;
}

/*
** This function is called when Device file is closed
*/
static int cnw_release(struct inode* inode, struct file* file){
	pr_info("File device closed..\n");
	return 0;
}

/* 
** This function is called when Device file is read
*/
static ssize_t cnw_read(struct file* filp, char __user* buf, size_t len, loff_t* off){
	pr_info("Read function\n");
	return 0;
}

/*
** This function is called when Device file is written into
*/
static ssize_t cnw_write(struct file* filp, const char __user *buf, size_t len, loff_t* off){
	pr_info("Write function\n");
	return len;
}


/*
** This is IOCTL function which is being called from our user-space application
*** Based on two command that we defined, and that are being passed when ioctl is called, using switch we decide what to do 
*/
static long cnw_ioctl(struct file* file, unsigned int cmd, unsigned long arg){
	int i;
	int zive_komsije = 0;
	switch(cmd){

		/* One command is defines ad UPISI_STANJE and it receives data from user space and based on that data
		** it calculates is conway cell going to live or die based on conway game rules */ 
		case UPISI_STANJE:
			if(copy_from_user(&ck, (struct celija_komsije *)arg, sizeof(ck))){
				pr_err("Write Data Error\n");
				break;
			}
			pr_info("IOCTL called, with command UPISI_STANJE\n");
			is_written = 1;

		  	for(i = 0; i < ck.dim_komsije; i++){
		    	if(ck.komsije[i]){
		      		zive_komsije++;
		    	}
		  	}

		  	novo_stanje = 0;
		  	if(ck.celija){
		    	if(zive_komsije == 2 || zive_komsije == 3){
		      		novo_stanje = 1;
		    	}
		  	}

		  	if(!ck.celija){
		    	if(zive_komsije == 3){
		      		novo_stanje = 1;
		    	}
		  	}
			break;
		

		/* Other command is defines as PROCITAJ_STANJE, and it sends data about liveness of cell to the user space */	
		case PROCITAJ_STANJE:
			if(is_written){				
				if(copy_to_user((int*)arg , &novo_stanje, sizeof(novo_stanje))){
					pr_err("Read data error\n");
					break;
				}	
				is_written = 0;
			
			} else {
				int greska = -1;
			
				pr_err("Read not possible. There is nothing	written in driver\n");
				if(copy_to_user((int*)arg , &greska, sizeof(novo_stanje))){
					pr_err("Read data error\n");
					break;
				}	
				return -1;			}
			pr_info("IOCTL called, with command PROCITAJ_STANJE\n");
			break;


		default:
			pr_info("Default\n");
			break;
		
	}
	return 0;
}

/*
** Module init function
*/
static int __init cnw_driver_init(void)
{
	/* Allocating major number */
	if(alloc_chrdev_region(&dev, 0, 1, "conway_driver") < 0){
		pr_err("Cannot allocate major numer\n");
		return -1;
	}
	pr_info("Major %d, Minor %d\n", MAJOR(dev), MINOR(dev));


	/* Creating cdev structure */
	cdev_init(&cnw_cdev, &fops);

	/* Adding char device to the system */
	if(cdev_add(&cnw_cdev, dev, 1) < 0){
		pr_err("Cannot add device to the system\n");
		goto r_class;
	}

	/* Creating struct class */
	if(IS_ERR(dev_class = class_create(THIS_MODULE, "cnw_class"))){
		pr_err("Cannot create struct class\n");
		goto r_class;
	}

	/* Creating device */
	if(IS_ERR(device_create(dev_class, NULL, dev, NULL, "cnw_device"))){
		pr_err("Cannot create device\n");
		goto r_device;
	}

	pr_info("Device driver inserted\n");
	return 0;

r_device:
	class_destroy(dev_class);

r_class:
	unregister_chrdev_region(dev, 1);
	return -1;

}

/* 
** Module exit function
*/
static void __exit cnw_driver_exit(void){
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&cnw_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("Device driver removed succesfully\n");
}


module_init(cnw_driver_init);
module_exit(cnw_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nemanja Tripic");
MODULE_DESCRIPTION("Conway game of life driver that calculates new state of cell based on previous state\n");
MODULE_VERSION("1.0");

















