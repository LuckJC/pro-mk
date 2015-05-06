#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <platform/upmu_common.h>
	//#include <platform/hardware_info.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
	//#include <platform/hardware_info.h>
#else
	#include <mach/mt_gpio.h>
	#include <mach/mt_pm_ldo.h>
	#include <mach/upmu_common.h>
	//#include <mach/hardware_info.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)

#define REGFLAG_DELAY             								0XFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER

//#define LCM_ID_HX8394D        0x94 
#define LCM_ID_HX8394D        0

#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define   TRUE     1
#endif

#ifndef FALSE
    #define   FALSE    0
#endif

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

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

 struct LCM_setting_table {
    unsigned cmd;
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


      {0xB9,3, {0xFF,0x83,0x94}},

      {0xBA,2, {0x33,0x83}}, 

      {0xB1,15,{0x6C,0x10,0x10,0x12,0x04,0x15,0xF5,0x81,0x77,0x54,0x23,0x80,0xC0,0xD2,0x58}},

      {0xB2,11,{0x00,0x64,0x0E,0x0D,0x32,0x1C,0x08,0x08,0x1C,0x4D,0x00}},

      {0xB4,12,{0x00,0xFF,0x51,0x5A,0x59,0x5A,0x03,0x5A,0x01,0x70,0x01,0x70}},

      {0xB6,3, {0x68,0x68,0xE0}},
     
      {0xD3,37,{0x00,0x07,0x00,0x40,0x07,0x10,0x00,0x08,0x10,0x08,
                  	0x00,0x08,0x54,0x15,0x0E,0x05,0x0E,0x02,0x15,0x06,
                	0x05,0x06,0x47,0x44,0x0A,0x0A,0x4B,0x10,0x07,0x07, 
                	0x08,0x00,0x00,0x00,0x0A,0x00,0x01}},

      {0xD5,44,{0x1A,0x1A,0x1B,0x1B,0x00,0x01,0x02,0x03,0x04,0x05,0x06,
                	 0x07,0x08,0x09,0x0A,0x0B,0x24,0x25,0x18,0x18,0x26,0x27,
                	 0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
               	 0x18,0x18,0x18,0x18,0x18,0x20,0x21,0x18,0x18,0x18,0x18}},

      {0xD6,44,{0x1A,0x1A,0x1B,0x1B,0x0B,0x0A,0x09,0x08,0x07,0x06,0x05,
                	 0x04,0x03,0x02,0x01,0x00,0x21,0x20,0x58,0x58,0x27,0x26,
                	 0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
                	 0x18,0x18,0x18,0x18,0x18,0x25,0x24,0x18,0x18,0x18,0x18}},

     {0xC7,4, {0x00,0xC0,0x00,0xC7}},

     {0xCC,1, {0x01}},
   #if 0
     {0xE0,42,{0x02,0x15,0x17,0x27,0x3A,0x3F,0x21,0x3B,0x08,0x0B,
              	0x0B,0x19,0x0F,0x13,0x14,0x0D,0x15,0x04,0x0C,0x0D,0x0A,
               
               	0x02,0x15,0x17,0x27,0x3A,0x3F,0x21,0x3B,0x08,0x0B,
               	0x0B,0x19,0x0F,0x13,0x14,0x1D,0x15,0x14,0x1C,0x1D,0x1A}},
   #else
     {0xE0,42,{0x00,0x10,0x10,0x24,0x38,0x38,0x1C,0x38,0x08,0x0B,     
               0x0B,0x19,0x0F,0x13,0x14,0x0D,0x15,0x04,0x0C,0x0D,0x0A,
                                                                      
               0x00,0x10,0x10,0x24,0x38,0x38,0x1C,0x37,0x08,0x0B,     
               0x0B,0x19,0x0F,0x13,0x14,0x1D,0x15,0x14,0x1C,0x1D,0x1A}},
   #endif
   
     
     {0x11,1, {0x00}},
     {REGFLAG_DELAY,120,{}},
     
     {0x29,1, {0x00}},
     {REGFLAG_DELAY,10,{}},
     
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

		params->width  = FRAME_WIDTH;//480;
		params->height = FRAME_HEIGHT;//850;

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
		//params->dbi.te_edge_polarity		= LCM_POLARITY_FALLING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif

		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting
		params->dsi.intermediat_buffer_num = 2;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 3;
		params->dsi.vertical_backporch				= 13;
		params->dsi.vertical_frontporch				= 13;
		params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active				= 32;
		params->dsi.horizontal_backporch				= 80;
		params->dsi.horizontal_frontporch				= 80;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

#if 0
		// Bit rate calculation
		params->dsi.pll_div1=29;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
#else
				// Bit rate calculation
		//1 Every lane speed
		//params->dsi.pll_select=1;
		//params->dsi.PLL_CLOCK  = LCM_DSI_6589_PLL_CLOCK_377;
		
		//params->dsi.PLL_CLOCK=228;
		params->dsi.pll_div1=1;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4
#if (LCM_DSI_CMD_MODE)
		params->dsi.fbk_div =9;
#else
		params->dsi.fbk_div = 16;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
#endif
#endif

		/* ESD or noise interference recovery For video mode LCM only. */ // Send TE packet to LCM in a period of n frames and check the response.
		params->dsi.lcm_int_te_monitor = FALSE;
		params->dsi.lcm_int_te_period = 1; // Unit : frames

		// Need longer FP for more opportunity to do int. TE monitor applicably.
		if(params->dsi.lcm_int_te_monitor)
			params->dsi.vertical_frontporch *= 2;

		// Monitor external TE (or named VSYNC) from LCM once per 2 sec. (LCM VSYNC must be wired to baseband TE pin.)
		params->dsi.lcm_ext_te_monitor = FALSE;
		// Non-continuous clock
		params->dsi.noncont_clock = TRUE;
		params->dsi.noncont_clock_period = 2; // Unit : frames
}

static void lcm_gpio_set(int num, int enable)
{
#ifdef BUILD_LK
printf("\n\n\n===================== %s %d: num:%d => %d =======================\n\n\n", __func__, __LINE__, num, enable);
#endif
	lcm_util.set_gpio_mode(num, GPIO_MODE_00);
	lcm_util.set_gpio_dir(num, GPIO_DIR_OUT);

	if (enable)
		lcm_util.set_gpio_out(num, GPIO_OUT_ONE);
	else
		lcm_util.set_gpio_out(num, GPIO_OUT_ZERO);
}

static void init_lcd_set()
{
       int data_array[20];
	   #if 0
	data_array[0]=0x00043902;//Enable external Command 
	data_array[1]=0x9483FFB9; 
	dsi_set_cmdq(&data_array, 2, 1); 
	MDELAY(1);

	data_array[0]=0x00033902;//Enable external Command 
	data_array[1]=0x008333BA; 
	dsi_set_cmdq(&data_array, 2, 1); 
	MDELAY(1);

	data_array[0]=0x00103902;//Enable external Command 
	data_array[1]=0x10106CB1; 
	data_array[2]=0xF5150412;
	data_array[3]=0x23547781;
	data_array[4]=0x58D2C080;
	dsi_set_cmdq(&data_array, 5, 1); 
	MDELAY(1);


	data_array[0]=0x000C3902;//Enable external Command 
	data_array[1]=0x0E6400B2; 
	data_array[2]=0x081C320D;
	data_array[3]=0x004D1C08;
	dsi_set_cmdq(&data_array, 4, 1); 
	MDELAY(1);

	data_array[0]=0x000D3902;//Enable external Command 
	data_array[1]=0x51FF00B4; 
	data_array[2]=0x035A595A;
	data_array[3]=0x0170015A;
	data_array[4]=0x00000070;
	dsi_set_cmdq(&data_array, 5, 1); 
	MDELAY(1);

	data_array[0]=0x00043902;//Enable external Command 
	data_array[1]=0xE06868B6; 
	dsi_set_cmdq(&data_array, 2, 1); 
	MDELAY(1);

	data_array[0]=0x00263902;//Enable external Command 
	data_array[1]=0x000700D3; 
	data_array[2]=0x00100740; 
	data_array[3]=0x00081008; 
	data_array[4]=0x0E155408; 
	data_array[5]=0x15020E05; 
	data_array[6]=0x47060506; 
	data_array[7]=0x4B0A0A44; 
	data_array[8]=0x08070710; 
	data_array[9]=0x0A000000; 
	data_array[10]=0x00000100;
	dsi_set_cmdq(&data_array, 11, 1); 
	MDELAY(1);

	data_array[0]=0x002D3902;//Enable external Command 
	data_array[1]=0x1B1A1AD5;  
	data_array[2]=0x0201001B;  
	data_array[3]=0x06050403;  
	data_array[4]=0x0A090807;  
	data_array[5]=0x1825240B;  
	data_array[6]=0x18272618;  
	data_array[7]=0x18181818;  
	data_array[8]=0x18181818;  
	data_array[9]=0x18181818;  
	data_array[10]=0x20181818; 
	data_array[11]=0x18181821; 
	data_array[12]=0x00000018; 
	dsi_set_cmdq(&data_array, 13, 1); 
	MDELAY(1);

	data_array[0]=0x002D3902;//Enable external Command 
	data_array[1]=0x1B1A1AD6; 
	data_array[2]=0x090A0B1B; 
	data_array[3]=0x05060708; 
	data_array[4]=0x01020304; 
	data_array[5]=0x58202100; 
	data_array[6]=0x18262758; 
	data_array[7]=0x18181818; 
	data_array[8]=0x18181818; 
	data_array[9]=0x18181818; 
	data_array[10]=0x25181818;
	data_array[11]=0x18181824;
	data_array[12]=0x00000018;
	dsi_set_cmdq(&data_array, 13, 1); 
	MDELAY(1);

	data_array[0]=0x00053902;//Enable external Command 
	data_array[1]=0x000C00C7; 
	data_array[1]=0x000000C7; 
	dsi_set_cmdq(&data_array, 3, 1); 
	MDELAY(1);
	// {0xCC,1, {0x01}},
	data_array[0]= 0x01CC1500; 
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(1);

	data_array[0]=0x002B3902;//Enable external Command 
	data_array[1]=0x101000E0;  
	data_array[2]=0x1C383824;  
	data_array[3]=0x0B0B0838;  
	data_array[4]=0x14130F19;  
	data_array[5]=0x0C04150D;  
	data_array[6]=0x10000A0D;  
	data_array[7]=0x38382410;  
	data_array[8]=0x0B08371C;  
	data_array[9]=0x130F190B;  
	data_array[10]=0x14151D14;  
	data_array[11]=0x001A1D1C;  
	dsi_set_cmdq(&data_array, 12, 1); 
	MDELAY(1);

	// {0x11,1, {0x00}}, 
	data_array[0]= 0x00111500; 
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(12);

	// {0x29,1, {0x00}},
	data_array[0]= 0x00291500; 
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(10);
#endif

#if 0
	data_array[0]=0x00043902;//Enable external Command 
	data_array[1]=0x9483FFB9; 
	dsi_set_cmdq(&data_array, 2, 1); 
	MDELAY(1);

	data_array[0]=0x00033902;//Enable external Command 
	data_array[1]=0x008333BA; 
	dsi_set_cmdq(&data_array, 2, 1); 
	MDELAY(1);

	data_array[0]=0x00103902;//Enable external Command 
	data_array[1]=0x10106CB1; 
	data_array[2]=0xF4140412;
	data_array[3]=0x234C3381;
	data_array[4]=0x08D2C080;
	dsi_set_cmdq(&data_array, 5, 1); 
	MDELAY(1);


	data_array[0]=0x000C3902;//Enable external Command 
	data_array[1]=0x0E6400B2; 
	data_array[2]=0x081C320D;
	data_array[3]=0x004D1C08;
	dsi_set_cmdq(&data_array, 4, 1); 
	MDELAY(1);

	data_array[0]=0x000D3902;//Enable external Command 
	data_array[1]=0x51FF00B4; 
	data_array[2]=0x035A595A;
	data_array[3]=0x0170015A;
	data_array[4]=0x00000070;
	dsi_set_cmdq(&data_array, 5, 1); 
	MDELAY(1);

	data_array[0]=0x00043902;//Enable external Command 
	data_array[1]=0xE06868B6; 
	dsi_set_cmdq(&data_array, 2, 1); 
	MDELAY(1);

	data_array[0]=0x00263902;//Enable external Command 
	data_array[1]=0x000700D3; 
	data_array[2]=0x00100740; 
	data_array[3]=0x00081008; 
	data_array[4]=0x0E155408; 
	data_array[5]=0x15020E05; 
	data_array[6]=0x47060506; 
	data_array[7]=0x4B0A0A44; 
	data_array[8]=0x08070710; 
	data_array[9]=0x0A000000; 
	data_array[10]=0x00000100;
	dsi_set_cmdq(&data_array, 11, 1); 
	MDELAY(1);

	data_array[0]=0x002D3902;//Enable external Command 
	data_array[1]=0x1B1A1AD5;  
	data_array[2]=0x0201001B;  
	data_array[3]=0x06050403;  
	data_array[4]=0x0A090807;  
	data_array[5]=0x1825240B;  
	data_array[6]=0x18272618;  
	data_array[7]=0x18181818;  
	data_array[8]=0x18181818;  
	data_array[9]=0x18181818;  
	data_array[10]=0x20181818; 
	data_array[11]=0x18181821; 
	data_array[12]=0x00000018; 
	dsi_set_cmdq(&data_array, 13, 1); 
	MDELAY(1);

	data_array[0]=0x002D3902;//Enable external Command 
	data_array[1]=0x1B1A1AD6; 
	data_array[2]=0x090A0B1B; 
	data_array[3]=0x05060708; 
	data_array[4]=0x01020304; 
	data_array[5]=0x58202100; 
	data_array[6]=0x18262758; 
	data_array[7]=0x18181818; 
	data_array[8]=0x18181818; 
	data_array[9]=0x18181818; 
	data_array[10]=0x25181818;
	data_array[11]=0x18181824;
	data_array[12]=0x00000018;
	dsi_set_cmdq(&data_array, 13, 1); 
	MDELAY(1);

	data_array[0]=0x00053902;//Enable external Command 
	data_array[1]=0x00C000C7; 
	data_array[2]=0x000000C7; 
	dsi_set_cmdq(&data_array, 3, 1); 
	MDELAY(1);
	// {0xCC,1, {0x01}},
	data_array[0]= 0x09CC1500; 
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(1);

		// {0xBC,1, {0x07}},
	data_array[0]= 0x07BC1500; 
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(1);

	#if 0
	data_array[0]=0x002B3902;//Enable external Command 
	data_array[1]=0x101000E0;  
	data_array[2]=0x1C383824;  
	data_array[3]=0x0B0B0838;  
	data_array[4]=0x14130F19;  
	data_array[5]=0x0C04150D;  
	data_array[6]=0x10000A0D;  
	data_array[7]=0x38382410;  
	data_array[8]=0x0B08371C;  
	data_array[9]=0x130F190B;  
	data_array[10]=0x14151D14;  
	data_array[11]=0x001A1D1C;  
	dsi_set_cmdq(&data_array, 12, 1); 
	MDELAY(1);
	#else
	data_array[0] = 0x002B3902;	
	data_array[1] = 0x171502E0;	
	data_array[2] = 0x213F3A27;	
	data_array[3] = 0x0B0B083B;	
	data_array[4] = 0x14130F19;
	data_array[5] = 0x0C04150D;	
	data_array[6] = 0x15020A0D;	
	data_array[7] = 0x3F3A2717;	
	data_array[8] = 0x0B083B21;	
	data_array[9] = 0x130F190B;	
	data_array[10] = 0x14151D14;
	data_array[11] = 0x001A1D1C;	
	dsi_set_cmdq(&data_array, 12, 1);	
	MDELAY(2);
	#endif

	// {0x11,1, {0x00}}, 
	data_array[0]= 0x00110500; 
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(120);

	// {0x29,1, {0x00}},
	data_array[0]= 0x00290500; 
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(5);

//	{REGFLAG_END_OF_TABLE, 0x00, {}}
	//data_array[0]= 0x00FE0500; 
	//dsi_set_cmdq(&data_array, 1, 1); 
	//MDELAY(5);
#endif

	//unsigned int data_array[16];
	data_array[0]=0x00043902;
	data_array[1]=0x9483ffb9;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10); 
	data_array[0]=0x00033902;
	data_array[1]=0x008373ba;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10); 
	data_array[0]=0x00053902;
	data_array[1]=0x7d0000b0;
	data_array[2]=0x0000000c;
	dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(10); 
	data_array[0]=0x00103902;
	data_array[1]=0x15156cb1;
	data_array[2]=0xf1110424;
	data_array[3]=0x2397e480;
	data_array[4]=0x58d2c080;
	dsi_set_cmdq(&data_array, 5, 1);
	MDELAY(10); 
	data_array[0]=0x000c3902;
	data_array[1]=0x106400b2;
	data_array[2]=0x081c2207;
	data_array[3]=0x004d1c08;
	dsi_set_cmdq(&data_array, 4, 1);
	MDELAY(10); 
	data_array[0]=0x000d3902;
	data_array[1]=0x03ff00b4;
	data_array[2]=0x035a035a;
	data_array[3]=0x306a015a;
	data_array[4]=0x0000006a;

	dsi_set_cmdq(&data_array, 5, 1);
	MDELAY(10); 
	data_array[0]=0x00023902;
	data_array[1]=0x000007bc;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);
	data_array[0]=0x00043902;
	data_array[1]=0x010e41bf;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10); 
	data_array[0]=0x00033902;
	data_array[1]=0x004c4cb6;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10); 
	data_array[0]=0x001f3902;
	data_array[1]=0x000600d3;
	data_array[2]=0x00080740;
	data_array[3]=0x00071032;
	data_array[4]=0x0f155407;
	data_array[5]=0x12020405;
	data_array[6]=0x33070510;
	data_array[7]=0x370b0b33;
	data_array[8]=0x00070710;

	dsi_set_cmdq(&data_array, 9, 1);
	MDELAY(10);
	data_array[0]=0x002d3902;
	data_array[1]=0x060504d5;
	data_array[2]=0x02010007;
	data_array[3]=0x22212003;
	data_array[4]=0x18181823;
	data_array[5]=0x18181818;
	data_array[6]=0x18191918;
	data_array[7]=0x1b181818;
	data_array[8]=0x181a1a1b;
	data_array[9]=0x18181818;
	data_array[10]=0x18181818;
	data_array[11]=0x18181818;
	data_array[12]=0x00000018;
	dsi_set_cmdq(&data_array, 13, 1);
	MDELAY(10);
	data_array[0]=0x002d3902;
	data_array[1]=0x010203d6;
	data_array[2]=0x05060700;
	data_array[3]=0x21222304;
	data_array[4]=0x18181820;
	data_array[5]=0x58181818;
	data_array[6]=0x19181858;
	data_array[7]=0x1b181819;
	data_array[8]=0x181a1a1b;
	data_array[9]=0x18181818;
	data_array[10]=0x18181818;
	data_array[11]=0x18181818;
	data_array[12]=0x00000018;
	dsi_set_cmdq(&data_array, 13, 1);
	MDELAY(10);
	data_array[0]=0x00023902;
	data_array[1]=0x000009cc;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10); 
	data_array[0]=0x00033902;
	data_array[1]=0x001430c0;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10); 
	data_array[0]=0x00053902;
	data_array[1]=0x40c000c7;
	data_array[2]=0x000000c0;
	dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(10); 

	data_array[0]=0x002b3902;
	data_array[1]=0x161000e0;
	data_array[2]=0x233f332d;
	data_array[3]=0x0d0b073e;
	data_array[4]=0x14120e17;
	data_array[5]=0x11061312;
	data_array[6]=0x0f001813;
	data_array[7]=0x3f332e16;
	data_array[8]=0x0b073d23;
	data_array[9]=0x120f180d;
	data_array[10]=0x07141214;
	data_array[11]=0x00171211;
	dsi_set_cmdq(&data_array, 12, 1);
	MDELAY(10); 

	data_array[0]=0x00023902;
	data_array[1]=0x00008edf;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10); 
	data_array[0]=0x00023902;
	data_array[1]=0x000066d2;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0] = 0x00110500;           
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(200);
	data_array[0] = 0x00290500;  
	dsi_set_cmdq(&data_array, 1, 1);            
	MDELAY(10); 

}
static void lcm_init(void)
{
#if 0
    SET_RESET_PIN(1);
    MDELAY(1);
    SET_RESET_PIN(0);
    MDELAY(5);
    SET_RESET_PIN(1);
    MDELAY(150);
#else
	mt_set_gpio_out(GPIO119,1);
	MDELAY(20); 
	//SET_RESET_PIN(0);
	mt_set_gpio_out(GPIO119,0);
	MDELAY(20); 
	//SET_RESET_PIN(1);
	mt_set_gpio_out(GPIO119,1);
	MDELAY(100); 
#endif

  // push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

    init_lcd_set();

}

static void lcm_suspend(void)
{
#if 0
	SET_RESET_PIN(1);
	MDELAY(1);
	SET_RESET_PIN(0);
	MDELAY(5);
	SET_RESET_PIN(1);
	MDELAY(150);
#else
	mt_set_gpio_out(GPIO119,1);
	MDELAY(20); 
	//SET_RESET_PIN(0);
	mt_set_gpio_out(GPIO119,0);
	MDELAY(20); 
	//SET_RESET_PIN(1);
	mt_set_gpio_out(GPIO119,1);
	MDELAY(100); 
#endif
}


static void lcm_resume(void)
{
	lcm_init();
}



static unsigned int lcm_compare_id(void)
{
#if 0
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
	MDELAY(10);//Must over 6 ms

	array[0]=0x00043902;
	array[1]=0x9483FFB9;// page enable
	dsi_set_cmdq(&array, 2, 1);
	MDELAY(10);

	array[0] = 0x00023700;// return byte number
	dsi_set_cmdq(&array, 1, 1);
	MDELAY(10);

	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0];

#ifdef BUILD_LK
	printf("HX8394D_dsi_truly %s, id = 0x%08x\n", __func__, id);
#else
	printk("HX8394D_dsi_truly %s, id = 0x%08x\n", __func__, id);
#endif

	return (LCM_ID_HX8394D == id)?1:0; 
#else
	unsigned int ret = 0;

	ret = mt_get_gpio_in(GPIO17);
#if defined(BUILD_LK)
	printf("%s, [jx]hx8394a GPIO92 = %d \n", __func__, ret);
#endif	

      if(ret == 1)
      	{
      	    mt_set_gpio_out(GPIO126,1);
      	}
	else{
	    mt_set_gpio_out(GPIO126,0);
	 }

	//mt_set_gpio_out(GPIO126,1);
	//return 1;

	return (ret == 1)?1:0; 
#endif
}


LCM_DRIVER q500_hx8394_khx5001e12_dsi_vdo_lcm_drv =
{
       .name			= "q500_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
       .update         = lcm_update,
#endif
};



