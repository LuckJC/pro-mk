//#include "cust_gpio_usage.h"
#include <cust_eint.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/hrtimer.h>  
#include <linux/ktime.h>  
#include <linux/dma-mapping.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <asm/uaccess.h>

#include "gpio-pwm.h"


static int pwm_major = 0;
static struct hrtimer hr_timer;
static ktime_t kt;
static int pwm_duty;
static struct class *pwm_class;
static struct work_struct wq_hrtimer;  

static unsigned char count=0; 

enum hrtimer_restart hrtimer_handler( struct hrtimer *timer )  
{  
	//schedule_work(&wq_hrtimer);
	//printk("hrtimer_handler\n");
	count++;
	if(count < pwm_duty)
		mt_set_gpio_out(GPIO_MOTO_PIN, GPIO_OUT_ONE);
	else
		mt_set_gpio_out(GPIO_MOTO_PIN, GPIO_OUT_ZERO);

	hrtimer_forward(timer, timer->base->get_time(), kt);

	return HRTIMER_RESTART;  
}  
 /* 
static void wq_func_hrtimer(struct work_struct *work)  
{
	count++;
	count &= ~0xff;
	if(count < pwm_duty)
		mt_set_gpio_out(GPIO20, GPIO_OUT_ONE);
	else
		mt_set_gpio_out(GPIO20, GPIO_OUT_ZERO);
}*/

static long gpio_pwm_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret;
	int a;

	printk("gpio_pwm_ioctl \n");
	printk("cmd  = 0x%02x, arg = %d \n", cmd, arg);

	ret = copy_from_user (&a, (int __user *) arg, sizeof(int));
	printk("ret  = %d, a = %d \n", ret, a);
	//if(ret == 0);
		//ret = -EFAULT;
		
	switch (cmd) 
	{
		case IO_PWM_SET_MODE:
			if(a == 1)
				hrtimer_start(&hr_timer, kt, HRTIMER_MODE_REL);
			break;
			
		case IO_PWM_SET_FREQ:
			if(a > 300 || a < 50)
				return -EINVAL;
			kt = ktime_set(0, 1000 * 1000 * 1000 / 256 /a);
			break;
			
		case IO_PWM_SET_DUTY:
			if(a > 255 || a < 0)
				return -EINVAL;
			pwm_duty = a;
			break;
			
		default:
			return -EINVAL;
	}
	
	return 0;
}

struct file_operations gpio_pwm_fops = 
{
	.unlocked_ioctl = gpio_pwm_ioctl,
};

static int __init gpio_pwm_init(void)
{
	 int ret;
	/*1、注册一个字符设备*/
	ret = register_chrdev(pwm_major, "iopwm", &gpio_pwm_fops);
	if(ret < 0)
	{
		printk(KERN_INFO "register_chrdev failed.\n");
		return ret;
	}
	if(!pwm_major)     //判断是必须的，如果second_major本身有值，则ret正确返回0
		pwm_major = ret;

	/*2、定一个类*/
	pwm_class = class_create(THIS_MODULE, "iopwm");
	if(IS_ERR(pwm_class))
	{
		printk(KERN_INFO"Err: failed in creating class.\n");
		goto out;
	}

	/*3、在类下面创建设备*/
	device_create(pwm_class, NULL, MKDEV(pwm_major, 0), NULL,  "iopwm%d", 0);

	/*4、硬件相关资源初始化*/
	/*4.1 初始化设备结构体内存*/
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	//hrtimer_start(&timer, kt, HRTIMER_MODE_REL);
	hr_timer.function = hrtimer_handler;
	kt = ktime_set(0, 5000); /* 1 sec, 0 nsec */
	//hrtimer_start(&hr_timer, kt, HRTIMER_MODE_REL);

	//INIT_WORK(&wq_hrtimer, wq_func_hrtimer);  

	mt_set_gpio_mode(GPIO_MOTO_PIN, GPIO_MOTO_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_MOTO_PIN, GPIO_DIR_OUT);
	mt_set_gpio_pull_enable(GPIO_MOTO_PIN, GPIO_PULL_ENABLE); //To disable GPIO PULL.
	mt_set_gpio_pull_select(GPIO_MOTO_PIN, GPIO_PULL_UP);

	mt_set_gpio_out(GPIO_MOTO_PIN, GPIO_OUT_ZERO);	

	return 0;

	 /*5、错误处理*/
out1:
	device_destroy(pwm_class, MKDEV(pwm_major, 0));
	class_destroy(pwm_class);
out:
	unregister_chrdev(pwm_major, "iopwm");
	return  -1;
}

static void __exit gpio_pwm_exit(void)
{
	int ret;  
	
	/*1、注销一个字符设备*/
	unregister_chrdev(pwm_major, "iopwm");
	/*2、销毁类下的设备*/
	device_destroy(pwm_class, MKDEV(pwm_major, 0));
	/*3、销毁类*/
	class_destroy(pwm_class);

	ret = hrtimer_cancel( &hr_timer );  
	if (ret)   
		printk("The timer was still in use...\n");  
}

module_init(gpio_pwm_init);
module_exit(gpio_pwm_exit);

MODULE_LICENSE("GPL");

