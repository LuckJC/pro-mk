#include <cust_vibrator.h>
#include <linux/types.h>
//lingjinming ji lu 
static struct vibrator_hw cust_vibrator_hw = {
	.vib_timer = 50,
  #ifdef CUST_VIBR_LIMIT
	.vib_limit = 9,
  #endif
  #ifdef CUST_VIBR_VOL
	.vib_vol = 0x04,//lingjinming modify for p201 2014 09 19 //0x5,//2.8V for vibr
  #endif
};

struct vibrator_hw *get_cust_vibrator_hw(void)
{
    return &cust_vibrator_hw;
}

