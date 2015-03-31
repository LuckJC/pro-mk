#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <TMD2771_cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 2,
    .polling_mode_ps = 1,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x72, 0x48, 0x78, 0x00},
    .als_level  = { 0,  6,  6,   6,  15,  15,  15, 15, 100,  1000,  2000, 6000, 14000, 18000, 20000},
    .als_value  = {40, 90, 225,  225, 225, 320,  320,  320,  640,  1280,  1280,  2600,  2600, 2600,  10240, 10240},
    .ps_threshold_high = 0x230,
    .ps_threshold_low = 0x250,
//    .ps_threshold = 300,
};
struct alsps_hw *TMD2771_get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

