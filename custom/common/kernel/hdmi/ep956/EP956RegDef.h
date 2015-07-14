/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP956RegDef.h

  Description :  Register Address definitions of EP956.

\******************************************************************************/

#ifndef EP956REGDEF_H
#define EP956REGDEF_H

// Registers								Word	BitMask
#define EP956_TX_PHY_Control_0				0x00
#define EP956_TX_PHY_Control_0__TERM_EN				0x80

#define EP956_TX_PHY_Control_1				0x01
#define EP956_TX_PHY_Control_1__RSEN_DIS			0x40
#define EP956_TX_PHY_Control_1__VCO_Gain			0x0C
#define EP956_TX_PHY_Control_1__PHD_CUR				0x03

#define EP956_EE_Checksum					0x02

#define EP956_General_Control_8				0x05
#define EP956_General_Control_8__AUDIO_INPUT		0x30
#define EP956_General_Control_8__PP_MODE			0x02
#define EP956_General_Control_8__MHL_MODE			0x01
#define EP955_General_Control_8__PP_MODE			0x40	// Moved in version 955
#define EP955_General_Control_8__MUX_EN				0x80	// New added in version 955

#define EP956_General_Control_1				0x08
#define EP956_General_Control_1__TSEL_HTP			0x80
#define EP956_General_Control_1__INT_OD				0x40
#define EP956_General_Control_1__INT_POL			0x20
#define EP956_General_Control_1__OSC_PD				0x10
#define EP956_General_Control_1__DSEL				0x08
#define EP956_General_Control_1__BSEL				0x04
#define EP956_General_Control_1__EDGE				0x02
#define EP956_General_Control_1__PU					0x01

#define EP956_General_Control_2				0x09
#define EP956_General_Control_2__RSEN				0x80
#define EP956_General_Control_2__HTPLG				0x40
#define EP956_General_Control_2__PIEF0				0x08
#define EP956_General_Control_2__RIEF				0x04
#define EP956_General_Control_2__VIEF				0x02
#define EP956_General_Control_2__MIEF				0x01

#define EP956_General_Control_3				0x0A	

#define EP956_Color_Space_Control			0x0C
#define EP956_Color_Space_Control__422_OUT			0x80
#define EP956_Color_Space_Control__YCC_OUT			0x40
#define EP956_Color_Space_Control__COLOR			0x20
#define EP956_Color_Space_Control__YCC_Range		0x10
#define EP956_Color_Space_Control__VMUTE			0x08
#define EP956_Color_Space_Control__AMUTE			0x04
#define EP956_Color_Space_Control__AV_MUTE			0x02
		
#define EP956_Pixel_Repetition_Control		0x0D	
#define EP956_Pixel_Repetition_Control__CS_M		0x80
#define EP956_Pixel_Repetition_Control__CTS_M		0x40
#define EP956_Pixel_Repetition_Control__ADSR		0x30
#define EP956_Pixel_Repetition_Control__OSCSEL		0x08
#define EP956_Pixel_Repetition_Control__VSYNC		0x04	
#define EP956_Pixel_Repetition_Control__PR			0x03

#define EP956_General_Control_4				0x0E
#define EP956_General_Control_4__FMT12				0x80
#define EP956_General_Control_4__422_IN				0x40
#define EP956_General_Control_4__YCC_IN				0x20
#define EP956_General_Control_4__E_SYNC				0x10
#define EP956_General_Control_4__VPOL_DET			0x08
#define EP956_General_Control_4__HPOL_DET			0x04
#define EP956_General_Control_4__EESS				0x02
#define EP956_General_Control_4__HDMI				0x01

#define EP956_General_Control_5				0x0F
#define EP956_General_Control_5__AKSV_RDY			0x80
#define EP956_General_Control_5__RPTR				0x10
#define EP956_General_Control_5__RI_RDY				0x02
#define EP956_General_Control_5__ENC_EN				0x01

#define EP956_BKSV							0x10			// BKSV1-BKSV5 0x10-0x14

#define EP956_AN							0x15			// AN1-AN8 0x15-0x1C

#define EP956_AKSV							0x1D			// AKSV1-AKSV5 0x1D-0x21

#define EP956_RI							0x22			// RI1-RI2 0x22-0x23

#define EP956_M0							0x25			// 0x25-0x32

#define EP956_DE_DLY						0x32			// 10 bit

#define EP956_DE_Control					0x33			// 10 bit
#define EP956_DE_Control__DE_GEN					0x40
#define EP956_DE_Control__VSO_POL					0x08
#define EP956_DE_Control__HSO_POL					0x04

#define EP956_DE_TOP						0x34			// 6 bit

#define EP956_DE_CNT						0x36			// 10 bit

#define EP956_DE_LIN						0x38			// 10 bit

#define EP956_H_RES							0x3A			// 11 bit

#define EP956_V_RES							0x3C			// 11 bit

#define EP956_IIS_Control					0x3F			// Default 0x00
#define EP956_IIS_Control__ACR_EN					0x80
#define EP956_IIS_Control__AVI_EN					0x40
#define EP956_IIS_Control__ADO_EN					0x20
#define EP956_IIS_Control__GC_EN					0x10
#define EP956_IIS_Control__AUDIO_EN					0x08
#define EP956_IIS_Control__WS_M						0x04
#define EP956_IIS_Control__WS_POL					0x02
#define EP956_IIS_Control__SCK_POL					0x01

#define EP956_Packet_Control				0x40			// Default 0x00
#define EP956_Packet_Control__FLAT					0x10
#define EP956_Packet_Control__VTX0					0x08
#define EP956_Packet_Control__PKT_RDY_0				0x01

#define EP956_Data_Packet		 			0x41			// HB0-HB2 0x41-0x43; PB0-PB27 0x44-0x5F

#define EP956_CTS		 					0x60			// 20bit (3 Byte)

#define EP956_N			 					0x63			// 20bit (3 Byte)

#define EP956_AVI_Packet 					0x66			// 14 Byte 0x66-0x73

#define EP956_ADO_Packet 					0x74			// 6 Byte 0x74-0x79

#define EP956_SPDIF_Sampling_Frequency 		0x7A			// 1 Byte

#define EP956_Channel_Status		 		0x7B			// 5 Byte 0x7B-0x7F

#define EP956_Embedded_Sync			 		0x80			// Default 0x00

#define EP956_H_Delay			 			0x81			// 10 bit (2 Byte)

#define EP956_H_Width			 			0x83			// 10 bit (2 Byte)

#define EP956_V_Delay			 			0x85			// 6 bit

#define EP956_V_Width			 			0x86			// 6 bit

#define EP956_V_Off_Set			 			0x87			// 12 bit (2 Byte)

#define EP956_Key_Add			 			0xF0			// 1 Byte


//////////////////////////////////////////////////////////////////////////////////////
// MHL add-on

#define EP956_CBUS_MSC_Dec_Capability		0xA0
#define EP956_CBUS_MSC_Dec_SrcPad			0xB0
#define EP956_CBUS_MSC_Dec_Interrupt		0xC0
#define EP956_CBUS_MSC_Dec_Status			0xC4
#define EP956_CBUS_MSC_RAP_RCP				0xC8

#define EP956_CBUS_MSC_Interrupt			0xCA
#define EP955_CBUS_MSC_Interrupt__RQ_T1_EN			0x80	// New added in version 955
#define EP955_CBUS_MSC_Interrupt__RTY_T1_EN			0x40	// New added in version 955
#define EP955_CBUS_MSC_Interrupt__HPD_IE			0x08	// Moved in version 955
#define EP955_CBUS_MSC_Interrupt__MSG_IE			0x04	// Moved in version 955
#define EP955_CBUS_MSC_Interrupt__SCR_IE			0x02	// Moved in version 955
#define EP955_CBUS_MSC_Interrupt__INT_IE			0x01	// Moved in version 955
#define EP956_CBUS_MSC_Interrupt__HPD_S				0x08
#define EP956_CBUS_MSC_Interrupt__MSG_F				0x04
#define EP956_CBUS_MSC_Interrupt__SCR_F				0x02
#define EP956_CBUS_MSC_Interrupt__INT_F				0x01

#define EP956_CBUS_RQ_Control				0xCB
#define EP956_CBUS_RQ_Control__RQ_DONE				0x80
#define EP956_CBUS_RQ_Control__RQ_ERR				0x40
#define EP956_CBUS_RQ_Control__CBUS_STATE			0x20
#define EP955_CBUS_RQ_Control__WB_SPT				0x10	// New added in version 955
#define EP956_CBUS_RQ_Control__CBUS_TRI				0x04
#define EP956_CBUS_RQ_Control__RQ_ABORT				0x02
#define EP956_CBUS_RQ_Control__RQ_START				0x01

#define EP956_CBUS_RQ_SIZE					0xCC
#define EP956_CBUS_RQ_SIZE__RX_SIZE					0x60
#define EP956_CBUS_RQ_SIZE__TX_SIZE					0x1F
#define EP956_CBUS_RQ_HEADER				0xCD
#define EP956_CBUS_RQ_HEADER__DDC_Packet			0x00
#define EP956_CBUS_RQ_HEADER__VS_Packet				0x02
#define EP956_CBUS_RQ_HEADER__MSC_Packet			0x04
#define EP956_CBUS_RQ_HEADER__isCommand				0x01
#define EP955_CBUS_RQ_HEADER__TD0_isCommand			0x08	// New added in version 955
#define EP955_CBUS_RQ_HEADER__TDn_isCommand			0x10	// New added in version 955
#define EP956_CBUS_RQ_CMD					0xCE
#define EP956_CBUS_RQ_TD					0xCF

#define EP956_CBUS_RQ_ACT_RX_SIZE			0xE1
#define EP956_CBUS_RQ_RD					0xE2

#define EP956_CBUS_Connection				0xE6
#define EP956_CBUS_Connection__CONNECTED			0x80
#define EP956_CBUS_Connection__CON_DONE				0x40
#define EP956_CBUS_Connection__ZM_RDY				0x20
#define EP956_CBUS_Connection__CON_BREAK			0x04
#define EP956_CBUS_Connection__CON_START			0x02
#define EP956_CBUS_Connection__ZM_EN				0x01

#define EP956_CBUS_Vendor_ID				0xE4
#define EP955_CBUS_BR_ADJ					0xE5			// New added in version 955

#define EP955_CBUS_TX_Re_Try				0xE7 			// New added in version 955

#define EP955_CBUS_Time_Out					0xE8 			// New added in version 955

#define EP956_Key_Data			 			0xF1			// 7 Byte

#endif