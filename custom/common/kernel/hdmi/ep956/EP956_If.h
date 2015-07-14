/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

  File        :  DDC_If.h 

  Description :  Head file of EP956 Interface  

\******************************************************************************/

#ifndef EP956_IF_H
#define EP956_IF_H

#include "CommonInclude.h"
#include "EP956RegDef.h"



// Command and Data for the function 

// MHL_MSC_Cmd_READ_DEVICE_CAP(BYTE offset, PBYTE pValue)
#define MSC_DEV_CAT					0X02
#define 	POW								0x10
#define		DEV_TYPE						0x0F

#define ADOPTER_ID_H				0x03
#define ADOPTER_ID_L				0x04

#define LOG_DEV_MAP					0x08
#define 	LD_DISPLAY						0X01
#define 	LD_VIDEO						0X02
#define 	LD_AUDIO						0X04
#define 	LD_MEDIA						0X08
#define 	LD_TUNER						0X10
#define 	LD_RECORD						0X20
#define 	LD_SPEAKER						0X40
#define 	LD_GUI							0X80

#define FEATURE_FLAG				0X0A
#define 	RCP_SUPPORT						0X01
#define 	RAP_SUPPORT						0X02
#define 	SP_SUPPORT						0X04

#define DEVICE_ID_H					0x0B
#define DEVICE_ID_L					0x0C

#define SCRATCHPAD_SIZE				0X0D

// MHL_MSC_Reg_Read(BYTE offset)
// MHL_MSC_Cmd_WRITE_STATE(BYTE offset, BYTE value)
#define MSC_RCHANGE_INT				0X20
#define 	DCAP_CHG						0x01
#define 	DSCR_CHG						0x02
#define 	REQ_WRT							0x04
#define 	GRT_WRT							0x08

#define MSC_DCHANGE_INT				0X21
#define 	EDID_CHG						0x02

#define MSC_STATUS_CONNECTED_RDY	0X30
#define 	DCAP_RDY						0x01

#define MSC_STATUS_LINK_MODE		0X31
#define 	CLK_MODE						0x07
#define 	CLK_MODE__Normal						0x03
#define 	CLK_MODE__PacketPixel					0x02
#define 	PATH_EN							0x08

#define MSC_SCRATCHPAD				0x40


//MHL_MSC_Cmd_MSC_MSG(BYTE SubCmd, BYTE ActionCode)
#define MSC_RAP						0x20
#define		RAP_POLL						0x00
#define		RAP_CONTENT_ON					0x10
#define		RAP_CONTENT_OFF					0x11

#define MSC_RAPK					0x21
#define		RAPK_No_Error					0x00
#define		RAPK_Unrecognized_Code			0x01
#define		RAPK_Unsupported_Code			0x02
#define		RAPK_Responder_Busy				0x03

#define MSC_RCP						0x10
#define 	RCP_Select						0x00
#define 	RCP_Up							0x01
#define 	RCP_Down						0x02
#define 	RCP_Left						0x03
#define 	RCP_Right						0x04
#define 	RCP_Right_Up					0x05
#define 	RCP_Right_Down					0x06
#define 	RCP_Left_Up						0x07
#define 	RCP_Left_Down					0x08
#define 	RCP_Root_Menu					0x09
#define 	RCP_Setup_Menu					0x0A
#define 	RCP_Contents_Menu				0x0B
#define 	RCP_Favorite_Menu				0x0C
#define 	RCP_Exit						0x0D

#define 	RCP_Numeric_0					0x20
#define 	RCP_Numeric_1					0x21
#define 	RCP_Numeric_2					0x22
#define 	RCP_Numeric_3					0x23
#define 	RCP_Numeric_4					0x24
#define 	RCP_Numeric_5					0x25
#define 	RCP_Numeric_6					0x26
#define 	RCP_Numeric_7					0x27
#define 	RCP_Numeric_8					0x28
#define 	RCP_Numeric_9					0x29
#define 	RCP_Dot							0x2A
#define 	RCP_Enter						0x2B
#define 	RCP_Clear						0x2C

#define 	RCP_Channel_Up					0x30
#define 	RCP_Channel_Down				0x31
#define 	RCP_Previous_Channel			0x32

#define 	RCP_Volume_Up					0x41
#define 	RCP_Volume_Down					0x42
#define 	RCP_Mute						0x43
#define 	RCP_Play						0x44
#define 	RCP_Stop						0x45
#define 	RCP_Pause						0x46
#define 	RCP_Record						0x47
#define 	RCP_Rewind						0x48
#define 	RCP_Fast_Forward				0x49
#define 	RCP_Eject						0x4A
#define 	RCP_Forward						0x4B
#define 	RCP_Backward					0x4C

#define 	RCP_Angle						0x50
#define 	RCP_Subpicture					0x51

#define 	RCP_Play_Function				0x60
#define 	RCP_Pause_Play_Function			0x61
#define 	RCP_Record_Function				0x62
#define 	RCP_Pause_Record_Function		0x63
#define 	RCP_Stop_Function				0x64
#define 	RCP_Mute_Function				0x65
#define 	RCP_Restore_Volume_Function		0x66
#define 	RCP_Tune_Function				0x67
#define 	RCP_Select_Media_Function		0x68

#define 	RCP_F1							0x71
#define 	RCP_F2							0x72
#define 	RCP_F3							0x73
#define 	RCP_F4							0x74

#define MSC_RCPK					0x11
#define MSC_RCPE					0x12
#define 	RCPK_No_Error					0x00
#define 	RCPK_Ineffective_Code			0x01
#define 	RCPK_Responder_Busy				0x02

#define MSC_UCP						0x30
#define MSC_UCPK					0x31
#define MSC_UCPE					0x32
#define 	UCPK_No_Error					0x00
#define 	UCPK_Ineffective_Code			0x01

// Return value for the function
// MHL_Tx_Connection_Status();
typedef enum {
	MHL_CBUS_CONNECTING = 0,	// CBUS is under connecting.
	MHL_NOT_CONNECTED, 			// CBUS connect fail.
	MHL_CBUS_CONNECTED, 		// CBUS is connected.
	MHL_HOTPLUG_DETECT	 		// Hot-Plug signal is detected. 
} MHL_CONNECTION_STATUS;

extern BYTE Device_Capability_Default[];

typedef enum {
	// Master
	SMBUS_STATUS_Success = 0x00,
	SMBUS_STATUS_Pending,//	SMBUS_STATUS_Abort,
	SMBUS_STATUS_NoAct = 0x02,
	SMBUS_STATUS_TimeOut,
	SMBUS_STATUS_ArbitrationLoss = 0x04
} SMBUS_STATUS;

typedef enum {
	SMBUS_Normal = 0,
	SMBUS_SkipStop,
	SMBUS_SkipStart,
	SMBUS_DataOnly		// 3 = SMBUS_SkipStop | SMBUS_SkipStart
} SMBUS_MODE;


typedef enum {
	HDMI_NO_CONNECT = 0, 	// No Connection.
	HDMI_HOTPLUG_DETECT, 	// Hot-Plug signal is detected. 
	HDMI_EDID_DETECT 		// Connection is confirmed by avaliable EDID.
} HDMI_CONNECTION_STATUS;

typedef enum {
	COLORSPACE_601 = 1,
	COLORSPACE_709,
	COLORSPACE_xvYCC601,
	COLORSPACE_xvYCC709
} COLORSPACE;

typedef enum {
	SYNCMODE_HVDE = 0,
	SYNCMODE_HV,
	SYNCMODE_Embeded
} SYNCMODE;

typedef enum {
	COLORFORMAT_RGB = 0,
	COLORFORMAT_YCC444,
	COLORFORMAT_YCC422
} COLORFORMAT;

typedef enum {
	AFAR_VideoCode = 8,
	AFAR_4_3,
	AFAR_16_9,
	AFAR_14_9
} AFAR;

// Video Output Congif Params
typedef struct _VDO_PARAMS {
	// Timing
	BYTE 			Interface;			// DK[3:1], DKEN, DSEL, BSEL, EDGE, FMT12
	BYTE 			VideoSettingIndex;	// VIC
	BYTE 			HVPol;				// x, x, x, x, VSO_POL, HSO_POL, x, x
	SYNCMODE 		SyncMode;			// 0 = HVDE, 1 = HV(DE Gen), 2 = Embedded Sync
	// Format
	COLORFORMAT 	FormatIn;			// 0 = RGB, 1 = YCC444, 2 = YCC422
	COLORFORMAT 	FormatOut;			// 0 = RGB, 1 = YCC444, 2 = YCC422
	COLORSPACE 		ColorSpace;			// 0 = Auto, 1 = 601, 2 = 709
	AFAR 			AFARate;			// 0 = Auto, 1 = 4:3, 2 = 16:9, 3 = 14:9
} VDO_PARAMS, *PVDO_PARAMS;

typedef enum {
	AUDIO_TYPE_SPDIF = 0x00,
	AUDIO_TYPE_IIS,
	AUDIO_TYPE_DSD, // One bit audio
	AUDIO_TYPE_HBR  // High bit rate audio
	// AUDIO_TYPE_DST // The compressed DSD
} AUDIO_TYPE;

typedef enum {
	ADSFREQ_32000Hz = 0x03,
	ADSFREQ_44100Hz = 0x00,
	ADSFREQ_48000Hz = 0x02,
	ADSFREQ_88200Hz = 0x08,
	ADSFREQ_96000Hz = 0x0A,
	ADSFREQ_176400Hz = 0x0C,
	ADSFREQ_192000Hz = 0x0E
} ADSFREQ;

// Audio Output Congif Params
typedef struct _ADO_PARAMS {
	BYTE       	Interface; 				// DSDCK_POL, x, x, x, x, WS_M, WS_POL, SCK_POL
	BYTE 		VideoSettingIndex;		// VIC
	AUDIO_TYPE 	AudioType;				// AUDIO_TYPE
	BYTE 		ChannelNumber;			// 1 = 2 ch, 2 = 3 ch, ... , 5 = 5.1 ch, 7 = 7.1 ch
	BYTE 		ADSRate;				// 1 = SF/2, 2 = SF/3, 3 = SF/4 (Down Sample)
	ADSFREQ		InputFrequency;			// ADSFREQ
	BYTE       	VFS; 					// 0 = 59.94Hz, 1 = 60Hz (Vertical Frequency Shift of Video)
	BYTE       	ChannelStatusControl; 	// Channel Status Control
	BYTE		ChannelStatus[5]; 		// Channel Status Over Write
} ADO_PARAMS, *PADO_PARAMS;

//==================================================================================================
//
// Public Functions
//
void delay_1ms(int ms);
void delay_100ms(int ms100);

//--------------------------------------------------------------------------------------------------
//
// General
//

// All Interface Inital
void EP956_If_Reset(void);
void EP956_AssignKeyAddress(PBYTE inBuf);
BOOL EP956_Load_HDCP_Key(void);
void Handle_AVMute(BOOL Enable);



//--------------------------------------------------------------------------------------------------
//
// MHL Transmiter Interface
//

void MHL_Tx_USB_Mode(BOOL USB_Mode);
BOOL MHL_Tx_MEASURE_1KOHM(void);
BYTE MHL_MSC_Get_Flags(void);
void MHL_Tx_CBUS_Connect(void);
void MHL_Tx_CBUS_Disconnect(void);
MHL_CONNECTION_STATUS MHL_Tx_Connection_Status(void);
void MHL_Clock_Mode(BOOL Packed_Pixel_Mode);

// Read/Write Reg (CBus Responder)
void MHL_MSC_Reg_Update(BYTE offset, BYTE value); // Update Device Cap
BYTE MHL_MSC_Reg_Read(BYTE offset); // Read Status, Read Interrupt
void MHL_MSC_Reg_ReadBlock(BYTE offset, PBYTE readBuf, BYTE length);
void MHL_RCP_RAP_Read(PBYTE pData);

// Send Command (CBus Requester)
BOOL MHL_MSC_Cmd_READ_DEVICE_CAP(BYTE offset, PBYTE pValue);
BOOL MHL_MSC_Cmd_WRITE_STATE(BYTE offset, BYTE value); // same as the SET_INT command
BOOL MHL_MSC_Cmd_MSC_MSG(BYTE SubCmd, BYTE ActionCode);
BOOL MHL_MSC_Cmd_WRITE_BURST(BYTE offset, PBYTE pData, BYTE size);

// Protected Functions
void MHL_MSC_Cmd_ACK(void);
void MHL_MSC_Cmd_ABORT(void);

// Access DDC
SMBUS_STATUS MHL_Tx_DDC_Cmd(BYTE Addr, BYTE *pDatas, WORD Length, SMBUS_MODE Mode);


//--------------------------------------------------------------------------------------------------
//
// HDMI Transmiter Interface
//

// Common
void HDMI_Tx_Power_Down(void);
void HDMI_Tx_Power_Up(void);
BYTE HDMI_Tx_Get_Flags(void);
HDMI_CONNECTION_STATUS HDMI_Tx_HTPLG(BOOL EDID_Polling);
BOOL HDMI_Tx_RSEN(void);
BOOL HDMI_Tx_VSYNC(void);
void HDMI_Tx_HDMI(void);
void HDMI_Tx_DVI(void);

// HDCP
void HDMI_Tx_Mute_Enable(void);
void HDMI_Tx_Mute_Disable(void);
void HDMI_Tx_HDCP_Enable(void);
void HDMI_Tx_HDCP_Disable(void);
void HDMI_Tx_RPTR_Set(void);
void HDMI_Tx_RPTR_Clear(void);
BOOL HDMI_Tx_RI_RDY(void);
void HDMI_Tx_write_AN(BYTE *pAN);
BOOL HDMI_Tx_AKSV_RDY(void);
BOOL HDMI_Tx_read_AKSV(BYTE *pAKSV);
void HDMI_Tx_write_BKSV(BYTE *pBKSV);
BOOL HDMI_Tx_read_RI(BYTE *pRI);
void HDMI_Tx_read_M0(BYTE *pM0);
SMBUS_STATUS HDMI_Tx_Get_Key(BYTE *Key);

// Special for EP956E
void HDMI_Tx_AMute_Enable(void);
void HDMI_Tx_AMute_Disable(void);
void HDMI_Tx_VMute_Enable(void);
void HDMI_Tx_VMute_Disable(void);
void HDMI_Tx_Video_Config(PVDO_PARAMS Params);
void HDMI_Tx_VideoFMT_Config(PVDO_PARAMS Params);
void HDMI_Tx_Audio_Config(PADO_PARAMS Params);


//--------------------------------------------------------------------------------------------------
//
// Hardware Interface
//

// EP956
SMBUS_STATUS EP956_Reg_Read(BYTE ByteAddr, BYTE* Data, WORD Size);
SMBUS_STATUS EP956_Reg_Write(BYTE ByteAddr, BYTE* Data, WORD Size);
SMBUS_STATUS EP956_Reg_Set_Bit(BYTE ByteAddr, BYTE BitMask);
SMBUS_STATUS EP956_Reg_Clear_Bit(BYTE ByteAddr, BYTE BitMask);



#endif // EP956_IF_H


