#ifdef BUILD_LK
#include <string.h>
#else
#include <linux/string.h>
#endif
 
#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#else
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/upmu_common.h>
#endif
#include "lcm_drv.h"


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define FRAME_WIDTH  (1024)
#define FRAME_HEIGHT (600)


#define LCM_DSI_CMD_MODE    0

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static LCM_UTIL_FUNCS lcm_util = {
    .set_gpio_out = NULL,
};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

#define REGFLAG_DELAY             0XFE
#define REGFLAG_END_OF_TABLE      0xFF   // END OF REGISTERS MARKER


struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};
// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)               lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

static struct LCM_setting_table lcm_display_on[] = {
    // Sleep Out
	{0x11,	0,	{0x00}},
    {REGFLAG_DELAY, 100, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_display_off[] = {

	// Sleep In
	{0x10,	0,	{0x00}},
	{REGFLAG_DELAY, 100, {}},
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

static void lcd_power_en(unsigned char enabled)
{
    if (enabled)
    {      
#ifdef BUILD_LK
        /* VGP2_PMU 3V */
        pmic_config_interface(DIGLDO_CON28, 0x5, PMIC_RG_VGP1_VOSEL_MASK, PMIC_RG_VGP1_VOSEL_SHIFT);
        pmic_config_interface(DIGLDO_CON7, 0x1, PMIC_RG_VGP1_EN_MASK, PMIC_RG_VGP1_EN_SHIFT);
#else
        //upmu_set_rg_vgp2_vosel(0x6);
        //upmu_set_rg_vgp2_en(0x1);
        hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_2800, "LCM");
#endif
		#if 0
        mt_set_gpio_mode(GPIO_VLCD_VCC_EN, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_VLCD_VCC_EN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_VLCD_VCC_EN, GPIO_OUT_ONE);
		#endif
    }
    else
    {      
#ifdef BUILD_LK
        /* VGP2_PMU 3V */
        pmic_config_interface(DIGLDO_CON7, 0x0, PMIC_RG_VGP1_EN_MASK, PMIC_RG_VGP1_EN_SHIFT);
        pmic_config_interface(DIGLDO_CON28, 0x0, PMIC_RG_VGP1_VOSEL_MASK, PMIC_RG_VGP1_VOSEL_SHIFT); 
#else
        //hwPowerDown(MT65XX_POWER_LDO_VGP2, "LCM");
        //hwPowerDown(MT6323_POWER_LDO_VGP2, "LCM");
        //upmu_set_rg_vgp2_en(0x0);        
        //upmu_set_rg_vgp2_vosel(0x0);
        hwPowerDown(MT6323_POWER_LDO_VGP1, "LCM");
#endif
		#if 0
        mt_set_gpio_mode(GPIO_VLCD_VCC_EN, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_VLCD_VCC_EN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_VLCD_VCC_EN, GPIO_OUT_ZERO);
		#endif
    }
}


static void lcd_reset(unsigned char enabled)
{
    if (enabled)
    {
        mt_set_gpio_mode(GPIO119, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO119, GPIO_DIR_OUT);
   //     mt_set_gpio_dir(GPIO119, GPIO_DIR_IN);
//	 mt_set_gpio_pull_enable(GPIO119,0);
       mt_set_gpio_out(GPIO119, GPIO_OUT_ONE);
    }
    else
    {	
        mt_set_gpio_mode(GPIO119, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO119, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO119, GPIO_OUT_ZERO);    	
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
		params->dbi.te_mode 				=LCM_DBI_TE_MODE_DISABLED ; //LCM_DBI_TE_MODE_VSYNC_ONLY
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
	params->dsi.mode   = BURST_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
	//	params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 

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
		params->dsi.word_count=600*3;	

		params->dsi.vertical_sync_active				= 4;
		params->dsi.vertical_backporch					= 8;
		params->dsi.vertical_frontporch					= 8;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 30;// 50  2
		params->dsi.horizontal_backporch				= 60 ;
		params->dsi.horizontal_frontporch				= 60 ;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		#if 0
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
		params->dsi.fbk_div =16;//20;      // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
		#else
		params->dsi.PLL_CLOCK=50; // 416
		#endif

}

extern void DSI_clk_HS_mode(unsigned char enter);
static void init_lcm_registers(void)
{
    unsigned int data_array[16];
    
#ifdef BUILD_LK
    printf("%s, LK \n", __func__);
#else
    printk("%s, kernel", __func__);
#endif
    
    //data_array[0] = 0x00010500;  //software reset					 
    //dsi_set_cmdq(data_array, 1, 1);
    //MDELAY(10);
    //DSI_clk_HS_mode(1);
    //MDELAY(80);
    
    //data_array[0] = 0x00290500;  //display on                        
    //dsi_set_cmdq(data_array, 1, 1);
    push_table(lcm_display_on, sizeof(lcm_display_on) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_init(void)
{
#if 0//def BUILD_LK
    return;//LK donothing
#else
   // lcd_reset(0);
   // lcd_power_en(0);
  //  lcd_power_en(1);
  //  MDELAY(50);//Must > 5ms
  //  lcd_reset(1);
  //  MDELAY(200);//Must > 50ms
    init_lcm_registers();
#endif  
}


static void lcm_suspend(void)
{
   // unsigned int data_array[16];
    
    //data_array[0] = 0x00280500;  //display off                        
   // dsi_set_cmdq(data_array, 1, 1);
   // MDELAY(5);
   // lcd_reset(0);
    //lcd_power_en(0);
    //DSI_clk_HS_mode(0);
    //MDELAY(10);
}


static void lcm_resume(void)
{
    //lcd_reset(0);
    //lcd_power_en(0);
   // lcd_power_en(1);
   // MDELAY(50);//Must > 5ms
  //  lcd_reset(1);
   // MDELAY(200);//Must > 50ms
    //init_lcm_registers();
}

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

	return 0;
}

LCM_DRIVER ek790161_dsi_vdo_lcm_drv = 
{
    .name           = "ek790161",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id    = lcm_compare_id
    #if defined(LCM_DSI_CMD_MODE)
        ,
        .update         = lcm_update
#endif
};



