/************************************************************************
* Copyright (C) 2012-2015, Focaltech Systems (R)£¬All Rights Reserved.
*
* File Name: focaltech_driver.c
*
* Author:
*
* Created: 2015-01-01
*
* Abstract: Function for driver initial, report point, resume, suspend
*
************************************************************************/
#include "tpd.h"
#include "tpd_custom_fts.h"
#include "cust_gpio_usage.h"
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
#include <linux/dma-mapping.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>



#define TP_DEBUG
#if defined(TP_DEBUG)
#define TPD_DEBUG(fmt, arg...)  printk("[tpd debug:6x06]" "[%s]" fmt "\r\n", __FUNCTION__ ,##arg)
#define TPD_DMESG(fmt, arg...)  printk("[tpd dmesg:6x06]" "[%s]" fmt "\r\n", __FUNCTION__ ,##arg)
#else
#define TPD_DEBUG(fmt, arg...)
#define TPD_DMESG(fmt, arg...)
#endif

/*if need these function, pls enable this MACRO*/
//#define MT_PROTOCOL_B
//#define TPD_PROXIMITY
#define FTS_GESTRUE
#define TPD_AUTO_UPGRADE
#define TPD_HAVE_BUTTON

#define FTS_CTL_IIC
#define SYSFS_DEBUG
#define FTS_APK_DEBUG
//#define HIDTOI2C_DISABLE

//for tp esd check
//#define GTP_ESD_PROTECT  
#ifdef GTP_ESD_PROTECT
	#define TPD_ESD_CHECK_CIRCLE        						200
	static struct delayed_work gtp_esd_check_work;
	static struct workqueue_struct *gtp_esd_check_workqueue = NULL;
	static void gtp_esd_check_func(struct work_struct *);
	//add for esd
	static int count_irq = 0;
	static unsigned long esd_check_circle = TPD_ESD_CHECK_CIRCLE;
	static u8 run_check_91_register = 0;
#endif


#ifdef FTS_CTL_IIC
	#include "focaltech_ctl.h"
#endif
#ifdef SYSFS_DEBUG
	#include "focaltech_ex_fun.h"
#endif

/*PROXIMITY*/
#ifdef TPD_PROXIMITY
	#include <linux/hwmsensor.h>
	#include <linux/hwmsen_dev.h>
	#include <linux/sensors_io.h>
#endif

#ifdef TPD_PROXIMITY
	#define APS_ERR(fmt,arg...)           	printk("<<proximity>> "fmt"\n",##arg)
	#define TPD_PROXIMITY_DEBUG(fmt,arg...) printk("<<proximity>> "fmt"\n",##arg)
	#define TPD_PROXIMITY_DMESG(fmt,arg...) printk("<<proximity>> "fmt"\n",##arg)
	static u8 tpd_proximity_flag 			= 0;
	//add for tpd_proximity by wangdongfang
	static u8 tpd_proximity_flag_one 		= 0; 
	//0-->close ; 1--> far away
	static u8 tpd_proximity_detect 		= 1;
#endif
/*GESTRUE*/
#ifdef FTS_GESTRUE
	#include "ft_gesture_lib.h"

	#define FTS_GESTRUE_POINTS 				255
	#define FTS_GESTRUE_POINTS_ONETIME  		62
	#define FTS_GESTRUE_POINTS_HEADER 		8
	#define FTS_GESTURE_OUTPUT_ADRESS 		0xD3
	#define FTS_GESTURE_OUTPUT_UNIT_LENGTH 	4


 /* 0x2x*/
#define SPEC_GESTURE_LINE_HORIZ_LEFT    0x20
#define SPEC_GESTURE_LINE_HORIZ_RIGHT   0x21
#define SPEC_GESTURE_LINE_VERT_UP       0x22
#define SPEC_GESTURE_LINE_VERT_DOWN     0x23
#define SPEC_GESTURE_DOUBLE_CLICK       0x24

	/* Character */
	/* 0x3x*/
#define SPEC_GESTURE_CHAR_O             0x30
#define SPEC_GESTURE_CHAR_W             0x31
#define SPEC_GESTURE_CHAR_M             0x32
#define SPEC_GESTURE_CHAR_E             0x33
	//#define SPEC_GESTURE_CHAR_C             0x10
	//#define SPEC_GESTURE_CHAR_G             0x20
#define SPEC_GESTURE_CHAR_A             0x36
#define SPEC_GESTURE_CHAR_D             0x37

	/* 0x4x*/
#define SPEC_GESTURE_CHAR_N             0x40
	//#define SPEC_GESTURE_CHAR_Z             0x02
#define SPEC_GESTURE_CHAR_B             0x42
	//#define SPEC_GESTURE_CHAR_Q             0x08
#define SPEC_GESTURE_CHAR_L             0x44
#define SPEC_GESTURE_CHAR_P             0x45
#define SPEC_GESTURE_CHAR_S             0x46
#define SPEC_GESTURE_CHAR_U             0x47

	/* 0x5x*/
#define SPEC_GESTURE_SIGN_AT            0x50
#define SPEC_GESTURE_SIGN_ARROW_LEFT    0x51
#define SPEC_GESTURE_SIGN_ARROW_RIGHT   0x52
#define SPEC_GESTURE_SIGN_ARROW_UP      0x53
#define SPEC_GESTURE_SIGN_ARROW_DOWN    0x54
	//#define SPEC_GESTURE_SIGN_TRI_ANGLE     0x20

	/* 0x6x*/
#define SPEC_GESTURE_NUMBER_3            0x60
#define SPEC_GESTURE_NUMBER_6            0x61
#define SPEC_GESTURE_NUMBER_9            0x62
#define SPEC_GESTURE_NUMBER_7            0x63
#define SPEC_GESTURE_NUMBER_8            0x64
#define SPEC_GESTURE_NUMBER_2            0x65


#define KEY_GESTURE_LINE_HORIZ_LEFT    KEY_LEFT
#define KEY_GESTURE_LINE_HORIZ_RIGHT   KEY_RIGHT
#define KEY_GESTURE_LINE_VERT_UP       KEY_UP
#define KEY_GESTURE_LINE_VERT_DOWN     KEY_DOWN
#define KEY_GESTURE_DOUBLE_CLICK       KEY_X

	/* Character */
	/* 0x3x*/
#define KEY_GESTURE_CHAR_O             KEY_O
#define KEY_GESTURE_CHAR_W             KEY_W
#define KEY_GESTURE_CHAR_M             KEY_M
#define KEY_GESTURE_CHAR_E             KEY_E
	//#define KEY_GESTURE_CHAR_C             KEY_C
	//#define KEY_GESTURE_CHAR_G             KEY_G
#define KEY_GESTURE_CHAR_A             KEY_A
#define KEY_GESTURE_CHAR_D             KEY_D

	/* 0x4x*/
#define KEY_GESTURE_CHAR_N             KEY_N
	//#define KEY_GESTURE_CHAR_Z             KEY_Z
#define KEY_GESTURE_CHAR_B             KEY_B
	//#define KEY_GESTURE_CHAR_Q             KEY_Q
#define KEY_GESTURE_CHAR_L             KEY_L
#define KEY_GESTURE_CHAR_P             KEY_P
#define KEY_GESTURE_CHAR_S             KEY_S
#define KEY_GESTURE_CHAR_U             KEY_U

	/* 0x5x*/
#define KEY_GESTURE_SIGN_AT            			KEY_ENTER
#define KEY_GESTURE_SIGN_ARROW_LEFT    	KEY_LEFTCTRL
#define KEY_GESTURE_SIGN_ARROW_RIGHT   	KEY_RIGHTCTRL
#define KEY_GESTURE_SIGN_ARROW_UP      	KEY_PAGEUP
#define KEY_GESTURE_SIGN_ARROW_DOWN    	KEY_PAGEDOWN
	//#define KEY_GESTURE_SIGN_TRI_ANGLE     0x20

	/* 0x6x*/
#define KEY_GESTURE_NUMBER_3            KEY_3
#define KEY_GESTURE_NUMBER_6            KEY_6
#define KEY_GESTURE_NUMBER_9            KEY_9
#define KEY_GESTURE_NUMBER_7            KEY_7
#define KEY_GESTURE_NUMBER_8            KEY_8
#define KEY_GESTURE_NUMBER_2            KEY_2



	/* ID */	
	#define GESTURE_E		    					0x33
	#define GESTURE_M		    					0x32
	#define GESTURE_L		    					0x44
	#define GESTURE_O		    					0x30
	#define GESTURE_S		    					0x46
	#define GESTURE_U 							0x47
	#define GESTURE_W		    					0x31
	#define GESTURE_Z		    					0x47

	#define GESTURE_UP                                           	0x22		/* ¡ü */
	#define GESTURE_DOWN                                     	0x23		/* ¡ý */
	#define GESTURE_LEFT                                        	0x20		/* ¡û */
	#define GESTURE_RIGHT                                     	0x21		/* ¡ú */
	#define GESTURE_DOUBLECLICK				0x24

	#define GESTURE_F1                                            0xE8		/* ^ */
	#define GESTURE_F2                                            0x52		/* V */
	#define GESTURE_F3                                            0x53		/* < */
	#define GESTURE_F4                                            0x54		/* > */
	#define GESTURE_F5                                            0x55		/* ¡÷ */

	/* Key map */	
	#define KEY_GESTURE_E 						KEY_E
	#define KEY_GESTURE_M 						KEY_M 
	#define KEY_GESTURE_L 						KEY_L
	#define KEY_GESTURE_O 						KEY_O
	#define KEY_GESTURE_S 						KEY_S 
	#define KEY_GESTURE_U 						KEY_U
	#define KEY_GESTURE_W 						KEY_W
	#define KEY_GESTURE_Z 						KEY_Z

	#define KEY_GESTURE_UP 						KEY_UP			/* ¡ü 103 */
	#define KEY_GESTURE_DOWN					KEY_DOWN		/* ¡ý 108 */
	#define KEY_GESTURE_LEFT					KEY_LEFT			/* ¡û 105 */
	#define KEY_GESTURE_RIGHT					KEY_RIGHT		/* ¡ú 106 */
	#define KEY_GESTURE_DOUBLECLICK			KEY_X			/* ¡Ñ 45 */
	
	#define KEY_GESTURE_F1						KEY_PAGEUP		/* ¡Ä 104 */
	#define KEY_GESTURE_F2						KEY_PAGEDOWN	/* ¡Å 109 */
	#define KEY_GESTURE_F3						KEY_LEFTCTRL	/* <    29 */
	#define KEY_GESTURE_F4						KEY_RIGHTCTRL	/* >    97 */	
	#define KEY_GESTURE_F5						KEY_T			/* ¡÷ 20 */

	

	unsigned short coordinate_x[150] = {0};
	unsigned short coordinate_y[150] = {0};
#endif

struct fts_keys
{
	unsigned int id;
	unsigned int key;
};

struct fts_keys shizhong_fts_keys[] = 
{
	{SPEC_GESTURE_LINE_HORIZ_LEFT,    		KEY_GESTURE_LINE_HORIZ_LEFT}, 
	{SPEC_GESTURE_LINE_HORIZ_RIGHT,       KEY_GESTURE_LINE_HORIZ_RIGHT},
	{SPEC_GESTURE_LINE_VERT_UP,           KEY_GESTURE_LINE_VERT_UP},    
	{SPEC_GESTURE_LINE_VERT_DOWN,         KEY_GESTURE_LINE_VERT_DOWN},  
	{SPEC_GESTURE_DOUBLE_CLICK,           KEY_GESTURE_DOUBLE_CLICK},    
	                                                                
	{SPEC_GESTURE_CHAR_O,                 KEY_GESTURE_CHAR_O},          
	{SPEC_GESTURE_CHAR_W,                 KEY_GESTURE_CHAR_W},          
	{SPEC_GESTURE_CHAR_M,                 KEY_GESTURE_CHAR_M},          
	{SPEC_GESTURE_CHAR_E,                 KEY_GESTURE_CHAR_E},          
	//{SPEC_GESTURE_CHAR_C,                 KEY_GESTURE_CHAR_C},          
	//{SPEC_GESTURE_CHAR_G,                 KEY_GESTURE_CHAR_G},          
	{SPEC_GESTURE_CHAR_A,                 KEY_GESTURE_CHAR_A},          
	{SPEC_GESTURE_CHAR_D,                 KEY_GESTURE_CHAR_D},          
	                                                                
	{SPEC_GESTURE_CHAR_N,                 KEY_GESTURE_CHAR_N},          
	//{SPEC_GESTURE_CHAR_Z,                 KEY_GESTURE_CHAR_Z},          
	{SPEC_GESTURE_CHAR_B,                 KEY_GESTURE_CHAR_B},          
	//{SPEC_GESTURE_CHAR_Q,                 KEY_GESTURE_CHAR_Q},          
	{SPEC_GESTURE_CHAR_L,                 KEY_GESTURE_CHAR_L},          
	{SPEC_GESTURE_CHAR_P,                 KEY_GESTURE_CHAR_P},          
	{SPEC_GESTURE_CHAR_S,                 KEY_GESTURE_CHAR_S},          
	{SPEC_GESTURE_CHAR_U,                 KEY_GESTURE_CHAR_U},          
	                                                                
	{SPEC_GESTURE_SIGN_AT,                KEY_GESTURE_SIGN_AT},         
	{SPEC_GESTURE_SIGN_ARROW_LEFT,        KEY_GESTURE_SIGN_ARROW_LEFT}, 
	{SPEC_GESTURE_SIGN_ARROW_RIGHT,       KEY_GESTURE_SIGN_ARROW_RIGHT},
	{SPEC_GESTURE_SIGN_ARROW_UP,          KEY_GESTURE_SIGN_ARROW_UP},   
	{SPEC_GESTURE_SIGN_ARROW_DOWN,        KEY_GESTURE_SIGN_ARROW_DOWN}, 
	//{SPEC_GESTURE_SIGN_TRI_ANGLE,         KEY_GESTURE_SIGN_TRI_ANGLE},  
	                                                                
	{SPEC_GESTURE_NUMBER_3,               KEY_GESTURE_NUMBER_3},        
	{SPEC_GESTURE_NUMBER_6,               KEY_GESTURE_NUMBER_6},        
	{SPEC_GESTURE_NUMBER_9,               KEY_GESTURE_NUMBER_9},        
	{SPEC_GESTURE_NUMBER_7,               KEY_GESTURE_NUMBER_7},        
	{SPEC_GESTURE_NUMBER_8,               KEY_GESTURE_NUMBER_8},        
	{SPEC_GESTURE_NUMBER_2,               KEY_GESTURE_NUMBER_2},        
};

/*ic update info*/
struct Upgrade_Info fts_updateinfo[] =
{
	{0x55,"FT5x06",TPD_MAX_POINTS_5,AUTO_CLB_NEED,50, 30, 0x79, 0x03, 10, 2000},
	{0x08,"FT5606",TPD_MAX_POINTS_5,AUTO_CLB_NEED,50, 10, 0x79, 0x06, 100, 2000},
	{0x0a,"FT5x16",TPD_MAX_POINTS_5,AUTO_CLB_NEED,50, 30, 0x79, 0x07, 10, 1500},
	{0x06,"FT6x06",TPD_MAX_POINTS_2,AUTO_CLB_NONEED,100, 30, 0x79, 0x08, 10, 2000},
	{0x36,"FT6x36",TPD_MAX_POINTS_2,AUTO_CLB_NONEED,10, 10, 0x79, 0x18, 10, 2000},
	{0x55,"FT5x06i",TPD_MAX_POINTS_5,AUTO_CLB_NEED,50, 30, 0x79, 0x03, 10, 2000},
	{0x14,"FT5336",TPD_MAX_POINTS_5,AUTO_CLB_NONEED,30, 30, 0x79, 0x11, 10, 2000},
	{0x13,"FT3316",TPD_MAX_POINTS_5,AUTO_CLB_NONEED,30, 30, 0x79, 0x11, 10, 2000},
	{0x12,"FT5436i",TPD_MAX_POINTS_5,AUTO_CLB_NONEED,30, 30, 0x79, 0x11, 10, 2000},
	{0x11,"FT5336i",TPD_MAX_POINTS_5,AUTO_CLB_NONEED,30, 30, 0x79, 0x11, 10, 2000},
	{0x54,"FT5x46",TPD_MAX_POINTS_5,AUTO_CLB_NONEED,2, 2, 0x54, 0x2c, 10, 2000},
	{0x58,"FT5x22",TPD_MAX_POINTS_5,AUTO_CLB_NONEED,2, 2, 0x58, 0x2c, 20, 2000},
	{0x59,"FT5x26",TPD_MAXPOINTS_10,AUTO_CLB_NONEED,30, 50, 0x79, 0x10, 1, 2000},
};
				
struct Upgrade_Info fts_updateinfo_curr;
struct i2c_client *i2c_client = NULL;
struct task_struct *thread = NULL;
extern struct tpd_device *tpd;
static int touch_id;

static DECLARE_WAIT_QUEUE_HEAD(waiter);
static DEFINE_MUTEX(i2c_access);
static DEFINE_MUTEX(i2c_rw_access);
static void tpd_eint_interrupt_handler(void);
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);
extern void mt_eint_mask(unsigned int eint_num);
extern void mt_eint_unmask(unsigned int eint_num);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);

/*dma declare, allocate and release*/
#define __MSG_DMA_MODE__
#ifdef __MSG_DMA_MODE__
	u8 *g_dma_buff_va = NULL;
	u8 *g_dma_buff_pa = NULL;
#endif

#ifdef __MSG_DMA_MODE__

	static void msg_dma_alloct()
	{
		g_dma_buff_va = (u8 *)dma_alloc_coherent(NULL, 4096, &g_dma_buff_pa, GFP_KERNEL);//DMA size 4096 for customer
	    	if(!g_dma_buff_va)
		{
	        	TPD_DMESG("[DMA][Error] Allocate DMA I2C Buffer failed!\n");
	    	}
	}
	static void msg_dma_release(){
		if(g_dma_buff_va)
		{
	     		dma_free_coherent(NULL, 4096, g_dma_buff_va, g_dma_buff_pa);
	        	g_dma_buff_va = NULL;
	        	g_dma_buff_pa = NULL;
			TPD_DMESG("[DMA][release] Allocate DMA I2C Buffer release!\n");
	    	}
	}
#endif
/*register define*/
#define FTS_RESET_PIN							GPIO_CTP_RST_PIN
#define TPD_OK 									0
#define DEVICE_MODE 							0x00
#define GEST_ID 									0x01
#define TD_STATUS 								0x02
#define TOUCH1_XH 								0x03
#define TOUCH1_XL 								0x04
#define TOUCH1_YH 								0x05
#define TOUCH1_YL 								0x06
#define TOUCH2_XH 								0x09
#define TOUCH2_XL 								0x0A
#define TOUCH2_YH 								0x0B
#define TOUCH2_YL 								0x0C
#define TOUCH3_XH 								0x0F
#define TOUCH3_XL 								0x10
#define TOUCH3_YH 								0x11
#define TOUCH3_YL 								0x12
#define TPD_MAX_RESET_COUNT 					3

/*touch event info*/
struct ts_event 
{
	u16 au16_x[CFG_MAX_TOUCH_POINTS];				/*x coordinate */
	u16 au16_y[CFG_MAX_TOUCH_POINTS];				/*y coordinate */
	u8 au8_touch_event[CFG_MAX_TOUCH_POINTS];		/*touch event: 0 -- down; 1-- up; 2 -- contact */
	u8 au8_finger_id[CFG_MAX_TOUCH_POINTS];			/*touch ID */
	u16 pressure[CFG_MAX_TOUCH_POINTS];
	u16 area[CFG_MAX_TOUCH_POINTS];
	u8 touch_point;
	int touchs;
	u8 touch_point_num;
};


#ifdef TPD_HAVE_BUTTON 
	static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
	static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
	static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
	static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
	static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
	static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif
static int tpd_flag = 0;
static int tpd_halt=0;
static int point_num = 0;
static int p_point_num = 0;
static u8 buf_addr[2] = { 0 };
static u8 buf_value[2] = { 0 };
/*
*	open/release/(I/O) control tpd device
*
*/
//#define VELOCITY_CUSTOM_fts
#ifdef VELOCITY_CUSTOM_fts
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

/*for magnify velocity*/
#ifndef TPD_VELOCITY_CUSTOM_X
	#define TPD_VELOCITY_CUSTOM_X 				10
#endif
#ifndef TPD_VELOCITY_CUSTOM_Y
	#define TPD_VELOCITY_CUSTOM_Y 				10
#endif

#define TOUCH_IOC_MAGIC 						'A'
#define TPD_GET_VELOCITY_CUSTOM_X 			_IO(TOUCH_IOC_MAGIC,0)
#define TPD_GET_VELOCITY_CUSTOM_Y 			_IO(TOUCH_IOC_MAGIC,1)

int g_v_magnify_x =TPD_VELOCITY_CUSTOM_X;
int g_v_magnify_y =TPD_VELOCITY_CUSTOM_Y;


/************************************************************************
* Name: tpd_misc_open
* Brief: open node
* Input: node, file point
* Output: no
* Return: fail <0
***********************************************************************/
static int tpd_misc_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}
/************************************************************************
* Name: tpd_misc_release
* Brief: release node
* Input: node, file point
* Output: no
* Return: 0
***********************************************************************/
static int tpd_misc_release(struct inode *inode, struct file *file)
{
	return 0;
}
/************************************************************************
* Name: tpd_unlocked_ioctl
* Brief: I/O control for apk
* Input: file point, command
* Output: no
* Return: fail <0
***********************************************************************/

static long tpd_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{

	void __user *data;
	
	long err = 0;
	
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if(err)
	{
		printk("tpd: access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch(cmd)
	{
		case TPD_GET_VELOCITY_CUSTOM_X:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &g_v_magnify_x, sizeof(g_v_magnify_x)))
			{
				err = -EFAULT;
				break;
			}				 
			break;

	   case TPD_GET_VELOCITY_CUSTOM_Y:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &g_v_magnify_y, sizeof(g_v_magnify_y)))
			{
				err = -EFAULT;
				break;
			}				 
			break;


		default:
			printk("tpd: unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;
			
	}

	return err;
}


static struct file_operations tpd_fops = {
	//.owner = THIS_MODULE,
	.open = tpd_misc_open,
	.release = tpd_misc_release,
	.unlocked_ioctl = tpd_unlocked_ioctl,
};

static struct miscdevice tpd_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "touch",
	.fops = &tpd_fops,
};
#endif
/*touch info*/
struct touch_info {
    int y[10];
    int x[10];
    int p[10];
    int id[10];
    int count;
};

/*register driver and device info*/ 
static const struct i2c_device_id fts_tpd_id[] = {{"fts",0},{}};

static struct i2c_board_info __initdata fts_i2c_tpd={ I2C_BOARD_INFO("fts", (0x70>>1))};
 
static struct i2c_driver tpd_i2c_driver = {
  .driver = {
  .name = "fts",
  //.owner = THIS_MODULE,
  },
  .probe = tpd_probe,
  .remove = __devexit_p(tpd_remove),
  .id_table = fts_tpd_id,
  .detect = tpd_detect,

 };
 
/************************************************************************
* Name: fts_i2c_Read
* Brief: i2c read
* Input: i2c info, write buf, write len, read buf, read len
* Output: get data in the 3rd buf
* Return: fail <0
***********************************************************************/
int fts_i2c_Read(struct i2c_client *client, char *writebuf,int writelen, char *readbuf, int readlen)
{
	int ret,i;

	// for DMA I2c transfer
	
	mutex_lock(&i2c_rw_access);
	
	if(writelen!=0)
	{
		//DMA Write
		memcpy(g_dma_buff_va, writebuf, writelen);
		client->addr = client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG;
		if((ret=i2c_master_send(client, (unsigned char *)g_dma_buff_pa, writelen))!=writelen)
			//dev_err(&client->dev, "###%s i2c write len=%x,buffaddr=%x\n", __func__,ret,*g_dma_buff_pa);
			printk("i2c write failed\n");
		client->addr = client->addr & I2C_MASK_FLAG &(~ I2C_DMA_FLAG);
	}

	//DMA Read 

	if(readlen!=0)

	{
		client->addr = client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG;

		ret = i2c_master_recv(client, (unsigned char *)g_dma_buff_pa, readlen);

		memcpy(readbuf, g_dma_buff_va, readlen);

		client->addr = client->addr & I2C_MASK_FLAG &(~ I2C_DMA_FLAG);
	}
	
	mutex_unlock(&i2c_rw_access);
	
	return ret;

}

/************************************************************************
* Name: fts_i2c_Write
* Brief: i2c write
* Input: i2c info, write buf, write len
* Output: no
* Return: fail <0
***********************************************************************/
int fts_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;
	int i = 0;

	mutex_lock(&i2c_rw_access);
	
 	//client->addr = client->addr & I2C_MASK_FLAG;

	//ret = i2c_master_send(client, writebuf, writelen);
	memcpy(g_dma_buff_va, writebuf, writelen);
	
	client->addr = client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG;
	if((ret=i2c_master_send(client, (unsigned char *)g_dma_buff_pa, writelen))!=writelen)
		//dev_err(&client->dev, "###%s i2c write len=%x,buffaddr=%x\n", __func__,ret,*g_dma_buff_pa);
		printk("i2c write failed\n");
	client->addr = client->addr & I2C_MASK_FLAG &(~ I2C_DMA_FLAG);
		
	mutex_unlock(&i2c_rw_access);
	
	return ret;
}
/************************************************************************
* Name: fts_write_reg
* Brief: write register
* Input: i2c info, reg address, reg value
* Output: no
* Return: fail <0
***********************************************************************/
int fts_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue)
{
	unsigned char buf[2] = {0};

	buf[0] = regaddr;
	buf[1] = regvalue;

	return fts_i2c_Write(client, buf, sizeof(buf));
}
/************************************************************************
* Name: fts_read_reg
* Brief: read register
* Input: i2c info, reg address, reg value
* Output: get reg value
* Return: fail <0
***********************************************************************/
int fts_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue)
{

	return fts_i2c_Read(client, &regaddr, 1, regvalue, 1);

}
/************************************************************************
* Name: fts_get_upgrade_array
* Brief: decide which ic
* Input: no
* Output: get ic info in fts_updateinfo_curr
* Return: no
***********************************************************************/
void fts_get_upgrade_array(void)
{

	u8 chip_id;
	u32 i;
	int ret = 0;
	//i2c_smbus_read_i2c_block_data(i2c_client,FTS_REG_CHIP_ID,1,&chip_id);

	ret = fts_read_reg(i2c_client, FTS_REG_CHIP_ID,&chip_id);
	if (ret<0) 
	{
		printk("[Focal][Touch] read value fail");
		//return ret;
	}
		
	printk("%s chip_id = %x\n", __func__, chip_id);

	for(i=0;i<sizeof(fts_updateinfo)/sizeof(struct Upgrade_Info);i++)
	{
		if(chip_id==fts_updateinfo[i].CHIP_ID)
		{
			memcpy(&fts_updateinfo_curr, &fts_updateinfo[i], sizeof(struct Upgrade_Info));
			break;
		}
	}

	if(i >= sizeof(fts_updateinfo)/sizeof(struct Upgrade_Info))
	{
		memcpy(&fts_updateinfo_curr, &fts_updateinfo[0], sizeof(struct Upgrade_Info));
	}
}
/************************************************************************
* Name: tpd_down
* Brief: down info
* Input: x pos, y pos, id number
* Output: no
* Return: no
***********************************************************************/
static void tpd_down(int x, int y, int p) {
	
	if((touch_id == 0 && x > TPD_RES_X) || (touch_id == 1 &&  x > 720))
	{
		TPD_DEBUG("warning: IC have sampled wrong value.\n");;
		return;
	}
	input_report_key(tpd->dev, BTN_TOUCH, 1);
	input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 20);
	input_report_abs(tpd->dev, ABS_MT_PRESSURE, 0x3f);
	if(touch_id == 1)
		x = 720 - x;
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	//printk("tpd:D[%4d %4d %4d] ", x, y, p);
	/* track id Start 0 */
     	//input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, p); 
	input_mt_sync(tpd->dev);
     	if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
     	{   
       	tpd_button(x, y, 1);  
     	}
	if((touch_id == 0 && y > TPD_RES_Y) || (touch_id == 1) && y > 1280) //virtual key debounce to avoid android ANR issue
	{
       	//msleep(50);
		printk("D virtual key \n");
	 }
	 TPD_EM_PRINT(x, y, x, y, p-1, 1);
 }
 /************************************************************************
* Name: tpd_up
* Brief: up info
* Input: x pos, y pos, count
* Output: no
* Return: no
***********************************************************************/
static  void tpd_up(int x, int y,int *count)
{
	 input_report_key(tpd->dev, BTN_TOUCH, 0);
	 //printk("U[%4d %4d %4d] ", x, y, 0);
	 input_mt_sync(tpd->dev);
	 TPD_EM_PRINT(x, y, x, y, 0, 0);

	if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
	{   
		tpd_button(x, y, 0); 
	}   		 
 }
 /************************************************************************
* Name: tpd_touchinfo
* Brief: touch info
* Input: touch info point, no use
* Output: no
* Return: success nonzero
***********************************************************************/
static int tpd_touchinfo(struct touch_info *cinfo, struct touch_info *pinfo)
{
	int i = 0;
	char data[128] = {0};
       u16 high_byte,low_byte,reg;
	u8 report_rate =0;
	p_point_num = point_num;
	if (tpd_halt)
	{
		TPD_DMESG( "tpd_touchinfo return ..\n");
		return false;
	}
	mutex_lock(&i2c_access);

       reg = 0x00;
	fts_i2c_Read(i2c_client, &reg, 1, data, 64);
	mutex_unlock(&i2c_access);
	
	/*get the number of the touch points*/

	point_num= data[2] & 0x0f;
	
	for(i = 0; i < point_num; i++)  
	{
		cinfo->p[i] = data[3+6*i] >> 6; //event flag 
     		cinfo->id[i] = data[3+6*i+2]>>4; //touch id
	   	/*get the X coordinate, 2 bytes*/
		high_byte = data[3+6*i];
		high_byte <<= 8;
		high_byte &= 0x0f00;
		low_byte = data[3+6*i + 1];
		cinfo->x[i] = high_byte |low_byte;	
		high_byte = data[3+6*i+2];
		high_byte <<= 8;
		high_byte &= 0x0f00;
		low_byte = data[3+6*i+3];
		cinfo->y[i] = high_byte |low_byte;
	}

	//printk(" tpd cinfo->x[0] = %d, cinfo->y[0] = %d, cinfo->p[0] = %d\n", cinfo->x[0], cinfo->y[0], cinfo->p[0]);
	return true;

};


 /************************************************************************
* Name: fts_read_Touchdata
* Brief: report the point information
* Input: event info
* Output: get touch data in pinfo
* Return: success is zero
***********************************************************************/
static unsigned int buf_count_add=0;
static unsigned int buf_count_neg=0;
//unsigned int buf_count_add1;
//unsigned int buf_count_neg1;
u8 buf_touch_data[30*POINT_READ_BUF] = { 0 };//0xFF
static int fts_read_Touchdata(struct ts_event *pinfo)
{
       u8 buf[POINT_READ_BUF] = { 0 };//0xFF
	int ret = -1;
	int i = 0;
	u8 pointid = FTS_MAX_ID;
	//u8 pt00f=0;
	if (tpd_halt)
	{
		TPD_DMESG( "tpd_touchinfo return ..\n");
		return false;
	}

	mutex_lock(&i2c_access);
	ret = fts_i2c_Read(i2c_client, buf, 1, buf, POINT_READ_BUF);
	if (ret < 0) 
	{
		dev_err(&i2c_client->dev, "%s read touchdata failed.\n",__func__);
		mutex_unlock(&i2c_access);
		return ret;
	}
	mutex_unlock(&i2c_access);

	buf_count_add++;
	//buf_count_add1=buf_count_add;
	memcpy( buf_touch_data+(((buf_count_add-1)%30)*POINT_READ_BUF), buf, sizeof(u8)*POINT_READ_BUF );


	
	
	
	return 0;
}

 /************************************************************************
* Name: fts_report_value
* Brief: report the point information
* Input: event info
* Output: no
* Return: success is zero
***********************************************************************/
static int fts_report_value(struct ts_event *data)
 {
	//struct ts_event *event = NULL;
	int i = 0;
	int up_point = 0;
 	int touchs = 0;
	u8 pointid = FTS_MAX_ID;
	u8 buf[POINT_READ_BUF] = { 0 };//0xFF

	buf_count_neg++;
	//buf_count_neg1=buf_count_neg;
	memcpy( buf,buf_touch_data+(((buf_count_neg-1)%30)*POINT_READ_BUF), sizeof(u8)*POINT_READ_BUF );


	memset(data, 0, sizeof(struct ts_event));
	data->touch_point_num=buf[FT_TOUCH_POINT_NUM] & 0x0F;

	data->touch_point = 0;
	//printk("tpd  fts_updateinfo_curr.TPD_MAX_POINTS=%d fts_updateinfo_curr.chihID=%d \n", fts_updateinfo_curr.TPD_MAX_POINTS,fts_updateinfo_curr.CHIP_ID);
	for (i = 0; i < fts_updateinfo_curr.TPD_MAX_POINTS; i++)
	{
		pointid = (buf[FTS_TOUCH_ID_POS + FTS_TOUCH_STEP * i]) >> 4;
		if (pointid >= FTS_MAX_ID)
			break;
		else
			data->touch_point++;
		data->au16_x[i] =
		    (s16) (buf[FTS_TOUCH_X_H_POS + FTS_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FTS_TOUCH_X_L_POS + FTS_TOUCH_STEP * i];
		data->au16_y[i] =
		    (s16) (buf[FTS_TOUCH_Y_H_POS + FTS_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FTS_TOUCH_Y_L_POS + FTS_TOUCH_STEP * i];
		data->au8_touch_event[i] =
		    buf[FTS_TOUCH_EVENT_POS + FTS_TOUCH_STEP * i] >> 6;
		data->au8_finger_id[i] =
		    (buf[FTS_TOUCH_ID_POS + FTS_TOUCH_STEP * i]) >> 4;

		data->pressure[i] =
			(buf[FTS_TOUCH_XY_POS + FTS_TOUCH_STEP * i]);//cannot constant value
		data->area[i] =
			(buf[FTS_TOUCH_MISC + FTS_TOUCH_STEP * i]) >> 4;
		if((data->au8_touch_event[i]==0 || data->au8_touch_event[i]==2)&&((data->touch_point_num==0)||(data->pressure[i]==0 && data->area[i]==0  )))
			return 1;

		
		
		//if ( pinfo->au16_x[i]==0 && pinfo->au16_y[i] ==0)
		//	pt00f++;
	}
	/*
	if (pt00f>0)
	{    
		for(i=0;i<POINT_READ_BUF;i++)
		{
        		printk(KERN_WARNING "The xy00 is %x \n",buf[i]);
		}
		printk(KERN_WARNING "\n");		
	}
	*/			
	//event = data;
	for (i = 0; i < data->touch_point; i++) 
	{
		 input_mt_slot(tpd->dev, data->au8_finger_id[i]);
 
		if (data->au8_touch_event[i]== 0 || data->au8_touch_event[i] == 2)
		{
			 input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER,true);
			 input_report_abs(tpd->dev, ABS_MT_PRESSURE,data->pressure[i]/*0x3f*/);
			 input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR,data->area[i]/*0x05*/);
			 if(touch_id == 1)
				data->au16_x[i] = 720 - data->au16_x[i];
			 input_report_abs(tpd->dev, ABS_MT_POSITION_X,data->au16_x[i]);
			 input_report_abs(tpd->dev, ABS_MT_POSITION_Y,data->au16_y[i]);
			 touchs |= BIT(data->au8_finger_id[i]);
   			 data->touchs |= BIT(data->au8_finger_id[i]);
             		 //printk("tpd D x[%d] =%d,y[%d]= %d",i,data->au16_x[i],i,data->au16_y[i]);
		}
		else
		{
			 up_point++;
			 input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER,false);
			 data->touchs &= ~BIT(data->au8_finger_id[i]);
		}				 
		 
	}
 	if(unlikely(data->touchs ^ touchs)){
		for(i = 0; i < CFG_MAX_TOUCH_POINTS; i++){
			if(BIT(i) & (data->touchs ^ touchs)){
				input_mt_slot(tpd->dev, i);
				input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER, false);
			}
		}
	}
	data->touchs = touchs;
	if(data->touch_point == up_point)
		 input_report_key(tpd->dev, BTN_TOUCH, 0);
	else
		 input_report_key(tpd->dev, BTN_TOUCH, 1);
 
	input_sync(tpd->dev);
	return 0;
    	//printk("tpd D x =%d,y= %d",event->au16_x[0],event->au16_y[0]);
 }

#ifdef TPD_PROXIMITY
 /************************************************************************
* Name: tpd_read_ps
* Brief: read proximity value
* Input: no
* Output: no
* Return: 0
***********************************************************************/
int tpd_read_ps(void)
{
	tpd_proximity_detect;
	return 0;    
}
 /************************************************************************
* Name: tpd_get_ps_value
* Brief: get proximity value
* Input: no
* Output: no
* Return: 0
***********************************************************************/
static int tpd_get_ps_value(void)
{
	return tpd_proximity_detect;
}
 /************************************************************************
* Name: tpd_enable_ps
* Brief: enable proximity
* Input: enable or not
* Output: no
* Return: 0
***********************************************************************/
static int tpd_enable_ps(int enable)
{
	u8 state;
	int ret = -1;
	
	//i2c_smbus_read_i2c_block_data(i2c_client, 0xB0, 1, &state);

	ret = fts_read_reg(i2c_client, 0xB0,&state);
	if (ret<0) 
	{
		printk("[Focal][Touch] read value fail");
		//return ret;
	}
	
	printk("[proxi_fts]read: 999 0xb0's value is 0x%02X\n", state);

	if (enable)
	{
		state |= 0x01;
		tpd_proximity_flag = 1;
		TPD_PROXIMITY_DEBUG("[proxi_fts]ps function is on\n");	
	}
	else
	{
		state &= 0x00;	
		tpd_proximity_flag = 0;
		TPD_PROXIMITY_DEBUG("[proxi_fts]ps function is off\n");
	}
	
	//ret = i2c_smbus_write_i2c_block_data(i2c_client, 0xB0, 1, &state);
	ret = fts_write_reg(i2c_client, 0xB0,&state);
	if (ret<0) 
	{
		printk("[Focal][Touch] write value fail");
		//return ret;
	}
	TPD_PROXIMITY_DEBUG("[proxi_fts]write: 0xB0's value is 0x%02X\n", state);
	return 0;
}
 /************************************************************************
* Name: tpd_ps_operate
* Brief: operate function for proximity 
* Input: point, which operation, buf_in , buf_in len, buf_out , buf_out len, no use
* Output: buf_out
* Return: fail <0
***********************************************************************/
int tpd_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,

		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data *sensor_data;
	TPD_DEBUG("[proxi_fts]command = 0x%02X\n", command);		
	
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;
		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{				
				value = *(int *)buff_in;
				if(value)
				{		
					if((tpd_enable_ps(1) != 0))
					{
						APS_ERR("enable ps fail: %d\n", err); 
						return -1;
					}
				}
				else
				{
					if((tpd_enable_ps(0) != 0))
					{
						APS_ERR("disable ps fail: %d\n", err); 
						return -1;
					}
				}
			}
			break;
		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;				
				if((err = tpd_read_ps()))
				{
					err = -1;;
				}
				else
				{
					sensor_data->values[0] = tpd_get_ps_value();
					TPD_PROXIMITY_DEBUG("huang sensor_data->values[0] 1082 = %d\n", sensor_data->values[0]);
					sensor_data->value_divide = 1;
					sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
				}					
			}
			break;
		default:
			APS_ERR("proxmy sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	return err;	
}
#endif

#ifdef FTS_GESTRUE
 /************************************************************************
* Name: check_gesture
* Brief: report gesture id
* Input: gesture id
* Output: no
* Return: no
***********************************************************************/
static void check_gesture(int gesture_id)
{
	int i;
	for(i = 0; i < sizeof(shizhong_fts_keys)/sizeof(shizhong_fts_keys[0]); i++)
	{
		if(gesture_id == shizhong_fts_keys[i].id)
		{
				input_report_key(tpd->dev, shizhong_fts_keys[i].key, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, shizhong_fts_keys[i].key, 0);
				input_sync(tpd->dev);
				break;
		}
	}
	#if 0
    	//printk("fts gesture_id==0x%x\n ",gesture_id);
	switch(gesture_id)
	{
		case GESTURE_E:
				input_report_key(tpd->dev, KEY_GESTURE_E, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, KEY_GESTURE_E, 0);
				input_sync(tpd->dev);
			break;
		case GESTURE_M:
				input_report_key(tpd->dev, KEY_GESTURE_M, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, KEY_GESTURE_M, 0);
				input_sync(tpd->dev);
			break;
		case GESTURE_L:
				input_report_key(tpd->dev, KEY_GESTURE_L, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, KEY_GESTURE_L, 0);
				input_sync(tpd->dev);
			break;
		case GESTURE_O:
				input_report_key(tpd->dev, KEY_GESTURE_O, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, KEY_GESTURE_O, 0);
				input_sync(tpd->dev);
			break;
		case GESTURE_S:
				input_report_key(tpd->dev, KEY_GESTURE_S, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, KEY_GESTURE_S, 0);
				input_sync(tpd->dev);
			break;
		case GESTURE_W:
				input_report_key(tpd->dev, KEY_GESTURE_W, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, KEY_GESTURE_W, 0);
				input_sync(tpd->dev);
			break;
		case GESTURE_Z:
				input_report_key(tpd->dev, KEY_GESTURE_Z, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, KEY_GESTURE_Z, 0);
				input_sync(tpd->dev);
			break;
		case GESTURE_UP:
				input_report_key(tpd->dev, KEY_GESTURE_UP, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, KEY_GESTURE_UP, 0);
				input_sync(tpd->dev);			
			break;
		case GESTURE_DOWN:
				input_report_key(tpd->dev, KEY_GESTURE_DOWN, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, KEY_GESTURE_DOWN, 0);
				input_sync(tpd->dev);
			break;
		case GESTURE_LEFT:
				input_report_key(tpd->dev, KEY_GESTURE_LEFT, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, KEY_GESTURE_LEFT, 0);
				input_sync(tpd->dev);
			break;
		case GESTURE_RIGHT:
				input_report_key(tpd->dev, KEY_GESTURE_RIGHT, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, KEY_GESTURE_RIGHT, 0);
				input_sync(tpd->dev);
			break;
		case GESTURE_DOUBLECLICK:
				input_report_key(tpd->dev, KEY_GESTURE_U, 1);
				input_sync(tpd->dev);
				input_report_key(tpd->dev, KEY_GESTURE_U, 0);
				input_sync(tpd->dev);
			break;
                case GESTURE_F1:
                                input_report_key(tpd->dev, KEY_GESTURE_F1, 1);
                                input_sync(tpd->dev);
                                input_report_key(tpd->dev, KEY_GESTURE_F1, 0);
                                input_sync(tpd->dev);
                       break;
                case GESTURE_F2:
                                input_report_key(tpd->dev, KEY_GESTURE_F2, 1);
                                input_sync(tpd->dev);
                                input_report_key(tpd->dev, KEY_GESTURE_F2, 0);
                                input_sync(tpd->dev);
                       break;
                case GESTURE_F3:
                                input_report_key(tpd->dev, KEY_GESTURE_F3, 1);
                                input_sync(tpd->dev);
                                input_report_key(tpd->dev, KEY_GESTURE_F3, 0);
                                input_sync(tpd->dev);
                       break;
              case GESTURE_F4:
                                input_report_key(tpd->dev, KEY_GESTURE_F4, 1);
                                input_sync(tpd->dev);
                                input_report_key(tpd->dev, KEY_GESTURE_F4, 0);
                                input_sync(tpd->dev);
			break;
		case GESTURE_F5:
                                input_report_key(tpd->dev, KEY_GESTURE_F5, 1);
                                input_sync(tpd->dev);
                                input_report_key(tpd->dev, KEY_GESTURE_F5, 0);
                                input_sync(tpd->dev);
                       break;					   
		default:
			break;
	}
	#endif
}
 /************************************************************************
* Name: fts_read_Gestruedata
* Brief: read data from TP register
* Input: no
* Output: no
* Return: fail <0
***********************************************************************/
static int fts_read_Gestruedata(void)
{
    unsigned char buf[FTS_GESTRUE_POINTS * 3] = { 0 };
    int ret = -1;
    int i = 0;
    buf[0] = 0xd3;
    int gestrue_id = 0;
    short pointnum = 0;

    pointnum = 0;
    ret = fts_i2c_Read(i2c_client, buf, 1, buf, FTS_GESTRUE_POINTS_HEADER);
	//printk( "tpd read FTS_GESTRUE_POINTS_HEADER.\n");
    if (ret < 0)
    {
	        	printk( "%s read touchdata failed.\n", __func__);
	        	return ret;
    }

    /* FW */
     if (fts_updateinfo_curr.CHIP_ID==0x54|| fts_updateinfo_curr.CHIP_ID==0x58)
     {
	     		 gestrue_id = buf[0];
			 pointnum = (short)(buf[1]) & 0xff;
		 	 buf[0] = 0xd3;
		 
		 	 if((pointnum * 4 + 8)<255)
		 	 {
		 	    	 ret = fts_i2c_Read(i2c_client, buf, 1, buf, (pointnum * 4 + 8));
		 	 }
		 	 else
		 	 {
		 	        ret = fts_i2c_Read(i2c_client, buf, 1, buf, 255);
		 	        ret = fts_i2c_Read(i2c_client, buf, 0, buf+255, (pointnum * 4 + 8) -255);
		 	 }
		 	 if (ret < 0)
		 	 {
		 	       printk( "%s read touchdata failed.\n", __func__);
		 	       return ret;
		 	 }
	        	 check_gesture(gestrue_id);	/*  */
				printk( "************************gestrue_id = 0x%02X\n", gestrue_id);
			 for(i = 0;i < pointnum;i++)
		        {
		        	coordinate_x[i] =  (((s16) buf[0 + (4 * i)]) & 0x0F) <<
		            	8 | (((s16) buf[1 + (4 * i)])& 0xFF);
		        	coordinate_y[i] = (((s16) buf[2 + (4 * i)]) & 0x0F) <<
		            	8 | (((s16) buf[3 + (4 * i)]) & 0xFF);
		   	 }
	        	 return -1;
     }

    if (0x24 == buf[0])

    {
	        	gestrue_id = 0x24;
	        	check_gesture(gestrue_id);
		 	printk( "<<<<<<<<<<<<<<<<<<<<<<<<gestrue_id = 0x%02X\n", gestrue_id);
	        	return -1;
    }
	
    pointnum = (short)(buf[1]) & 0xff;
    buf[0] = 0xd3;
    if((pointnum * 4 + 8)<255)
    {
    			ret = fts_i2c_Read(i2c_client, buf, 1, buf, (pointnum * 4 + 8));
    }
    else
    {
	         	ret = fts_i2c_Read(i2c_client, buf, 1, buf, 255);
	         	ret = fts_i2c_Read(i2c_client, buf, 0, buf+255, (pointnum * 4 + 8) -255);
    }
    if (ret < 0)
    {
	        	printk( "%s read touchdata failed.\n", __func__);
	        	return ret;
    }
	
    gestrue_id = fetch_object_sample(buf, pointnum);
	printk( ">>>>>>>>>>>>>>>>>>>>gestrue_id =  0x%02X\n", gestrue_id);
    check_gesture(gestrue_id);
    //printk( "tpd %d read gestrue_id.\n", gestrue_id);

    for(i = 0;i < pointnum;i++)
    {
	        	coordinate_x[i] =  (((s16) buf[0 + (4 * i)]) & 0x0F) <<
	            			8 | (((s16) buf[1 + (4 * i)])& 0xFF);
	        	coordinate_y[i] = (((s16) buf[2 + (4 * i)]) & 0x0F) <<
	            			8 | (((s16) buf[3 + (4 * i)]) & 0xFF);
    }
    return -1;
}
#endif
 /************************************************************************
* Name: touch_event_handler
* Brief: interrupt event from TP, and read/report data to Android system 
* Input: no use
* Output: no
* Return: 0
***********************************************************************/
 static int touch_event_handler(void *unused)
 {
	struct touch_info cinfo, pinfo;
	struct ts_event pevent;
	int i=0;
	int ret = 0;

	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	sched_setscheduler(current, SCHED_RR, &param);
 
	#ifdef TPD_PROXIMITY
		int err;
		hwm_sensor_data sensor_data;
		u8 proximity_status;
	#endif
	u8 state;
	do
	{
		 if(touch_id == 0)
		 	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
		 else
		 	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_2_NUM); 
		 set_current_state(TASK_INTERRUPTIBLE); 
		 wait_event_interruptible(waiter,tpd_flag!=0);
						 
		 tpd_flag = 0;
			 
		 set_current_state(TASK_RUNNING);
		 //printk("tpd touch_event_handler\n");
	 	 #ifdef FTS_GESTRUE
			//i2c_smbus_read_i2c_block_data(i2c_client, 0xd0, 1, &state);
			ret = fts_read_reg(i2c_client, 0xd0,&state);
			if (ret<0) 
			{
				printk("[Focal][Touch] read value fail\n");
				//return ret;
			}
			//printk("tpd fts_read_Gestruedata state=%d\n",state);
		     	if(state ==1)
		     	{
		     		printk("-----------------[Focal][Touch] fts_read_Gestruedata\n");
			        fts_read_Gestruedata();
			        continue;
		    	}
		 #endif

		 #ifdef TPD_PROXIMITY

			 if (tpd_proximity_flag == 1)
			 {

				//i2c_smbus_read_i2c_block_data(i2c_client, 0xB0, 1, &state);

				ret = fts_read_reg(i2c_client, 0xB0,&state);
				if (ret<0) 
				{
					printk("[Focal][Touch] read value fail\n");
					//return ret;
				}
	           		TPD_PROXIMITY_DEBUG("proxi_fts 0xB0 state value is 1131 0x%02X\n", state);
				if(!(state&0x01))
				{
					tpd_enable_ps(1);
				}
				//i2c_smbus_read_i2c_block_data(i2c_client, 0x01, 1, &proximity_status);
				ret = fts_read_reg(i2c_client, 0x01,&proximity_status);
				if (ret<0) 
				{
					printk("[Focal][Touch] read value fail\n");
					//return ret;
				}
	            		TPD_PROXIMITY_DEBUG("proxi_fts 0x01 value is 1139 0x%02X\n", proximity_status);
				if (proximity_status == 0xC0)
				{
					tpd_proximity_detect = 0;	
				}
				else if(proximity_status == 0xE0)
				{
					tpd_proximity_detect = 1;
				}

				TPD_PROXIMITY_DEBUG("tpd_proximity_detect 1149 = %d\n", tpd_proximity_detect);
				if ((err = tpd_read_ps()))
				{
					TPD_PROXIMITY_DMESG("proxi_fts read ps data 1156: %d\n", err);	
				}
				sensor_data.values[0] = tpd_get_ps_value();
				sensor_data.value_divide = 1;
				sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
				//if ((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
				//{
				//	TPD_PROXIMITY_DMESG(" proxi_5206 call hwmsen_get_interrupt_data failed= %d\n", err);	
				//}
			}  

		#endif
                                
		#ifdef MT_PROTOCOL_B
		{
            		ret = fts_read_Touchdata(&pevent);
			//if (ret == 0)
			fts_report_value(&pevent);
		}
		#else
		{
			if (tpd_touchinfo(&cinfo, &pinfo)) 
			{
		    		//printk("tpd point_num = %d\n",point_num);
				TPD_DEBUG_SET_TIME;
				if(point_num >0) 
				{
				    for(i =0; i<point_num; i++)//only support 3 point
				    {
				         tpd_down(cinfo.x[i], cinfo.y[i], cinfo.id[i]);
				    }
				    input_sync(tpd->dev);
				}
				else  
	    			{
	              		tpd_up(cinfo.x[0], cinfo.y[0],&cinfo.id[0]);
	        	    		//TPD_DEBUG("release --->\n");         	   
	        	    		input_sync(tpd->dev);
	        		}
        		}
		}
		#endif
 	}while(!kthread_should_stop());
	return 0;
 }
  /************************************************************************
* Name: fts_reset_tp
* Brief: reset TP
* Input: pull low or high
* Output: no
* Return: 0
***********************************************************************/
void fts_reset_tp(int HighOrLow)
{
	if(touch_id == 0)
	{
		if(HighOrLow)
		{
			mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		    	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		    	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);  
		}
		else
		{
			mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		    	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		    	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
		}
	}
	else
	{
		if(HighOrLow)
		{
			mt_set_gpio_mode(GPIO_CTP2_RST_PIN, GPIO_CTP2_RST_PIN_M_GPIO);
		    	mt_set_gpio_dir(GPIO_CTP2_RST_PIN, GPIO_DIR_OUT);
		    	mt_set_gpio_out(GPIO_CTP2_RST_PIN, GPIO_OUT_ONE);  
		}
		else
		{
			mt_set_gpio_mode(GPIO_CTP2_RST_PIN, GPIO_CTP2_RST_PIN_M_GPIO);
		    	mt_set_gpio_dir(GPIO_CTP2_RST_PIN, GPIO_DIR_OUT);
		    	mt_set_gpio_out(GPIO_CTP2_RST_PIN, GPIO_OUT_ZERO);
		}
	}
}
   /************************************************************************
* Name: tpd_detect
* Brief: copy device name
* Input: i2c info, board info
* Output: no
* Return: 0
***********************************************************************/
 static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info) 
 {
	 	strcpy(info->type, TPD_DEVICE);	
	  	return 0;
 }
/************************************************************************
* Name: tpd_eint_interrupt_handler
* Brief: deal with the interrupt event
* Input: no
* Output: no
* Return: no
***********************************************************************/
 static void tpd_eint_interrupt_handler(void)
 {
	 //TPD_DEBUG("TPD interrupt has been triggered\n");
	 TPD_DEBUG_PRINT_INT;
	 tpd_flag = 1;
	 wake_up_interruptible(&waiter);
 }
/************************************************************************
* Name: fts_init_gpio_hw
* Brief: initial gpio
* Input: no
* Output: no
* Return: 0
***********************************************************************/
 static int fts_init_gpio_hw(void)
{

	int ret = 0;
	int i = 0;

	if(touch_id == 0)
	{
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	}
	else
	{
		mt_set_gpio_mode(GPIO_CTP2_RST_PIN, GPIO_CTP2_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP2_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP2_RST_PIN, GPIO_OUT_ONE);
	}

	return ret;
}
#if 0
static struct timer_list s_timer; 

static void second_handler(unsigned long arg)
{
	(void)arg;
	
	//unsigned int ret;
	//ret = mt_get_gpio_in(GPIO_EXPAND_SWITCH_PIN);

	printk("----------------------second_handler--------------------------\n");
	
	// if(ret == 1)
	 {
		input_report_key(tpd->dev, KEY_POWER, 1);
		input_sync(tpd->dev);
		input_report_key(tpd->dev, KEY_POWER, 0);
		input_sync(tpd->dev);
	 }
	 /*else
	 {
	 	input_report_key(tpd->dev, KEY_POWER, 1);
		input_sync(tpd->dev);
	 	input_report_key(tpd->dev, KEY_POWER, 0);
		input_sync(tpd->dev);
	 }*/
}

 static void switch_eint_interrupt_handler(void)
 {
	mt_eint_mask(CUST_EINT_SWITCH_NUM);

	printk("switch_eint_interrupt_handler\n");
	//tpd_flag = 1;

	mod_timer(&s_timer, jiffies + HZ);
	//wake_up_interruptible(&waiter);
	printk("----------------------switch_eint_interrupt_handler--------------------------\n");
	mt_eint_unmask(CUST_EINT_SWITCH_NUM);
 }
 #endif

/************************************************************************
* Name: tpd_probe
* Brief: driver entrance function for initial/power on/create channel 
* Input: i2c info, device id
* Output: no
* Return: 0
***********************************************************************/
 static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
 {	 
	int retval = TPD_OK;
	char data;
	u8 report_rate=0;
	//int err=0;
	int i;
	int reset_count = 0;
	unsigned char uc_reg_value;
	unsigned char uc_reg_addr;
	#ifdef TPD_PROXIMITY
		int err;
		struct hwmsen_object obj_ps;
	#endif

	reset_proc:   
		i2c_client = client;

   	#ifdef TPD_CLOSE_POWER_IN_SLEEP	 
		
	#else

		if(touch_id == 0)
		{
			mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		    	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		    	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
		}
		else
		{
			mt_set_gpio_mode(GPIO_CTP2_RST_PIN, GPIO_CTP2_RST_PIN_M_GPIO);
		    	mt_set_gpio_dir(GPIO_CTP2_RST_PIN, GPIO_DIR_OUT);
		    	mt_set_gpio_out(GPIO_CTP2_RST_PIN, GPIO_OUT_ZERO);  
		}
		msleep(10);
		
	#endif	

	//power on, need confirm with SA
	#ifdef TPD_POWER_SOURCE_CUSTOM
		hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
	#else
		hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
	#endif
	#ifdef TPD_POWER_SOURCE_1800
		hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
	#endif 


	#ifdef TPD_CLOSE_POWER_IN_SLEEP	 
		hwPowerDown(TPD_POWER_SOURCE,"TP");
		hwPowerOn(TPD_POWER_SOURCE,VOL_3300,"TP");
		msleep(100);
	#else
		
		
		msleep(10);
		TPD_DMESG(" fts reset\n");
	    	printk(" fts reset\n");
		if(touch_id == 0)
		{
			mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		    	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		    	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
		}
		else
		{
			mt_set_gpio_mode(GPIO_CTP2_RST_PIN, GPIO_CTP2_RST_PIN_M_GPIO);
		    	mt_set_gpio_dir(GPIO_CTP2_RST_PIN, GPIO_DIR_OUT);
		    	mt_set_gpio_out(GPIO_CTP2_RST_PIN, GPIO_OUT_ONE);
		}
	#endif	

	if(touch_id == 0)
	{
		mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	    	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	    	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
	    	mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
	}
	else
	{
		mt_set_gpio_mode(GPIO_CTP2_EINT_PIN, GPIO_CTP2_EINT_PIN_M_EINT);
	    	mt_set_gpio_dir(GPIO_CTP2_EINT_PIN, GPIO_DIR_IN);
	    	mt_set_gpio_pull_enable(GPIO_CTP2_EINT_PIN, GPIO_PULL_ENABLE);
	    	mt_set_gpio_pull_select(GPIO_CTP2_EINT_PIN, GPIO_PULL_UP);
	}

	msleep(150);

	msg_dma_alloct();
	
       fts_init_gpio_hw();
	
	uc_reg_addr = FTS_REG_POINT_RATE;				
	fts_i2c_Write(i2c_client, &uc_reg_addr, 1);
	fts_i2c_Read(i2c_client, &uc_reg_addr, 0, &uc_reg_value, 1);
	printk("mtk_tpd[FTS] report rate is %dHz.\n",uc_reg_value * 10);

	uc_reg_addr = FTS_REG_FW_VER;
	fts_i2c_Write(i2c_client, &uc_reg_addr, 1);
	fts_i2c_Read(i2c_client, &uc_reg_addr, 0, &uc_reg_value, 1);
	printk("mtk_tpd[FTS] Firmware version = 0x%x\n", uc_reg_value);


	uc_reg_addr = FTS_REG_CHIP_ID;
	fts_i2c_Write(i2c_client, &uc_reg_addr, 1);
	retval=fts_i2c_Read(i2c_client, &uc_reg_addr, 0, &uc_reg_value, 1);
	printk("mtk_tpd[FTS] chip id is %d.\n",uc_reg_value);
    	if(retval<0)
    	{
       	 printk("mtk_tpd[FTS] Read I2C error! driver NOt load!! CTP chip id is %d.\n",uc_reg_value);
		return 0;
	}
	
	tpd_load_status = 1;
	/*
	mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
	mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 1); 
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	*/
	if(touch_id == 0)
	{
		mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_TYPE, tpd_eint_interrupt_handler, 1);
	    	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	}
	else
	{
		mt_eint_registration(CUST_EINT_TOUCH_PANEL_2_NUM, CUST_EINT_TOUCH_PANEL_2_TYPE, tpd_eint_interrupt_handler, 1);
	    	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_2_NUM);
	}

	
    	#ifdef VELOCITY_CUSTOM_fts
		if((err = misc_register(&tpd_misc_device)))
		{
			printk("mtk_tpd: tpd_misc_device register failed\n");
		
		}
	#endif

	thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
	 if (IS_ERR(thread))
	{ 
		  retval = PTR_ERR(thread);
		  TPD_DMESG(TPD_DEVICE " failed to create kernel thread: %d\n", retval);
	}


	
	#ifdef SYSFS_DEBUG
                fts_create_sysfs(i2c_client);
	#endif
	fts_get_upgrade_array();
	#ifdef FTS_CTL_IIC
		 if (fts_rw_iic_drv_init(i2c_client) < 0)
			 dev_err(&client->dev, "%s:[FTS] create fts control iic driver failed\n", __func__);
	#endif
	
	#ifdef FTS_APK_DEBUG
		fts_create_apk_debug_channel(i2c_client);
	#endif
	
	#ifdef TPD_AUTO_UPGRADE
		if(touch_id == 0)
		{
			printk("********************Enter CTP Auto Upgrade********************\n");
			fts_ctpm_auto_upgrade(i2c_client);
		}
	#endif

	#ifdef TPD_PROXIMITY
		{
			obj_ps.polling = 1; //0--interrupt mode;1--polling mode;
			obj_ps.sensor_operate = tpd_ps_operate;
			if ((err = hwmsen_attach(ID_PROXIMITY, &obj_ps)))
			{
				TPD_DEBUG("hwmsen attach fail, return:%d.", err);
			}
		}
	#endif
	#ifdef GTP_ESD_PROTECT
   		INIT_DELAYED_WORK(&gtp_esd_check_work, gtp_esd_check_func);
    		gtp_esd_check_workqueue = create_workqueue("gtp_esd_check");
    		queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, TPD_ESD_CHECK_CIRCLE);
	#endif

	#ifdef FTS_GESTRUE
		if(touch_id == 0)
			init_para(320,320,60,0,0);
		else
			init_para(720,1280,60,0,0);

	 	/* modify by huanchao */
		/*input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_E); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_M); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_L);
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_O);
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_S);
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_U);
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_W);
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_Z);

		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_UP); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_DOWN);
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_LEFT); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_RIGHT);
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_DOUBLECLICK);
		
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_F1);
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_F2);
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_F3);
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_F4);
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_F5);*/

		for(i = 0; i < sizeof(shizhong_fts_keys) / sizeof(shizhong_fts_keys[0]); i++)
		{
			input_set_capability(tpd->dev, EV_KEY, shizhong_fts_keys[i].key); 
		}
/*
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_LINE_HORIZ_LEFT); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_LINE_HORIZ_RIGHT); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_LINE_VERT_UP); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_LINE_VERT_DOWN); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_DOUBLE_CLICK); 
		
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_O); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_W); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_M); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_E); 
		//input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_C); 
		//input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_G); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_A); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_D); 

		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_N); 
		//input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_Z); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_B); 
		//input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_Q); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_L); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_P); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_S); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_CHAR_U); 

		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_SIGN_AT); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_SIGN_ARROW_LEFT); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_SIGN_ARROW_RIGHT); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_SIGN_ARROW_UP); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_SIGN_ARROW_DOWN); 
		//input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_SIGN_TRI_ANGLE); 

		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_NUMBER_3); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_NUMBER_6); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_NUMBER_9); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_NUMBER_7); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_NUMBER_8); 
		input_set_capability(tpd->dev, EV_KEY, KEY_GESTURE_NUMBER_2); */
		/* end modify by huangchao */

	#endif

	#ifdef MT_PROTOCOL_B
		#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
			input_mt_init_slots(tpd->dev, MT_MAX_TOUCH_POINTS);
		#endif
		input_set_abs_params(tpd->dev, ABS_MT_TOUCH_MAJOR,0, 255, 0, 0);
		if(touch_id == 0)
		{
			input_set_abs_params(tpd->dev, ABS_MT_POSITION_X, 0, TPD_RES_X, 0, 0);
			input_set_abs_params(tpd->dev, ABS_MT_POSITION_Y, 0, TPD_RES_Y, 0, 0);
		}
		else
		{
			input_set_abs_params(tpd->dev, ABS_MT_POSITION_X, 0, 720, 0, 0);
			input_set_abs_params(tpd->dev, ABS_MT_POSITION_Y, 0, 1280, 0, 0);
		}
		input_set_abs_params(tpd->dev, ABS_MT_PRESSURE, 0, 255, 0, 0);
	#endif

	/* switch screen */
	#if 0
	input_set_capability(tpd->dev, EV_KEY, KEY_POWER);
	
	mt_set_gpio_dir(GPIO_EXPAND_SWITCH_PIN, GPIO_DIR_IN);
    	mt_set_gpio_pull_enable(GPIO_EXPAND_SWITCH_PIN, GPIO_PULL_DISABLE);
	mt_set_gpio_mode(GPIO_EXPAND_SWITCH_PIN, GPIO_MODE_00);
	
	TPD_DEBUG("mt_eint_registration for switch.\n");
	mt_eint_registration(CUST_EINT_SWITCH_NUM, CUST_EINTF_TRIGGER_RISING | CUST_EINTF_TRIGGER_FALLING, switch_eint_interrupt_handler, 1);
    	mt_eint_unmask(CUST_EINT_SWITCH_NUM);
	TPD_DEBUG("mt_eint_registration for switch ok.\n");

	init_timer(&s_timer);
	s_timer.function = second_handler;
	//s_timer.data = (unsigned long)second_devp;
	//s_timer.expires = jiffies + HZ;
	add_timer(&s_timer);
	#endif
	
   	printk("fts Touch Panel Device Probe %s\n", (retval < TPD_OK) ? "FAIL" : "PASS");
   	return 0;
   
 }
/************************************************************************
* Name: tpd_remove
* Brief: remove driver/channel
* Input: i2c info
* Output: no
* Return: 0
***********************************************************************/
 static int __devexit tpd_remove(struct i2c_client *client)
 
 {
     msg_dma_release();

     #ifdef FTS_CTL_IIC
     		fts_rw_iic_drv_exit();
     #endif
     #ifdef SYSFS_DEBUG
     		fts_release_sysfs(client);
     #endif
     #ifdef GTP_ESD_PROTECT
    		destroy_workqueue(gtp_esd_check_workqueue);
     #endif

     #ifdef FTS_APK_DEBUG
     		fts_release_apk_debug_channel();
     #endif

	 //del_timer(&s_timer);

	 TPD_DEBUG("TPD removed\n");
 
   return 0;
 }
#ifdef GTP_ESD_PROTECT
/************************************************************************
* Name: force_reset_guitar
* Brief: reset
* Input: no
* Output: no
* Return: 0
***********************************************************************/
static void force_reset_guitar(void)
{
    	s32 i;
    	s32 ret;

	if(touch_id == 0)
	{
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
		msleep(10);
	    	TPD_DMESG("force_reset_guitar\n");

		hwPowerDown(MT6323_POWER_LDO_VGP1,  "TP");
		msleep(200);
		hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_2800, "TP");
		msleep(5);

		
		msleep(10);
		TPD_DMESG(" fts ic reset\n");
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);

		mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
		mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
		mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
		mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
	}
	else
	{
		mt_set_gpio_mode(GPIO_CTP2_RST_PIN, GPIO_CTP2_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP2_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP2_RST_PIN, GPIO_OUT_ZERO);  
		msleep(10);
	    	TPD_DMESG("force_reset_guitar\n");

		hwPowerDown(MT6323_POWER_LDO_VGP1,  "TP");
		msleep(200);
		hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_2800, "TP");
		msleep(5);

		
		msleep(10);
		TPD_DMESG(" fts ic reset\n");
		mt_set_gpio_mode(GPIO_CTP2_RST_PIN, GPIO_CTP2_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP2_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP2_RST_PIN, GPIO_OUT_ONE);

		mt_set_gpio_mode(GPIO_CTP2_EINT_PIN, GPIO_CTP2_EINT_PIN_M_EINT);
		mt_set_gpio_dir(GPIO_CTP2_EINT_PIN, GPIO_DIR_IN);
		mt_set_gpio_pull_enable(GPIO_CTP2_EINT_PIN, GPIO_PULL_ENABLE);
		mt_set_gpio_pull_select(GPIO_CTP2_EINT_PIN, GPIO_PULL_UP);
	}

	msleep(300);
	
#ifdef TPD_PROXIMITY
	if (FT_PROXIMITY_ENABLE == tpd_proximity_flag) 
	{
		tpd_enable_ps(FT_PROXIMITY_ENABLE);
	}
#endif
}
//0 for no apk upgrade, 1 for apk upgrade
extern int apk_debug_flag; 
#define A3_REG_VALUE								0x12
#define RESET_91_REGVALUE_SAMECOUNT 				5
static u8 g_old_91_Reg_Value = 0x00;
static u8 g_first_read_91 = 0x01;
static u8 g_91value_same_count = 0;
/************************************************************************
* Name: gtp_esd_check_func
* Brief: esd check function
* Input: struct work_struct
* Output: no
* Return: 0
***********************************************************************/
static void gtp_esd_check_func(struct work_struct *work)
{
	int i;
	int ret = -1;
	u8 data, data_old;
	u8 flag_error = 0;
	int reset_flag = 0;
	u8 check_91_reg_flag = 0;

	if (tpd_halt ) 
	{
		return;
	}
	if(apk_debug_flag) 
	{
		queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, esd_check_circle);
		return;
	}

	run_check_91_register = 0;
	for (i = 0; i < 3; i++) 
	{
		//ret = fts_i2c_smbus_read_i2c_block_data(i2c_client, 0xA3, 1, &data);
		ret = fts_read_reg(i2c_client, 0xA3,&data);
		if (ret<0) 
		{
			printk("[Focal][Touch] read value fail");
			//return ret;
		}
		if (ret==1 && A3_REG_VALUE==data) 
		{
		    break;
		}
	}

	if (i >= 3) 
	{
		force_reset_guitar();
		printk("focal--tpd reset. i >= 3  ret = %d	A3_Reg_Value = 0x%02x\n ", ret, data);
		reset_flag = 1;
		goto FOCAL_RESET_A3_REGISTER;
	}

	//esd check for count
  	//ret = fts_i2c_smbus_read_i2c_block_data(i2c_client, 0x8F, 1, &data);
	ret = fts_read_reg(i2c_client, 0x8F,&data);
	if (ret<0) 
	{
		printk("[Focal][Touch] read value fail\n");
		//return ret;
	}
	printk("0x8F:%d, count_irq is %d\n", data, count_irq);
			
	flag_error = 0;
	if((count_irq - data) > 10) 
	{
		if((data+200) > (count_irq+10) )
		{
			flag_error = 1;
		}
	}
	
	if((data - count_irq ) > 10) 
	{
		flag_error = 1;		
	}
		
	if(1 == flag_error) 
	{	
		printk("focal--tpd reset.1 == flag_error...data=%d	count_irq\n ", data, count_irq);
	    	force_reset_guitar();
		reset_flag = 1;
		goto FOCAL_RESET_INT;
	}

	run_check_91_register = 1;
	//ret = fts_i2c_smbus_read_i2c_block_data(i2c_client, 0x91, 1, &data);
	ret = fts_read_reg(i2c_client, 0x91,&data);
	if (ret<0) 
	{
		printk("[Focal][Touch] read value fail");
		//return ret;
	}
	printk("focal---------91 register value = 0x%02x	old value = 0x%02x\n",	data, g_old_91_Reg_Value);
	if(0x01 == g_first_read_91) 
	{
		g_old_91_Reg_Value = data;
		g_first_read_91 = 0x00;
	} 
	else 
	{
		if(g_old_91_Reg_Value == data)
		{
			g_91value_same_count++;
			printk("focal 91 value ==============, g_91value_same_count=%d\n", g_91value_same_count);
			if(RESET_91_REGVALUE_SAMECOUNT == g_91value_same_count) 
			{
				force_reset_guitar();
				printk("focal--tpd reset. g_91value_same_count = 5\n");
				g_91value_same_count = 0;
				reset_flag = 1;
			}
			
			//run_check_91_register = 1;
			esd_check_circle = TPD_ESD_CHECK_CIRCLE / 2;
			g_old_91_Reg_Value = data;
		} 
		else 
		{
			g_old_91_Reg_Value = data;
			g_91value_same_count = 0;
			//run_check_91_register = 0;
			esd_check_circle = TPD_ESD_CHECK_CIRCLE;
		}
	}
FOCAL_RESET_INT:
FOCAL_RESET_A3_REGISTER:
	count_irq=0;
	data=0;
	//fts_i2c_smbus_write_i2c_block_data(i2c_client, 0x8F, 1, &data);
	ret = fts_write_reg(i2c_client, 0x8F,&data);
	if (ret<0) 
	{
		printk("[Focal][Touch] write value fail");
		//return ret;
	}
	if(0 == run_check_91_register)
	{
		g_91value_same_count = 0;
	}
	
	if( (1 == reset_flag) && ( FT_PROXIMITY_ENABLE == tpd_proximity_flag) )
	{
		if((tpd_enable_ps(FT_PROXIMITY_ENABLE) != 0))
		{
			APS_ERR("enable ps fail\n"); 
			return -1;
		}
	}
	//end esd check for count

    	if (!tpd_halt)
    	{
        	//queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, TPD_ESD_CHECK_CIRCLE);
        	queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, esd_check_circle);
    	}

    	return;
}
#endif

 /************************************************************************
* Name: tpd_local_init
* Brief: add driver info
* Input: no
* Output: no
* Return: fail <0
***********************************************************************/
 static int tpd_local_init(void)
 {
  	TPD_DMESG("Focaltech fts I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);
   	if(i2c_add_driver(&tpd_i2c_driver)!=0)
   	{
        	TPD_DMESG("fts unable to add i2c driver.\n");
      		return -1;
    	}
    	if(tpd_load_status == 0) 
    	{
       	 TPD_DMESG("fts add error touch panel driver.\n");
    		i2c_del_driver(&tpd_i2c_driver);
    		return -1;
    	}
	//TINNO_TOUCH_TRACK_IDS <--- finger number
	//TINNO_TOUCH_TRACK_IDS	5
	#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 8, 0))
		//for linux 3.8
		input_set_abs_params(tpd->dev, ABS_MT_TRACKING_ID, 0, (TPD_MAX_POINTS_5-1), 0, 0);
	#endif

	
   	#ifdef TPD_HAVE_BUTTON     
		// initialize tpd button data
		if(touch_id == 0)
    			tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);
	#endif   
  
	#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
    		TPD_DO_WARP = 1;
    		memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    		memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
	#endif 

	#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
    		memcpy(tpd_calmat, tpd_def_calmat_local, 8*4);
    		memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);	
	#endif  
	TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
	tpd_type_cap = 1;
    	return 0; 
 }
 /************************************************************************
* Name: tpd_resume
* Brief: system wake up 
* Input: no use
* Output: no
* Return: no
***********************************************************************/
 static void tpd_resume( struct early_suspend *h )
 {
	int i=0,ret = 0;
	
 	TPD_DMESG("TPD wake up\n");
	buf_addr[0]=0xC0;
	buf_addr[1]=0x8B;
				
	for(i=0;i<2;i++)
	{
		ret = fts_write_reg(i2c_client, buf_addr[i], buf_value[i]);
		if (ret<0) 
		{
			printk("[Focal][Touch] write value fail");
			//return ret;
		}
	}
	
  	#ifdef TPD_PROXIMITY	
		if (tpd_proximity_flag == 1)
		{
			if(tpd_proximity_flag_one == 1)
			{
				tpd_proximity_flag_one = 0;	
				TPD_DMESG(TPD_DEVICE " tpd_proximity_flag_one \n"); 
				return;
			}
		}
	#endif	

 	#ifdef FTS_GESTRUE
    		fts_write_reg(i2c_client,0xD0,0x00);
	#endif
	#ifdef TPD_CLOSE_POWER_IN_SLEEP	
		hwPowerOn(TPD_POWER_SOURCE,VOL_3300,"TP");
	#else

	if(touch_id == 0)
	{
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	    	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	    	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
	    	msleep(1);  
	    	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	    	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	    	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	}
	else
	{
		mt_set_gpio_mode(GPIO_CTP2_RST_PIN, GPIO_CTP2_RST_PIN_M_GPIO);
	    	mt_set_gpio_dir(GPIO_CTP2_RST_PIN, GPIO_DIR_OUT);
	    	mt_set_gpio_out(GPIO_CTP2_RST_PIN, GPIO_OUT_ZERO);  
	    	msleep(1);  
	    	mt_set_gpio_mode(GPIO_CTP2_RST_PIN, GPIO_CTP2_RST_PIN_M_GPIO);
	    	mt_set_gpio_dir(GPIO_CTP2_RST_PIN, GPIO_DIR_OUT);
	    	mt_set_gpio_out(GPIO_CTP2_RST_PIN, GPIO_OUT_ONE);
	}
	#endif
	if(touch_id == 0)
		mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
	else
		mt_eint_unmask(CUST_EINT_TOUCH_PANEL_2_NUM);  
	msleep(30);
	tpd_halt = 0;
	
	#ifdef GTP_ESD_PROTECT
    		queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, TPD_ESD_CHECK_CIRCLE);
	#endif

	TPD_DMESG("TPD wake up done\n");

 }
 /************************************************************************
* Name: tpd_suspend
* Brief: system sleep
* Input: no use
* Output: no
* Return: no
***********************************************************************/
 static void tpd_suspend( struct early_suspend *h )
 {
	static char data = 0x3;
	int i=0,ret = 0;
	buf_addr[0]=0xC0;
	buf_addr[1]=0x8B;
				
	for(i=0;i<2;i++)
	{
		ret = fts_read_reg(i2c_client, buf_addr[i], (buf_value+i));
		if (ret<0) 
		{
			printk("[Focal][Touch] read value fail");
			//return ret;
		}
	}

			
	TPD_DMESG("TPD enter sleep\n");
	#ifdef TPD_PROXIMITY
	if (tpd_proximity_flag == 1)
	{
		tpd_proximity_flag_one = 1;	
		return;
	}
	#endif

	#ifdef FTS_GESTRUE
        	fts_write_reg(i2c_client, 0xd0, 0x01);
		if (fts_updateinfo_curr.CHIP_ID==0x54 || fts_updateinfo_curr.CHIP_ID==0x58)
		{
		  	fts_write_reg(i2c_client, 0xd1, 0xff);
			fts_write_reg(i2c_client, 0xd2, 0xff);
			fts_write_reg(i2c_client, 0xd5, 0xff);
			fts_write_reg(i2c_client, 0xd6, 0xff);
			fts_write_reg(i2c_client, 0xd7, 0xff);
			fts_write_reg(i2c_client, 0xd8, 0xff);
		}
        	return;
	#endif
	#ifdef GTP_ESD_PROTECT
    		cancel_delayed_work_sync(&gtp_esd_check_work);
	#endif
 	 tpd_halt = 1;

	 if(touch_id == 0)
	 	mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	 else
	 	mt_eint_mask(CUST_EINT_TOUCH_PANEL_2_NUM);
	 mutex_lock(&i2c_access);
	#ifdef TPD_CLOSE_POWER_IN_SLEEP	
		hwPowerDown(TPD_POWER_SOURCE,"TP");
	#else
		if ((fts_updateinfo_curr.CHIP_ID==0x59))
		{
			//i2c_smbus_write_i2c_block_data(i2c_client, 0xA5, 1, &data);  //TP enter sleep mode
			ret = fts_write_reg(i2c_client, 0xA5,&data);
			if (ret<0) 
			{
				printk("[Focal][Touch] write value fail");
				//return ret;
			}
		}
		else
		{
			data = 0x02;
			//i2c_smbus_write_i2c_block_data(i2c_client, 0xA5, 1, &data);  //TP enter sleep mode
			ret = fts_write_reg(i2c_client, 0xA5,&data);
			if (ret<0) 
			{
				printk("[Focal][Touch] write value fail");
				//return ret;
			}
		}
	#endif
	mutex_unlock(&i2c_access);

	//disable_irq_nosync(ts->pdata->intr_gpio);
	//

	//if ((fts_updateinfo_curr.CHIP_ID==0x59))
		//fts_write_reg(ts->client,0xa5,0x02);
	//else
		//fts_write_reg(ts->client,0xa5,0x03);	
	//msleep(10);
	/*release add touches*/
			
	//for (i = 0; i <CFG_MAX_TOUCH_POINTS; i++) 
	//{
	//	input_mt_slot(ts->input_dev, i);
	//	input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 0);
	//}
	//input_mt_report_pointer_emulation(ts->input_dev, false);
	//input_sync(ts->input_dev);
	
    	TPD_DMESG("TPD enter sleep done\n");

 } 


 static struct tpd_driver_t tpd_device_driver = {
       	 .tpd_device_name = "fts",
		 .tpd_local_init = tpd_local_init,
		 .suspend = tpd_suspend,
		 .resume = tpd_resume,
	#ifdef TPD_HAVE_BUTTON
		 .tpd_have_button = 1,
	#else
		 .tpd_have_button = 0,
	#endif		
 };

  /************************************************************************
* Name: tpd_suspend
* Brief:  called when loaded into kernel
* Input: no
* Output: no
* Return: 0
***********************************************************************/
 static int __init tpd_driver_init(void) {
 	unsigned int ret = 0;
        printk("MediaTek fts touch panel driver init\n");
	 ret = mt_get_gpio_in(GPIO17);
	touch_id = 0;
	 if(ret == 1)
	 {
	 	touch_id = 1;
		tpd_device_driver.tpd_have_button = 0;
		fts_i2c_tpd.addr = (0x80 >> 1);
	 }
        i2c_register_board_info(IIC_PORT, &fts_i2c_tpd, 1);
	 if(tpd_driver_add(&tpd_device_driver) < 0)
        	TPD_DMESG("add fts driver failed\n");
	 return 0;
 }
 
 
/************************************************************************
* Name: tpd_driver_exit
* Brief:  should never be called
* Input: no
* Output: no
* Return: 0
***********************************************************************/
 static void __exit tpd_driver_exit(void) 
 {
        TPD_DMESG("MediaTek fts touch panel driver exit\n");
	 //input_unregister_device(tpd->dev);
	 tpd_driver_remove(&tpd_device_driver);
 }
 
 module_init(tpd_driver_init);
 module_exit(tpd_driver_exit);


