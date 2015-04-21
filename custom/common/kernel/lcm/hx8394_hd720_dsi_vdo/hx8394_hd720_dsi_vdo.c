#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"
#include <cust_gpio_usage.h>
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

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#ifndef BUILD_LK
static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
#endif
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V3(para_tbl,size,force_update)        lcm_util.dsi_set_cmdq_V3(para_tbl,size,force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define   LCM_DSI_CMD_MODE							0

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


/*static struct LCM_setting_table lcm_initialization_setting[] = {
	


};*/

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



static LCM_setting_table_V3 lcm_initialization_setting2[] = {
	
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

{0x39, 0xB9,  3 ,{0xFF, 0x83, 0x94}},

{0x39, 0xBA,  2 ,{0x78,0x83}},

{0x39, 0xB0,  2 ,{0x00,0x00,0x7d,0x0c}},

{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3,5,{}},

{0x39, 0xB1, 15, {0x6C, 0x15, 0x15, 0x24, 0x04, 0x11, 
                           0xF1, 0x80, 0xE4,  0x97,  0x23, 0x80, 0xC0, 0xD2, 0x58}},



{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3,10,{}},

{0x39, 0xB2,  11 ,{0x00, 0x64, 0x10, 0x07, 0x22, 0x1C, 0x08, 0x08, 0x1C, 0x4D, 0x00}},

{0x39, 0xB4, 12, {0x00, 0xFF, 0x03, 0x5A, 0x03, 0x5A, 0x03, 0x5A, 0x01, 0x6A, 0x30, 0x6A}},
	
{0x39, 0xBC, 1, {0x07}},

{0x39, 0xBF, 3, {0x41, 0x0E, 0x01}},

{0x39, 0xD3, 30, {0x00, 0x06, 0x00, 0x40, 0x07, 0x08, 0x00, 
                        0x32, 0x10, 0x07, 0x00, 0x07, 0x54, 0x15, 
                        0x0F, 0x05, 0x04, 0x02, 0x12, 0x10, 0x05,
                        0x07, 0x33, 0x33, 0x0B, 0x0B, 0x37, 0x10, 0x07, 0x07}},

{0x39, 0xD5, 44, {0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x20,
                      0x21, 0x22, 0x23, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
                      0x18, 0x18, 0x19, 0x19, 0x18, 0x18, 0x18, 0x18, 0x1B, 
                      0x1B, 0x1A, 0x1A, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
                      0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18}},


{0x39, 0xD6, 44, {0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04, 0x23,
                        0x22, 0x21, 0x20, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
                        0x58, 0x58, 0x18, 0x18, 0x19, 0x19, 0x18, 0x18, 0x1B, 
                        0x1B, 0x1A, 0x1A, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
                        0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18}},

{0x39, 0xCC, 1, {0x01}},

{0x39, 0xB6, 2, {0x4C, 0x4C}},


{0x39, 0xE0, 42, {0x00, 0x10, 0x16, 0x2D, 0x33, 0x3F, 0x23, 0x3E,
                        0x07, 0x0B, 0x0D, 0x17, 0x0E, 0x12, 0x14, 0x12,
                        0x13, 0x06, 0x11, 0x13, 0x18, 0x00, 0x0F, 0x16, 
                        0x2E, 0x33, 0x3F, 0x23, 0x3D, 0x07, 0x0B, 0x0D,
                        0x18, 0x0F, 0x12, 0x14, 0x12, 0x14, 0x07, 0x11, 0x12, 0x17}},

{0x39, 0xC0, 2, {0x30, 0x14}},


{0x39, 0xC7, 4, {0x00, 0xC0, 0x40, 0xC0}},

{0x39, 0xDF, 1, {0x8E}},


{0x39, 0xD2, 1, {0x66}},
	
{0x05, 0x11,0,{}},//
{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3,120,{}},

{0x05, 0x29,0,{}},//
{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3,20,{}},	  

	/* FIXME */
	/*
		params->dsi.horizontal_sync_active				= 0x16;// 50  2
		params->dsi.horizontal_backporch				= 0x38;
		params->dsi.horizontal_frontporch				= 0x18;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		params->dsi.horizontal_blanking_pixel =0;    //lenovo:fix flicker issue
	    //params->dsi.LPX=8; 
	*/

};


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
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

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = BURST_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
        #endif
	
		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Video mode setting		
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		
		params->dsi.vertical_sync_active				= 0x05;// 3    2
		params->dsi.vertical_backporch					= 0x0d;// 20   1
		params->dsi.vertical_frontporch					= 0x08; // 1  12
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 0x12;// 50  2
		params->dsi.horizontal_backporch				= 0x5f;
		params->dsi.horizontal_frontporch				= 0x5f;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	    //params->dsi.LPX=8; 

		// Bit rate calculation
		//params->dsi.PLL_CLOCK = 240;
		//1 Every lane speed
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4	
		params->dsi.fbk_div =8;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

}

static void lcm_init(void)
{
              mt_set_gpio_out(GPIO119,1);
	       MDELAY(20); 
		//SET_RESET_PIN(0);
		mt_set_gpio_out(GPIO119,0);
		MDELAY(20); 
		//SET_RESET_PIN(1);
		mt_set_gpio_out(GPIO119,1);
		MDELAY(100); 
	       //push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
		//dsi_set_cmdq_V3(lcm_initialization_setting2,sizeof(lcm_initialization_setting2)/sizeof(lcm_initialization_setting2[0]),1);

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

	unsigned int data_array[16];
	data_array[0]=0x0043902;
	data_array[1]=0x9483ffb9;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10); 
	data_array[0]=0x00033902;
	data_array[1]=0x8373ba;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10); 
	data_array[0]=0x0053902;
	data_array[1]=0x7d0000b0;
	data_array[2]=0x00000004;
	dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(10); 
	data_array[0]=0x00103902;
	data_array[1]=0x15156cb1;
	data_array[2]=0xf21203c4;
	data_array[3]=0x2397e480;
	data_array[4]=0x58d2c080;
	dsi_set_cmdq(&data_array, 5, 1);
	MDELAY(10); 
	data_array[0]=0x00c3902;
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
	data_array[0]=0x0023902;
	data_array[1]=0x07bc;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);
	data_array[0]=0x0043902;
	data_array[1]=0x010e41bf;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10); 
	data_array[0]=0x0033902;
	data_array[1]=0x4c4cb6;
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
	data_array[0]=0x0023902;
	data_array[1]=0x01cc;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10); 
	data_array[0]=0x0033902;
	data_array[1]=0x1430c0;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10); 
	data_array[0]=0x0053902;
	data_array[1]=0x40c000c7;
	data_array[2]=0xc0;
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

	data_array[0]=0x0023902;
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



static void lcm_suspend(void)
{
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);


}


static void lcm_resume(void)
{
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
	
}
         
#if (LCM_DSI_CMD_MODE)
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
#endif

static unsigned int lcm_compare_id(void)
{

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

	return (ret == 1)?1:0; 


}


static unsigned int lcm_esd_check(void)
{
  #ifndef BUILD_LK
	char  buffer[3];
	int   array[4];

	if(lcm_esd_test)
	{
		lcm_esd_test = FALSE;
		return TRUE;
	}

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x36, buffer, 1);
	if(buffer[0]==0x90)
	{
		return FALSE;
	}
	else
	{			 
		return TRUE;
	}
#else
	return FALSE;
#endif

}

static unsigned int lcm_esd_recover(void)
{
	lcm_init();
	lcm_resume();

	return TRUE;
}



LCM_DRIVER hx8394_hd720_dsi_vdo_lcm_drv = 
{
    .name			= "hx8394_hd720_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	//.esd_check = lcm_esd_check,
	//.esd_recover = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };

