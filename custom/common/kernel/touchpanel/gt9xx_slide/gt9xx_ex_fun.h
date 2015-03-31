#ifndef __LINUX_FT5X06_EX_FUN_H__
#define __LINUX_FT5X06_EX_FUN_H__

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>

#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>

#define    AUTO_CLB
#define FTS_DBG
#ifdef FTS_DBG
#define DBG(fmt, args...) printk("[FTS]" fmt, ## args)
#else
#define DBG(fmt, args...) do{}while(0)
#endif

/*create sysfs for debug*/
int gt9xx_create_sysfs(struct i2c_client * client);
void gt9xx_release_sysfs(struct i2c_client * client);

int gt9xx_create_apk_debug_channel(struct i2c_client *client);
void gt9xx_release_apk_debug_channel(void);
int fts_ctpm_auto_upgrade(struct i2c_client *client);
/*
*gt9xx_write_reg- write register
*@client: handle of i2c
*@regaddr: register address
*@regvalue: register value
*/
int gt9xx_write_reg(struct i2c_client * client,u8 regaddr, u8 regvalue);
int gt9xx_read_reg(struct i2c_client * client,u8 regaddr, u8 *regvalue);

#endif
