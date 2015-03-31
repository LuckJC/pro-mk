#ifndef RCV_CUST_KEYS_H
#define RCV_CUST_KEYS_H

#include <linux/input.h>


//lingjinming modify for p210 you ni sen 2014 09 19
 
#if defined(P201_UNISON_KEYPAD)

#define RECOVERY_KEY_DOWN     KEY_VOLUMEUP
#define RECOVERY_KEY_UP       KEY_VOLUMEUP
#define RECOVERY_KEY_ENTER    KEY_VOLUMEDOWN


#else
#define RECOVERY_KEY_DOWN     KEY_VOLUMEDOWN
#define RECOVERY_KEY_UP       KEY_VOLUMEDOWN
#define RECOVERY_KEY_ENTER    KEY_VOLUMEUP
#endif
//lingjinming
#define RECOVERY_KEY_MENU     KEY_POWER

#endif /* RCV_CUST_KEYS_H */
