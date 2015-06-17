#include <linux/init.h>
#include <linux/module.h>
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
#include <linux/types.h>
#include <linux/miscdevice.h>

#include <asm/uaccess.h>

#include "hello.h"

static char dev_buffer[MAX_BUFFER];

static int hello_open(struct inode *inode, struct file *filp)
{
	printk("hello_open\n");

	filp->private_data = dev_buffer;
	filp->f_pos = 0;

	return 0;
}

static int hello_close(struct inode *inode, struct file *filp)
{
	printk("hello_close\n");
	
	return 0;
}

static ssize_t hello_read(struct file *filp, char __user *userbuf, size_t count, loff_t *offset)
{
	char *buffer;

	printk("hello_read *offset = %d\n", *offset);

	printk("count = %d\n", count);

	buffer = filp->private_data;

	if(count > MAX_BUFFER)
		count = MAX_BUFFER;

	if(copy_to_user(userbuf, buffer, count))
		return -EFAULT;

	return count;
}

static int hello_write(struct file *filp, const char __user *userbuf, size_t count, loff_t *offset)
{
	char *buffer;

	printk("hello_write *offset = %d\n", *offset);

	printk("count = %d, user_buf = %s\n", count, userbuf);

	buffer = filp->private_data;

	if(count > MAX_BUFFER)
		count = MAX_BUFFER;
	
	if(copy_from_user(buffer, userbuf, count))
		return -EFAULT;

	return count;
}

static struct file_operations hello_fops = {
	.open = hello_open,
	.release = hello_close,
	.read = hello_read,
	.write = hello_write
};

 static struct miscdevice hello_dev = {
	HELLO_MINOR,
	"hellodrv",
	&hello_fops
};

static int __init hello_init(void)
{
	printk("hello_init\n");

	memset(dev_buffer, 0, sizeof(dev_buffer));

	misc_register(&hello_dev);
	
	return 0;
}

static void __exit hello_exit(void)
{
	printk("hello_exit\n");

	misc_deregister(&hello_dev);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");

