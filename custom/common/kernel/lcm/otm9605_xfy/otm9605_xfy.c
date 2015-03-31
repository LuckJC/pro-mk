/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#if defined(BUILD_LK)
#include <string.h>
#else
#include <linux/string.h>
#endif


#if defined(BUILD_LK)
#include "cust_gpio_usage.h"
#include <platform/mt_gpio.h>
#else
#include "cust_gpio_usage.h"
#include <mach/mt_gpio.h>
#endif

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  			(540)
#define FRAME_HEIGHT 			(960)

#define REGFLAG_DELAY             	0xFD
#define REGFLAG_END_OF_TABLE      	0xFE   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE		0


#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#define GPIO_LCM_ID	GPIO18
#define LCM_TDT		0

//bool lcm_ili9806e_vendor=LCM_TDT;	//default to choose byd panel


//set LCM IC ID
//#define LCM_ID_ILI9807		(0x980700)

#define LCM_DEBUG

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    	(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 		(lcm_util.udelay(n))
#define MDELAY(n) 		(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};



static struct LCM_setting_table lcm_tdt_initialization_setting[] = {
//gamma2.2
/////////////OTM9605_HSD5.0/////////////////////////////
{0x00,	1,	{0x00}},
{0xFF,	3,	{0x96,0x05,0x01}},

{0x00,	1,	{0x80}},
{0xFF,	2,	{0x96,0x05}},

{0x00,	1,	{0x00}},
{0xA0,	1,	{0x00}},

{0x00,	1,	{0xC5}},
{0xB0,	1,	{0x03}},

{0x00,	1,	{0x83}},
{0xB2,	1,	{0x80}},

{0x00,	1,	{0x89}},
{0xC0,	1,	{0x01}},

{0x00,	1,	{0xB4}},
{0xC0,	1,	{0x50}},

{0x00,	1,	{0x80}},
{0xC1,	2,	{0x36,0X66}},

{0x00,	1,	{0xA0}},
{0xC1,	1,	{0x00}},

{0x00,	1,	{0x80}},
{0xC4,	1,	{0x9C}},

{0x00,	1,	{0x80}},
{0xC5,	4,	{0x08,0X00,0XA0,0X11}},

{0x00,	1,	{0x90}},
{0xC5,	7,	{0x96,0X76,0X01,0X76,0X33,0X33,0X34}},

{0x00,	1,	{0xB0}},
{0xC5,	2,	{0x04,0X28}},

{0x00,	1,	{0xC0}},
{0xC5,	1,	{0X00}},

{0x00,	1,	{0x00}},
{0xD0,	1,	{0X40}},

{0x00,	1,	{0x00}},
{0xD1,	2,	{0X00,0X00}},

{0x00,	1,	{0xB2}},
{0xF5,	4,	{0X15,0X00,0X15,0X00}},

{0x00,	1,	{0x80}},
{0xCB,	10,	{0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00}},

{0x00,	1,	{0x90}},
{0xCB,	15,	{0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00}},

{0x00,	1,	{0xA0}},
{0xCB,	15,	{0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00}},

{0x00,	1,	{0xB0}},
{0xCB,	10,	{0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00}},

{0x00,	1,	{0xC0}},
{0xCB,	15,	{0X04,0X04,0X04,0X04,0X00,0X04,0X00,0X04,0X00,0X04,0X00,0X04,0X04,0X04,0X00}},

{0x00,	1,	{0xD0}},
{0xCB,	15,	{0X00,0X00,0X00,0X00,0X00,0X04,0X04,0X04,0X04,0X00,0X04,0X00,0X04,0X00,0X04}},

{0x00,	1,	{0xE0}},
{0xCB,	10,	{0X00,0X04,0X04,0X04,0X00,0X00,0X00,0X00,0X00,0X00}},

{0x00,	1,	{0xF0}},
{0xCB,	10,	{0X00,0XCC,0XCC,0X00,0X00,0X00,0XCC,0XCC,0X0F,0X00}},

{0x00,	1,	{0x80}},
{0xCC,	10,	{0X26,0X25,0X21,0X22,0X00,0X0C,0X00,0X0A,0X00,0X10}},

{0x00,	1,	{0x90}},
{0xCC,	15,	{0X00,0X0E,0X02,0X04,0X00,0X00,0X00,0X00,0X00,0X00,0X26,0X25,0X21,0X22,0X00}},

{0x00,	1,	{0xA0}},
{0xCC,	15,	{0X0B,0X00,0X09,0X00,0X0F,0X00,0X0D,0X01,0X03,0X00,0X00,0X00,0X00,0X00,0X00}},

{0x00,	1,	{0xB0}},
{0xCC,	10,	{0X25,0X26,0X21,0X22,0X00,0X0D,0X00,0X0F,0X00,0X09}},

{0x00,	1,	{0xC0}},
{0xCC,	15,	{0X00,0X0B,0X03,0X01,0X00,0X00,0X00,0X00,0X00,0X00,0X25,0X26,0X21,0X22,0X00}},

{0x00,	1,	{0xD0}},
{0xCC,	15,	{0X0E,0X00,0X10,0X00,0X0A,0X00,0X0C,0X04,0X02,0X00,0X00,0X00,0X00,0X00,0X00}},

{0x00,	1,	{0x80}},
{0xCE,	12,	{0X8B,0X03,0X28,0X8A,0X03,0X28,0X89,0X03,0X28,0X88,0X03,0X28}},

{0x00,	1,	{0x90}},
{0xCE,	14,	{0XF0,0X00,0X00,0XF0,0X00,0X00,0XF0,0X00,0X00,0XF0,0X00,0X00,0X00,0X00}},

{0x00,	1,	{0xA0}},
{0xCE,	14,	{0X38,0X07,0X03,0XC0,0X00,0X18,0X00,0X38,0X06,0X03,0XC1,0X00,0X18,0X00}},

{0x00,	1,	{0xB0}},
{0xCE,	14,	{0X38,0X05,0X03,0XC2,0X00,0X18,0X00,0X38,0X04,0X03,0XC3,0X00,0X18,0X00}},

{0x00,	1,	{0xC0}},
{0xCE,	14,	{0X38,0X03,0X03,0XC4,0X00,0X18,0X00,0X38,0X02,0X03,0XC5,0X00,0X18,0X00}},

{0x00,	1,	{0xD0}},
{0xCE,	14,	{0X38,0X01,0X03,0XC6,0X00,0X18,0X00,0X38,0X00,0X03,0XC7,0X00,0X18,0X00}},

{0x00,	1,	{0xC0}},
{0xCF,	1,	{0X02}},

{0x00,	1,	{0xC7}},
{0xCF,	1,	{0X80}},

{0x00,	1,	{0xC9}},
{0xCF,	1,	{0X08}},

{0x00,	1,	{0x00}},
{0xD8,	2,	{0X6F,0X6F}},

{0x00,	1,	{0x00}},
{0xD9,	1,	{0X39}},//3d

{0x00,	1,	{0x00}},
{0xE1,	16,	{0X01,0X08,0X0D,0X0D,0X06,0X0C,0X0A,0X09,0X05,0X08,0X10,0X08,0X0F,0X10,0X09,0X04}},

{0x00,	1,	{0x00}},
{0xE2,	16,	{0X01,0X08,0X0D,0X0D,0X06,0X0C,0X0A,0X09,0X05,0X08,0X10,0X08,0X0F,0X10,0X09,0X04}},
///////
{0x00,	1,	{0xb1}},
{0xC5,	1,	{0X28}},

{0x00,	1,	{0x80}},
{0xC4,	1,	{0X9c}},

{0x00,	1,	{0xc0}},
{0xC5,	1,	{0X00}},

{0x00,	1,	{0xb2}},
{0xf5,	4,	{0X15,0X00,0X15,0X00}},

{0x00,	1,	{0x93}},
{0xC5,	1,	{0X03}},

{0x00,	1,	{0x80}},
{0xC1,	2,	{0X36,0X66}},

{0x00,	1,	{0x89}},
{0xC0,	1,	{0X01}},

{0x00,	1,	{0xa0}},
{0xC1,	1,	{0X00}},

{0x00,	1,	{0xc5}},
{0xb0,	1,	{0X03}},
////////

{0x00,	1,	{0x00}},
{0xFF,	3,	{0XFF,0XFF,0XFF}},	

{0x11, 1, {0x00}},
{REGFLAG_DELAY, 20, {}},
{0x29, 1, {0x00}},
{REGFLAG_DELAY, 120, {}},
{REGFLAG_END_OF_TABLE, 0x00, {}}


};

static struct LCM_setting_table lcm_tdt_initialization_debug1[] = {

	{0xFF,	5,	{0xFF, 0x98, 0x06, 0x04, 0x01}},// Change to Page 1
	{0x08,	1,	{0x10}},	// output SDA
	{0x21,	1,	{0x01}},	// DE = 1 Active
	{0x30,	1,	{0x01}},	// 480 X 854
	{0x31,	1,	{0x02}},	// 2-dot Inversion
	{0x40,	1,	{0x16}},	// BT
	{0x41,	1,	{0x33}},	// DVDDH DVDDL clamp 
	{0x42,	1,	{0x03}},	// VGH/VGL 
	{0x43,	1,	{0x09}},	// VGH_CLAMP 0FF ;
	{0x44,	1,	{0x07}},	// VGL_CLAMP OFF ; 
	{0x45,	1,	{0x16}},	// VGL_REG  -11V 
	{0x50,	1,	{0x78}},	// VGMP
	{0x51,	1,	{0x78}},	// VGMN
	{0x52,	1,	{0x00}},	// Flicker 
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_tdt_initialization_debug2[] = {
	{0x60,  1,  {0x07}},	  // SDTI
	{0x61,  1,  {0x00}},	  // CRTI
	{0x62,  1,  {0x08}},	  // EQTI
	{0x63,  1,  {0x00}},	  // PCTI
	//++++++++++++++++++ Gamma Setting ++++++++++++++++++//
	{0xA0,  1,  {0x00}},	  // Gamma 255	   
	{0xA1,  1,  {0x0A}},	  // Gamma 251	  
	{0xA2,  1,  {0x10}},	  // Gamma 247	 
	{0xA3,  1,  {0x0C}},	  // Gamma 239	 
	{0xA4,  1,  {0x04}},	  // Gamma 231	
	{0xA5,  1,  {0x07}},	  // Gamma 203	
	{0xA6,  1,  {0x07}},	  // Gamma 175	
	{0xA7,  1,  {0x05}},	  // Gamma 147	 
	{0xA8,  1,  {0x07}},	  // Gamma 108	
	{0xA9,  1,  {0x0B}},	  // Gamma 80  
	{0xAA,  1,  {0x13}},	  // Gamma 52  
	{0xAB,  1,  {0x08}},	  // Gamma 24	
	{0xAC,  1,  {0x10}},	  // Gamma 16	 
	{0xAD,  1,  {0x12}},	  // Gamma 8  
	{0xAE,  1,  {0x0C}},	  // Gamma 4	  
	{0xAF,  1,  {0x00}},	  // Gamma 0   
	///==============Nagitive		
	{0xC0,  1,  {0x00}},	  // Gamma 255	  
	{0xC1,  1,  {0x0A}},	  // Gamma 251	 
	{0xC2,  1,  {0x10}},	  // Gamma 247	
	{0xC3,  1,  {0x0C}},	  // Gamma 239	
	{0xC4,  1,  {0x04}},	  // Gamma 231 
	{0xC5,  1,  {0x07}},	  // Gamma 203	
	{0xC6,  1,  {0x07}},	  // Gamma 175	 
	{0xC7,  1,  {0x05}},	  // Gamma 147	 
	{0xC8,  1,  {0x07}},	  // Gamma 108	 
	{0xC9,  1,  {0x0B}},	  // Gamma 80	
	{0xCA,  1,  {0x13}},	  // Gamma 52	
	{0xCB,  1,  {0x08}},	  // Gamma 24	 
	{0xCC,  1,  {0x10}},	  // Gamma 16	 
	{0xCD,  1,  {0x12}},	  // Gamma 8   
	{0xCE,  1,  {0x0C}},	  // Gamma 4	
	{0xCF,  1,  {0x00}},	  // Gamma 0	

	{0xFF,  5,  {0xFF, 0x98, 0x06, 0x04, 0x06}},// Change to Page 6
	{0x00,  1,  {0x20}},	  //
	{0x01,  1,  {0x0A}},	  //
	{0x02,  1,  {0x00}},	  //
	{0x03,  1,  {0x00}},	  //
	{0x04,  1,  {0x01}},	  //
	{0x05,  1,  {0x01}},	  //
	{0x06,  1,  {0x98}},	  //
	{0x07,  1,  {0x06}},	  //
	{0x08,  1,  {0x01}},	  //
	{0x09,  1,  {0x80}},	  //
	{0x0A,  1,  {0x00}},	  //
	{0x0B,  1,  {0x00}},	  //
	{0x0C,  1,  {0x01}},	  //
	{0x0D,  1,  {0x01}},	  //
	{0x0E,  1,  {0x00}},	  //
	{0x0F,  1,  {0x00}},	  //
	{0x10,  1,  {0xF0}},	  //
	{0x11,  1,  {0xF4}},	  //
	{0x12,  1,  {0x01}},	  //
	{0x13,  1,  {0x00}},	  //
	{0x14,  1,  {0x00}},	  //
	{0x15,  1,  {0xC0}},	  //
	{0x16,  1,  {0x08}},	  //
	{0x17,  1,  {0x00}},	  //
	{0x18,  1,  {0x00}},	  //
	{0x19,  1,  {0x00}},	  //
	{0x1A,  1,  {0x00}},	  //
	{0x1B,  1,  {0x00}},	  //
	{0x1C,  1,  {0x00}},	  //
	{0x1D,  1,  {0x00}},	  //
	{0x20,  1,  {0x01}},	  //
	{0x21,  1,  {0x23}},	  //
	{0x22,  1,  {0x45}},	  //
	{0x23,  1,  {0x67}},	  //
	{0x24,  1,  {0x01}},	  //
	{0x25,  1,  {0x23}},	  //
	{0x26,  1,  {0x45}},	  //
	{0x27,  1,  {0x67}},	  //
	{0x30,  1,  {0x11}},	  //
	{0x31,  1,  {0x11}},	  //
	{0x32,  1,  {0x00}},	  //
	{0x33,  1,  {0xEE}},	  //
	{0x34,  1,  {0xFF}},	  //  
	{0x35,  1,  {0xBB}},	  //  
	{0x36,  1,  {0xAA}},	  //
	{0x37,  1,  {0xDD}},	  //
	{0x38,  1,  {0xCC}},	  //
	{0x39,  1,  {0x66}},	  //	  
	{0x3A,  1,  {0x77}},	  //
	{0x3B,  1,  {0x22}},	  //	  
	{0x3C,  1,  {0x22}},	  //
	{0x3D,  1,  {0x22}},	  //  
	{0x3E,  1,  {0x22}},	  //
	{0x3F,  1,  {0x22}},	  //
	{0x40,  1,  {0x22}},	  //
	{0x52,  1,  {0x10}},	  //
	{0x53,  1,  {0x10}},	  //VGLO tie to VGL;

	{0xFF,  5,  {0xFF, 0x98, 0x06, 0x04, 0x07}},// Change to Page 7
	{0x18,  1,  {0x1D}},	  // VREG1 VREG2 output
	{0x17,  1,  {0x32}},	  // VGL_REG ON
	{0x02,  1,  {0x77}},	  //  

	{0xFF,  5,  {0xFF, 0x98, 0x06, 0x04, 0x00}},// Change to Page 0
	{0x11,  1,  {0x00}},	  // Sleep-Out
	{REGFLAG_DELAY, 150, {}},
	{0x29,  1,  {0x00}},	  // Display on

	//{0xFF,  5,  {0xFF, 0x98, 0x06, 0x04, 0x01}},// Change to Page 1

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#if 0
static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif


static struct LCM_setting_table lcm_sleep_out_setting[] = {

	//{0xFF, 5, {0xFF, 0x98, 0x06, 0x04, 0x00}},// Change to Page 0

	// Sleep Out
	{0x11, 1, {0x00}},
    	{REGFLAG_DELAY, 150, {}},

	// Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

	//{0x2C, 1, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
//	{0xFF, 5, {0xFF, 0x98, 0x06, 0x04, 0x00}},// Change to Page 0

	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

	// Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++) {

        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {

            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
		dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }

}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		//params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		//params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif

		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		params->dsi.packet_size=256;

		// Video mode setting	
		params->dsi.intermediat_buffer_num = 2;	
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.word_count=480*3;

		//here is for esd protect by legen
		params->dsi.noncont_clock = true;
		params->dsi.noncont_clock_period=2;
		params->dsi.lcm_ext_te_enable=true;
		//for esd protest end by legen

		params->dsi.vertical_sync_active=2; //4
		params->dsi.vertical_backporch=18;  //16
		params->dsi.vertical_frontporch=20;
		params->dsi.vertical_active_line=FRAME_HEIGHT;

		params->dsi.horizontal_sync_active=10;  //10
		params->dsi.horizontal_backporch=60;    //50  60 
		params->dsi.horizontal_frontporch=60;   //50   200
		  
		params->dsi.horizontal_active_pixel = FRAME_WIDTH;
		
		params->dsi.pll_div1=1;         //  div1=0,1,2,3;  div1_real=1,2,4,4
		params->dsi.pll_div2=1;         // div2=0,1,2,3;div2_real=1,2,4,4
        	params->dsi.fbk_div =30;        // fref=26MHz,  fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)   //32

}

//legen add for detect lcm vendor
static bool lcm_select_panel(void)
{
	int value=0;
	
	mt_set_gpio_mode(GPIO_LCM_ID,GPIO_MODE_00);

	mt_set_gpio_pull_enable(GPIO_LCM_ID, GPIO_PULL_DISABLE);

	mt_set_gpio_dir(GPIO_LCM_ID, GPIO_DIR_IN);

	value+=mt_get_gpio_in(GPIO_LCM_ID);
	if(value)
		return LCM_TDT;
	
	return LCM_TDT;
}
//legen add end 

static int first_init=0;
static void lcm_init(void)
{
    unsigned int data_array[16];
/*
#if defined(BUILD_LK)
  	lcm_ili9806e_vendor=lcm_select_panel();
#else
  	if(!first_init)
  	{
  		first_init=1;
		lcm_ili9806e_vendor=lcm_select_panel();
  	}
#endif

#ifdef BUILD_LK
	printf("[%s]lk,ili9806e,zrl choose lcm vendor:%d-%s\n",__func__,lcm_ili9806e_vendor,lcm_ili9806e_vendor?"BYD":"TDT");
#else
	printk("[%s]kernel,ili9806e,zrl choose lcm vendor:%d-%s\n",__func__,lcm_ili9806e_vendor,lcm_ili9806e_vendor?"BYD":"TDT");
#endif
*/
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);

    push_table(lcm_tdt_initialization_setting, sizeof(lcm_tdt_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


#if 0

static void lcm_resume(void)
{
	static int temp = 0x46;
	unsigned int data_array[16];

	//zrl add for debug for Cut  screen, 121015
	push_table(lcm_tdt_initialization_debug1, sizeof(lcm_tdt_initialization_debug1) / sizeof(struct LCM_setting_table), 1);

	data_array[0]= 0x00023902;
	data_array[1]= 0x00|0x00|(temp<<8)|0x53;
	dsi_set_cmdq(&data_array, 2, 1);

	push_table(lcm_tdt_initialization_debug2, sizeof(lcm_tdt_initialization_debug2) / sizeof(struct LCM_setting_table), 1);

#if defined(BUILD_LK)
	printf("zrl lcm_resume,temp = 0x%x,data_array[1] = 0x%x\n",temp,data_array[1]);
#else
	printk("zrl lcm_resume,temp = 0x%x,data_array[1] = 0x%x\n",temp,data_array[1]);
#endif

	//x1=temp%256;
	//x2=temp/256;
	temp+=2;

}

#else

static void lcm_resume(void)
{

	//lcm_init();  
	//MDELAY(200);

	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

#endif


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(data_array, 7, 0);

}


static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK

	unsigned char buffer[4];
	unsigned int array[16];

#if defined(LCM_DEBUG)
		printk("ili9806e: lcm_esd_check enter\n");
#endif

	array[0]=0x00063902;
	array[1]=0x0698ffff;
	array[2]=0x00000004;
	dsi_set_cmdq(array, 3, 1);
	MDELAY(10);

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x0A, buffer, 1);
	
#if defined(LCM_DEBUG)
	printk("lcm_esd_check  0x0A = %x\n",buffer[0]);
#endif

	if(buffer[0] != 0x9C)
	{
		return 1;
	}

	array[0] = 0x00043700;
	dsi_set_cmdq(array,1,1);
	read_reg_v2(0x0B, buffer[0], 1);

	array[0] = 0x00043700;
	dsi_set_cmdq(array,1,1);
	read_reg_v2(0x0C, buffer[1], 1);

	array[0] = 0x00043700;
	dsi_set_cmdq(array,1,1);
	read_reg_v2(0x0D, buffer[2], 1);

	array[0] = 0x00043700;
	dsi_set_cmdq(array,1,1);
	read_reg_v2(0x0E, buffer[3], 1);
	

#if defined(LCM_DEBUG)
	printk("lcm_esd_check  0x09(bit0~3) = %x, %x, %x, %x\n",buffer[0],buffer[1],buffer[2],buffer[3]);
#endif

	//if ((buffer[0]==0x80)&&(buffer[1]==0x73)&&(buffer[2]==0x04)&&(buffer[3]==0x00))
	if ((buffer[0]==0x80)&&(buffer[1]==0x73)&&(buffer[2]==0x04)&&((buffer[3]==0x00) || (buffer[3]==0x01)))
	{
	#if defined(LCM_DEBUG)
		printk("ili9806e: lcm_esd_check exit\n");
	#endif
		return 0;
	}
	else
	{
		return 1;
	}
	
#endif
}


static unsigned int lcm_esd_recover(void)
{
#ifndef BUILD_LK

   printk("ili9806e: lcm_esd_recover enter");
   lcm_init();
   return 1;

 #endif 
}


// ---------------------------------------------------------------------------
//  Get LCM ID Information
// ---------------------------------------------------------------------------
static unsigned int lcm_compare_id();
static unsigned int lcm_compare_id()
{
	unsigned int id1 = 0, id2 = 0, id3 = 0, id4 = 0,id;
	unsigned char buffer[5];

	unsigned int data_array[16];
	
	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(120);	

	// Enable CMD2 Page1
	#if 0
	data_array[0]=0x00013902;
	data_array[1]=0x00000000;
	dsi_set_cmdq(data_array, 2, 1);
	data_array[0]=0x00033902;
	data_array[1]=0x010596FF;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);
      #endif
	// read id return five byte : 0x01 0x8B 0x96 0x08 0xFF
	data_array[0] = 0x00053700;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);

	read_reg_v2(0xA1, buffer, 5);
	id1= buffer[0]; //should be 0x01
	id2= buffer[1]; //should be 0x8B
	id3= buffer[2]; //should be 0x96
	id4= buffer[3]; //should be 0x08
	id=(id3 << 8) | id4;
	#ifdef BUILD_LK
    printf("9605   id = %x\r\n",id);
#else 
    printk("9605   id = %x\r\n",id);
#endif
       //return 1;
	return ((0x9605==id)?1:0);
//	return (LCM_ID == (id3 << 8) | id4)?1:0;
} 

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER otm9605_xfy_lcm_drv =
{
    .name			= "otm9605_xfy_lcm_drv",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
#if (LCM_DSI_CMD_MODE)
	.update         = lcm_update,
#endif
	
//	.set_backlight	= lcm_setbacklight,
//	.set_pwm        = lcm_setpwm,
//	.get_pwm        = lcm_getpwm,
   
//	.esd_check   = lcm_esd_check,
//   	.esd_recover   = lcm_esd_recover,
	.compare_id    = lcm_compare_id,
};

