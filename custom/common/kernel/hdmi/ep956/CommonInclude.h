/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2013
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

  File        :  CommonInclude.h 

  Description :  Head file of Unix IO Interface 

\******************************************************************************/

#ifndef COMMON_INCLUDE_H
#define COMMON_INCLUDE_H

#define MHL_CODE
#define IN_SPIDIF
#define OUT_HDCP_DIS

#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/syscalls.h> 
#include <linux/fcntl.h> 
#include <linux/regulator/consumer.h>

#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

//==================================================================================================
//
// Protected Data Member
//

#define DEVICE_ADDR_EP955	0x50

#if 1
#define EP_LOG_FUNCTION_NAME_ENTRY	printk(KERN_INFO		"[MHL] ## %s() ++ ##\n",  __func__);
#define EP_LOG_FUNCTION_NAME_EXIT	printk(KERN_INFO		"[MHL] ## %s() -- ##\n",  __func__);
#define EP_DEV_DBG_ERROR(fmt, args...)	printk(KERN_ERR		"[MHL] Exception [%s][%d] :: "fmt, __func__, __LINE__, ##args);
#define EP_DEV_DBG(fmt, args...) 			printk(KERN_DEBUG	"[MHL] [%s] :: " fmt, __func__, ##args);
#else
#define EP_LOG_FUNCTION_NAME_ENTRY 
#define EP_LOG_FUNCTION_NAME_EXIT  
#define EP_DEV_DBG_ERROR(fmt, args...)
#define EP_DEV_DBG(fmt, args...)
#endif

//
// Type Definition
//
#define FALSE 0
#define TRUE  1

#ifndef BOOL
#define BOOL char
#endif
#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef PBYTE
#define PBYTE BYTE *
#endif
#ifndef WORD
#define WORD unsigned int
#endif
#ifndef PWORD
#define PWORD WORD *
#endif
#ifndef DWORD
#define DWORD unsigned long
#endif
#ifndef PDWORD
#define PDWORD DWORD *
#endif
#ifndef USHORT
#define USHORT unsigned short
#endif

#ifndef ULONG
#define ULONG unsigned long
#endif
#ifndef PULONG
#define PULONG ULONG *
#endif

#endif // ANDROID_EP955_H


