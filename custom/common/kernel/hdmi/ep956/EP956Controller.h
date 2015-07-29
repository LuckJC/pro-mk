/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP956Controller.h

  Description :  Head file of EP956Controller.

\******************************************************************************/

#ifndef EP956CONTROLLER_H
#define EP956CONTROLLER_H

// Our includes
#include "EP956ERegDef.h"
#include "EP956_If.h"
#include "HDCP.h"

#define EP956C_VERSION_MAJOR      1  // 
#define EP956C_VERSION_MINOR      34 // v1.34

#define LINK_TASK_TIME_BASE       55 // 80 -> 55 ms to detect CBUS disconnec faster to catch up the [VBUS Stable Time] test
#define EP956C_TIMER_PERIOD       HDCP_TIMER_PERIOD		//     The EP956Controller.c must be re-compiled if user want to change this value.

typedef enum {
	EP956C_TASK_Idle = 0,
	EP956C_TASK_Error,
	EP956C_TASK_Pending
} EP956C_TASK_STATUS;

typedef enum {
	CBUS_LINK_STATE__USB_Mode = 0, 
	CBUS_LINK_STATE__1KOHM_Detected,	
	CBUS_LINK_STATE__Start_Connect,
	CBUS_LINK_STATE__Check_DCAP,
	CBUS_LINK_STATE__Connected	
} CBUS_LINK_STATE;

typedef enum {
	TXS_Search_EDID,
	TXS_Wait_Upstream,
	TXS_Stream,
	TXS_HDCP
} TX_STATE;

typedef struct _EP956C_REGISTER_MAP {

	// Read
	unsigned short		VendorID;				// EP956E_Vendor_ID
	unsigned short		DeviceID;				// EP956E_Device_ID
	unsigned char		Version_Major;			// EP956E_Firmware_Revision__Major
	unsigned char		Version_Minor;			// EP956E_Firmware_Revision__Minor

	unsigned char		Interrupt_Flags;		// EP956E_Interrupt_Flags...

	unsigned char		System_Status;			// EP956E_System_Status...
	unsigned char		MSC_Message[2];			// EP956E_MSC_Message

	unsigned char		HDCP_Status;			// EP956E_HDCP_Status...
	unsigned char		HDCP_State;				// EP956E_HDCP_State
	unsigned char		HDCP_AKSV[5];			// EP956E_HDCP_AKSV
	unsigned char		HDCP_BKSV[5];			// EP956E_HDCP_BKSV
	unsigned char		HDCP_BCAPS3[3];			// EP956E_HDCP_BCAPS, EP956E_HDCP_BSTATUS
	unsigned char		HDCP_KSV_FIFO[5*16];	// EP956E_HDCP_BSTATUS

	unsigned char		EDID_Status;			// EP956E_EDID_Status...
	unsigned char		EDID_VideoDataAddr;		//
	unsigned char		EDID_AudioDataAddr;		//
	unsigned char		EDID_SpeakerDataAddr;	//
	unsigned char		EDID_VendorDataAddr;	//
	unsigned char		EDID_ASFreq;			// EP956E_EDID_Status_ASFreq
	unsigned char		EDID_AChannel_PCM;		// EP956E_EDID_Status_AChannel
	unsigned char		EDID_AChannel_DSD;		//
						
	VDO_PARAMS 			Video_Params_Backup;	// 0x05 (Video Status)

	ADO_PARAMS 			Audio_Params_Backup;	// 0x06 (Audio Status)

	unsigned char		Readed_EDID[256];		// EP956E_EDID_Data

	// Read / Write

	unsigned char		Power_Control;			// EP956E_Power_Control...
	unsigned char		System_Configuration;	// EP956E_System_Configuration...

	unsigned char		Interrupt_Enable;		// EP956E_Interrupt_Enable...

	unsigned char		Video_Interface[2];		// EP956E_Video_Interface_Setting_0..., EP956E_Video_Interface_Setting_1...

	unsigned char		Audio_Interface;		// EP956E_Audio_Interface_Setting...

	unsigned char		Video_Input_Format[3];	// EP956E_Video_Output_Format_VIC..., EP956E_Video_Output_Format_1... EP956E_Video_Output_Format_2...

	unsigned char		Audio_Input_Format[6];	// EP956E_Audio_Output_Format...

	unsigned char 		Revocation_List[5*4];	// EP956E_KSV_Revocation_List

	unsigned char		Select_Packet[31];		// EP956E_Select_Packet

} EP956C_REGISTER_MAP, *PEP956C_REGISTER_MAP;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef void (*EP956C_CALLBACK)(void);

void EP956Controller_Initial(PEP956C_REGISTER_MAP pEP956C_RegMap, EP956C_CALLBACK IntCall);

unsigned char EP956Controller_Task(void);

void EP956Controller_Timer(void);

void EP956Controller_Interrupt(void);

void HDMI_Dump_Status();

// -----------------------------------------------------------------------------
#endif

