#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <em30713_cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 2,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    //.i2c_addr   = {0x0C, 0x48, 0x78, 0x00},
    

	//lingjinming modify 2014 09 19
    .als_level  = {5,  9, 36, 59, 82, 132, 205, 273, 500, 845, 1136, 1545, 2364, 4095},	/* als_code */    
    .als_value  = {0, 10, 40, 65, 90, 145, 225, 300, 550, 930, 1250, 1700, 2600, 5120, 7680},    /* lux */
	#if 0
    .als_level  = { 0,  1,  1,   7,  15,  15,  100, 1000, 2000,  3000,  6000, 10000, 14000, 18000, 20000},
    .als_value  = {40, 40, 90,  90, 160, 160,  225,  320,  640,  1280,  1280,  2600,  2600, 2600,  10240, 10240},
    #endif
    //lingjinming
    
    .ps_threshold_high = 130,
    .ps_threshold_low = 120,
    .ps_threshold = 60, //70, //lingjinming 2014 09 18 //200,  //改小就是较远的时候起作用!!!!????
};
struct alsps_hw *em30713_get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

