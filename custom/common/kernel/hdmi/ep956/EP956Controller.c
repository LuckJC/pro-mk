/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005 
                           ALL RIGHTS RESERVED 
 
--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP956Controller.c 

  Description :  EP956Controller program 
                 Handle the HDMI(MHL) Link State.
                 Get the information form EDID and Signal Source(from pEP956C_Registers) and make the decision to config EP936 output.

  Codeing     :  Shihken

  2011.06.13  :  1) Set the Version Number to 1.30
                 
  2011.06.29  :  1) Set the Version Number to 1.31 
                 2) Add Revocation List.
                 3) Fix the VBUS control problem.

\******************************************************************************/


#include "Edid.h"
#include "DDC_If.h"
#include "VBUS_If.h"
#include "EP956Controller.h"
#include "EP956SettingsData.h"

//
// Defines
//

#define AV_STABLE_TIME            500

#define CmpInTolarence(Value, Compared, Tolarence) ((Value >= (Compared - Tolarence)) && (Value <= (Compared + Tolarence)))

#ifndef min
#define min(a,b) (a<b)? a:b
#endif





// HDCP Key  
//unsigned char HDCP_Key[41*8];
#if 0
unsigned char HDCP_Key_Default[41*8];
#endif


const unsigned char Revocation_List[] = {
	0x0B, 0x37, 0x21, 0xB4, 0x7D,
	0xF4, 0xC8, 0xDE, 0x4B, 0x82,
	0x23, 0xDE, 0x5C, 0x43, 0x93,
	0x4E, 0x4D, 0xC7, 0x12, 0x7C,
};

//
// Global State and Flags
//

// System flags
BOOL is_Cap_HDMI;
BOOL is_Cap_YCC444;
BOOL is_Cap_YCC422;

BOOL is_Connected;
BOOL is_Source_Ready;
BOOL is_ReceiverSense;
BOOL is_Hot_Plug;
BOOL is_VBus_Power;
BOOL is_CBUS_OK;
BOOL is_PATH_EN;
BOOL is_PackedPixelMode;

BOOL is_VideoChanging;
BOOL is_AudioChanging;

BOOL is_DACP_Change;
BOOL is_HDMI_Mode_Set;

//
// Global Data
//

// Temp Data
static int i;
static SMBUS_STATUS status;
static BYTE Temp_Byte[16];

WORD Dump_Timer = 0;

// System Data
CBUS_LINK_STATE ConnectionState;
BYTE HP_ChangeCount, TimeOut_Count, RSEN_ChangeCount;
TX_STATE TX_State;
WORD VideoChg_TimeCount, AudioChg_TimeCount, ReadEDID_TimeCount, HDMI_Mode_Set_TimeCount;


static VDO_PARAMS Video_Params;
static ADO_PARAMS Audio_Params;
BYTE Select_Packet_Backup[4]; // Including the Header and Data

// Register
PEP956C_REGISTER_MAP pEP956C_Registers;

//
// Private Functions
//

void EP956Controller_Reset(void);

CBUS_LINK_STATE EP956Control_Link_Task(void);
void Exchange_DCAP(void);
void Set_DCAP_Ready(void);
void TXS_RollBack_Wait_Upstream(void);
void TXS_RollBack_Stream(void);
void TXS_RollBack_HDCP(void);

EP956C_CALLBACK EP956C_GenerateInt;




//--------------------------------------------------------------------------------------------------------------------

void EP956Controller_Initial(PEP956C_REGISTER_MAP pEP956C_RegMap, EP956C_CALLBACK IntCall)
{
	// Save the Logical Hardware Assignment
	pEP956C_Registers = pEP956C_RegMap;
	EP956C_GenerateInt = IntCall;

	// Reset all EP956C registers
	memset(pEP956C_Registers, 0, sizeof(EP956C_REGISTER_MAP));
	pEP956C_Registers->Video_Interface[0] = 0x80;
	pEP956C_Registers->Power_Control = EP956E_Power_Control__PD_HDMI;
	pEP956C_Registers->Audio_Interface = 0x10; // 2 Channel audio
	// Update Version Registers
	pEP956C_Registers->VendorID = 0x177A;
	pEP956C_Registers->DeviceID = 0x0956;
	pEP956C_Registers->Version_Major = EP956C_VERSION_MAJOR;
	pEP956C_Registers->Version_Minor = EP956C_VERSION_MINOR;
	// Initial HDCP Info
	memset(pEP956C_Registers->HDCP_AKSV, 0x00, sizeof(pEP956C_Registers->HDCP_AKSV));
	memset(pEP956C_Registers->HDCP_BKSV, 0x00, sizeof(pEP956C_Registers->HDCP_BKSV));

	// Set Revocation List address
	memcpy(pEP956C_Registers->Revocation_List, Revocation_List, sizeof(pEP956C_Registers->Revocation_List));
	HDCP_Assign_RKSV_List(pEP956C_Registers->Revocation_List, sizeof(pEP956C_Registers->Revocation_List));
	HDCP_Extract_BKSV(pEP956C_Registers->HDCP_BKSV);
	HDCP_Extract_BCAPS3(pEP956C_Registers->HDCP_BCAPS3);
	HDCP_Extract_FIFO((unsigned char*)pEP956C_Registers->HDCP_KSV_FIFO, sizeof(pEP956C_Registers->HDCP_KSV_FIFO));
	HDCP_Stop();

	// Reset EP956 Control Program
	EP956Controller_Reset();
}

void EP956Controller_Reset(void)
{
	// Initial Variables
	//EP956_Reg_Set_Bit(EP956_Pixel_Repetition_Control, EP956_Pixel_Repetition_Control__OSCSEL);

	//Temp_Byte[0] = EP956_General_Control_2__RIEF | EP956_General_Control_2__VIEF; // we use polling
	//EP956_Reg_Write(EP956_General_Control_2, Temp_Byte, 1);

//#ifndef OUT_HDCP_DIS
#if 0
	// Copy the Test HDCP Key
	//memcpy(HDCP_Key, HDCP_Key_Default, sizeof(HDCP_Key_Default));
	EP956_AssignKeyAddress(HDCP_Key_Default);
	status = 0;
#else
	// Read HDCP Key for EEPROM
	//status = HDMI_Tx_Get_Key(HDCP_Key);
	status = 0;
#endif


	HDCP_Fake(0);
	pEP956C_Registers->System_Status &= ~EP956E_System_Status__KEY_FAIL;
	if(status) {
		// Do not upload the default Key!
		pEP956C_Registers->System_Configuration |= EP956E_System_Configuration__HDCP_DIS;
		EP_DEV_DBG("No HDCP Key\n");
	}
	else {
		// Check HDCP key and up load the key
//		Temp_Byte[0] = 0;
//		for(i=0; i<328; ++i) {
//			Temp_Byte[0] += *((unsigned char *)HDCP_Key+i);

//			if(!(i % 16))
//				EP_DEV_DBG("\n",HDCP_Key[i]);
//			if(!(i % 8))
//				EP_DEV_DBG("\t",HDCP_Key[i]);
//			EP_DEV_DBG("%02X ",HDCP_Key[i]);
//		}	

		EP_DEV_DBG("HDCP Key Check Sum 0x%02X\n", Temp_Byte[0] );
		if(0)//(/*HDCP_Key[3*8+7] != 0x50 || HDCP_Key[12*8+7] != 0x01 ||*/ Temp_Byte[0] != 0x00) 
		{// || HDCP_Key[40*8+0] != 0xA5) {
			HDCP_Fake(1);
			pEP956C_Registers->System_Status |= EP956E_System_Status__KEY_FAIL;
			EP_DEV_DBG("Check Key failed!\n");
		}
		else {
		
			// Upload the HDCP key
			if(EP956_Load_HDCP_Key()) {
			
				// Check AKSV
		    	if(!HDMI_Tx_read_AKSV(pEP956C_Registers->HDCP_AKSV)) {
					// Test failed
					pEP956C_Registers->System_Status |= EP956E_System_Status__KEY_FAIL;
					EP_DEV_DBG("Upload KSV failed!\n");
				}
			}
			else {
				pEP956C_Registers->System_Status |= EP956E_System_Status__KEY_FAIL;
				EP_DEV_DBG("Upload Key failed!\n");
			}
		}
	}
//#endif

	// EP956 Interface Reset
	EP956_If_Reset();

	// VBus Interface Reset
	VBUS_If_Reset();


	// Reset Variables

	// bit
	//Event_HDMI_Int = 0;
	is_Cap_HDMI = 0;
	is_Cap_YCC444 = is_Cap_YCC422 = 0;

	is_Connected = 0;
	is_Source_Ready = 0;
	is_ReceiverSense = 0;
	is_Hot_Plug = 0;
	is_VBus_Power = 0;
	is_PATH_EN = 0;
	is_PackedPixelMode = 0;

	is_VideoChanging = 0;
	is_AudioChanging = 0;
	is_HDMI_Mode_Set = 0;

	// data
	ConnectionState = 0;
	HP_ChangeCount = 0;
	RSEN_ChangeCount = 0;
	TX_State = TXS_Search_EDID;
	//VideoChg_TimeCount = 0;
	//AudioChg_TimeCount = 0;
	//ReadEDID_TimeCount = 0;
	//HDMI_Mode_Set_TimeCount = 0;

	// data
	if(TX_State > TXS_Search_EDID) {
		EP_DEV_DBG("State Transist: Reset -> [TXS_Wait_Upstream]\n");
		TX_State = TXS_Wait_Upstream;
	}

	MHL_Tx_CBUS_Disconnect();
}

void EP956Controller_Timer(void)
{
	//if(is_VideoChanging) ++VideoChg_TimeCount; //debouncing 500ms
	//if(is_AudioChanging) ++AudioChg_TimeCount; //debouncing 500ms
	//if(is_HDMI_Mode_Set) ++HDMI_Mode_Set_TimeCount;
	//if(TX_State == TXS_HDCP) HDCP_Timer();
	//++ReadEDID_TimeCount;
}

CBUS_LINK_STATE EP956Control_Link_Task(void)
{
	//
	// Check the basic connection status
	//
	
	// Check RSEN
	if(HDMI_Tx_RSEN()) {
		is_ReceiverSense = 1;
		RSEN_ChangeCount = 0;
	}
	else {
		// RSEN Deglitch timing: min = 100 ms; Max = 200 ms
		if(++RSEN_ChangeCount > (150/LINK_TASK_TIME_BASE)) { // Accept continuous 1 error = 80 ms
			RSEN_ChangeCount = 0;
			is_ReceiverSense = 0;
		}
	}

	// Update RSEN
	if(is_ReceiverSense) {
		pEP956C_Registers->System_Status |= EP956E_System_Status__RSEN;
	}
	else {
		pEP956C_Registers->System_Status &= ~EP956E_System_Status__RSEN;
	}


#ifdef MHL_CODE

	// CBUS connections state 
	// The delay between each state must be 80 ms.
	switch(ConnectionState) {

		case CBUS_LINK_STATE__USB_Mode: // Measure the CBUS Impedance
			if( MHL_Tx_MEASURE_1KOHM() ) {
				EP_DEV_DBG("CBUS Connection: 1KOhm Detected\n");						
				MHL_Tx_USB_Mode(FALSE);
				is_VBus_Power = VBUS_Power(0);
				ConnectionState = CBUS_LINK_STATE__1KOHM_Detected;
			}
			else {
				// USB Mode	
				MHL_Tx_USB_Mode(TRUE);
			}
			break;

		case CBUS_LINK_STATE__1KOHM_Detected: // Try VBus On
			is_VBus_Power = VBUS_Power(1);
			MHL_Tx_CBUS_Connect();
			EP_DEV_DBG("CBUS Connection: Start Connect\n");
			ConnectionState = CBUS_LINK_STATE__Start_Connect;
			TimeOut_Count = 0;
			break;

		case CBUS_LINK_STATE__Start_Connect: // Check CBUS Connection status
			if( MHL_Tx_Connection_Status() ) {
			
				if( MHL_Tx_Connection_Status() == MHL_NOT_CONNECTED) {
					MHL_Tx_CBUS_Disconnect();
					
					is_VBus_Power = VBUS_Power(0); // VBUS Off
					
					EP_DEV_DBG("CBUS Connection: Connect Fail\n");
					ConnectionState = CBUS_LINK_STATE__USB_Mode;
				}
				else {
					is_CBUS_OK = 0;
					is_DACP_Change = 0;
	
					EP_DEV_DBG("CBUS Connection: Connected\n");
					ConnectionState = CBUS_LINK_STATE__Check_DCAP;
					TimeOut_Count = 0;
				}
			}
			else if(TimeOut_Count >= (2000/LINK_TASK_TIME_BASE)) { // 2 sec timeout
				MHL_Tx_CBUS_Disconnect();
				is_VBus_Power = VBUS_Power(0); // VBUS Off

				EP_DEV_DBG("CBUS Connection: Connection Timeout\n");
				ConnectionState = CBUS_LINK_STATE__USB_Mode;
			}
			else {
				TimeOut_Count++;
				
				if( !MHL_Tx_VBUS_Power() ) {
					MHL_Tx_CBUS_Disconnect();
					is_VBus_Power = VBUS_Power(0); // VBUS Off
					
					EP_DEV_DBG("CBUS Connection: VBUS missing\n");
					ConnectionState = CBUS_LINK_STATE__USB_Mode;
				}
			}
			break;
	
		case CBUS_LINK_STATE__Check_DCAP: // Check DCAP 

			//if(MHL_MSC_Reg_Read(MSC_RCHANGE_INT) & DCAP_CHG) {
			if(is_DACP_Change) { // Use interrupt pin to detect the DCAP_CHG event

				if(MHL_MSC_Reg_Read(MSC_STATUS_CONNECTED_RDY) & DCAP_RDY) {
					
					// Read Device Cap
					EP_DEV_DBG("Device Cap Change\n");
					Exchange_DCAP();
					Set_DCAP_Ready();
					ConnectionState = CBUS_LINK_STATE__Connected;
				}
			}
			else {
				if(MHL_MSC_Reg_Read(MSC_STATUS_CONNECTED_RDY) & DCAP_RDY) {
				
					// Read Device Cap
					EP_DEV_DBG("Device Cap Ready\n");
					Exchange_DCAP();
					Set_DCAP_Ready();
					ConnectionState = CBUS_LINK_STATE__Connected;
				}
			}	

			// Make sure the RSEN is OK
			if(TimeOut_Count >= (100/LINK_TASK_TIME_BASE)) { // RXRSEN_CHK = 300 - 500 ms
				if(!is_ReceiverSense) {
		
					MHL_Tx_CBUS_Disconnect();
					is_VBus_Power = VBUS_Power(0); // VBus Off
		
					EP_DEV_DBG("CBUS Connection: RSEN missing\n");
					ConnectionState = CBUS_LINK_STATE__USB_Mode;
				}
			}
			
			// Time-out
			if(TimeOut_Count >= (2000/LINK_TASK_TIME_BASE)) { // 2 sec timeout
			
				// Read Device Cap
				EP_DEV_DBG("Device Cap waiting Timeout\n");
				//Exchange_DCAP(); // Force to read DCAP or Skip? (Skip it for 6.3.6.4 - 3 test)
				Set_DCAP_Ready();
				ConnectionState = CBUS_LINK_STATE__Connected;
			}

			TimeOut_Count++;
			break;

		case CBUS_LINK_STATE__Connected: // CBus Connected

			// Check Hot-Plug / EDID Change Interrupt

			if( MHL_Tx_Connection_Status() == MHL_HOTPLUG_DETECT ) { // CBUS Connect && MSC_HPD detect
				if(MHL_MSC_Reg_Read(MSC_DCHANGE_INT) & EDID_CHG) {
					is_Connected = is_Hot_Plug = 0;
				}
				else {
					is_Connected = is_Hot_Plug = 1;
				}
			}
			else {
				is_Connected = is_Hot_Plug = 0;					
			}

			// Check Receiver Sense
			if(!is_ReceiverSense) {

				is_Connected = is_Hot_Plug = 0;

				MHL_Tx_CBUS_Disconnect();
				is_VBus_Power = VBUS_Power(0); // VBus Off

				EP_DEV_DBG("CBUS Connection: Disconnect\n");
				ConnectionState = CBUS_LINK_STATE__USB_Mode;
			}
			// Check PATH_EN
			if(MHL_MSC_Reg_Read(MSC_STATUS_LINK_MODE) & PATH_EN) {
				if(!is_PATH_EN) {
					EP_DEV_DBG("CBUS Connection: Get PATH_EN = 1\n");
					is_PATH_EN = 1;
					
					if(is_PackedPixelMode) 
						MHL_MSC_Cmd_WRITE_STATE(MSC_STATUS_LINK_MODE, CLK_MODE__PacketPixel | PATH_EN);
					else
						MHL_MSC_Cmd_WRITE_STATE(MSC_STATUS_LINK_MODE, CLK_MODE__Normal    | PATH_EN);
				}
			}
			else {
				if(is_PATH_EN) {
					EP_DEV_DBG("CBUS Connection: Get PATH_EN = 0\n");
					is_PATH_EN = 0;
		
					if(is_PackedPixelMode) 
						MHL_MSC_Cmd_WRITE_STATE(MSC_STATUS_LINK_MODE, CLK_MODE__PacketPixel);
					else
						MHL_MSC_Cmd_WRITE_STATE(MSC_STATUS_LINK_MODE, CLK_MODE__Normal);
				}
			}
			break;
	}

#else // HDMI code

	ConnectionState = HDMI_Tx_HTPLG((pEP956C_Registers->System_Configuration & EP956E_System_Configuration__HDMI_DIS)?1:0);

	// Update is_Hot_Plug
	is_Hot_Plug = (ConnectionState == HDMI_HOTPLUG_DETECT)? 1:0;

	// Update is_Connected
	if(is_Connected != ((ConnectionState)?1:0) ) {
		if(++HP_ChangeCount >= 1) { // Accept continuous 1 error = 1*100 ms = 100 ms (Skip when low period < 100 ms)
			HP_ChangeCount = 0;

			is_Connected = ((ConnectionState)?1:0);
			// When set to HDMI output, is_Connected == is_Hot_Plug
			// When set to DVI output,  is_Connected == EDID avaliable | is_Hot_Plug
		}
	}
	else {
		HP_ChangeCount = 0;
	}

	is_PATH_EN = 1;
#endif

	// Update Hot-Plug
	if(is_Hot_Plug) {
		pEP956C_Registers->System_Status |= EP956E_System_Status__HTPLG;
	}
	else {
		pEP956C_Registers->System_Status &= ~EP956E_System_Status__HTPLG;
	}

	// Powered Up and have Input
	if(TX_State > TXS_Wait_Upstream) { 

		// Read HSO VSO POL information
		EP956_Reg_Read(EP956_General_Control_4, Temp_Byte, 1);
		Video_Params.HVPol = Temp_Byte[0] & (EP956_DE_Control__VSO_POL | EP956_DE_Control__HSO_POL);	
	}

	return ConnectionState;
}


unsigned char EP956Controller_Task(void)
{
	//
	// Read from EP956 and Report Status
	//

	// Polling Interrupt Flag and updat the internal information
	EP956Controller_Interrupt();


	// Polling Hot-Plug (MHL Link) / RSEN every 55ms
	EP956Control_Link_Task();	 
	
	
	//
	// Update some controls from the user any time
	//

	// Power Control
	if(pEP956C_Registers->Power_Control & (EP956E_Power_Control__PD_HDMI | EP956E_Power_Control__PD_TOT) ) {
		is_Source_Ready = 0;
		pEP956C_Registers->Power_Control |= EP956E_Power_Control__PD_HDMI;
		
#ifndef MHL_CODE
		EP956_Reg_Set_Bit(EP956_TX_PHY_Control_1, EP956_TX_PHY_Control_1__RSEN_DIS);
#endif
	}
	else {
	
#ifndef MHL_CODE
		EP956_Reg_Clear_Bit(EP956_TX_PHY_Control_1, EP956_TX_PHY_Control_1__RSEN_DIS);
#endif

		if( is_VideoChanging  || is_AudioChanging || (pEP956C_Registers->Video_Input_Format[0] == 0) ) {
			is_Source_Ready = 0;
					
//			if(ConnectionState == CBUS_LINK_STATE__Connected) {
//				MHL_MSC_Cmd_MSC_MSG(MSC_RAP, RAP_CONTENT_OFF);
//			}
		}
		else {
			is_Source_Ready = 1;
					
//			if(ConnectionState == CBUS_LINK_STATE__Connected) {
//				MHL_MSC_Cmd_MSC_MSG(MSC_RAP, RAP_CONTENT_ON);
//			}
		}
	}

	// Mute Control
	if( (pEP956C_Registers->System_Configuration & EP956E_System_Configuration__AUDIO_DIS) || (TX_State < TXS_Stream) ) {
		HDMI_Tx_AMute_Enable();	
	}
	else {
		HDMI_Tx_AMute_Disable();
	}
	if( (pEP956C_Registers->System_Configuration & EP956E_System_Configuration__VIDEO_DIS) || (TX_State < TXS_Stream) ) {
		HDMI_Tx_VMute_Enable();
	}
	else {
		HDMI_Tx_VMute_Disable();
	}
	
	// Auto Handle AVMute
	Handle_AVMute(pEP956C_Registers->System_Configuration & EP956E_System_Configuration__AVMUTE_EN);



	//
	// Handle the main HDMI Link State and HDCP Authentication State
	// The state transition: [Search_EDID] => [Wait_Upstream] => [Stream] => [HDCP]
	//

	switch(TX_State) {
		case TXS_Search_EDID:
			if(is_Connected && is_PATH_EN) {
				//if(ReadEDID_TimeCount > 200/EP956C_TIMER_PERIOD) 
				//EP_DEV_DBG("delay 200ms+\n");
				delay_100ms(2);
				//EP_DEV_DBG("delay 200ms-\n");
				{
					unsigned char EDID_DDC_Status;
					static BYTE detectCount = 0;

					// Confirm Hot-Plug (time-out after 1s)
					if(!is_Hot_Plug) {
						//if(ReadEDID_TimeCount <= 1000/EP956C_TIMER_PERIOD) break;
						if(detectCount++ < 5) break;
						detectCount = 0;
						EP_DEV_DBG("Err: EDID detected without Hot-Plug for 1s\n");
					}

					// Read EDID
					EP_DEV_DBG("State Transist: Read EDID -> [TXS_Wait_Upstream]\n");				
					memset(pEP956C_Registers->Readed_EDID, 0xFF, 256);
					EDID_DDC_Status = Downstream_Rx_read_EDID(pEP956C_Registers->Readed_EDID);
					if(EDID_DDC_Status) {
						static BYTE failCount = 0;
						EP_DEV_DBG("Err: EDID read failed 0x%02X\n", (int)EDID_DDC_Status);
						//if(ReadEDID_TimeCount <= 500/EP956C_TIMER_PERIOD) break;
						if(failCount++ < 3)	break;
						failCount = 0;
					}
					//ReadEDID_TimeCount = 0;
 
					// Set Output
					if(pEP956C_Registers->System_Configuration & EP956E_System_Configuration__FORCE_HDMI_CAP) {
						is_Cap_HDMI = 1;
					}
					else {
						is_Cap_HDMI = EDID_GetHDMICap(pEP956C_Registers->Readed_EDID);
					}
					if(is_Cap_HDMI) {			
						EP_DEV_DBG("Support HDMI");

						// Default Capability
						is_Cap_YCC444 =	is_Cap_YCC422 = 0;
						pEP956C_Registers->EDID_ASFreq = 0x07;
						pEP956C_Registers->EDID_AChannel_PCM = 1;

						pEP956C_Registers->EDID_VideoDataAddr = 0x00;
						pEP956C_Registers->EDID_AudioDataAddr = 0x00;
						pEP956C_Registers->EDID_SpeakerDataAddr = 0x00;
						pEP956C_Registers->EDID_VendorDataAddr = 0x00;

						if(!EDID_DDC_Status) {
							if(pEP956C_Registers->Readed_EDID[131] & 0x20) {	// Support YCC444
								is_Cap_YCC444 = 1;
								EP_DEV_DBG(" YCC444");
							}
							if(pEP956C_Registers->Readed_EDID[131] & 0x10) {	// Support YCC422
								is_Cap_YCC422 = 1;
								EP_DEV_DBG(" YCC422");
							}
							EP_DEV_DBG("\n");
							pEP956C_Registers->EDID_ASFreq = EDID_GetPCMFreqCap(pEP956C_Registers->Readed_EDID);
							pEP956C_Registers->EDID_AChannel_PCM = EDID_GetChannelCap(pEP956C_Registers->Readed_EDID, 1); // Get PCM Max Channel
							pEP956C_Registers->EDID_AChannel_DSD = EDID_GetChannelCap(pEP956C_Registers->Readed_EDID, 9); // Get DSD Max Channel
							
							pEP956C_Registers->EDID_VideoDataAddr = EDID_GetDataBlockAddr(pEP956C_Registers->Readed_EDID, 0x40);
							pEP956C_Registers->EDID_AudioDataAddr = EDID_GetDataBlockAddr(pEP956C_Registers->Readed_EDID, 0x20);
							pEP956C_Registers->EDID_SpeakerDataAddr = EDID_GetDataBlockAddr(pEP956C_Registers->Readed_EDID, 0x80);
							pEP956C_Registers->EDID_VendorDataAddr = EDID_GetDataBlockAddr(pEP956C_Registers->Readed_EDID, 0x60);
						}
					}
					else {
						EP_DEV_DBG("Support DVI RGB only\n");
						is_Cap_YCC444 =	is_Cap_YCC422 = 0;
						pEP956C_Registers->EDID_ASFreq = pEP956C_Registers->EDID_AChannel_PCM = 0;
					}
					
					if(is_Cap_HDMI)
						pEP956C_Registers->EDID_Status = EDID_DDC_Status | EP956E_EDID_Status__HDMI;
					else
						pEP956C_Registers->EDID_Status = EDID_DDC_Status;
					EP_DEV_DBG("Support Max PCM Audio Channel %d\n", (int)pEP956C_Registers->EDID_AChannel_PCM+1);
					EP_DEV_DBG("Support Audio Freq 0x%02X\n", (int)pEP956C_Registers->EDID_ASFreq);

					// Report EDID Change
					pEP956C_Registers->Interrupt_Flags |= EP956E_Interrupt_Flags__EDID_CHG;
					if(EP956C_GenerateInt && (pEP956C_Registers->Interrupt_Enable & EP956E_Interrupt_Enable__EDID_CHG) ) EP956C_GenerateInt();
	
					TX_State = TXS_Wait_Upstream;
				}
			}
			else {	
				pEP956C_Registers->EDID_Status = EDID_STATUS_NoAct;
				//ReadEDID_TimeCount = 0;
			}
			break;
			
		case TXS_Wait_Upstream:

			if(!is_Connected || !is_PATH_EN) {

				TXS_RollBack_Wait_Upstream();
				TX_State = TXS_Search_EDID;
			}
			else if(is_Source_Ready && is_ReceiverSense) { // EP956 can detect RSEN in power down mode
				EP_DEV_DBG("State Transist: Power Up -> [TXS_Stream]\n");							

				// Set to DVI mode
				HDMI_Tx_DVI();	

				// Power Up
				HDMI_Tx_Power_Up();

				is_HDMI_Mode_Set = 1;
				//HDMI_Mode_Set_TimeCount = 0;

				TX_State = TXS_Stream;
			}
			else {
				// Check Force HDMI BOOL
				if(!is_Cap_HDMI) {
					if(pEP956C_Registers->System_Configuration & EP956E_System_Configuration__FORCE_HDMI_CAP) {
						TXS_RollBack_Wait_Upstream();
						TX_State = TXS_Search_EDID;
					}
				}
			}
			break;

		case TXS_Stream:

			//if(HDMI_Mode_Set_TimeCount > 10/EP956C_TIMER_PERIOD) 
			//EP_DEV_DBG("delay 10ms+\n");

			//EP_DEV_DBG("delay 10ms-\n");
			if(is_HDMI_Mode_Set)			
			{
				delay_1ms(10);
				//HDMI_Mode_Set_TimeCount = 0;
				is_HDMI_Mode_Set = 0;

				// HDMI Mode
				if(!is_Cap_HDMI || (pEP956C_Registers->System_Configuration & EP956E_System_Configuration__HDMI_DIS) ) {
					HDMI_Tx_DVI();	// Set to DVI mode (The Info Frame and Audio Packets would not be send)
				}
				else {
					HDMI_Tx_HDMI();	// Set to HDMI mode
				}
#ifdef MHL_CODE
				if(EP956_VDO_Settings[Video_Params.VideoSettingIndex].Pix_Freq_Type > PIX_FREQ_74250KHz) {
					is_PackedPixelMode = 1;
					
					if(is_PATH_EN)
						MHL_MSC_Cmd_WRITE_STATE(MSC_STATUS_LINK_MODE, CLK_MODE__PacketPixel | PATH_EN);
					else
						MHL_MSC_Cmd_WRITE_STATE(MSC_STATUS_LINK_MODE, CLK_MODE__PacketPixel);
			
					MHL_Clock_Mode(1);			
				}
				else {
					is_PackedPixelMode = 0;
					
					if(is_PATH_EN)
						MHL_MSC_Cmd_WRITE_STATE(MSC_STATUS_LINK_MODE, CLK_MODE__Normal | PATH_EN);
					else
						MHL_MSC_Cmd_WRITE_STATE(MSC_STATUS_LINK_MODE, CLK_MODE__Normal);
			
					MHL_Clock_Mode(0);
				}
#endif
			}
			
			if(!is_Connected || !is_PATH_EN) {

				TXS_RollBack_Stream();
				TXS_RollBack_Wait_Upstream();
				TX_State = TXS_Search_EDID;
			}
			else if(!is_Source_Ready || !is_ReceiverSense) {

				TXS_RollBack_Stream();
				TX_State = TXS_Wait_Upstream;
			}
			else if(!(pEP956C_Registers->System_Configuration & EP956E_System_Configuration__HDCP_DIS)) {
				if(is_HDMI_Mode_Set == 0) {
					EP_DEV_DBG("State Transist: Start HDCP -> [TXS_HDCP]\n");
					
					// Enable mute for transmiter video and audio
					HDMI_Tx_Mute_Enable();
	
					TX_State = TXS_HDCP;
				}
			}
			
			if(Dump_Timer++ > 2000/EP956C_TIMER_PERIOD)
			{
			
				Dump_Timer = 0;
				EP956_Reg_Read(EP956_CTS, Temp_Byte, 3);
				EP_DEV_DBG("CTS %02X %02X %02X\n", Temp_Byte[0], Temp_Byte[1], Temp_Byte[2]);
				
				EP956_Reg_Read(EP956_N, Temp_Byte, 3);
				EP_DEV_DBG("N %02X %02X %02X\n", Temp_Byte[0], Temp_Byte[1], Temp_Byte[2]);

				EP956_Reg_Read(EP956_Color_Space_Control, Temp_Byte, 1);
				EP_DEV_DBG("EP956_Color_Space_Control %X\n", Temp_Byte[0]);
				EP956_Reg_Read(EP956_General_Control_8, Temp_Byte, 1);
				EP_DEV_DBG("EP956_General_Control_8 %X\n", Temp_Byte[0]);
				EP956_Reg_Read(EP956_IIS_Control, Temp_Byte, 1);
				EP_DEV_DBG("EP956_IIS_Control %X\n", Temp_Byte[0]);
				
				HDMI_Dump_Status();
			}

			break;

		case TXS_HDCP:
		
			if(!is_Connected || !is_PATH_EN) {

				TXS_RollBack_HDCP();
				TXS_RollBack_Stream();
				TXS_RollBack_Wait_Upstream();
				TX_State = TXS_Search_EDID;
			}
			else if(!is_Source_Ready || !is_ReceiverSense) {

				TXS_RollBack_HDCP();
				TXS_RollBack_Stream();
				TX_State = TXS_Wait_Upstream;
			}
			else if(pEP956C_Registers->System_Configuration & EP956E_System_Configuration__HDCP_DIS) {

				TXS_RollBack_HDCP();
				TX_State = TXS_Stream;
			}
			else if(!is_Hot_Plug || is_VideoChanging || (pEP956C_Registers->System_Configuration & EP956E_System_Configuration__VIDEO_DIS) ) {
			
				// Enable mute for transmiter video and audio
				HDMI_Tx_Mute_Enable();

				HDCP_Stop();
			}
			else {
				pEP956C_Registers->HDCP_State = HDCP_Authentication_Task();
				pEP956C_Registers->HDCP_Status = HDCP_Get_Status();
			}
			break;
	}



	//
	// Combine the EDID capability and the source information to set the EP956 video/audio settings 
	//

	//
	// Update Video Params
	//

	// Video Interface
	Video_Params.Interface = pEP956C_Registers->Video_Interface[0];

	// Video Timing
	if(pEP956C_Registers->Video_Input_Format[0]) { 
		// Semi Auto Detect Video Timing
		if(pEP956C_Registers->Video_Input_Format[0] < 128) {
			if(pEP956C_Registers->Video_Input_Format[0] < 112) {
				Video_Params.VideoSettingIndex = pEP956C_Registers->Video_Input_Format[0];
			}
			else {
				Video_Params.VideoSettingIndex = pEP956C_Registers->Video_Input_Format[0] - (112 - EP956_VDO_Settings_Rep_Start);
			}
		}
		else {
			Video_Params.VideoSettingIndex = pEP956C_Registers->Video_Input_Format[0] - (128 - EP956_VDO_Settings_IT_Start);
		}
	} 

	// Select Sync Mode
	Video_Params.SyncMode = (pEP956C_Registers->Video_Interface[1] & EP956E_Video_Interface_Setting_1__SYNC) >> 2;

	// Select Color Space
	switch(pEP956C_Registers->Video_Interface[1] & EP956E_Video_Interface_Setting_1__COLOR) {
		default:
		case EP956E_Video_Interface_Setting_1__COLOR__Auto:
			switch(Video_Params.VideoSettingIndex) {
				case  4: case  5: case 16: case 19: case 20: case 31: case 32: 
				case 33: case 34: case 39: case 40: case 41: case 46: case 47:		// HD Timing
					Video_Params.ColorSpace = COLORSPACE_709;
					break;

				default:
					if(Video_Params.VideoSettingIndex && Video_Params.VideoSettingIndex < EP956_VDO_Settings_IT_Start) { // SD Timing
						Video_Params.ColorSpace = COLORSPACE_601;
					}
					else {															// IT Timing
						Video_Params.ColorSpace = COLORSPACE_709;
					}
			}
			break;
		case EP956E_Video_Interface_Setting_1__COLOR__601:
			Video_Params.ColorSpace = COLORSPACE_601;
			break;
		case EP956E_Video_Interface_Setting_1__COLOR__709:
			Video_Params.ColorSpace = COLORSPACE_709;
			break;
	}

	// Set Input Format
	switch(pEP956C_Registers->Video_Interface[1] & EP956E_Video_Interface_Setting_1__VIN_FMT) {
		default:
		case EP956E_Video_Interface_Setting_1__VIN_FMT__RGB:
			Video_Params.FormatIn = COLORFORMAT_RGB;
			break;
		case EP956E_Video_Interface_Setting_1__VIN_FMT__YCC444:
			Video_Params.FormatIn = COLORFORMAT_YCC444;
			break;
		case EP956E_Video_Interface_Setting_1__VIN_FMT__YCC422:
			Video_Params.FormatIn = COLORFORMAT_YCC422;
			break;
	}

	// Set Output Format
	switch(pEP956C_Registers->Video_Input_Format[2] & EP956E_Video_Output_Format_2__VOUT_FMT) {
		default:
		case EP956E_Video_Output_Format_2__VOUT_FMT__Auto:
			if(is_Cap_YCC444) {
				Video_Params.FormatOut = COLORFORMAT_YCC444;
			}
			else if(is_Cap_YCC422) {
				Video_Params.FormatOut = COLORFORMAT_YCC422;
			}
			else {
				Video_Params.FormatOut = COLORFORMAT_RGB;
			}
			break;
		case EP956E_Video_Output_Format_2__VOUT_FMT__YCC444:
			Video_Params.FormatOut = COLORFORMAT_YCC444;
			break;
		case EP956E_Video_Output_Format_2__VOUT_FMT__YCC422:
			Video_Params.FormatOut = COLORFORMAT_YCC422;
			break;
		case EP956E_Video_Output_Format_2__VOUT_FMT__RGB:
			Video_Params.FormatOut = COLORFORMAT_RGB;
			break;
	}
#ifdef MHL_CODE
	// Pack Pixel Mode
	if(EP956_VDO_Settings[Video_Params.VideoSettingIndex].Pix_Freq_Type > PIX_FREQ_74250KHz) {
		// Force to YCC422
		Video_Params.FormatOut = COLORFORMAT_YCC422;
	}
#endif
	
	// DVI mode settings overwrite
	if(!is_Cap_HDMI || (pEP956C_Registers->System_Configuration & EP956E_System_Configuration__HDMI_DIS) ) {
		Video_Params.FormatOut = COLORFORMAT_RGB;
	}

	// AFAR
	Video_Params.AFARate = ((pEP956C_Registers->Video_Input_Format[1] & EP956E_Video_Output_Format_1__AFAR) >> 4) | 0x08;

	// Video Change
	if(memcmp(&Video_Params, &pEP956C_Registers->Video_Params_Backup, 4) != 0) {
		// Timing change that AVMute is needed
		pEP956C_Registers->Video_Params_Backup = Video_Params;
		
		//VideoChg_TimeCount = 0;
		is_VideoChanging = 1;
		EP_DEV_DBG("Video Changing......\n");
	}
	else if(memcmp(&Video_Params, &pEP956C_Registers->Video_Params_Backup, sizeof(VDO_PARAMS)) != 0) {
		// Format change which can by applied immediately
		pEP956C_Registers->Video_Params_Backup = Video_Params;
		HDMI_Tx_VideoFMT_Config(&Video_Params);
	}

	// Video Change Debouncing
	if(is_VideoChanging) {
		//if(VideoChg_TimeCount > AV_STABLE_TIME/EP956C_TIMER_PERIOD) 
		//EP_DEV_DBG("delay 500ms+\n");
		delay_100ms(5);
		//EP_DEV_DBG("delay 500ms-\n");
		{
			HDMI_Tx_Video_Config(&Video_Params);
			is_VideoChanging = 0;
			//VideoChg_TimeCount = 0;
			
			// Report Video Change
			pEP956C_Registers->Interrupt_Flags |= EP956E_Interrupt_Flags__VIDEO_CHG;
			if(EP956C_GenerateInt && (pEP956C_Registers->Interrupt_Enable & EP956E_Interrupt_Enable__VIDEO_CHG) ) EP956C_GenerateInt();
		}
	}

	//
	// Update Audio Params
	//
	Audio_Params.Interface = pEP956C_Registers->Audio_Interface & 0x8F; // DSDCK_POL, ..., SD_DIR, WS_M, WS_POL, SCK_POL
	Audio_Params.VideoSettingIndex = Video_Params.VideoSettingIndex;

	// Update Audio Type
	Audio_Params.AudioType = (pEP956C_Registers->Audio_Input_Format[0] & EP956E_Audio_Output_Format__AUDIO_TYPE) >> 6;

	// Update Audio Channel Number
	if(Audio_Params.AudioType == AUDIO_TYPE_IIS) {
		if(EP956_VDO_Settings[Video_Params.VideoSettingIndex].Pix_Freq_Type <= PIX_FREQ_27027KHz) {
			Audio_Params.ChannelNumber = 1;
		}
		else {
			Audio_Params.ChannelNumber = min(((pEP956C_Registers->Audio_Interface & 0x70) >> 4), pEP956C_Registers->EDID_AChannel_PCM);
		}
	}
	else if(Audio_Params.AudioType == AUDIO_TYPE_DSD) {
		if(EP956_VDO_Settings[Video_Params.VideoSettingIndex].Pix_Freq_Type <= PIX_FREQ_27027KHz) {
			Audio_Params.ChannelNumber = 1;
		}
		else {
			Audio_Params.ChannelNumber = min(((pEP956C_Registers->Audio_Interface & 0x70) >> 4), pEP956C_Registers->EDID_AChannel_DSD);
		}
	}
	else {
		Audio_Params.ChannelNumber = (pEP956C_Registers->Audio_Interface & 0x70) >> 4;
	}

	// Update VFS
	if(Audio_Params.VideoSettingIndex < EP956_VDO_Settings_IT_Start) {
		// Pixel Clock Type shift (59.94/60)
		Audio_Params.VFS = (pEP956C_Registers->Video_Input_Format[1] & EP956E_Video_Output_Format_1__VIF)? 1:0;
	}
	else {
		Audio_Params.VFS = 0;
	}

	// Update Channel Status Control (Copy Right Info)
	Audio_Params.ChannelStatusControl = pEP956C_Registers->Audio_Input_Format[0] & EP956E_Audio_Output_Format__CS_OW_EN;
	if(Audio_Params.ChannelStatusControl & EP956E_Audio_Output_Format__CS_OW_EN) {
		memcpy(Audio_Params.ChannelStatus, &pEP956C_Registers->Audio_Input_Format[1], sizeof(Audio_Params.ChannelStatus));
	}
	
	// Update Frequency and Downsample information (Use ADO_FREQ or Auto)					 
	switch( pEP956C_Registers->Audio_Input_Format[0] & EP956E_Audio_Output_Format__ADO_FREQ) {

		case EP956E_Audio_Output_Format__ADO_FREQ__32000Hz:
			Audio_Params.InputFrequency = ADSFREQ_32000Hz;
			// Disable Down Sample
			Audio_Params.ADSRate = 0;
			break;

		default:
		case EP956E_Audio_Output_Format__ADO_FREQ__44100Hz:
			Audio_Params.InputFrequency = ADSFREQ_44100Hz;
			// Disable Down Sample
			Audio_Params.ADSRate = 0;
			break;

		case EP956E_Audio_Output_Format__ADO_FREQ__48000Hz:
			Audio_Params.InputFrequency = ADSFREQ_48000Hz;
			// Disable Down Sample
			Audio_Params.ADSRate = 0;
			break;

		case EP956E_Audio_Output_Format__ADO_FREQ__88200Hz:
			Audio_Params.InputFrequency = ADSFREQ_88200Hz;
			if(pEP956C_Registers->EDID_ASFreq & 0x08) { // 88.2kHz
				// Disable Down Sample
				Audio_Params.ADSRate = 0;
			}
			else {
				// Enable Down Sample 1/2
				Audio_Params.ADSRate = 1;
			}
			break;

		case EP956E_Audio_Output_Format__ADO_FREQ__96000Hz:
			Audio_Params.InputFrequency = ADSFREQ_96000Hz;
			if(pEP956C_Registers->EDID_ASFreq & 0x10) { // 96kHz
				// Disable Down Sample
				Audio_Params.ADSRate = 0;
			}
			else {
				if(pEP956C_Registers->EDID_ASFreq & 0x04) { // 48kHz
					// Enable Down Sample 1/2
					Audio_Params.ADSRate = 1;
				}
				else {
					// Enable Down Sample 1/3
					Audio_Params.ADSRate = 2;
				}
			}
			break;

		case EP956E_Audio_Output_Format__ADO_FREQ__176400Hz:
			Audio_Params.InputFrequency = ADSFREQ_176400Hz;
			if(pEP956C_Registers->EDID_ASFreq & 0x20) { // 176kHz
				// Disable Down Sample
				Audio_Params.ADSRate = 0;
			}
			else {
				if(pEP956C_Registers->EDID_ASFreq & 0x08) { // 88.2kHz
					// Enable Down Sample 1/2
					Audio_Params.ADSRate = 1;
				}
				else {
					// Enable Down Sample 1/4
					Audio_Params.ADSRate = 3;
				}
			}
			break;

		case EP956E_Audio_Output_Format__ADO_FREQ__192000Hz:
			Audio_Params.InputFrequency = ADSFREQ_192000Hz;
			if(pEP956C_Registers->EDID_ASFreq & 0x40) { // 192kHz
				// Disable Down Sample
				Audio_Params.ADSRate = 0;
			}
			else {
				if(pEP956C_Registers->EDID_ASFreq & 0x10) { // 96kHz
					// Enable Down Sample 1/2
					Audio_Params.ADSRate = 1;
				}
				else {
					// Enable Down Sample 1/4
					Audio_Params.ADSRate = 3;
				}
			}
			break;
	}

	// Audio Change
	if(memcmp(&Audio_Params, &pEP956C_Registers->Audio_Params_Backup, sizeof(ADO_PARAMS)) != 0) {
		pEP956C_Registers->Audio_Params_Backup = Audio_Params;

		//AudioChg_TimeCount = 0;
		is_AudioChanging = 1;
		EP_DEV_DBG("Audio Changing......\n");
	}

	// Audio Change Debouncing
	if(is_AudioChanging) {
		//if(AudioChg_TimeCount > AV_STABLE_TIME/EP956C_TIMER_PERIOD) 
		//EP_DEV_DBG("delay 500ms+\n");
		delay_100ms(5);
		//EP_DEV_DBG("delay 500ms-\n");
		{
			HDMI_Tx_Audio_Config(&Audio_Params);
			is_AudioChanging = 0;
			//AudioChg_TimeCount = 0;

			// Report Audio Change
			pEP956C_Registers->Interrupt_Flags |= EP956E_Interrupt_Flags__AUDIO_CHG;
			if(EP956C_GenerateInt && (pEP956C_Registers->Interrupt_Enable & EP956E_Interrupt_Enable__AUDIO_CHG) ) EP956C_GenerateInt();
		}
	}
		

	// Update Select Packet
	if(memcmp(pEP956C_Registers->Select_Packet, Select_Packet_Backup, sizeof(Select_Packet_Backup)) != 0 ) {
		memcpy(Select_Packet_Backup, pEP956C_Registers->Select_Packet, sizeof(Select_Packet_Backup));

		// Update Packet Data
		EP956_Reg_Write(EP956_Data_Packet, pEP956C_Registers->Select_Packet, 31);
	}

	
	
	return ConnectionState;
/*
	// Return the status
	if(pEP956C_Registers->Power_Control & (EP956E_Power_Control__PD_HDMI | EP956E_Power_Control__PD_TOT)) {
		return EP956C_TASK_Idle;
	}
	else {
		return EP956C_TASK_Pending;
	}

	return TX_State;
*/
}

void Exchange_DCAP(void)
{
//#define DCAP_DBG
	is_DACP_Change = 0;

	{
		BYTE i;
		for(i=0; i<16; ++i) {
			is_CBUS_OK = MHL_MSC_Cmd_READ_DEVICE_CAP(i, &Temp_Byte[i]);
			if( !is_CBUS_OK ) return;
		}

#ifdef DCAP_DBG
		EP_DEV_DBG("DCAP:\n");
		for(i=0; i<16; ++i) {
			EP_DEV_DBG("0x%02X ", Temp_Byte[i]);
		}
		EP_DEV_DBG("\n");
#endif
	}

	// Check POW bit from Sink	
	if(Temp_Byte[MSC_DEV_CAT] & POW) {
		is_VBus_Power = VBUS_Power(0); // VBus Off
	}

	// Check Scratchpad/RAP/RCP support
	if(Temp_Byte[FEATURE_FLAG] & RCP_SUPPORT) {
		EP_DEV_DBG("RCP_SUPPORT = 1\n");
	}
	if(Temp_Byte[FEATURE_FLAG] & RAP_SUPPORT) {
		EP_DEV_DBG("RAP_SUPPORT = 1\n");

		if(is_Source_Ready) {
			MHL_MSC_Cmd_MSC_MSG(MSC_RAP, RAP_CONTENT_ON);
		}	
	}
	if(Temp_Byte[FEATURE_FLAG] & SP_SUPPORT) {
		EP_DEV_DBG("SP_SUPPORT = 1\n");

		// Remember the ADOPTER_ID and DEVICE_ID
		//RSP_Adopter_ID[0] = Temp_Byte[ADOPTER_ID_H];
		//RSP_Adopter_ID[1] = Temp_Byte[ADOPTER_ID_L];
	}

	//
	// Add the customer code here ...
	//
	
	
}	
	
void Set_DCAP_Ready(void)
{	
	//
	// At last
	//
	
	// Reset the PATH_EN State
	is_PATH_EN = 0;
	
	// Set DCAP_RDY bit to the other side Status Register
	is_CBUS_OK = MHL_MSC_Cmd_WRITE_STATE(MSC_STATUS_CONNECTED_RDY, DCAP_RDY);
	if( !is_CBUS_OK ) return;
	
	// Set DCAP_CHG bit to the other side Interrupt Register
	is_CBUS_OK = MHL_MSC_Cmd_WRITE_STATE(MSC_RCHANGE_INT, DCAP_CHG);
	if( !is_CBUS_OK ) return;
	
}

void TXS_RollBack_Wait_Upstream(void)
{
	EP_DEV_DBG("State Rollback: Reset EDID -> [TXS_Search_EDID]\n");

	// Reset EDID
	memset(pEP956C_Registers->Readed_EDID, 0xFF, 256);

	// Report EDID Change
	pEP956C_Registers->Interrupt_Flags |= EP956E_Interrupt_Flags__EDID_CHG;
	if(EP956C_GenerateInt && (pEP956C_Registers->Interrupt_Enable & EP956E_Interrupt_Enable__EDID_CHG) ) EP956C_GenerateInt();
	//ReadEDID_TimeCount = 0;
}

void TXS_RollBack_Stream(void)
{
	EP_DEV_DBG("State Rollback: Power Down -> [TXS_Wait_Upstream]\n");

	// Power Down
	HDMI_Tx_Power_Down();

	is_HDMI_Mode_Set = 0;
}

void TXS_RollBack_HDCP(void)
{
	EP_DEV_DBG("State Rollback: Stop HDCP -> [TXS_Stream]\n");

	HDCP_Stop();
	pEP956C_Registers->HDCP_Status = 0;
	pEP956C_Registers->HDCP_State = 0;

	// Disable mute for transmiter video and audio
	HDMI_Tx_Mute_Disable();
}

//----------------------------------------------------------------------------------------------------------------------

// This functions should be polling ASAP or triggered by interrupt pin
void EP956Controller_Interrupt(void) 
{
	BYTE IntFlag_Byte;

	// HDMI Interrupt
	IntFlag_Byte = HDMI_Tx_Get_Flags();
	if(IntFlag_Byte & EP956_General_Control_2__RIEF) {
		HDCP_Ext_Ri_Trigger();
	}
	if(IntFlag_Byte & EP956_General_Control_2__VIEF) {
		if(pEP956C_Registers->System_Configuration & EP956E_System_Configuration__PACKET_RDY) {
			// Send Packet every field (frame)
			EP956_Reg_Set_Bit(EP956_Packet_Control, EP956_Packet_Control__PKT_RDY_0);
		}
	}
#ifdef MHL_CODE
	// MHL Interrupt
	IntFlag_Byte = MHL_MSC_Get_Flags();
	if(IntFlag_Byte & EP956_CBUS_MSC_Interrupt__MSG_F) {
		
		MHL_RCP_RAP_Read(pEP956C_Registers->MSC_Message);

		switch(pEP956C_Registers->MSC_Message[0]) {

			// RAP
			case MSC_RAP:
				EP_DEV_DBG("RAP Received: 0x%02X\n", pEP956C_Registers->MSC_Message[1]);
				MHL_MSC_Cmd_MSC_MSG(MSC_RAPK, RAPK_No_Error);
				break;

			case MSC_RAPK:
				if(pEP956C_Registers->MSC_Message[1])	EP_DEV_DBG("RAP Send Err: 0x%02X\n", pEP956C_Registers->MSC_Message[1]);
				break;

			// RCP
			case MSC_RCP:
				EP_DEV_DBG("RCP Received: 0x%02X\n", pEP956C_Registers->MSC_Message[1]);
				MHL_MSC_Cmd_MSC_MSG(MSC_RCPK, pEP956C_Registers->MSC_Message[1]);
				break;

			case MSC_RCPE:
				if(pEP956C_Registers->MSC_Message[1])	EP_DEV_DBG("RCP Send Err: 0x%02X\n", pEP956C_Registers->MSC_Message[1]);
				break;
				
			// UCP
			case MSC_UCP:
				EP_DEV_DBG("UCP Received: 0x%02X\n", pEP956C_Registers->MSC_Message[1]);
				MHL_MSC_Cmd_MSC_MSG(MSC_UCPK, pEP956C_Registers->MSC_Message[1]);
				break;

			case MSC_UCPE:
				if(pEP956C_Registers->MSC_Message[1])	EP_DEV_DBG("UCP Send Err: 0x%02X\n", pEP956C_Registers->MSC_Message[1]);
				break;
		}
		
		// Report MSC Message received
		pEP956C_Registers->Interrupt_Flags |= EP956E_Interrupt_Flags__MSC_MSG;
		if(EP956C_GenerateInt && (pEP956C_Registers->Interrupt_Enable & EP956E_Interrupt_Flags__MSC_MSG) ) EP956C_GenerateInt();
	}
	if(IntFlag_Byte & EP956_CBUS_MSC_Interrupt__INT_F) {
		BYTE RCHANGE_INT_value;

		RCHANGE_INT_value = MHL_MSC_Reg_Read(MSC_RCHANGE_INT);

		// Detect Device Capability Change Interrupt (during the connected state)
		if(RCHANGE_INT_value & DCAP_CHG) {
			EP_DEV_DBG("INT Received: DCAP_CHG\n");
			
			is_DACP_Change = 1;

			//if(is_Connected) {
				// Check POW bit from Sink
				is_CBUS_OK = MHL_MSC_Cmd_READ_DEVICE_CAP(MSC_DEV_CAT, Temp_Byte);
				if(Temp_Byte[0] & POW) {
					is_VBus_Power = VBUS_Power(0); // VBus Off
				}
			//}
		}

		// Scratchpad Transmit Handling
		if(RCHANGE_INT_value & GRT_WRT) {

			////////////////////////////////////////////////////////////////////////
			// Customer should implement their own code here
			//
			static unsigned int WriteBurstCount = 0;

			BYTE BurstData[16] = {
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
			};
			BurstData[0] = Device_Capability_Default[3]; // ADOPTER_ID_H
			BurstData[1] = Device_Capability_Default[4]; // ADOPTER_ID_L
			BurstData[2] = WriteBurstCount >> 8;
			BurstData[3] = WriteBurstCount;

			EP_DEV_DBG("INT Received: GRT_WRT\n");
			MHL_MSC_Cmd_WRITE_BURST(MSC_SCRATCHPAD, BurstData, 16);
			WriteBurstCount++;
			//
			////////////////////////////////////////////////////////////////////////
			
			MHL_MSC_Cmd_WRITE_STATE(MSC_RCHANGE_INT, DSCR_CHG);
		}
		
		// Scratchpad Receive Handling
		if(RCHANGE_INT_value & DSCR_CHG) { //  1st priority
			EP_DEV_DBG("INT Received: DSCR_CHG\n");	

			////////////////////////////////////////////////////////////////////////
			// Customer should implement their own code here
			//
			for(i=0; i< Device_Capability_Default[SCRATCHPAD_SIZE]; i++) {
				if(i==0) {
					EP_DEV_DBG("Burst_ID[0]=%02X", MHL_MSC_Reg_Read(MSC_SCRATCHPAD+i) );
				}
				else if(i==1) {
					EP_DEV_DBG("%02X, ", MHL_MSC_Reg_Read(MSC_SCRATCHPAD+i) );
				}
				else {
					EP_DEV_DBG("%02X ", MHL_MSC_Reg_Read(MSC_SCRATCHPAD+i) );
				}
			} EP_DEV_DBG("\n");
			//
			////////////////////////////////////////////////////////////////////////
		}
		if(RCHANGE_INT_value & REQ_WRT) { // Lowest prioirty to Grant to Write
			EP_DEV_DBG("INT Received: REQ_WRT\n");
			MHL_MSC_Cmd_WRITE_STATE(MSC_RCHANGE_INT, GRT_WRT);
		}

	}
#endif
}

