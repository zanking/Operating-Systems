

#include<linux/init.h>
#include<linux/module.h>

#include<linux/fs.h>
#include<asm/uaccess.h>
#define BUFFER_SIZE 1024
#define DEV_NAME "simple_char_driver.c" //define device name
#define MAJOR_NUMBER 300 //define device major number


static char device_buffer[BUFFER_SIZE];
static int opens = 0;
static int closes = 0;
ssize_t simple_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer to where you are writing the data you want to be read from the device file*/
	/*  length is the length of the userspace buffer*/
	/*  current position of the opened file*/
	/* copy_to_user function. source is device_buffer (the buffer defined at the start of the code) and destination is the userspace buffer *buffer */
	printk(KERN_ALERT "Reading...\n");
	printk(KERN_ALERT "Read %d bytes.\n", length);
	if(copy_to_user(buffer, device_buffer, BUFFER_SIZE)) return 0;
	// we want to read the entire buffer, so that we can see previous data

	return length;
}



ssize_t simple_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer where you are writing the data you want to be written in the device file*/
	/*  length is the length of the userspace buffer*/
	/*  current position of the opened file*/
	/* copy_from_user function. destination is device_buffer (the buffer defined at the start of the code) and source is the userspace 		buffer *buffer */
	// *offset = strlen(device_buffer);
	if (*offset >= BUFFER_SIZE) *offset = BUFFER_SIZE - 1;

	if ((*offset + length) >= BUFFER_SIZE) length = (BUFFER_SIZE - *offset) - 1;

	printk(KERN_ALERT "Writing...\n");
	copy_from_user(&device_buffer[*offset],buffer, length);
	// we want to append to the end of the device buffer so that nothing is overwritten
	printk(KERN_ALERT "Wrote %d bytes.\n", length);
	*offset+=length;
	return length;
}


int simple_char_driver_open (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is opened and also print the number of times this device has been opened until now*/
	opens++;
	printk(KERN_ALERT "Opened %d times. Device is opened.\n",opens);
	return 0;
}


int simple_char_driver_close (struct inode *pinode, struct file *pfile)
{
	closes++;
	printk(KERN_ALERT "Closed %d times. Device is closed.\n",opens);
	/* print to the log file that the device is closed and also print the number of times this device has been closed until now*/
	return 0;
}

struct file_operations simple_char_driver_file_operations = {

	.owner   = THIS_MODULE,
	/* add the function pointers to point to the corresponding file operations. look at the file fs.h in the linux souce code*/
	.write = simple_char_driver_write,
	.read = simple_char_driver_read,
	.open = simple_char_driver_open,
	.release = simple_char_driver_close,
};

static int simple_char_driver_init(void)
{
	/* print to the log file that the init function is called.*/
	printk(KERN_ALERT "Initializing...");
	/* register the device */
	register_chrdev(300, DEV_NAME, &simple_char_driver_file_operations);
	return 0;
}

static int simple_char_driver_exit(void)
{
	/* print to the log file that the exit function is called.*/
	printk(KERN_ALERT "Exiting...");
	/* unregister  the device using the register_chrdev() function. */
	unregister_chrdev(300, DEV_NAME);
	return 0;
}


/* add module_init and module_exit to point to the corresponding init and exit function*/
module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);
