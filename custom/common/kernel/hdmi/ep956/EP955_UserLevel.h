/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2013
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

  File        :  EP955_UserLevel.h 

  Description :  Head file of EP955 User Level Interface  

\******************************************************************************/

#ifndef EP955_USERLEVLE_H
#define EP955_USERLEVLE_H
#include "CommonInclude.h"
#include "EP956Controller.h"
//==================================================================================================
//
// Protected Data Member
//
typedef enum {
	HDMI_RX_OUTPUT_NORMAL = 0,
	HDMI_RX_OUTPUT_MUXED,
	HDMI_RX_OUTPUT_EMBEDED_SYNC
}RX_OUTPUT_SETTING;

typedef enum {
	HDMI_RX_OUTPUT_RGB = 0,
	HDMI_RX_OUTPUT_YUV444,
	HDMI_RX_OUTPUT_YUV422
}RX_OUTPUT_COLOR;

typedef enum {
	HDMI_RX_OUTPUT_480p = 2,
	HDMI_RX_OUTPUT_720p = 4,
	HDMI_RX_OUTPUT_1080p30 = 34,
	HDMI_RX_OUTPUT_1080p60 = 16
}RX_OUTPUT_RSLT;

typedef enum {
	AUD_FREQ_32000Hz = 0x03,
	AUD_FREQ_44100Hz = 0x00,
	AUD_FREQ_48000Hz = 0x02,
	AUD_FREQ_88200Hz = 0x08,
	AUD_FREQ_96000Hz = 0x0A,
	AUD_FREQ_176400Hz = 0x0C,
	AUD_FREQ_192000Hz = 0x0E,
	AUD_FREQ_Max
} AUD_FREQ;

typedef enum {
	AUD_TYPE_STD = 0,
	AUD_TYPE_DSD,
	AUD_TYPE_HBR,
	AUD_TYPE_Max
} AUD_TYPE;

//==================================================================================================
//
// Public Functions
//
typedef void (*EP955_INT_CALLBACK)(CBUS_LINK_STATE status);
int EP955_UserLevel_Init(EP955_INT_CALLBACK inCallBack);
void EP955_UserLevel_InitMemory(struct i2c_client *client);
void EP955_UserLevel_PowerON(void);
void EP955_UserLevel_PowerOFF(void);
void EP955_UserLevel_Suspend(void);
void EP955_UserLevel_Resume(void);
//==================================================================================================
// rxOutput => [7:4] struct RX_OUTPUT_COLOR | [3:0] struct RX_OUTPUT_SETTING
// inputVIC => 480p : 2, 720p : 4, 1080p : 16 struct RX_OUTPUT_RSLT
void EP955_UserLevel_VideoConfig(BYTE rxOutput, RX_OUTPUT_RSLT inputVIC);
//==================================================================================================
// isAudioIIS => true : IIS, false : SPDIF
// AudioFreq => [7:0] struct AUD_FREQ
void EP955_UserLevel_AudioConfig(BOOL isAudioIIS,BYTE AudioFreq);
//==================================================================================================
int EP955_UserLevel_VideoEnable(BOOL enable);
int EP955_UserLevel_AudioEnable(BOOL enable);
CBUS_LINK_STATE EP955_UserLevel_GetState(void);

//--------------------------------------------------------------------------------------------------
//
// General
//



#endif // EP955_USERLEVLE_H


