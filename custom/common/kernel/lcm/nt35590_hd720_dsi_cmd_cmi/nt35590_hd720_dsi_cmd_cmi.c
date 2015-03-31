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

#ifndef BUILD_LK
#include <linux/string.h>
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#define LCM_ID_NT35590 (0x90)
#define REGFLAG_DELAY             							0xAB
#define REGFLAG_END_OF_TABLE      							0xAA   // END OF REGISTERS MARKER

#define LCM_ID1												0x00
#define LCM_ID2												0x00
#define LCM_ID3												0x00

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
#define LCM_DSI_CMD_MODE									1

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


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

static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/

	{0xC2,	1,	{0x08}},
	{0xFF,	1,	{0x00}},
	{0xBA,	1,	{0x02}},		// 3lane
		
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}},
	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


	// Setting ending by predefined flag
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
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},

    // Sleep Mode On
	{0x10, 1, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#if 0
static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

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
		//unsigned int div2_real=0;
		//unsigned int cycle_time = 0;
		//unsigned int ui = 0;
		//unsigned int hs_trail_m, hs_trail_n;
		//#define NS_TO_CYCLE(n, c)	((n) / c + (( (n) % c) ? 1 : 0))

		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = BURST_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;	//LCM_THREE_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.word_count=720*3;	//DSI CMD mode need set these two bellow params, different to 6577
		params->dsi.vertical_active_line=1280;

		// Bit rate calculation, calculation forma be different to mt657x
#ifdef CONFIG_MT6589_FPGA
		params->dsi.pll_div1=2;		// div1=0,1,2,3;div1_real=1,2,4,4
		params->dsi.pll_div2=2;		// div2=0,1,2,3;div1_real=1,2,4,4
		params->dsi.fbk_div =8;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
#else
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4
		params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4
		params->dsi.fbk_div =15;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)		
#endif
		params->dsi.compatibility_for_nvk = 1;

#if 0
		div2_real=params->dsi.pll_div2 ? params->dsi.pll_div2*0x02 : 0x1;
		cycle_time = (8 * 1000 * div2_real)/ (26 * (params->dsi.pll_div1+0x01));
		ui = (1000 * div2_real)/ (26 * (params->dsi.pll_div1+0x01)) + 1;
		
		hs_trail_m=params->dsi.LANE_NUM;
		hs_trail_n=NS_TO_CYCLE(((params->dsi.LANE_NUM * 4 * ui) + 60), cycle_time);

		params->dsi.HS_TRAIL	= ((hs_trail_m > hs_trail_n) ? hs_trail_m : hs_trail_n);//min max(n*8*UI, 60ns+n*4UI)
		params->dsi.HS_ZERO 	= NS_TO_CYCLE((105 + 6 * ui), cycle_time);//min 105ns+6*UI
		params->dsi.HS_PRPR 	= NS_TO_CYCLE((40 + 4 * ui), cycle_time);//min 40ns+4*UI; max 85ns+6UI
		// HS_PRPR can't be 1.
		if (params->dsi.HS_PRPR < 2)
			params->dsi.HS_PRPR = 2;

		params->dsi.LPX 		= NS_TO_CYCLE(50, cycle_time);//min 50ns
		
		params->dsi.TA_SACK 	= 1;
		params->dsi.TA_GET		= 5 * NS_TO_CYCLE(50, cycle_time);//5*LPX
		params->dsi.TA_SURE 	= 3 * NS_TO_CYCLE(50, cycle_time) / 2;//min LPX; max 2*LPX;
		params->dsi.TA_GO		= 4 * NS_TO_CYCLE(50, cycle_time);//4*LPX
	
		params->dsi.CLK_TRAIL	= NS_TO_CYCLE(60, cycle_time);//min 60ns
		// CLK_TRAIL can't be 1.
		if (params->dsi.CLK_TRAIL < 2)
			params->dsi.CLK_TRAIL = 2;
		params->dsi.CLK_ZERO	= NS_TO_CYCLE((300-38), cycle_time);//min 300ns-38ns
		params->dsi.LPX_WAIT	= 1;
		params->dsi.CONT_DET	= 0;
		
		params->dsi.CLK_HS_PRPR = NS_TO_CYCLE((38 + 95) / 2, cycle_time);//min 38ns; max 95ns

#endif		

}


#if 0
static void lcm_init(void)
{
	//int i;
	//unsigned char buffer[10];
	//unsigned int  array[16];
	unsigned int data_array[16];

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(20);

	data_array[0]=0x00023902;
	data_array[1]=0x000008C2;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00110500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x000002BA;//MIPI lane
	dsi_set_cmdq(data_array, 2, 1);

	//{0x44,	2,	{((FRAME_HEIGHT/2)>>8), ((FRAME_HEIGHT/2)&0xFF)}},
	data_array[0] = 0x00033902;
	data_array[1] = (((FRAME_HEIGHT/2)&0xFF) << 16) | (((FRAME_HEIGHT/2)>>8) << 8) | 0x44;
	dsi_set_cmdq(data_array, 2, 1);
	
	data_array[0] = 0x00351500;// TE ON
	dsi_set_cmdq(data_array, 1, 1);
	//MDELAY(10);

	data_array[0]=0x00290500;
	dsi_set_cmdq(data_array, 1, 1);
	
//	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}
#else
static void lcm_init_reg(void)
{
	unsigned int data_array[16];

#if 1//yjc  四方，35590+AUO5.0.txt
     //海天客户，安卓睿模组，InitialCode NT35590+AUO5.0_MIPI.MCR
	data_array[0] = 0x00023902;//CMD2,Page0  
	data_array[1] = 0x000001FF; 				
	dsi_set_cmdq(&data_array, 2, 1);   

	data_array[0] = 0x00023902;//CMD2,Page0  
	data_array[1] = 0x000001FB; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00004A00; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x00003301; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x00005302; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x00005503; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00005504; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00003305; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x00002206; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00005608; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00008F09; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000CF0B; //	0x00009F0B			
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000CF0C; // 0x00009F0C				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00002F0D; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000290E; //0x0000240E				
	dsi_set_cmdq(&data_array, 2, 1);  
	
	data_array[0] = 0x00023902; 
	data_array[1] = 0x00007336; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000A0F; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000EEFF; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x000001FB; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00005012; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000213; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000606A; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x000005FF; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x000001FB; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000001; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00008202; 				
	dsi_set_cmdq(&data_array, 2, 1);  
	
	data_array[0] = 0x00023902;
	data_array[1] = 0x00008203; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00008204; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00003306; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000107; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000008; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00004609; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000460A; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000B0D; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00001D0E; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000080F; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00005310; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000011; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000012; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000114; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000015; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000516; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000017; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00007F19; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000FF1A; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000F1B; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000001C; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000001D; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000001E; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000071F; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000020; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000021; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00005522; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00004D23; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000022D; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000183; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000589E; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000689F; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x000001A0; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x000010A2; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000ABB;				 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000ABC; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000128; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000022F; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000832; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000B833; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000136; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000037; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000043; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000214B; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000034C; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00002150; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000351; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00002158; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000359; 				
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000215D; 				
	dsi_set_cmdq(&data_array, 2, 1);   

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000035E; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000006C; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000006D; 				
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x000000FF; 			
	dsi_set_cmdq(&data_array, 2, 1);	 

	data_array[0] = 0x00023902;
	data_array[1] = 0x000001FB; 				
	dsi_set_cmdq(&data_array, 2, 1);	  

	data_array[0] = 0x00023902; //MIPI 4 Lane
	data_array[1] = 0x000003BA; //0xba -> 0x01 2 Lane, 0x03 4 Lane				
	dsi_set_cmdq(&data_array, 2, 1);	 

	data_array[0] = 0x00023902;
	data_array[1] = 0x000008C2; //0bc2:video;08C2:cmd mode		
	dsi_set_cmdq(&data_array, 2, 1); 


	data_array[0] = 0x00023902;
	data_array[1] = 0x000003FF; 				
	dsi_set_cmdq(&data_array, 2, 1);	 

	data_array[0] = 0x00023902;
	data_array[1] = 0x000008FE; 				
	dsi_set_cmdq(&data_array, 2, 1);   

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000018; 				
	dsi_set_cmdq(&data_array, 2, 1);
	
	data_array[0] = 0x00023902;
	data_array[1] = 0x00000019; 				
	dsi_set_cmdq(&data_array, 2, 1);
	
	data_array[0] = 0x00023902;
	data_array[1] = 0x0000001A; 				
	dsi_set_cmdq(&data_array, 2, 1);
	
	data_array[0] = 0x00023902;
	data_array[1] = 0x00002625; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000000; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000401; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000802; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000C03; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00001004; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00001405; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00001806; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00001C07; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00002008; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00002409; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000280A; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00002C0B; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000300C; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000340D; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000380E; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00003C0F; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x000001FB; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x000000FF; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x000001FE; 				
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00110500; 			   
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(200);   

	data_array[0] = 0x00290500; 			   
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(50); 
	
	data_array[0] = 0x00023902; 	
	data_array[1] = 0x00000035; 		
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);	
	
	data_array[0] = 0x00023902;
	data_array[1] = 0x0000FF51; 				
	dsi_set_cmdq(&data_array, 2, 1);

    data_array[0] = 0x00023902;
    data_array[1] = 0x00002C53; 				
    dsi_set_cmdq(&data_array, 2, 1);

    data_array[0] = 0x00023902;
    data_array[1] = 0x00008055; 				
    dsi_set_cmdq(&data_array, 2, 1);

// MIRROR
// add by yangzhiqiang
	//data_array[0] = 0x00023902;
	//data_array[1] = 0x000005FF; 				
	//dsi_set_cmdq(&data_array, 2, 1); 

	//data_array[0] = 0x00023902;
	//data_array[1] = 0x00000321; 				
	//dsi_set_cmdq(&data_array, 2, 1); 
// yangzhiqiang
#else
	data_array[0] = 0x00023902;//CMD1                           
	data_array[1] = 0x000000FF;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;//MIPI command mode  
	data_array[1] = 0x000003BA;                 
	dsi_set_cmdq(&data_array, 2, 1);          

	data_array[0] = 0x00023902;//MIPI command mode  
	data_array[1] = 0x000008C2;                 
	dsi_set_cmdq(&data_array, 2, 1);   

	data_array[0] = 0x00023902;//CMD2,Page0  
	data_array[1] = 0x000001FF;                 
	dsi_set_cmdq(&data_array, 2, 1);   

	data_array[0] = 0x00023902;//CMD2,Page0  
	data_array[1] = 0x000001FB;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00004A00;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x00003301;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x00005302;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x00005503;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00005504;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00003305;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x00002206;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00005608;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00008F09;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00007336;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00009F0B;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00009F0C;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00002F0D;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000240E;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00008311;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000312;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00002C71;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000036F;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000A0F;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x000005FF;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x000001FB;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000001;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00008202;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00008203;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00008204;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00003005;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00003306;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000107;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000008;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00004609;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000460A;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000B0D;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00001D0E;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000080F;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00005310;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000011;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000012;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000114;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000015;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000516;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000017;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00007F19;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000FF1A;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000F1B;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000001C;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000001D;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000001E;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000071F;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000020;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000021;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00005522;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00004D23;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000022D;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000183;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000589E;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00006A9F;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x000001A0;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x000010A2;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x000000ABB;                 
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000ABC;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000128;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000022F;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000832;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000B833;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000136;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000037;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000043;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000214B;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000034C;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00002150;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000351;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00002158;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000359;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000215D;                 
	dsi_set_cmdq(&data_array, 2, 1);   

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000035E;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000006C;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000006D;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x000001FF;                 
	dsi_set_cmdq(&data_array, 2, 1);     

	data_array[0] = 0x00023902;
	data_array[1] = 0x000001FB;                 
	dsi_set_cmdq(&data_array, 2, 1);      

	data_array[0] = 0x00023902;
	data_array[1] = 0x000002FF;                 
	dsi_set_cmdq(&data_array, 2, 1);     

	data_array[0] = 0x00023902;
	data_array[1] = 0x000001FB;                 
	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00023902;
	data_array[1] = 0x000004FF;                 
	dsi_set_cmdq(&data_array, 2, 1);     

	data_array[0] = 0x00023902;
	data_array[1] = 0x000001FB;                 
	dsi_set_cmdq(&data_array, 2, 1);   

	data_array[0] = 0x00023902;
	data_array[1] = 0x000000FF;                 
	dsi_set_cmdq(&data_array, 2, 1);                                                                  

	data_array[0] = 0x00110500;                
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(200); 

	data_array[0] = 0x00023902;//MIPI 2 Lane                  
	data_array[1] = 0x000003BA;    //0xba -> 0x01 2 line, 0x03 4 line             
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;//MIPI command mode  
	data_array[1] = 0x000008C2;                 
	dsi_set_cmdq(&data_array, 2, 1);   

// add by yangzhiqiang
	//data_array[0] = 0x00023902;
	//data_array[1] = 0x0000C136;                 
	//data_array[1] = 0x0000d436;                 
	//dsi_set_cmdq(&data_array, 2, 1); 
// yangzhiqiang

	data_array[0] = 0x00290500;                
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(200); 
	data_array[0] = 0x00023902;     
	data_array[1] = 0x00000035;         
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(200);      

#if !defined(PROJECT_W8901_HZT)
// MIRROR
// add by yangzhiqiang
	data_array[0] = 0x00023902;
	data_array[1] = 0x000005FF;                 
	dsi_set_cmdq(&data_array, 2, 1); 

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000321;                 
	dsi_set_cmdq(&data_array, 2, 1); 
// yangzhiqiang
#endif
#endif
}

static void lcm_init(void)
{
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(120);	

//	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	lcm_init_reg();
}
#endif


static void lcm_suspend(void)
{
	//push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(20);	//MDELAY(150);
}


static void lcm_resume(void)
{
	//lcm_init();
	lcm_init_reg();
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


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
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}

#if 0
static void lcm_setbacklight(unsigned int level)
{
	unsigned int default_level = 145;
	unsigned int mapped_level = 0;

	//for LGE backlight IC mapping table
	if(level > 255) 
			level = 255;

	if(level >0) 
			mapped_level = default_level+(level)*(255-default_level)/(255);
	else
			mapped_level=0;

	// Refresh value of backlight level.
	lcm_backlight_level_setting[0].para_list[0] = mapped_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_UBOOT
        if(lcm_esd_test)
        {
            lcm_esd_test = FALSE;
            return TRUE;
        }

        /// please notice: the max return packet size is 1
        /// if you want to change it, you can refer to the following marked code
        /// but read_reg currently only support read no more than 4 bytes....
        /// if you need to read more, please let BinHan knows.
        /*
                unsigned int data_array[16];
                unsigned int max_return_size = 1;
                
                data_array[0]= 0x00003700 | (max_return_size << 16);    
                
                dsi_set_cmdq(&data_array, 1, 1);
        */

        if(read_reg(0xB6) == 0x42)
        {
            return FALSE;
        }
        else
        {            
            return TRUE;
        }
#endif
}

static unsigned int lcm_esd_recover(void)
{
    unsigned char para = 0;

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(120);
	  push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    MDELAY(10);
	  push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
    MDELAY(10);
    dsi_set_cmdq_V2(0x35, 1, &para, 1);     ///enable TE
    MDELAY(10);

    return TRUE;
}
/*
static unsigned int lcm_compare_id(void)
{
	unsigned int id1, id2, id3;
	unsigned char buffer[2];
	unsigned int array[16];

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(10);

	// Set Maximum return byte = 1
	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	id1 = read_reg(0xDA);
	id2 = read_reg(0xDB);
	id2 = read_reg(0xDC);

#if defined(BUILD_UBOOT)
	printf("%s, Module ID = {%x, %x, %x} \n", __func__, id1, id2, id3);
#endif

    return (LCM_ID1 == id1 && LCM_ID2 == id2)?1:0;
}
*/
#endif

extern void DSI_clk_HS_mode(bool enter);

//35590读ID,需加{0xFF,1，{ 0x00}},{0xFB,1，{ 0x01}},直接读0XF4，读出来的是90
//读3B也可以,读3B的话就要改成{0xFF,1，{ 0x01}},{0xFB,1，{ 0x01}}
static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];  

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(20); 

	//DSI_clk_HS_mode(0);
	//MDELAY(5); 
	
	//array[0] = 0x00000508;// HS Mode Write 0x00
	//dsi_set_cmdq(array, 1, 1);
	
	//array[0] = 0x00023902;
	//array[1] = 0x000002FF;                 
	//dsi_set_cmdq(&array, 2, 1);   
	
	//array[0] = 0x00023902;
	//array[1] = 0x000001FB;                 
	//dsi_set_cmdq(&array, 2, 1);   

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; //we only need ID
	
    #ifdef BUILD_LK
		printf("nt35590_hd720_dsi_cmd_auo id = 0x%x, buffer[1] = 0x%x\n",buffer[0], buffer[1]);
    #else
	    printk("nt35590_hd720_dsi_cmd_auo id = 0x%x, buffer[1] = 0x%x\n",buffer[0], buffer[1]);
    #endif

    if(id == 0x90)
    	return 1;
    else
        return 0;


}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER nt35590_hd720_dsi_cmd_cmi_lcm_drv = 
{
    .name			= "nt35590_hd720_dsi_cmd_cmi_lcm_drv",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.update         = lcm_update,
	//.set_backlight	= lcm_setbacklight,
//	.set_pwm        = lcm_setpwm,
//	.get_pwm        = lcm_getpwm,
	//.esd_check   = lcm_esd_check,
    //.esd_recover   = lcm_esd_recover,
#endif
};
