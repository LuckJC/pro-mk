#ifndef BUILD_LK
#include <linux/string.h>
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <string.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (320)
#define FRAME_HEIGHT (320)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
       

#define LCM_DSI_CMD_MODE


#define REGFLAG_DELAY             0XFE
#define REGFLAG_END_OF_TABLE      0xFF   // END OF REGISTERS MARKER


struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table initialization_setting[] = {

	{0xF0,	5,	{0x55, 0xAA, 0x52, 0x08, 0x00}},
	{0xBD,	5,	{0x03, 0x20, 0x14, 0x4B, 0x00}},
	{0xBE,	5,	{0x03, 0x20, 0x14, 0x4B, 0x01}},
	{0xBF,	5,	{0x03, 0x20, 0x14, 0x4B, 0x00}},
	{0xBB,    3,    {0x07, 0x07, 0x07}},
	{0xC7,    1,    {0x40}},
	{REGFLAG_DELAY, 10, {}},
	
	{0xF0,	5,	{0x55, 0xAA, 0x52, 0x08, 0x02}},
	{0xEB,  1,   {0x02}},
	{0xFE,    2,    {0x08, 0x50}},
	{0xC3,    3,    {0xF2, 0x95, 0x04}},
	{0xE9, 3,  {0x00, 0x36, 0x38}},
	{0xCA,    1,    {0x04}},
	{REGFLAG_DELAY, 10, {}},
	
	{0xF0,	5,	{0x55, 0xAA, 0x52, 0x08, 0x01}},
	{0xB0,    3,    {0x03, 0x03, 0x03}},
	{0xB1,    3,    {0x05, 0x05, 0x05}},
	{0xB2,    3,    {0x01, 0xC1, 0x01}},
	{0xB4,    3,    {0x07, 0x07, 0x07}},
	{0xB5,    3,    {0x03, 0x03, 0x03}},
	{0xB6,    3,    {0x55, 0x55, 0x55}},
	{0xB7,    3,    {0x36, 0x36, 0x36}},
	{0xB8,    3,    {0x23, 0x23, 0x23}},
	{0xB9,    3,    {0x03, 0x03, 0xC3}},
	{0xBA,    3,    {0x03, 0x03, 0x03}},
	{0xBE,    3,    {0x32, 0x30, 0x70}},
	{REGFLAG_DELAY, 10, {}},

	{0xCF,    7,    {0xFF, 0xD4, 0x95, 0xE8, 0x4F, 0x00, 0x04}},
	{0x35,	1,	{0x01}},
	{0x36,	1,	{0x00}},
	{0xC0,	1,	{0x20}},

	{0xC2,	6,	{0x17, 0x17, 0x17, 0x17, 0x17, 0x0B}},

//	{0x2A,    4,    {0x00, 0x00, 0x01, 0x40}},
//	{0x2B,    4,    {0x00, 0x00, 0x01, 0x40}},
	//{0x32, 0, {}},
    // Sleep Out
	{0x11,	1,	{0x00}},
    {REGFLAG_DELAY, 100, {}},

    // Display ON
	{0x29,	1,	{0x00}},

	{REGFLAG_DELAY, 100, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_display_on[] = {
    // Sleep Out
	{0x11,	1,	{0x00}},
    {REGFLAG_DELAY, 100, {}},

    // Display ON
	{0x29,	1,	{0x00}},
    {REGFLAG_DELAY, 10, {0}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_display_off[] = {
    // Display Off
	{0x28,	1,	{0x00}},
    {REGFLAG_DELAY, 10, {0}},

	// Sleep In
	{0x10,	1,	{0x00}},
	{REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0xF0,	5,	{0x55, 0xAA, 0x52, 0x08, 0x01}},
	{0xCF,    7,    {0xFF, 0xD4, 0x95, 0xE8, 0x4F, 0x00, 0x04}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
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


static void init_lcm_registers(void)
{
	unsigned int data_array[16];

#if defined(LCM_DSI_CMD_MODE)
	{
		push_table(initialization_setting, sizeof(initialization_setting) / sizeof(struct LCM_setting_table), 1);

	}
#else
	{
		data_array[0] = 0x00043902; // SET password
		data_array[1] = 0x6983FFB9; //
		dsi_set_cmdq(&data_array, 2, 1);
		MDELAY(10);

		data_array[0] = 0x00143902; //// SET Power
		data_array[1] = 0x340085B1; //
		data_array[2] = 0x0F0F0007; //
		data_array[3] = 0x3F3F322A; //
		data_array[4] = 0xE6013A01; //
		data_array[5] = 0xE6E6E6E6;
		dsi_set_cmdq(&data_array, 6, 1);
		MDELAY(10);

		data_array[0] = 0x00103902; //// SET Display 480x800
		data_array[1] = 0x032300B2; //
		data_array[2] = 0xFF007003; //
		data_array[3] = 0x00000000; //
		data_array[4] = 0x01000303; //
		dsi_set_cmdq(&data_array, 5, 1);
		MDELAY(10);

		data_array[0] = 0x00063902; // SET Display
		data_array[1] = 0x801800B4;
		data_array[2] = 0x00000206;
		dsi_set_cmdq(&data_array, 3, 1);
		MDELAY(10);

		data_array[0] = 0x00033902; //// SET VCOM
		data_array[1] = 0x004242B6; 
		dsi_set_cmdq(&data_array, 2, 1);
		MDELAY(10);

		data_array[0] = 0x001B3902; //// SET GIP
		data_array[1] = 0x030400D5; 
		data_array[2] = 0x28050100; 
		data_array[3] = 0x00030170; 
		data_array[4] = 0x51064000; 
		data_array[5] = 0x41000007;
		data_array[6] = 0x07075006;
		data_array[7] = 0x0000040F;
		dsi_set_cmdq(&data_array, 8, 1);
		MDELAY(10);

		data_array[0] = 0x00233902; //// SET GAMMA
		data_array[1] = 0x191300E0; //
		data_array[2] = 0x283F3D38; //
		data_array[3] = 0x0E0D0746; //
		data_array[4] = 0x14121512; //
		data_array[5] = 0x1300170F;
		data_array[6] = 0x3F3D3819;
		data_array[7] = 0x0D074628;
		data_array[8] = 0x1215120E;
		data_array[9] = 0x00170F14;
		dsi_set_cmdq(&data_array, 10, 1);
		MDELAY(10);

#if 0
		switch(g_ColorFormat)
		{
			case PACKED_RGB565:
				data_array[0] = 0x553A1500; // SET pixel format
			break;
			
			case LOOSED_RGB666:
				data_array[0] = 0x663A1500; // SET pixel format
			break; 
			
			case PACKED_RGB888:
				data_array[0] = 0x773A1500; // SET pixel format
			break;
			
			case PACKED_RGB666:
				data_array[0] = 0x663A1500; // SET pixel format
			break;   		

			default:
				//dbg_print("Format setting error \n\r");
				while(1);
			break;
		}

		dsi_set_cmdq(&data_array, 1, 1);
		MDELAY(100);
#endif
		data_array[0] = 0x000E3902; // SET MIPI (1 or 2 Lane)
		data_array[1] = 0xC6A000BA; //
		data_array[2] = 0x10000A00; //
		if (0)//(g_LaneNumber==1)
		{
			data_array[3] = 0x10026F30;
		}
		else
		{
			data_array[3] = 0x11026F30;
		}
		data_array[4] = 0x00004018;
		dsi_set_cmdq(&data_array, 5, 1);
		MDELAY(10);

		data_array[0] = 0x00110500;		// Sleep Out
		dsi_set_cmdq(&data_array, 1, 1);
		MDELAY(10);

		data_array[0] = 0x00290500;		// Display On
		dsi_set_cmdq(&data_array, 1, 1);
		MDELAY(10);
	}
#endif

}
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

#if 0

static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;
#if defined(LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_EVENT_VDO_MODE;
#endif	
		params->ctrl   = LCM_CTRL_PARALLEL_DBI;
		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;
	
		// DBI
		//params->dbi.port                    = 0;
		params->dbi.clock_freq              = LCM_DBI_CLOCK_FREQ_104M;
		params->dbi.data_width              = LCM_DBI_DATA_WIDTH_16BITS;
		params->dbi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dbi.data_format.trans_seq   = LCM_DBI_TRANS_SEQ_MSB_FIRST;
		params->dbi.data_format.padding     = LCM_DBI_PADDING_ON_LSB;
		params->dbi.data_format.format      = LCM_DBI_FORMAT_RGB888;
		params->dbi.data_format.width       = LCM_DBI_DATA_WIDTH_24BITS;
	  params->dbi.cpu_write_bits          = LCM_DBI_CPU_WRITE_16_BITS;
	  params->dbi.io_driving_current      = 0;

		// enable tearing-free
		//params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		params->dbi.te_edge_polarity		= LCM_POLARITY_FALLING;//LCM_POLARITY_RISING;
	
		// DPI
		params->dpi.format			  = LCM_DPI_FORMAT_RGB888;
		params->dpi.rgb_order		  = LCM_COLOR_ORDER_RGB;
		params->dpi.intermediat_buffer_num = 2;
	
		// DSI
		//params->dsi.DSI_WMEM_CONTI=0x3C;
		//params->dsi.DSI_RMEM_CONTI=0x3E;
		//params->dsi.LANE_NUM=LCM_ONE_LANE;
		params->dsi.LANE_NUM=LCM_ONE_LANE;
		//params->dsi.VC_NUM=0x0;
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
/*

		//params->dsi.word_count=320*3;
	
		params->dsi.vertical_sync_active=2;
		params->dsi.vertical_backporch=240;
		params->dsi.vertical_frontporch=300;
		params->dsi.vertical_active_line=320;
	
		//params->dsi.line_byte=2256;		// 2256 = 752*3
		params->dsi.horizontal_sync_active_byte=26;
		params->dsi.horizontal_backporch_byte=110;
		params->dsi.horizontal_frontporch_byte=110;	
		params->dsi.rgb_byte=(320*3+6);	
	
		params->dsi.horizontal_sync_active_word_count=20;	
		params->dsi.horizontal_backporch_word_count=300;
		params->dsi.horizontal_frontporch_word_count=200;
*/

		params->dsi.vertical_sync_active				= 2;
		params->dsi.vertical_backporch					= 8;
		params->dsi.vertical_frontporch					= 10;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;
		params->dsi.horizontal_backporch				= 20;
		params->dsi.horizontal_frontporch				= 40;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		
	
		params->dsi.HS_TRAIL=0x14;
		params->dsi.HS_ZERO=0x14;
		params->dsi.HS_PRPR=0x0A;
		params->dsi.LPX=0x05;
	
		params->dsi.TA_SACK=0x01;
		params->dsi.TA_GET=0x37;
		params->dsi.TA_SURE=0x16;	
		params->dsi.TA_GO=0x10;
	
		params->dsi.CLK_TRAIL=0x14;
		params->dsi.CLK_ZERO=0x14;	
		params->dsi.LPX_WAIT=0x0A;
		params->dsi.CONT_DET=0x00;
	
		params->dsi.CLK_HS_PRPR=0x0A;
	
		params->dsi.pll_div1=24;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
	
		//params->dsi.pll_div2=4;			// div2=0~15: fout=fvo/(2*div2)
		params->dsi.pll_div2=1;			// div2=0~15: fout=fvo/(2*div2)

}
#endif


static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));
    
    params->type   = LCM_TYPE_DSI;
    params->lcm_order = 0;
    
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;
    
    // enable tearing-free
    params->dbi.te_mode 			= LCM_DBI_TE_MODE_DISABLED;// LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity		= LCM_POLARITY_FALLING;

#if defined(LCM_DSI_CMD_MODE)   
    params->dsi.mode   = CMD_MODE;
#else
    params->dsi.mode   = BURST_VDO_MODE;
#endif

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_ONE_LANE;

    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
    
    params->dsi.intermediat_buffer_num = 1;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
    
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    
  //  params->dsi.word_count=320*3;	//DSI CMD mode need set these two bellow params, different to 6577
  //  params->dsi.vertical_active_line=320;
   
		params->dsi.vertical_sync_active				= 1;// 3    2
		params->dsi.vertical_backporch					= 1;// 20   1
		params->dsi.vertical_frontporch					= 2; // 1  12
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 2;// 50  2
		params->dsi.horizontal_backporch				= 12;
		params->dsi.horizontal_frontporch				= 12;
		params->dsi.horizontal_active_pixel				=FRAME_WIDTH;  //FRAME_WIDTH*3;
 
    // Bit rate calculation
    params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4
    params->dsi.pll_div2=1;		// div2=0,1,2,3;div2_real=1,2,4,4
    params->dsi.fbk_div =12;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)		
}


static void lcm_init(void)
{
    SET_RESET_PIN(0);
    MDELAY(25);
    SET_RESET_PIN(1);
    MDELAY(50);

//    init_lcm_registers();

    push_table(initialization_setting, sizeof(initialization_setting) / sizeof(struct LCM_setting_table), 1);


}


static void lcm_suspend(void)
{
	push_table(lcm_display_off, sizeof(lcm_display_off) / sizeof(struct LCM_setting_table), 1);

}


static void lcm_resume(void)
{
	push_table(lcm_display_on, sizeof(lcm_display_on) / sizeof(struct LCM_setting_table), 1);

}

#if 0
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
	//data_array[6]= 0x002c3901;

	dsi_set_cmdq(&data_array, 7, 0);

	//push_table(lcm_set_window, sizeof(lcm_set_window) / sizeof(struct LCM_setting_table), 0);

}

#endif

static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	x0= x0>160?x0+160:x0;
	x1= x1>160?x1+160:x1;

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



static unsigned int lcm_compare_id(void)
{
    unsigned int id = 0;

	return 1;
}

static void lcm_setbacklight(unsigned int level)
{
	unsigned int default_level = 0;
	unsigned int mapped_level = 0;

	//for LGE backlight IC mapping table
	if(level > 255) 
		level = 255;

	if(level >0) 
		mapped_level = default_level+(level)*(255-default_level)/(255);
	else
		mapped_level=0;

	// Refresh value of backlight level.
	lcm_backlight_level_setting[1].para_list[0] = mapped_level;

#if defined(BUILD_LK)
	printf("%s, [lcm]rm69032 level = %d, mapped_level = %d \n", __func__, level, mapped_level);
#else
	printk("%s, [lcm]rm69032 level = %d, mapped_level = %d \n", __func__, level, mapped_level);
#endif	


	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}


LCM_DRIVER rm69032_dsi_cmd_drv = 
{
    .name			= "rm69032",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.set_backlight	= lcm_setbacklight,
	.compare_id     	= lcm_compare_id,
#if defined(LCM_DSI_CMD_MODE)
        .update         = lcm_update,
#endif
};
