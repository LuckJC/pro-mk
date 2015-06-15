#include <cust_eint.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/hrtimer.h>  
#include <linux/ktime.h>  
#include <linux/dma-mapping.h>
#include <linux/miscdevice.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <asm/uaccess.h>

#include "hello.h"

static char dev_buffer[MAX_BUFFER];

static int hello_open(struct inode *inode, struct file *filp)
{
	printk("hello_open");

	filp.private_data = dev_buffer;

	return 0;
}

static int hello_close(struct inode *inode, struct file *filp)
{
	printk("hello_close");
	
	return 0;
}

static ssize_t hello_read(struct file *filp, char __user *userbuf, size_t bytes, loff_t *offset)
{
	char *buffer;
	int count;

	printk("hello_read");

	buffer = filp.private_data;

	if(*offset > MAX_BUFFER)
		return -EINVAL;

	count = bytes;
	if(*offset + count > MAX_BUFFER)
		count = MAX_BUFFER - *offset;

	count = copy_to_user(userbuf, buffer + *offset, count);

	return count;
}

static int hello_write(struct file *filp, const char __user *userbuf, loff_t offset, size_t count)
{
	char *buffer;
	int count;

	buffer = filp.private_data;

	if(*offset > MAX_BUFFER)
		return -EINVAL;

	count = bytes;
	if(*offset + count > MAX_BUFFER)
		count = MAX_BUFFER - *offset;

	count = copy_from_user(buffer + *off, userbuf, count);

	return count;
}

static file_operations hello_fops = {
	.open = hello_open,
	.release = hello_close,
	.read = hello_read,
	.wirte = hello_write,
};

 static struct miscdevice hello_dev = {
	HELLO_MINOR,
	"hellodrv",
	&hello_fops
};

static int __init hello_init(void)
{
	printk("hello_init");

	misc_register(&hello_dev);
	
	return 0;
}

static void __exit hello_exit(void)
{
	printk("hello_exit");

	misc_deregister(&hello_dev);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
