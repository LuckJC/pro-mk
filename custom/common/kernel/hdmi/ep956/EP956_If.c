/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

  File        :  EP956_If.c

  Description :  EP956 IIC Interface

\******************************************************************************/

#include "EP956_If.h"
#include "DDC_If.h"
#include "EP956SettingsData.h"

//#include "../hal/ep_hal.h"
#include <linux/i2c.h>

//--------------------------------------------------------------------------------------------------

//#define MSC_DBG
//#define DDC_DBG

#define EP956_ADDR				0x52
#define KEY_ADDR				0xA8

#define MHL_CMD_RETRY_TIME		4
#define CBUS_TIME_OUT_CheckErr	0x50 // 0x01 = 1ms
#define CBUS_TIME_OUT_Normal	0x6E // default = 110ms

extern struct i2c_client *EP955_i2c_client;

typedef enum {
	RQ_STATUS_Success = 0,
	RQ_STATUS_Abort,
	RQ_STATUS_Timeout,
	RQ_STATUS_Error
} RQ_STATUS;

//--------------------------------------------------------------------------------------------------

BYTE Device_Capability_Default[16] = 
{
	0x00, // 0: DEV_STATE		x
	0x20, // 1: MHL_VERSION		-
	0x12, // 2: DEV_CAT			- POW[4], DEV_TYPE[3:0]
	0x01, // 3: ADOPTER_ID_H	- Explore ADOPTER ID is 263, then ADOPTER_ID_H = 0x01
	0x07, // 4: ADOPTER_ID_L	- Explore ADOPTER ID is 263, then ADOPTER_ID_L = 0x07.
	0x3F, // 5: VID_LINK_MODE	- SUPP_VGA[5], SUPP_ISLANDS[4], SUPP_PPIXEL[3], SUPP_YCBCR422[2], SUPP_YCBCR444[1], SUPP_RGB444[0]
	0x03, // 6: AUD_LINK_MODE	- AUD_8CH[1], AUD_2CH[0]
	0x00, // 7: VIDEO_TYPE		x SUPP_VT[7], VT_GAME[3], VT_CINEMA[2], VT_PHOTO[1], VT_GRAPHICS[0]
	0x86, // 8: LOG_DEV_MAP		- LD_GUI[7], LD_SPEAKER[6], LD_RECORD[5], LD_TUNER[4], LD_MEDIA[3], LD_AUDIO[2], LD_VIDEO[1], LD_DISPLAY[0]
	0x0F, // 9: BANDWIDTH		x
	0x07, // A: FEATURE_FLAG	- UCP_RECV_SUPPORT[4], UCP_SEND_SUPPORT[3], SP_SUPPORT[2], RAP_SUPPORT[1], RCP_SUPPORT[0]
	0x00, // B: DEVICE_ID_H		-
	0x00, // C: DEVICE_ID_L		-
	0x10, // D: SCRATCHPAD_SIZE	-
	0x33, // E: INT_STAT_SIZE	- STAT_SIZE[7:4], INT_SIZE[3:0]
	0x00, // F: Reserved
};

//--------------------------------------------------------------------------------------------------

// Temp
static int i, j;
static SMBUS_STATUS status;
static BYTE Temp_Data[16];

// Private Data
//SMBUS_ID IIC_Bus;			// IIC Bus
//SMBUS_PORT IIC_Port;		// IIC Port
//BYTE IIC_EP956_Addr;		// IIC Address
BYTE Chip_Revision;

//
PBYTE EP_Key = NULL;

// Global date for HDMI Transmiter
BOOL is_HDCP_AVMute;
BOOL is_AMute;
BOOL is_VMute;
BOOL is_AVMute_En;
BYTE Cache_EP956_DE_Control, Cache_EP956_Flags, Cache_EP956_MHL_Flags;


// Private Functions
//SMBUS_STATUS IIC_Access(BYTE IICAddr, BYTE RegAddr, void *Data, WORD Size);
//#define IIC_Write IIC_Access
SMBUS_STATUS IIC_Read(BYTE IICAddr, BYTE RegAddr, void *Data, WORD Size);
SMBUS_STATUS EP956_Reg_Read(BYTE ByteAddr, BYTE*  Data, WORD Size);
SMBUS_STATUS EP956_Reg_Write(BYTE ByteAddr, BYTE*  Data, WORD Size);
SMBUS_STATUS EP956_Reg_Set_Bit(BYTE ByteAddr, BYTE BitMask);
SMBUS_STATUS EP956_Reg_Clear_Bit(BYTE ByteAddr, BYTE BitMask);

// CBus Requester
RQ_STATUS MHL_CBus_RQ_Go(void);
RQ_STATUS MHL_CBus_RQ_Check(BYTE Size, PBYTE pData);


uint8_t I2C_ReadBlock(uint8_t deviceID, uint8_t offset,uint8_t *buf, uint8_t len)
{
     printk("hdmi enter %s (0x%02x, 0x%02x, 0x%02x)\n", __func__, deviceID, offset, len);
     return i2c_smbus_read_i2c_block_data(EP955_i2c_client, offset, len, buf);
}

void I2C_WriteBlock(uint8_t deviceID, uint8_t offset, uint8_t *buf, uint8_t len)
{
    printk("hdmi enter %s (0x%02x, 0x%02x, 0x%02x)\n",__func__, deviceID, offset, len);
    i2c_smbus_write_i2c_block_data(EP955_i2c_client, offset, len, buf);
}


//==================================================================================================
//
// Public Function Implementation
//

//--------------------------------------------------------------------------------------------------
// Hardware Interface

void EP956_If_Reset(void)
{
	// Global date for HDMI Transmiter
EP_LOG_FUNCTION_NAME_ENTRY
	is_HDCP_AVMute = 0;
	is_AMute = 1;
	is_VMute = 1;
	is_AVMute_En = 0;
	Cache_EP956_DE_Control = 0x03;
	Cache_EP956_Flags = 0x00;
	Cache_EP956_MHL_Flags = 0x00;

	// Initial Settings
	EP956_Reg_Set_Bit(EP956_Packet_Control, EP956_Packet_Control__VTX0);
	EP956_Reg_Set_Bit(EP956_IIS_Control, EP956_IIS_Control__GC_EN);
	EP956_Reg_Set_Bit(EP956_General_Control_1, EP956_General_Control_1__INT_OD);
	EP956_Reg_Set_Bit(EP956_TX_PHY_Control_0, EP956_TX_PHY_Control_0__TERM_EN);
	
#ifdef MHL_CODE
	EP956_Reg_Set_Bit(EP956_General_Control_8, EP956_General_Control_8__MHL_MODE);
	EP_DEV_DBG("==== Output MHL Mode ====\n");
#else
	EP956_Reg_Clear_Bit(EP956_General_Control_8, EP956_General_Control_8__MHL_MODE);
	EP_DEV_DBG("==== Output HDMI Mode ====\n");
#endif

	// Default Audio Mute
	EP956_Reg_Set_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__AMUTE);
	EP956_Reg_Set_Bit(EP956_Pixel_Repetition_Control, EP956_Pixel_Repetition_Control__CTS_M);
	// Default Video Mute
	EP956_Reg_Set_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__VMUTE);

	//
	// Set Default AVI Info Frame
	//
	memset(Temp_Data, 0x00, 14);

	// Set AVI Info Frame to RGB
	Temp_Data[1] &= 0x60;
	Temp_Data[1] |= 0x00; // RGB

	// Set AVI Info Frame to 601
	Temp_Data[2] &= 0xC0;
	Temp_Data[2] |= 0x40;

	// Write AVI Info Frame
	Temp_Data[0] = 0;
	for(i=1; i<14; ++i) {
		Temp_Data[0] += Temp_Data[i];
	}
	Temp_Data[0] = ~(Temp_Data[0] - 1);
	EP956_Reg_Write(EP956_AVI_Packet, Temp_Data, 14);


	//
	// Set Default ADO Info Frame
	//
	memset(Temp_Data, 0x00, 6);

	// Write ADO Info Frame
	Temp_Data[0] = 0;
	for(i=1; i<6; ++i) {
		Temp_Data[0] += Temp_Data[i];
	}
	Temp_Data[0] = ~(Temp_Data[0] - 1);
	EP956_Reg_Write(EP956_ADO_Packet, Temp_Data, 6);

#ifdef MHL_CODE
	
	
	// Set the CBUS Time-Out time
	Temp_Data[0] = CBUS_TIME_OUT_Normal;
	EP956_Reg_Write(EP955_CBUS_Time_Out, &Temp_Data[0], 1);
	
	// Check chip version
	EP956_Reg_Read(EP955_CBUS_Time_Out, &Temp_Data[0], 1);
	if(Temp_Data[0] == CBUS_TIME_OUT_Normal) {
		// Chip revision 955
		EP_DEV_DBG("EP955 version D\n");
		Chip_Revision = 0x0D;
	}
	else {
		// Chip revision 956
		EP_DEV_DBG("EP956 version C\n");
		Chip_Revision = 0x0C;
	}

	// Check SP_SUPPORT bit
	if(Device_Capability_Default[0x0A] | 0x04) {
		// Enable Write Burst support
		EP956_Reg_Set_Bit(EP956_CBUS_RQ_Control, EP955_CBUS_RQ_Control__WB_SPT); 
	}

	// Set the CBUS Re-try time
	Temp_Data[0] = 0x20; 
	EP956_Reg_Write(EP955_CBUS_TX_Re_Try, &Temp_Data[0], 1);


	//
	// Set Default Device Capability
	//
	memcpy(Temp_Data, Device_Capability_Default, sizeof(Device_Capability_Default));
	EP956_Reg_Write(EP956_CBUS_MSC_Dec_Capability, Temp_Data, sizeof(Device_Capability_Default) );	


	// Set BR-ADJ
	Temp_Data[0] = 50;
	EP956_Reg_Write(EP955_CBUS_BR_ADJ, Temp_Data, 1);

	Temp_Data[0] = 0x42; // 0x21 // 0x81 for external RC
	EP956_Reg_Write(EP956_TX_PHY_Control_0, Temp_Data, 1);

	Temp_Data[0] = 0x03; // 0x2F
	EP956_Reg_Write(EP956_TX_PHY_Control_1, Temp_Data, 1);
#endif
}

void EP956_AssignKeyAddress(PBYTE inBuf)
{
	EP_Key = inBuf;
}

void EP956_GetPartialKey(PBYTE inBuf, int offset, BYTE size)
{
#ifdef LAB_Test
	if(EP_Key != NULL)
	{
		memcpy(inBuf,&EP_Key[offset],size);
	}
#else
	if(offset < 255)
	{
		IIC_Read(KEY_ADDR, offset, inBuf, size);
	}
	else
	{
		IIC_Read(KEY_ADDR+2, (offset - 256), inBuf, size);
	}

#endif
}

BOOL EP956_Load_HDCP_Key(void)
{
	// Upload the key 0-39
	for(i=0; i<40; ++i) {
		Temp_Data[0] = (unsigned char)i;
		status |= EP956_Reg_Write(EP956_Key_Add, Temp_Data, 1);
		//memcpy(Temp_Data,&EP_Key[i*8],7);
		EP956_GetPartialKey(Temp_Data, i*8, 7);
		status |= EP956_Reg_Write(EP956_Key_Data, Temp_Data, 7);
	}
	// Read and check	
//	for(i=0; i<40; ++i) {
//		Temp_Data[0] = (unsigned char)i;
//		status |= EP956_Reg_Write(EP956_Key_Add, Temp_Data, 1);
//		status |= EP956_Reg_Read(EP956_Key_Data, Temp_Data, 7);
//		if((memcmp(Temp_Data,&EP_Key[i*8],7) != 0) || status) {
//			// Test failed
//			return FALSE;
//		}
//	}
	// Upload final KSV 40
	Temp_Data[0] = 40;
	status |= EP956_Reg_Write(EP956_Key_Add, Temp_Data, 1);
	//memcpy(Temp_Data,&EP_Key[40*8],7);
	EP956_GetPartialKey(Temp_Data, 40*8, 7);
	status |= EP956_Reg_Write(EP956_Key_Data, Temp_Data, 7);

	return TRUE;
}

void Handle_AVMute(BOOL Enable)
{
	if(Enable) {
		if(!is_AVMute_En) {
			is_AVMute_En = 1;
		}
	}
	else {
		if(is_AVMute_En) {
			is_AVMute_En = 0;
			EP956_Reg_Clear_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__AV_MUTE);
		}
	}
}

//--------------------------------------------------------------------------------------------------
//
// MHL Transmiter (EP956-Tx Implementation)
//

void MHL_Tx_USB_Mode(BOOL USB_Mode)
{
	if(USB_Mode) {
		// Enable the USB Mux to the USB mode
		EP956_Reg_Set_Bit(EP956_General_Control_8, EP955_General_Control_8__MUX_EN);
		
		// Disable RSEN detect
		EP956_Reg_Set_Bit(EP956_TX_PHY_Control_1, EP956_TX_PHY_Control_1__RSEN_DIS);
		
	}
	else {
		// Disable the USB Mux to the MHL mode
		EP956_Reg_Clear_Bit(EP956_General_Control_8, EP955_General_Control_8__MUX_EN);
		
		// Enable RSEN detect
		EP956_Reg_Clear_Bit(EP956_TX_PHY_Control_1, EP956_TX_PHY_Control_1__RSEN_DIS);
	}
}

BOOL MHL_Tx_MEASURE_1KOHM(void)
{
	BOOL Imp_Match = FALSE;
	
	EP956_Reg_Set_Bit(EP956_CBUS_Connection, EP956_CBUS_Connection__ZM_EN);

	EP956_Reg_Read(EP956_CBUS_Connection, &Temp_Data[0], 1);
	if(Temp_Data[0] & EP956_CBUS_Connection__ZM_RDY) {
		// 1K Ohm found
		Imp_Match = TRUE;
	}

	EP956_Reg_Clear_Bit(EP956_CBUS_Connection, EP956_CBUS_Connection__ZM_EN);

	return Imp_Match;
}

BYTE MHL_MSC_Get_Flags(void)
{
	EP956_Reg_Read(EP956_CBUS_MSC_Interrupt, &Temp_Data[0], 1);
	Temp_Data[0] |= Cache_EP956_MHL_Flags;
	Cache_EP956_MHL_Flags = 0;
	return Temp_Data[0];
}

void MHL_Tx_CBUS_Connect(void)
{
	EP956_Reg_Clear_Bit(EP956_General_Control_1, EP956_General_Control_1__INT_OD);
	EP956_Reg_Clear_Bit(EP956_TX_PHY_Control_1, EP956_TX_PHY_Control_1__RSEN_DIS);
	EP956_Reg_Clear_Bit(EP956_General_Control_1, EP956_General_Control_1__OSC_PD);
	EP956_Reg_Set_Bit(EP956_CBUS_Connection, EP956_CBUS_Connection__CON_BREAK); // Disconnect first
	EP956_Reg_Set_Bit(EP956_CBUS_Connection, EP956_CBUS_Connection__CON_START);
}

void MHL_Tx_CBUS_Disconnect(void)
{
	EP956_Reg_Set_Bit(EP956_General_Control_1, EP956_General_Control_1__INT_OD);
	EP956_Reg_Set_Bit(EP956_TX_PHY_Control_1, EP956_TX_PHY_Control_1__RSEN_DIS);
	EP956_Reg_Set_Bit(EP956_CBUS_Connection, EP956_CBUS_Connection__CON_BREAK);	
	EP956_Reg_Set_Bit(EP956_General_Control_1, EP956_General_Control_1__OSC_PD);
}

MHL_CONNECTION_STATUS MHL_Tx_Connection_Status(void)
{
	// CBUS Connections Status
	EP956_Reg_Read(EP956_CBUS_Connection, &Temp_Data[0], 1);

	if(Temp_Data[0] & EP956_CBUS_Connection__CON_DONE) {
		if(Temp_Data[0] & EP956_CBUS_Connection__CONNECTED) {

			// Hot-Plug Detect
			EP956_Reg_Read(EP956_CBUS_MSC_Interrupt, &Temp_Data[0], 1);
			Cache_EP956_MHL_Flags |= Temp_Data[0] & 0x07;
		
			if(Temp_Data[0] & EP956_CBUS_MSC_Interrupt__HPD_S) {
				return MHL_HOTPLUG_DETECT;
			}
			else {
				return MHL_CBUS_CONNECTED;
			}
		}
		return MHL_NOT_CONNECTED;
	}
	return MHL_CBUS_CONNECTING;
}

void MHL_Clock_Mode(BOOL Packed_Pixel_Mode)
{
	if(Packed_Pixel_Mode) {
		if(Chip_Revision == 0x0C) {
			EP956_Reg_Set_Bit(EP956_General_Control_8, EP956_General_Control_8__PP_MODE);
		}
		else {
			EP956_Reg_Set_Bit(EP956_General_Control_8, EP955_General_Control_8__PP_MODE);
		}
		EP_DEV_DBG("Set Packed Pixel Mode On\n");
	}
	else {
		if(Chip_Revision == 0x0C) {
			EP956_Reg_Clear_Bit(EP956_General_Control_8, EP956_General_Control_8__PP_MODE);
		}
		else {
			EP956_Reg_Clear_Bit(EP956_General_Control_8, EP955_General_Control_8__PP_MODE);
		}
		EP_DEV_DBG("Set Packed Pixel Mode Off\n");
	}
}

void MHL_MSC_Reg_Update(BYTE offset, BYTE value)
{
	if(offset >= 0x40) {
		offset -= 0x40;
		// Scratchpad
		EP956_Reg_Write(EP956_CBUS_MSC_Dec_SrcPad + offset, &value, 1);
	}
	else if(offset >= 0x30) {
		offset -= 0x30;
		// Status Registers
		EP956_Reg_Write(EP956_CBUS_MSC_Dec_Status + offset, &value, 1);
	}
	else if(offset >= 0x20) {
		offset -= 0x20;
		// Interrupt Registers
		EP956_Reg_Write(EP956_CBUS_MSC_Dec_Interrupt + offset, &value, 1);
	}
	else {
		// Capability Registers
		EP956_Reg_Write(EP956_CBUS_MSC_Dec_Capability + offset, &value, 1);
	}
}

BYTE MHL_MSC_Reg_Read(BYTE offset)
{
	if(offset >= 0x40) {
		offset -= 0x40;
		// Scratchpad
		EP956_Reg_Read(EP956_CBUS_MSC_Dec_SrcPad + offset, &Temp_Data[0], 1);
	}
	else if(offset >= 0x30) {
		offset -= 0x30;
		// Status Registers
		EP956_Reg_Read(EP956_CBUS_MSC_Dec_Status + offset, &Temp_Data[0], 1);
	}
	else if(offset >= 0x20) {
		offset -= 0x20;
		// Interrupt Registers
		EP956_Reg_Read(EP956_CBUS_MSC_Dec_Interrupt + offset, &Temp_Data[0], 1);
	}
	else {
		// Capability Registers
		EP956_Reg_Read(EP956_CBUS_MSC_Dec_Capability + offset, &Temp_Data[0], 1);
	}
	return Temp_Data[0];
}

void MHL_MSC_Reg_ReadBlock(BYTE offset, PBYTE readBuf, BYTE length)
{
	if(offset >= 0x40) {
		offset -= 0x40;
		// Scratchpad
		EP956_Reg_Read(EP956_CBUS_MSC_Dec_SrcPad + offset, readBuf, length);
	}
	else if(offset >= 0x30) {
		offset -= 0x30;
		// Status Registers
		EP956_Reg_Read(EP956_CBUS_MSC_Dec_Status + offset, readBuf, length);
	}
	else if(offset >= 0x20) {
		offset -= 0x20;
		// Interrupt Registers
		EP956_Reg_Read(EP956_CBUS_MSC_Dec_Interrupt + offset, readBuf, length);
	}
	else {
		// Capability Registers
		EP956_Reg_Read(EP956_CBUS_MSC_Dec_Capability + offset, readBuf, length);
	}
}

void MHL_RCP_RAP_Read(PBYTE pData)
{
	EP956_Reg_Read(EP956_CBUS_MSC_RAP_RCP, pData, 2);
}

BOOL MHL_MSC_Cmd_READ_DEVICE_CAP(BYTE offset, PBYTE pValue)
{
	RQ_STATUS error;

	for(i=0; i<MHL_CMD_RETRY_TIME; ++i) {

		//
		// Fill in the Command
		//

		// Size (TX Size is not including the Header)
		Temp_Data[1] = 1 | (2<<5); // TX Size | (RX Size << 5)

		// Header
		Temp_Data[2] = EP956_CBUS_RQ_HEADER__MSC_Packet | EP956_CBUS_RQ_HEADER__isCommand;

		// Command
		Temp_Data[3] = 0x61; // MSC_READ_DEVCAP

		// Data
		Temp_Data[4] = offset; // offset

		EP956_Reg_Write(EP956_CBUS_RQ_SIZE, &Temp_Data[1], 4);
	
	
		//
		// Start to Send the Command
		//
	
		// Set the CBUS Time-Out time
		Temp_Data[0] = CBUS_TIME_OUT_CheckErr;
		EP956_Reg_Write(EP955_CBUS_Time_Out, &Temp_Data[0], 1);

		error = MHL_CBus_RQ_Go();
		error = MHL_CBus_RQ_Check(1, pValue);
		
		// Set the CBUS Time-Out time
		Temp_Data[0] = CBUS_TIME_OUT_Normal;
		EP956_Reg_Write(EP955_CBUS_Time_Out, &Temp_Data[0], 1);


		if(!error) {
			if(i>0) EP_DEV_DBG("Retry %d READ_DEVICE_CAP Success\n", i);

			return TRUE;
		}
	}

	EP_DEV_DBG("READ_DEVICE_CAP Fail, offset = 0x%02X\n", offset);
	return FALSE;
}

BOOL MHL_MSC_Cmd_WRITE_STATE(BYTE offset, BYTE value)
{
	RQ_STATUS error;

	for(i=0; i<MHL_CMD_RETRY_TIME; ++i) {

		//
		// Fill in the Command and Parameters
		//

		// Size (TX Size is not including the Header)
		Temp_Data[1] = 2 | (1<<5); // TX Size | (RX Size << 5)

		// Header
		Temp_Data[2] = EP956_CBUS_RQ_HEADER__MSC_Packet | EP956_CBUS_RQ_HEADER__isCommand;

		// Command
		Temp_Data[3] = 0x60; // MSC_WRITE_STATE

		// Data
		Temp_Data[4] = offset; // offset
		Temp_Data[5] = value; // value

		EP956_Reg_Write(EP956_CBUS_RQ_SIZE, &Temp_Data[1], 5);


		//
		// Start to Send the Command
		//

		error = MHL_CBus_RQ_Go();
		//if(!error) {
			error = MHL_CBus_RQ_Check(0, NULL);
			if(!error) {
				if(i>0) EP_DEV_DBG("Retry %d WRITE_STATE Success\n", i);

				return TRUE;
			}
		//}
	}
	EP_DEV_DBG("WRITE_STATE Fail\n");
	return FALSE;
}

BOOL MHL_MSC_Cmd_MSC_MSG(BYTE SubCmd, BYTE ActionCode)
{
	RQ_STATUS error;

	for(i=0; i<MHL_CMD_RETRY_TIME; ++i) {

		//
		// Fill in the Command and Parameters
		//

		// Size (TX Size is not including the Header)
		Temp_Data[1] = 2 | (1<<5); // TX Size | (RX Size << 5)

		// Header
		Temp_Data[2] = EP956_CBUS_RQ_HEADER__MSC_Packet | EP956_CBUS_RQ_HEADER__isCommand;

		// Command
		Temp_Data[3] = 0x68; // MSC_MSG

		// Data
		Temp_Data[4] = SubCmd; // SubCmd
		Temp_Data[5] = ActionCode; // ActionCode

		EP956_Reg_Write(EP956_CBUS_RQ_SIZE, &Temp_Data[1], 5);


		//
		// Start to Send the Command
		//

		error = MHL_CBus_RQ_Go();
		//if(!error) {
			error = MHL_CBus_RQ_Check(0, NULL);
			if(!error) {
				if(i>0) EP_DEV_DBG("Retry %d MSC_MSG Success\n", i);

				return TRUE;
			}
		//}
	}
	EP_DEV_DBG("MSC_MSG Fail\n");
	return FALSE;
}

BOOL MHL_MSC_Cmd_WRITE_BURST(BYTE offset, PBYTE pData, BYTE size)
{
	RQ_STATUS error;
	size = min(size, 16);

if(Chip_Revision == 0x0C) {

	// The code for Chip Revision C
	for(i=0; i<MHL_CMD_RETRY_TIME; ++i) {

		//
		// Fill in the Command and Parameters
		//

		// Size (TX Size is not including the Header)
		Temp_Data[0] = (1+size) | (0<<5); // TX Size | (RX Size << 5)

		// Header
		Temp_Data[1] = EP956_CBUS_RQ_HEADER__MSC_Packet | EP956_CBUS_RQ_HEADER__isCommand;

		// Command
		Temp_Data[2] = 0x6C; // MSC_WRITE_BURST

		// Data[0]
		Temp_Data[3] = offset; // offset
		
		EP956_Reg_Write(EP956_CBUS_RQ_SIZE, Temp_Data, 4);
		
		// Data[1] - [16]
		EP956_Reg_Write(EP956_CBUS_RQ_TD+1, pData, size);


		//
		// Start to Send the Command
		//

		error = MHL_CBus_RQ_Go();
		if(!error) {
		
			//
			// Fill in the Command and Parameters
			//
	
			// Size (TX Size is not including the Header)
			Temp_Data[0] = (0) | (1<<5); // TX Size | (RX Size << 5)
	
			// Header
			Temp_Data[1] = EP956_CBUS_RQ_HEADER__MSC_Packet | EP956_CBUS_RQ_HEADER__isCommand;
	
			// Command
			Temp_Data[2] = 0x32; // EOF
	
			EP956_Reg_Write(EP956_CBUS_RQ_SIZE, Temp_Data, 3);
	
	
			//
			// Start to Send the Command
			//
	
			error = MHL_CBus_RQ_Go();
			if(!error) {
				error = MHL_CBus_RQ_Check(0, NULL);
				if(!error) {
					if(i>0) EP_DEV_DBG("Retry %d WRITE_BURST Success\n", i);
					return TRUE;
				}
			}
		}
		EP_DEV_DBG("WRITE_BURST error\n");
	}
	
}
else {

	// The code for Chip Revision D
	
	for(i=0; i<MHL_CMD_RETRY_TIME; ++i) {

		//
		// Fill in the Command and Parameters
		//

		// Size (TX Size is not including the Header)
		Temp_Data[0] = (2+size) | (1<<5); // TX Size | (RX Size << 5)

		// Header
		Temp_Data[1] = EP956_CBUS_RQ_HEADER__MSC_Packet | EP956_CBUS_RQ_HEADER__isCommand | EP955_CBUS_RQ_HEADER__TDn_isCommand;

		// Command
		Temp_Data[2] = 0x6C; // MSC_WRITE_BURST
		
		// Data[0]
		Temp_Data[3] = offset; // offset
		
		EP956_Reg_Write(EP956_CBUS_RQ_SIZE, Temp_Data, 4);
		
		// Data[1] - [16]
		EP956_Reg_Write(EP956_CBUS_RQ_TD+1, pData, size);
		
		// EOF
		Temp_Data[0] = 0x32; // EOF
		EP956_Reg_Write(EP956_CBUS_RQ_TD+1+size, Temp_Data, 1);


		//
		// Start to Send the Command
		//

		error = MHL_CBus_RQ_Go();
		//if(!error) {
			error = MHL_CBus_RQ_Check(0, NULL);
			if(!error) {
				if(i>0) EP_DEV_DBG("Retry %d WRITE_BURST Success\n", i);
				return TRUE;
			}
		//}
	}
}

	EP_DEV_DBG("WRITE_BURST Fail\n");
	return FALSE;
}

//--------------------------------------------------------------------------------------------------
// MHL Protected(Internal) Commands

void MHL_MSC_Cmd_ABORT(void)
{
	//
	// Fill in the Command
	//

	// Size (TX Size is not including the Header)
	Temp_Data[1] = 0 | (0<<5); // TX Size | (RX Size << 5)

	// Header
	Temp_Data[2] = EP956_CBUS_RQ_HEADER__MSC_Packet | EP956_CBUS_RQ_HEADER__isCommand;

	// Command
	Temp_Data[3] = 0x35; // MSC_ABORT

	EP956_Reg_Write(EP956_CBUS_RQ_SIZE, &Temp_Data[1], 3);


	//
	// Start to Send the Command
	//

	MHL_CBus_RQ_Go();

	delay_1ms(110); // delay 100ms
}

void MHL_DDC_Cmd_ABORT(void)
{
	//
	// Fill in the Command
	//		   

	// Size (TX Size is not including the Header)
	Temp_Data[1] = 0 | (0<<5); // TX Size | (RX Size << 5)

	// Header
	Temp_Data[2] = EP956_CBUS_RQ_HEADER__DDC_Packet | EP956_CBUS_RQ_HEADER__isCommand;

	// Command
	Temp_Data[3] = 0x35; // DDC_ABORT

	EP956_Reg_Write(EP956_CBUS_RQ_SIZE, &Temp_Data[1], 3);


	//
	// Start to Send the Command
	//

	MHL_CBus_RQ_Go();
}

void MHL_DDC_Cmd_EOF(void)
{
	//
	// Fill in the Command
	//		   

	// Size (TX Size is not including the Header)
	Temp_Data[1] = 0 | (0<<5); // TX Size | (RX Size << 5)

	// Header
	Temp_Data[2] = EP956_CBUS_RQ_HEADER__DDC_Packet | EP956_CBUS_RQ_HEADER__isCommand;

	// Command
	Temp_Data[3] = 0x32; // DDC_EOF

	EP956_Reg_Write(EP956_CBUS_RQ_SIZE, &Temp_Data[1], 3);


	//
	// Start to Send the Command
	//

	MHL_CBus_RQ_Go();
}

//--------------------------------------------------------------------------------------------------
// MHL Access DDC

SMBUS_STATUS MHL_Tx_DDC_Cmd(BYTE Addr, BYTE *pDatas, WORD Length, SMBUS_MODE Mode)
{
	WORD i=0;
	SMBUS_STATUS status = SMBUS_STATUS_Success;
	RQ_STATUS error;

	if(!(Mode & SMBUS_SkipStart)) {

		//
		// Fill in the Command and Parameters
		//

		// Size
		Temp_Data[1] = 1 | (1<<5); // TX Size | (RX Size << 5)
	
		// Header
		Temp_Data[2] = EP956_CBUS_RQ_HEADER__DDC_Packet | EP956_CBUS_RQ_HEADER__isCommand;
	
		// Command
		Temp_Data[3] = 0x30; // DDC SOF
	
		// Data
		Temp_Data[4] = Addr; // Addr
	
		EP956_Reg_Write(EP956_CBUS_RQ_SIZE, &Temp_Data[1], 4);


		//
		// Start to Send the Command
		//
		
		error = MHL_CBus_RQ_Go();
		
		//if(!error) {
		
			//
			// Check the read data
			//
		
			EP956_Reg_Read(EP956_CBUS_RQ_ACT_RX_SIZE, &Temp_Data[0], 2);
			
			if(!(Temp_Data[0] & 0x40)) { // CMD0 bit (shall be command but not)
#ifdef DDC_DBG
				EP_DEV_DBG("Err: CBUS DDC1 - CMD_BIT_ERR, 0x%02X\n", Temp_Data[0]);
#endif
				status = SMBUS_STATUS_ArbitrationLoss;
				MHL_DDC_Cmd_ABORT();
			}
			else if( (Temp_Data[0] & 0x03) != 1) { // ACT_RX_SIZE
#ifdef DDC_DBG
				EP_DEV_DBG("Err: CBUS DDC1 - RX_SIZE_ERR\n");	
#endif
				status = SMBUS_STATUS_ArbitrationLoss;
				MHL_DDC_Cmd_ABORT();
			}
			else if(Temp_Data[1] == 0x33) { // ACK
			}
			else if(Temp_Data[1] == 0x34) { // NACK
#ifdef DDC_DBG
				EP_DEV_DBG("Err: CBUS DDC1 - NO_ACK\n");
#endif	
				status = SMBUS_STATUS_NoAct;
				MHL_DDC_Cmd_EOF();
			}
			else if(Temp_Data[1] == 0x35) { // ABORT
#ifdef DDC_DBG
				EP_DEV_DBG("Err: CBUS DDC1 - ABORT\n");	
#endif	
				Mode |= SMBUS_SkipStop;
				status = SMBUS_STATUS_Pending;
			}
			else {
				status = SMBUS_STATUS_ArbitrationLoss;
				MHL_DDC_Cmd_ABORT();
			}

		//}
		//else {
		//	status = SMBUS_STATUS_TimeOut;
		//}			  
	}

	if(status == SMBUS_STATUS_Success) {
		if(Addr & 0x01) {	// Read
			//for(i = 0; (i < Length) && (status == 0); ++i) {

				//
				// Fill in the Command and Parameters
				//
		
				// Size
				Temp_Data[1] = 0 | (1<<5);
			
				// Header
				Temp_Data[2] = EP956_CBUS_RQ_HEADER__DDC_Packet | EP956_CBUS_RQ_HEADER__isCommand;
			
				// Command
				Temp_Data[3] = 0x50; // DDC CONT
			
				EP956_Reg_Write(EP956_CBUS_RQ_SIZE, &Temp_Data[1], 3);
		
				for(i = 0; (i < Length) && (status == 0); ++i) {
		
				//
				// Start to Send the Command
				//
				
				error = MHL_CBus_RQ_Go();
				
				//if(!error) {
				
					//
					// Check the read data
					//
				
					EP956_Reg_Read(EP956_CBUS_RQ_ACT_RX_SIZE, &Temp_Data[0], 2);

					if( (Temp_Data[0] & 0x03) != 1) { // ACT_RX_SIZE
#ifdef DDC_DBG
						EP_DEV_DBG("Err: CBUS DDC2 #%d - RX_SIZE_ERR\n", i);	
#endif	
						status = SMBUS_STATUS_ArbitrationLoss;
						MHL_DDC_Cmd_ABORT();
					}
					if(Temp_Data[0] & 0x40) { // CMD0 bit (shall NOT be command)
#ifdef DDC_DBG
						EP_DEV_DBG("Err: CBUS DDC2 #%d - CMD_BIT_ERR\n", i);
#endif
						if(Temp_Data[1] == 0x35) { // ABORT
							Mode |= SMBUS_SkipStop;
							status = SMBUS_STATUS_Pending;
						}
						else {
							status = SMBUS_STATUS_ArbitrationLoss;
							MHL_DDC_Cmd_ABORT();
						}
					}

					pDatas[i] = Temp_Data[1];
				//}
				//else {
				//	status = SMBUS_STATUS_TimeOut;
				//}							
			}
		}
		else {
			for(i = 0; (i < Length) && (status == 0); ++i) {
				
				//
				// Fill in the Command and Parameters
				//
		
				// Size
				Temp_Data[1] = 0 | (1<<5);
			
				// Header
				Temp_Data[2] = EP956_CBUS_RQ_HEADER__DDC_Packet;
			
				// Command
				Temp_Data[3] = pDatas[i]; // DDC OFFSET / DATA
			
				EP956_Reg_Write(EP956_CBUS_RQ_SIZE, &Temp_Data[1], 3);
		
		
				//
				// Start to Send the Command
				//
				
				error = MHL_CBus_RQ_Go();
				
				//if(!error) {
				
					//
					// Check the read data
					//
				
					EP956_Reg_Read(EP956_CBUS_RQ_ACT_RX_SIZE, &Temp_Data[0], 2);

					if(!(Temp_Data[0] & 0x40)) { // CMD0 bit (shall be command but not)
		#ifdef DDC_DBG
						EP_DEV_DBG("Err: CBUS DDC1 - CMD_BIT_ERR, 0x%02X\n", Temp_Data[0]);
		#endif
						status = SMBUS_STATUS_ArbitrationLoss;
						MHL_DDC_Cmd_ABORT();
					}
					else if( (Temp_Data[0] & 0x03) != 1) { // ACT_RX_SIZE
		#ifdef DDC_DBG
						EP_DEV_DBG("Err: CBUS DDC1 - RX_SIZE_ERR\n");	
		#endif
						status = SMBUS_STATUS_ArbitrationLoss;
						MHL_DDC_Cmd_ABORT();
					}
					else if(Temp_Data[1] == 0x33) { // ACK
					}
					else if(Temp_Data[1] == 0x34) { // NACK
		#ifdef DDC_DBG
						EP_DEV_DBG("Err: CBUS DDC1 - NO_ACK\n");	
		#endif	
						status = SMBUS_STATUS_NoAct;
						MHL_DDC_Cmd_EOF();
					}
					else if(Temp_Data[1] == 0x35) { // ABORT
		#ifdef DDC_DBG
						EP_DEV_DBG("Err: CBUS DDC1 - ABORT\n");	
		#endif	
						Mode |= SMBUS_SkipStop;
						status = SMBUS_STATUS_Pending;
					}
					else {
						status = SMBUS_STATUS_ArbitrationLoss;
						MHL_DDC_Cmd_ABORT();
					}

				//}
				//else {
				//	status = SMBUS_STATUS_TimeOut;
				//}							
			}
		}
	}
	
	if(!(Mode & SMBUS_SkipStop)) {
	
// EP957 has a speed up function to send two command at once.

		//
		// Fill in the Command and Parameters
		//

		// Size
		Temp_Data[1] = 1 | (0<<5);
	
		// Header
		Temp_Data[2] = EP956_CBUS_RQ_HEADER__DDC_Packet | EP956_CBUS_RQ_HEADER__isCommand | EP955_CBUS_RQ_HEADER__TD0_isCommand;
	
		// Command
		Temp_Data[3] = 0x51; // DDC STOP
		Temp_Data[4] = 0x32; // DDC EOF
	
		EP956_Reg_Write(EP956_CBUS_RQ_SIZE, &Temp_Data[1], 4);


		//
		// Start to Send the Command
		//
		
		error = MHL_CBus_RQ_Go();
		
		//if(error) {
		//	status = SMBUS_STATUS_TimeOut;
		//}
	}

	return status;
}


//--------------------------------------------------------------------------------------------------
//
// HDMI Transmiter (EP956-Tx Implementation)
//

void HDMI_Tx_Power_Down(void)
{
	// Software power down
	EP956_Reg_Clear_Bit(EP956_General_Control_1, EP956_General_Control_1__PU);
	EP_DEV_DBG("Set TX Power Down\n");
}

void HDMI_Tx_Power_Up(void)
{
	// Software power up
	EP956_Reg_Set_Bit(EP956_General_Control_1, EP956_General_Control_1__PU);	
	EP_DEV_DBG("Set TX Power Up\n");	
}

BYTE HDMI_Tx_Get_Flags(void)
{
	EP956_Reg_Read(EP956_General_Control_2, &Temp_Data[0], 1);
	Temp_Data[0] |= Cache_EP956_Flags;
	Cache_EP956_Flags = 0;
	return Temp_Data[0];
}

// return 0: No Connection.
// return 1: Hot-Plug signal is detected. 
// return 2: Connection is confirmed by avaliable EDID.
HDMI_CONNECTION_STATUS HDMI_Tx_HTPLG(BOOL EDID_Polling)
{
	// Software HotPlug Detect
	EP956_Reg_Read(EP956_General_Control_2, &Temp_Data[0], 1);
	Cache_EP956_Flags |= Temp_Data[0] & 0x3F;

	if(Temp_Data[0] & EP956_General_Control_2__HTPLG) return HDMI_HOTPLUG_DETECT;

	// This is for old DVI monitor compatibility. For HDMI TV, there is no need to poll the EDID.
	if(EDID_Polling) {
		if(Downstream_Rx_poll_EDID()) return HDMI_EDID_DETECT;
	}
	return HDMI_NO_CONNECT;
}

BOOL HDMI_Tx_RSEN(void)
{
	// RSEN Detect
	EP956_Reg_Read(EP956_General_Control_2, &Temp_Data[0], 1);
	Cache_EP956_Flags |= Temp_Data[0] & 0x3F;

	return (Temp_Data[0] & EP956_General_Control_2__RSEN)? TRUE:FALSE;
}

BOOL HDMI_Tx_VSYNC(void)
{
	EP956_Reg_Read(EP956_Pixel_Repetition_Control, &Temp_Data[0], 1);

	return (Temp_Data[0] & EP956_Pixel_Repetition_Control__VSYNC)? TRUE:FALSE;
}

void HDMI_Tx_HDMI(void)
{
	EP956_Reg_Set_Bit(EP956_General_Control_4, EP956_General_Control_4__HDMI);
	EP_DEV_DBG("Set to HDMI mode\n");
}

void HDMI_Tx_DVI(void)
{
	EP956_Reg_Clear_Bit(EP956_General_Control_4, EP956_General_Control_4__HDMI);
	EP_DEV_DBG("Set to DVI mode\n");
}

//------------------------------------
// HDCP

void HDMI_Tx_Mute_Enable(void)
{
	is_HDCP_AVMute = 1;
	EP956_Reg_Set_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__AMUTE | EP956_Color_Space_Control__VMUTE);
	EP956_Reg_Clear_Bit(EP956_IIS_Control, EP956_IIS_Control__AUDIO_EN);
	EP956_Reg_Set_Bit(EP956_Pixel_Repetition_Control, EP956_Pixel_Repetition_Control__CTS_M);
	if(is_AVMute_En) EP956_Reg_Set_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__AV_MUTE);
}

void HDMI_Tx_Mute_Disable(void)
{
	is_HDCP_AVMute = 0;
	
	if(!is_AMute) {
		EP956_Reg_Clear_Bit(EP956_Pixel_Repetition_Control, EP956_Pixel_Repetition_Control__CTS_M);
		EP956_Reg_Set_Bit(EP956_IIS_Control, EP956_IIS_Control__AUDIO_EN);
		EP956_Reg_Clear_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__AMUTE);
	}
	if(!is_VMute) {
		EP956_Reg_Clear_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__VMUTE);
	}
	if(is_AVMute_En) EP956_Reg_Clear_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__AV_MUTE);
}

void HDMI_Tx_HDCP_Enable(void)
{
	EP956_Reg_Set_Bit(EP956_General_Control_5, EP956_General_Control_5__ENC_EN);
}

void HDMI_Tx_HDCP_Disable(void)
{
	EP956_Reg_Clear_Bit(EP956_General_Control_5, EP956_General_Control_5__ENC_EN);
}

void HDMI_Tx_RPTR_Set(void)
{
	EP956_Reg_Set_Bit(EP956_General_Control_5, EP956_General_Control_5__RPTR);
}

void HDMI_Tx_RPTR_Clear(void)
{
	EP956_Reg_Clear_Bit(EP956_General_Control_5, EP956_General_Control_5__RPTR);
}

void HDMI_Tx_write_AN(BYTE *pAN)
{
	EP956_Reg_Write(EP956_AN, pAN, 8);
}

BOOL HDMI_Tx_AKSV_RDY(void)
{
	status = EP956_Reg_Read(EP956_General_Control_5, &Temp_Data[0], 1);
	if(status != SMBUS_STATUS_Success) {
		EP_DEV_DBG("Err: AKSV RDY\n", (int)status);
		return FALSE;
	}
	return (Temp_Data[0] & EP956_General_Control_5__AKSV_RDY)? TRUE:FALSE;
}

BOOL HDMI_Tx_read_AKSV(BYTE *pAKSV)
{
	status = EP956_Reg_Read(EP956_AKSV, pAKSV, 5);
	if(status != SMBUS_STATUS_Success) {
		EP_DEV_DBG("Err: AKSV read\n", (int)status);
		return FALSE;
	}
	
	i = 0;
	j = 0;
	while (i < 5) {
		Temp_Data[0] = 1;
		while (Temp_Data[0]) {
			if (pAKSV[i] & Temp_Data[0]) j++;
			Temp_Data[0] <<= 1;
		}
		i++;
	}
	if(j != 20) {
		EP_DEV_DBG("Err: AKSV read - Key Wrong\n");
		return FALSE;
	}
	return TRUE;
}

void HDMI_Tx_write_BKSV(BYTE *pBKSV) 
{
	EP956_Reg_Write(EP956_BKSV, pBKSV, 5);
}

BOOL HDMI_Tx_RI_RDY(void)
{
	EP956_Reg_Read(EP956_General_Control_5, &Temp_Data[0], 1);
	return (Temp_Data[0] & EP956_General_Control_5__RI_RDY)? TRUE:FALSE;
}

BOOL HDMI_Tx_read_RI(BYTE *pRI)
{
	status = EP956_Reg_Read(EP956_RI, pRI, 2);
	if(status != SMBUS_STATUS_Success) {
		EP_DEV_DBG("Err: Tx Ri read\n", (int)status);
		return FALSE;
	}
	return TRUE;
}

void HDMI_Tx_read_M0(BYTE *pM0) 
{
	status = EP956_Reg_Read(EP956_M0, pM0, 8);
}

SMBUS_STATUS HDMI_Tx_Get_Key(BYTE *Key)
{
	#define HDCP_EEPROM_LESS256		0xA8
	#define HDCP_EEPROM_OVER256		0xAA

	I2C_ReadBlock(HDCP_EEPROM_LESS256,   0,   &(Key[0]), 128);
	I2C_ReadBlock(HDCP_EEPROM_LESS256, 128, &(Key[128]), 128);
	I2C_ReadBlock(HDCP_EEPROM_OVER256,   0, &(Key[128]), 72);
	return 1;//IIC_Read(KEY_ADDR, 0, Key, 328);
}

//------------------------------------
// Special for config

void HDMI_Tx_AMute_Enable(void)
{
	if(!is_AMute) {
		is_AMute = 1;
		EP956_Reg_Set_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__AMUTE);
		EP956_Reg_Clear_Bit(EP956_IIS_Control, EP956_IIS_Control__AUDIO_EN);
		EP956_Reg_Set_Bit(EP956_Pixel_Repetition_Control, EP956_Pixel_Repetition_Control__CTS_M);
	}
}

void HDMI_Tx_AMute_Disable(void)
{
	if(is_AMute) {
		is_AMute = 0; 
		if(!is_HDCP_AVMute) {
			EP956_Reg_Clear_Bit(EP956_Pixel_Repetition_Control, EP956_Pixel_Repetition_Control__CTS_M);
			EP956_Reg_Set_Bit(EP956_IIS_Control, EP956_IIS_Control__AUDIO_EN);
			EP956_Reg_Clear_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__AMUTE);
		}
	}
}

void HDMI_Tx_VMute_Enable(void)
{		
	if(!is_VMute) {
		is_VMute = 1; 
		
		EP956_Reg_Set_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__VMUTE);
		if(is_AVMute_En) EP956_Reg_Set_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__AV_MUTE);
	}
}

void HDMI_Tx_VMute_Disable(void)
{
	if(is_VMute) {
		is_VMute = 0; 
		if(!is_HDCP_AVMute) {
			EP956_Reg_Clear_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__VMUTE);
			if(is_AVMute_En) EP956_Reg_Clear_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__AV_MUTE);
		}
	}
}

void HDMI_Tx_Video_Config(PVDO_PARAMS Params)
{
	EP_DEV_DBG("Start Tx Video Config\n");

	//
	// Disable Video
	//
	EP956_Reg_Clear_Bit(EP956_IIS_Control, EP956_IIS_Control__AVI_EN);

	//
	// Video Settings
	//
	// Interface
	EP956_Reg_Read(EP956_General_Control_3, Temp_Data, 1);
	Temp_Data[0] &= ~0xF0;
	Temp_Data[0] |= Params->Interface & 0xF0;
	EP956_Reg_Write(EP956_General_Control_3, Temp_Data, 1);

	EP956_Reg_Read(EP956_General_Control_1, Temp_Data, 1);
	Temp_Data[0] &= ~0x0E;
	Temp_Data[0] |= Params->Interface & 0x0E;
	EP956_Reg_Write(EP956_General_Control_1, Temp_Data, 1);

	if(Params->Interface & 0x01) {
		EP956_Reg_Set_Bit(EP956_General_Control_4, EP956_General_Control_4__FMT12);
	}
	else {
		EP956_Reg_Clear_Bit(EP956_General_Control_4, EP956_General_Control_4__FMT12);
	}

	// Sync Mode
	switch(Params->SyncMode) {
		default:
	 	case SYNCMODE_HVDE:
			// Disable E_SYNC
			EP956_Reg_Clear_Bit(EP956_General_Control_4, EP956_General_Control_4__E_SYNC);
			// Disable DE_GEN
			Cache_EP956_DE_Control &= ~EP956_DE_Control__DE_GEN;
			//EP956_Reg_Write(EP956_DE_Control, &Cache_EP956_DE_Control, 1);
	
			// Regular VSO_POL, HSO_POL
			if((Params->HVPol & VNegHPos) != (EP956_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VNegHPos)) { // V
				Cache_EP956_DE_Control |= EP956_DE_Control__VSO_POL; // Invert
			}
			else {
				Cache_EP956_DE_Control &= ~EP956_DE_Control__VSO_POL;
			}
			if((Params->HVPol & VPosHNeg) != (EP956_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VPosHNeg)) { // H
				Cache_EP956_DE_Control |= EP956_DE_Control__HSO_POL; // Invert
			}
			else {
				Cache_EP956_DE_Control &= ~EP956_DE_Control__HSO_POL;
			}
			EP_DEV_DBG("Set Sync mode to DE mode\n");
			break;

		case SYNCMODE_HV:
			// Disable E_SYNC
			EP956_Reg_Clear_Bit(EP956_General_Control_4, EP956_General_Control_4__E_SYNC);
			// Enable DE_GEN
			Cache_EP956_DE_Control |= EP956_DE_Control__DE_GEN;
			//EP956_Reg_Write(EP956_DE_Control, &Cache_EP956_DE_Control, 1);

			// Regular VSO_POL, HSO_POL
			if((Params->HVPol & VNegHPos) != (EP956_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VNegHPos)) { // V
				Cache_EP956_DE_Control |= EP956_DE_Control__VSO_POL; // Invert
			}
			else {
				Cache_EP956_DE_Control &= ~EP956_DE_Control__VSO_POL;
			}
			if((Params->HVPol & VPosHNeg) != (EP956_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VPosHNeg)) { // H
				Cache_EP956_DE_Control |= EP956_DE_Control__HSO_POL; // Invert
			}
			else {
				Cache_EP956_DE_Control &= ~EP956_DE_Control__HSO_POL;
			}

			// Set DE generation params
			if(Params->VideoSettingIndex < EP956_VDO_Settings_Max) {
				Cache_EP956_DE_Control &= ~0x03;
				Cache_EP956_DE_Control |= EP956_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY >> 8;

				Temp_Data[0] = EP956_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY & 0xFF;
				EP956_Reg_Write(EP956_DE_DLY, Temp_Data, 1);

				Temp_Data[0] = EP956_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_TOP;
				EP956_Reg_Write(EP956_DE_TOP, Temp_Data, 1);
				
				Temp_Data[0] = EP956_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT & 0xFF;
				Temp_Data[1] = EP956_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT >> 8;
				EP956_Reg_Write(EP956_DE_CNT, Temp_Data, 2);

				Temp_Data[0] = EP956_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN & 0xFF;
				Temp_Data[1] = EP956_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN >> 8;
				EP956_Reg_Write(EP956_DE_LIN, Temp_Data, 2);
	
				EP_DEV_DBG("Update DE_GEN params %u", (WORD)EP956_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY);
				EP_DEV_DBG(", %u", (WORD)EP956_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT);
				EP_DEV_DBG(", %u", (WORD)EP956_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_TOP);
				EP_DEV_DBG(", %u", (WORD)EP956_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN);
				EP_DEV_DBG("\n");
			}
			else {
				EP_DEV_DBG("Err: VideoCode overflow DE_GEN table\n");
			}
			break;
			
		case SYNCMODE_Embeded:
			// Disable DE_GEN
			Cache_EP956_DE_Control &= ~EP956_DE_Control__DE_GEN;
			//EP956_Reg_Write(EP956_DE_Control, &Cache_EP956_DE_Control, 1);
			// Enable E_SYNC
			EP956_Reg_Set_Bit(EP956_General_Control_4, EP956_General_Control_4__E_SYNC);
			
			// Set E_SYNC params
			if(Params->VideoSettingIndex < EP956_VDO_Settings_Max) {
				USHORT TempUSHORT;

				Temp_Data[0] = EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.CTL;
				EP956_Reg_Write(EP956_Embedded_Sync, Temp_Data, 1);

				TempUSHORT = EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_DLY;
				if(!(Params->Interface & 0x04)) { // Mux Mode
					TempUSHORT += 2;
				}
				Temp_Data[0] = TempUSHORT & 0xFF;
				Temp_Data[1] = TempUSHORT >> 8;
				EP956_Reg_Write(EP956_H_Delay, Temp_Data, 2);

				Temp_Data[0] = EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH & 0xFF;
				Temp_Data[1] = EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH >> 8;
				EP956_Reg_Write(EP956_H_Width, Temp_Data, 2);

				Temp_Data[0] = EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_DLY;
				EP956_Reg_Write(EP956_V_Delay, Temp_Data, 1);

				EP956_Reg_Write(EP956_V_Width, &EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_WIDTH, 1);

				Temp_Data[0] = EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST & 0xFF;
				Temp_Data[1] = EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST >> 8;
				EP956_Reg_Write(EP956_V_Off_Set, Temp_Data, 2);
	
				EP_DEV_DBG("Update VIC %02X E_SYNC params 0x%02X", (WORD)Params->VideoSettingIndex,(WORD)EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.CTL);
				EP_DEV_DBG(", %u", (WORD)EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_DLY);
				EP_DEV_DBG(", %u", (WORD)EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH);
				EP_DEV_DBG(", %u", (WORD)EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_DLY);
				EP_DEV_DBG(", %u", (WORD)EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_WIDTH);
				EP_DEV_DBG(", %u", (WORD)EP956_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST);
				EP_DEV_DBG("\n");

				// Regular VSO_POL, HSO_POL
				if(EP956_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VNegHPos) { // VNeg?
					Cache_EP956_DE_Control |= EP956_DE_Control__VSO_POL;
				}
				else {
					Cache_EP956_DE_Control &= ~EP956_DE_Control__VSO_POL;
				}
				if(EP956_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VPosHNeg) { // HNeg?
					Cache_EP956_DE_Control |= EP956_DE_Control__HSO_POL;
				}
				else {
					Cache_EP956_DE_Control &= ~EP956_DE_Control__HSO_POL;
				}
			}
			else {
				EP_DEV_DBG("Err: VideoCode overflow E_SYNC table\n");
			}
			break;
	}
	EP956_Reg_Write(EP956_DE_Control, &Cache_EP956_DE_Control, 1);
	
	// Pixel Repetition
	EP956_Reg_Read(EP956_Pixel_Repetition_Control, Temp_Data, 1);
	Temp_Data[0] &= ~EP956_Pixel_Repetition_Control__PR;
	if(Params->VideoSettingIndex < EP956_VDO_Settings_Max) {
		Temp_Data[0] |= EP956_VDO_Settings[Params->VideoSettingIndex].AR_PR & 0x03;
	}
	EP956_Reg_Write(EP956_Pixel_Repetition_Control, Temp_Data, 1);

	// Config Format
	HDMI_Tx_VideoFMT_Config(Params);


	//
	// Enable Video
	//
	EP956_Reg_Set_Bit(EP956_IIS_Control, EP956_IIS_Control__AVI_EN);
}

void HDMI_Tx_VideoFMT_Config(PVDO_PARAMS Params)
{
	// Color Space
	switch(Params->FormatIn) {
		default:
	 	case COLORFORMAT_RGB:
			EP956_Reg_Clear_Bit(EP956_General_Control_4, EP956_General_Control_4__YCC_IN | EP956_General_Control_4__422_IN);
			EP_DEV_DBG("Set to RGB In\n");
			break;
	 	case COLORFORMAT_YCC444:
			EP956_Reg_Set_Bit(EP956_General_Control_4, EP956_General_Control_4__YCC_IN);
			EP956_Reg_Clear_Bit(EP956_General_Control_4, EP956_General_Control_4__422_IN);
			EP_DEV_DBG("Set to YCC444 In\n");
			break;
	 	case COLORFORMAT_YCC422:
			EP956_Reg_Set_Bit(EP956_General_Control_4, EP956_General_Control_4__YCC_IN | EP956_General_Control_4__422_IN);
			EP_DEV_DBG("Set to YCC422 In\n");
			break;
	}
	
	switch(Params->FormatOut) {
		default:
	 	case COLORFORMAT_RGB:
			// Set to RGB
			EP956_Reg_Clear_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__YCC_OUT | EP956_Color_Space_Control__422_OUT);
			EP_DEV_DBG("Set to RGB Out\n");
			break;

	 	case COLORFORMAT_YCC444:
			// Set to YCC444
			EP956_Reg_Set_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__YCC_OUT);
			EP956_Reg_Clear_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__422_OUT);
			EP_DEV_DBG("Set to YCC444 Out\n");
			break;
	 	case COLORFORMAT_YCC422:
			// Set to YCC422
			EP956_Reg_Set_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__YCC_OUT | EP956_Color_Space_Control__422_OUT);
			EP_DEV_DBG("Set to YCC422 Out\n");
			break;
	}

	// Range Convert
	if(Params->VideoSettingIndex < EP956_VDO_Settings_IT_Start) { 
		// CE Timing (RGB Limit -> YCC Full)
		EP956_Reg_Set_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__YCC_Range); // In/Output limit range RGB
	}
	else { 
		// IT Timing (RGB Full -> YCC Limit)
		EP956_Reg_Clear_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__YCC_Range); // In/Output full range RGB
	}

	// Color Space
	switch(Params->ColorSpace) {
		default:
	 	case COLORSPACE_601:
	 	case COLORSPACE_xvYCC601:
			// Set to 601
			EP956_Reg_Clear_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__COLOR);
			EP_DEV_DBG("Set to 601 color definition\n");
			break;

	 	case COLORSPACE_709:
	 	case COLORSPACE_xvYCC709:
			// Set to 709
			EP956_Reg_Set_Bit(EP956_Color_Space_Control, EP956_Color_Space_Control__COLOR);
			EP_DEV_DBG("Set to 709 color definition\n");
			break;
	}

	//
	// Update AVI Info Frame
	//
	// Read AVI Info Frame
	memset(Temp_Data, 0x00, 14);
	//Temp_Data[1] &= 0x60;
	switch(Params->FormatOut) {
		default:
	 	case COLORFORMAT_RGB:
			// Set AVI Info Frame to RGB
			Temp_Data[1] |= 0x00; // RGB
			break;

	 	case COLORFORMAT_YCC444:
			// Set AVI Info Frame to RGB
			Temp_Data[1] |= 0x40; // YCC 444
			break;
	 	case COLORFORMAT_YCC422:
			// Set AVI Info Frame to RGB
			Temp_Data[1] |= 0x20; // YCC 422
			break;
	}
	Temp_Data[1] |= 0x10; // Active Format Information
	//Temp_Data[2] &= 0xC0;
	switch(Params->ColorSpace) {
		default:
	 	case COLORSPACE_601:
			// Set AVI Info Frame to 601
			Temp_Data[2] |= 0x40;
			break;

	 	case COLORSPACE_709:
			// Set AVI Info Frame to 709
			Temp_Data[2] |= 0x80;
			break;

	 	case COLORSPACE_xvYCC601:
			// Set AVI Info Frame to xvYCC 601
			Temp_Data[2] |= 0xC0;
			break;

	 	case COLORSPACE_xvYCC709:
			// Set AVI Info Frame to xvYCC 709
			Temp_Data[2] |= 0xC0;
			Temp_Data[3] |= 0x10;
			break;
	}
	//Temp_Data[2] &= 0x30;
	if(Params->VideoSettingIndex < EP956_VDO_Settings_Max) {
		Temp_Data[2] |= EP956_VDO_Settings[Params->VideoSettingIndex].AR_PR & 0x30;
	}
	//Temp_Data[2] &= 0x0F;
	Temp_Data[2] |= Params->AFARate & 0x0F;
	if(Params->VideoSettingIndex < EP956_VDO_Settings_IT_Start) {
		Temp_Data[4] |= EP956_VDO_Settings[Params->VideoSettingIndex].VideoCode;
	}
	if(Params->VideoSettingIndex < EP956_VDO_Settings_Max) {
		Temp_Data[5] |= (EP956_VDO_Settings[Params->VideoSettingIndex].AR_PR & 0x0C) >> 2;
	}

	// Write AVI Info Frame
	Temp_Data[0] = 0x91;
	for(i=1; i<6; ++i) {
		Temp_Data[0] += Temp_Data[i];
	}
	Temp_Data[0] = ~(Temp_Data[0] - 1);
	EP956_Reg_Write(EP956_AVI_Packet, Temp_Data, 14);
#ifdef DBG
	EP_DEV_DBG("AVI Info: ");
	for(i=0; i<6; ++i) {
		EP_DEV_DBG("0x%02X,", (int)Temp_Data[i] );
	}
	EP_DEV_DBG("\n");
#endif
}

void HDMI_Tx_Audio_Config(PADO_PARAMS Params)
{
	BYTE N_CTS_Index;
	unsigned long N_Value, CTS_Value;
	ADSFREQ FinalFrequency;
	BYTE FinalADSRate, FinalSF;

	EP_DEV_DBG("Start Tx Audio Config\n");

	//
	// Audio Settings
	//
	EP_DEV_DBG("Params->AudioType %X+\n",Params->AudioType);
	// Audio Type Reset
	EP956_Reg_Read(EP956_General_Control_8, Temp_Data, 1);
	Temp_Data[0] &= ~EP956_General_Control_8__AUDIO_INPUT;
	Temp_Data[0] |= ((~Params->AudioType)&0x03) << 4;
	EP956_Reg_Write(EP956_General_Control_8, Temp_Data, 1);

	// Update WS_M, WS_POL, SCK_POL
	EP956_Reg_Read(EP956_IIS_Control, Temp_Data, 1);
	Temp_Data[0] &= ~0x07;
	Temp_Data[0] |= Params->Interface & 0x07;
	EP956_Reg_Write(EP956_IIS_Control, Temp_Data, 1);

	// Update AUDIO_INPUT (AuioType)
	EP956_Reg_Read(EP956_General_Control_8, Temp_Data, 1);
	Temp_Data[0] &= ~EP956_General_Control_8__AUDIO_INPUT;
	//Params->AudioType = 1;
	Temp_Data[0] |= Params->AudioType << 4;
	EP956_Reg_Write(EP956_General_Control_8, Temp_Data, 1);

	// Update Channel Status	
	//Params->AudioType = 1;
	EP_DEV_DBG("Params->AudioType %X-\n",Params->AudioType);	
	switch(Params->AudioType) {
	
		default:
		case AUDIO_TYPE_SPDIF:

			// No Downsample
			FinalADSRate = 0;
			FinalFrequency = Params->InputFrequency;
			FinalSF = 0; // Info for Audio Infoframe
	
			// Update Flat
			EP956_Reg_Clear_Bit(EP956_Packet_Control, EP956_Packet_Control__FLAT);

			// Disable Down Sample and Bypass Channel Status
			EP956_Reg_Clear_Bit(EP956_Pixel_Repetition_Control, EP956_Pixel_Repetition_Control__ADSR | EP956_Pixel_Repetition_Control__CS_M);
			
			// Channel Status Overwrite
			if(Params->ChannelStatusControl & 0x08) {
				// Set Channel Status
				memcpy(Temp_Data, Params->ChannelStatus, sizeof(Params->ChannelStatus));
				EP956_Reg_Write(EP956_Channel_Status, Temp_Data, 5);

				EP956_Reg_Set_Bit(EP956_Pixel_Repetition_Control, EP956_Pixel_Repetition_Control__CS_M);
			}

			Params->ChannelNumber = 0; // Speaker Mapping apply only to multi-channel uncompressed audio.
			EP_DEV_DBG("Set to SPDIF\n");
			break;

		case AUDIO_TYPE_IIS:

			// Downsample Convert
			FinalADSRate = Params->ADSRate;
			switch(Params->ADSRate) {
				default:
				case 0: // Bypass
					FinalADSRate = 0;
					FinalFrequency = Params->InputFrequency;
					break;
				case 1: // 1/2
					switch(Params->InputFrequency) {
						default: // Bypass
							FinalADSRate = 0;
							FinalFrequency = Params->InputFrequency;
							break;
						case ADSFREQ_88200Hz:
							FinalFrequency = ADSFREQ_44100Hz;
							break;
						case ADSFREQ_96000Hz:
							FinalFrequency = ADSFREQ_48000Hz;
							break;
						case ADSFREQ_176400Hz:
							FinalFrequency = ADSFREQ_88200Hz;
							break;
						case ADSFREQ_192000Hz:
							FinalFrequency = ADSFREQ_96000Hz;
							break;
					}
					break;
				case 2: // 1/3
					switch(Params->InputFrequency) {
						default: // Bypass
							FinalADSRate = 0;
							FinalFrequency = Params->InputFrequency;
							break;
						case ADSFREQ_96000Hz:
							FinalFrequency = ADSFREQ_32000Hz;
							break;
					}
					break;
				case 3: // 1/4
					switch(Params->InputFrequency) {
						default: // Bypass
							FinalADSRate = 0;
							FinalFrequency = Params->InputFrequency;
							break;
						case ADSFREQ_176400Hz:
							FinalFrequency = ADSFREQ_44100Hz;
							break;
						case ADSFREQ_192000Hz:
							FinalFrequency = ADSFREQ_48000Hz;
							break;
					}
					break;
			}
			FinalSF = 0; // Info for Audio Infoframe
		
			// Update Down Sample FinalADSRate
			EP956_Reg_Read(EP956_Pixel_Repetition_Control, Temp_Data, 1);
			Temp_Data[0] &= ~0x30;
			Temp_Data[0] |= (FinalADSRate << 4) & 0x30;
			EP956_Reg_Write(EP956_Pixel_Repetition_Control, Temp_Data, 1);

			// Update Flat
			EP956_Reg_Clear_Bit(EP956_Packet_Control, EP956_Packet_Control__FLAT);
		
			// Channel Status
			if(Params->ChannelStatusControl & 0x08) {
				// Set Channel Status
				memcpy(Temp_Data, Params->ChannelStatus, sizeof(Params->ChannelStatus));
				EP956_Reg_Write(EP956_Channel_Status, Temp_Data, 5);
			}
			else {
				// Set Channel Status
				memset(Temp_Data, 0x00, 5);
				Temp_Data[0] = 0x00;//(Params->ChannelStatusControl & 0x10)? 0x04:0x00;
				Temp_Data[1] = 0x00; 				// Category code ??
				Temp_Data[2] = 0x00; 				// Channel number ?? | Source number ??
				Temp_Data[3] = FinalFrequency; 	// Clock accuracy ?? | Sampling frequency
				Temp_Data[4] = 0x01; 				// Original sampling frequency ?? | Word length ??
				EP956_Reg_Write(EP956_Channel_Status, Temp_Data, 5);
			}
			EP956_Reg_Set_Bit(EP956_Pixel_Repetition_Control, EP956_Pixel_Repetition_Control__CS_M);
		
			EP_DEV_DBG("Set to IIS\n");
			break;
	}

	// Set CTS/N
	if(Params->VideoSettingIndex < EP956_VDO_Settings_Max) {	
		N_CTS_Index = EP956_VDO_Settings[Params->VideoSettingIndex].Pix_Freq_Type;
		if(EP956_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.Vprd % 500) { // 59.94/60 Hz
			N_CTS_Index += Params->VFS;	
			EP_DEV_DBG("N_CTS_Index Shift = %d\n", (int)Params->VFS);				 
		}
	}
	else {
		EP_DEV_DBG("Use default N_CTS_Index\n");
		N_CTS_Index = PIX_FREQ_25200KHz;
	}
	switch(FinalFrequency) {
		default:
		case ADSFREQ_32000Hz:
			EP_DEV_DBG("Set to 32KHz");
			N_Value = N_CTS_32K[N_CTS_Index].N;
			CTS_Value = N_CTS_32K[N_CTS_Index].CTS;
			break;
		case ADSFREQ_44100Hz:
			EP_DEV_DBG("Set to 44.1KHz");
			N_Value = N_CTS_44K1[N_CTS_Index].N;
			CTS_Value = N_CTS_44K1[N_CTS_Index].CTS;
			break;
		case ADSFREQ_48000Hz:
			EP_DEV_DBG("Set to 48KHz");
			N_Value = N_CTS_48K[N_CTS_Index].N;
			CTS_Value = N_CTS_48K[N_CTS_Index].CTS;
			break;
		case ADSFREQ_88200Hz:
			EP_DEV_DBG("Set to 88.2KHz");
			N_Value = N_CTS_44K1[N_CTS_Index].N * 2;
			CTS_Value = N_CTS_44K1[N_CTS_Index].CTS * 2;
			break;
		case ADSFREQ_96000Hz:
			EP_DEV_DBG("Set to 96KHz");
			N_Value = N_CTS_48K[N_CTS_Index].N * 2;
			CTS_Value = N_CTS_48K[N_CTS_Index].CTS * 2;
			break;
		case ADSFREQ_176400Hz:
			EP_DEV_DBG("Set to 176.4KHz");
			N_Value = N_CTS_44K1[N_CTS_Index].N * 4;
			CTS_Value = N_CTS_44K1[N_CTS_Index].CTS * 4;
			break;
		case ADSFREQ_192000Hz:
			EP_DEV_DBG("Set to 192KHz");
			N_Value = N_CTS_48K[N_CTS_Index].N * 4;
			CTS_Value = N_CTS_48K[N_CTS_Index].CTS * 4;
			break;
	}
	EP956_Reg_Write(EP956_N, ((BYTE *)&N_Value)+1, 3);
	EP_DEV_DBG(", (N/CTS)[%u] = (%lu/%lu)\n", N_CTS_Index, N_Value, CTS_Value);
	EP956_Reg_Write(EP956_CTS, ((BYTE *)&CTS_Value)+1, 3);

	//
	// Update ADO Info Frame
	//
	// Set Default ADO Info Frame
	memset(Temp_Data, 0x00, 6);

	// Overwrite ADO Info Frame	
	Temp_Data[1] = Params->ChannelNumber; // CT CC
	Temp_Data[2] = FinalSF; // SF SS
	Temp_Data[4] = EP956_ADO_Settings[Params->ChannelNumber].SpeakerMapping; // Speaker Mapping apply only to multi-channel uncompressed audio.
	
	// Write ADO Info Frame back
	Temp_Data[0] = 0x8F;
	for(i=1; i<6; ++i) {
		Temp_Data[0] += Temp_Data[i];
	}
	Temp_Data[0] = ~(Temp_Data[0] - 1);
	EP956_Reg_Write(EP956_ADO_Packet, Temp_Data, 6);
#ifdef DBG
	EP_DEV_DBG("ADO Info: ");
	for(i=0; i<6; ++i) {
		EP_DEV_DBG("0x%02X,", Temp_Data[i] );
	}
	EP_DEV_DBG("\n");
#endif

	EP956_Reg_Set_Bit(EP956_IIS_Control, EP956_IIS_Control__ACR_EN | EP956_IIS_Control__ADO_EN | EP956_IIS_Control__AUDIO_EN);
}

//--------------------------------------------------------------------------------------------------
//
// Hardware Interface
//

SMBUS_STATUS EP956_Reg_Read(BYTE RegAddr, BYTE *Data, WORD Size)
{
	////////////////////////////////////////////////////////////////////////
	// Customer can overwrite the IIC interface here
	//
	int nbyte=0;
	nbyte = I2C_ReadBlock(DEVICE_ADDR_EP955, RegAddr, Data, Size);
	if(nbyte<0) EP_DEV_DBG( "less than Size. read bytes : %d", nbyte);
	return 0; // Success
	//
	////////////////////////////////////////////////////////////////////////
}

SMBUS_STATUS EP956_Reg_Write(BYTE RegAddr, BYTE *Data, WORD Size)
{
	////////////////////////////////////////////////////////////////////////
	// Customer can overwrite the IIC interface here
	//
	I2C_WriteBlock(DEVICE_ADDR_EP955, RegAddr, Data, Size);
	//EP_DEV_DBG("write result = %d", result);
	return 0;
	//
	////////////////////////////////////////////////////////////////////////
}

SMBUS_STATUS EP956_Reg_Set_Bit(BYTE RegAddr, BYTE BitMask)
{
	EP956_Reg_Read(RegAddr, Temp_Data, 1);

	// Write back to Reg Reg_Addr
	Temp_Data[0] |= BitMask;
	
	return EP956_Reg_Write(RegAddr, Temp_Data, 1);
}

SMBUS_STATUS EP956_Reg_Clear_Bit(BYTE RegAddr, BYTE BitMask)
{
	EP956_Reg_Read(RegAddr, Temp_Data, 1);

	// Write back to Reg Reg_Addr
	Temp_Data[0] &= ~BitMask;
	
	return EP956_Reg_Write(RegAddr, Temp_Data, 1);
}


//==================================================================================================
//
// Private Functions
//

SMBUS_STATUS IIC_Access(BYTE IICAddr, BYTE RegAddr, void *Data, WORD Size)
{
	BYTE i, j;

	// Not realy a Robust Algorithm for multi-master arbitration retry
	for(i=10, j=0; j<3; ++j) { // Time-Out to prevent dead lock
		if(IICAddr & 0x01) { // Read
//			status = SMBUS_master_rw_synchronous(IIC_Bus, IICAddr-1, &RegAddr, 1, SMBUS_SkipStop);
//			status |= SMBUS_master_rw_synchronous(IIC_Bus, IICAddr, Data, Size, SMBUS_Normal);
		}
		else { // Write
//			status = SMBUS_master_rw_synchronous(IIC_Bus, IICAddr, &RegAddr, 1, SMBUS_SkipStop);
//			status |= SMBUS_master_rw_synchronous(IIC_Bus, IICAddr, Data, Size, SMBUS_SkipStart);
		}
		if(status) { // failed and retry
			EP_DEV_DBG("Err: IIC failed %bu, IICAddr=0x%02X, RegAddr=0x%02X\n", status, IICAddr, RegAddr);
			delay_1ms(i);
			if(i>0) i-= 3; // Become faster to prevent starvation
		}
		else break;
	}
	return status;
}

SMBUS_STATUS IIC_Read(BYTE IICAddr, BYTE RegAddr, void *Data, WORD Size)
{
	//if(!(IICAddr & 0x01)) IICAddr |= 1;
	I2C_ReadBlock(IICAddr, RegAddr, Data, Size);//IIC_Access(IICAddr, RegAddr, Data, Size);
	return 0;
}

RQ_STATUS MHL_CBus_RQ_Go()
{
	int i;
	BYTE RQ_Check;

	// Set the CBUS Re-try time
	Temp_Data[0] = 0x00; 
	EP956_Reg_Write(EP955_CBUS_TX_Re_Try, &Temp_Data[0], 1);


	//
	// Start to Send the Command
	//

	EP956_Reg_Set_Bit(EP956_CBUS_RQ_Control, EP956_CBUS_RQ_Control__RQ_START);


	//
	// Wait for Complete
	//

	// When IIC clock speed = 170KHz => 400 round = 100ms
	// When IIC clock speed = 200KHz => 470 round = 100ms
	// When IIC clock speed = 400KHz => 940 round = 100ms
	for(i=0; i<500; i++) { // > 100ms timeout
		//delay5us();
		EP956_Reg_Read(EP956_CBUS_RQ_Control, &RQ_Check, 1);
		if(!(RQ_Check & EP956_CBUS_RQ_Control__RQ_START)) {
			if(RQ_Check & EP956_CBUS_RQ_Control__RQ_DONE) {
				break;
			}
		}
	}
	
	// Set the CBUS Re-try time
	Temp_Data[0] = 0x20; 
	EP956_Reg_Write(EP955_CBUS_TX_Re_Try, &Temp_Data[0], 1);


	// Check Error
	if(RQ_Check & EP956_CBUS_RQ_Control__RQ_DONE) {
		if(!(RQ_Check & EP956_CBUS_RQ_Control__RQ_ERR)) {
			return RQ_STATUS_Success; // No error
		}
	}
	else {
		EP956_Reg_Set_Bit(EP956_CBUS_RQ_Control, EP956_CBUS_RQ_Control__RQ_ABORT);
//		EP_DEV_DBG("Err: CBUS RQ Start - RQ_Timeout\n");
		return RQ_STATUS_Timeout;
	}

//	EP_DEV_DBG("Err: CBUS RQ Start - RQ_ERR\n");
	return RQ_STATUS_Error;
}

RQ_STATUS MHL_CBus_RQ_Check(BYTE Size, PBYTE pData)
{
	RQ_STATUS error = RQ_STATUS_Success;

	//
	// Check the read data
	//

	EP956_Reg_Read(EP956_CBUS_RQ_ACT_RX_SIZE, &Temp_Data[0], Size + 2);

	if( (Temp_Data[0]&0x03) == 0 ) {
		error = RQ_STATUS_Error;
#ifdef MSC_DBG
		EP_DEV_DBG("Err: CBUS RQ - No Data Received\n");
#endif
	}
	else if(!(Temp_Data[0] & 0x40)) { // CMD0 bit (shall be command)
#ifdef MSC_DBG
		EP_DEV_DBG("Err: CBUS RQ - CMD_BIT0_ERR, 0x%02X\n", Temp_Data[0]);
#endif
		error = RQ_STATUS_Error;
		MHL_MSC_Cmd_ABORT();
	}
	else {
		if( (Temp_Data[0]&0x03) != (Size + 1) ) { // ACT_RX_SIZE
#ifdef MSC_DBG
			EP_DEV_DBG("Err: CBUS RQ - RX_SIZE_ERR, 0x%02X\n", Temp_Data[0]);
#endif
			error = RQ_STATUS_Error;
			delay_1ms(30); // delay 30ms
			MHL_MSC_Cmd_ABORT();					 
		}
	}
	if(!error) {
		if(Temp_Data[1] == 0x35) { // ABORT
#ifdef MSC_DBG
			EP_DEV_DBG("Err: CBUS RQ - ABORT, 0x%02X\n", Temp_Data[1]);
#endif
			delay_100ms(20); // Delay 2 second
			error = RQ_STATUS_Abort;
		}
		else if(Temp_Data[1] != 0x33) { // ACK
#ifdef MSC_DBG
			EP_DEV_DBG("Err: CBUS RQ - NOT_ACK, 0x%02X\n", Temp_Data[1]);
#endif
			error = RQ_STATUS_Abort;
		}
	}
	if((Temp_Data[0] & 0x03) > 1) {
		if((Temp_Data[0] & 0x80)) { // CMD1 bit (shall not be command)
#ifdef MSC_DBG
			EP_DEV_DBG("Err: CBUS RQ - CMD_BIT1_ERR, 0x%02X\n", Temp_Data[0]);
#endif
			error = RQ_STATUS_Error;
			MHL_MSC_Cmd_ABORT();
		}
	}

	if(!error) { // if No error
		// Copy the data
		if(pData) memcpy(pData, &Temp_Data[2], Size);
	}

	return error;
}

void delay_1ms(int ms)
{
	msleep(ms);
}

void delay_100ms(int ms100)
{
	msleep(ms100*100);
}
