/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2013
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP955_UserLevel.c

  Description :  main program 

  2013.06.26  :  1) Beta1 Release

\******************************************************************************/

#include "Edid.h"
#include "DDC_If.h"
#include "VBUS_If.h"
#include "EP956_If.h"
#include "EP956Controller.h"
#include "EP956SettingsData.h"

#include "EP955_UserLevel.h"


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

// EP955 used

void HDMI_Power_Up(void);
void HDMI_Power_Down(void);

//
// Android
//

void callback_work_Int(struct work_struct *p);
void callback_work_Link_Task(struct work_struct *p);
static enum hrtimer_restart callback_EP955_timer(struct hrtimer *timer);
void callback_EP955_Timer_Task(struct work_struct *p);
extern irqreturn_t callback_irq(int irq, void *handle);
static enum hrtimer_restart callback_timer(struct hrtimer *timer);


// HiRes Timer
static struct hrtimer pTimer_Link_Task;
static struct hrtimer Timer_EP955;

//
void On_Ext_Int_EP956E();

// Work Queue
struct work_struct work_Int;
struct work_struct work_Link_Task;
struct work_struct work_Timer_Task;

void Send_RCP_Key_Code(BYTE rcp_key_code);
void Send_Key_Event(unsigned int key_event);
inline int Init_Keyboard(void);

static struct input_dev *dev_keyboard;

//
// Keyboard
//
inline int Init_Keyboard(void);
void Send_Key_Event(unsigned int key_event);
void Send_RCP_Key_Code(BYTE rcp_key_code);

//
// Global Data
//

static EP956C_REGISTER_MAP EP956C_Registers;


// Link State
static CBUS_LINK_STATE Link_State = 0;
static CBUS_LINK_STATE BackupLink_State;
//static TX_STATE TX_State = 0;

//
// State and Flags
//
BOOL Event_EDID_CH = 0;
BOOL Event_VIDEO_CH = 0;
BOOL Event_AUDIO_CH = 0;
BOOL Event_MSC_MSG = 0;


EP955_INT_CALLBACK EP955_IntCB;

struct i2c_client *EP955_i2c_client = NULL;
//==================================================================================================
//
// Interface for API
//
//==================================================================================================

int EP955_UserLevel_Init(EP955_INT_CALLBACK inCallBack)
{
	int ret;
	EP_DEV_DBG("EP955 attach start!!!\n");
	// init call back function	
	EP955_IntCB = inCallBack;
	

	//EP955_MHL_module_probe(client, id);
	INIT_WORK(&work_Int, callback_work_Int);
	INIT_WORK(&work_Link_Task, callback_work_Link_Task);
	//INIT_WORK(&work_Timer_Task, callback_EP955_Timer_Task);
	
	//ret = request_irq(client->irq, callback_irq, IRQF_TRIGGER_FALLING , "MHL_INT", NULL); 
	//if (ret) {
	//	EP_DEV_DBG_ERROR("unable to request irq MHL_INT err:: %d\n", ret);	
	//	return 0;
	//}
	//EP_DEV_DBG("MHL int reques successful %d\n", ret);
	//ret = Init_Keyboard();
	//if (ret) 
	//{
	//	EP_DEV_DBG_ERROR("Failed to install keyboard\n");
	//}

	hrtimer_init(&pTimer_Link_Task, 1, HRTIMER_MODE_REL);
	pTimer_Link_Task.function = &callback_timer;

	//hrtimer_init(&Timer_EP955, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	//Timer_EP955.function = &callback_EP955_timer;
	
	

	// Interrupt Enable
	EP956C_Registers.Interrupt_Enable =		EP956E_Interrupt_Enable__EDID_CHG | 
											EP956E_Interrupt_Enable__VIDEO_CHG | 
											EP956E_Interrupt_Enable__AUDIO_CHG | 
											EP956E_Interrupt_Enable__MSC_MSG |
											0;



	return ret;
}

// need I2C ready
void EP955_UserLevel_InitMemory(struct i2c_client *client)
{
	// GPIO reset EP955 first by MCU
	
	//if(EP955_i2c_client != client)
	{
		EP955_i2c_client = client;
		
		// EP955 Controller initial
		Link_State = 0;
		EP_DEV_DBG("Inital all memory\n");
		EP956Controller_Initial(&EP956C_Registers, On_Ext_Int_EP956E);
	}
}

void EP955_UserLevel_PowerON(void)
{


#ifdef OUT_HDCP_DIS
	// Disable HDCP
	EP_DEV_DBG("==== HDCP Disabled ====\n");
	EP956C_Registers.System_Configuration = EP956E_System_Configuration__HDCP_DIS;
#endif

	// start polling task	
	hrtimer_start(&pTimer_Link_Task, ktime_set(0, LINK_TASK_TIME_BASE*1000000), HRTIMER_MODE_REL);
	//hrtimer_start(&Timer_EP955, ktime_set(0, EP956C_TIMER_PERIOD*1000000), HRTIMER_MODE_REL);

	HDMI_Power_Up();
}

void EP955_UserLevel_PowerOFF(void)
{
	// remove polling task
	//hrtimer_stop(&pTimer_Link_Task);

	HDMI_Power_Down();
}

void EP955_UserLevel_Suspend(void)
{
	// remove polling task
	//hrtimer_stop(&pTimer_Link_Task);
	HDMI_Power_Down();
}

void EP955_UserLevel_Resume(void)
{
	// start polling task
	//hrtimer_start(&pTimer_Link_Task, ktime_set(0, LINK_TASK_TIME_BASE*1000000), HRTIMER_MODE_REL);
	HDMI_Power_Up();
}

void EP955_UserLevel_VideoConfig(BYTE rxOutput, RX_OUTPUT_RSLT inputVIC)
{
	//HDMI_Tx_Video_Config();

	// Color Format (Default RGB Input, assume that CE timing is Limit Range and IT timing is Full Range)
	switch((rxOutput & 0xF0) >> 4)
	{
		case HDMI_RX_OUTPUT_RGB:
			EP_DEV_DBG("Rx Output Color : RGB\n");
			EP956C_Registers.Video_Interface[1] = EP956E_Video_Interface_Setting_1__VIN_FMT__RGB;
		break;

		case HDMI_RX_OUTPUT_YUV444:
			EP_DEV_DBG("Rx Output Color : YUV\n");
			EP956C_Registers.Video_Interface[1] = EP956E_Video_Interface_Setting_1__VIN_FMT__YCC444;
		break;
		
		case HDMI_RX_OUTPUT_YUV422:
			EP_DEV_DBG("Rx Output Color : YUV422\n");
			EP956C_Registers.Video_Interface[1] = EP956E_Video_Interface_Setting_1__VIN_FMT__YCC422;
		break;				
	}

	//Rx output Video Interface Setting
	switch(rxOutput & 0x0F)
	{
		case HDMI_RX_OUTPUT_NORMAL:
			EP_DEV_DBG("Rx Output Normal Mode\n");
			EP956C_Registers.Video_Interface[0] =   //	EP956E_Video_Interface_Setting_0__DSEL  |
														EP956E_Video_Interface_Setting_0__BSEL  |
													//	EP956E_Video_Interface_Setting_0__EDGE  |
													//	EP956E_Video_Interface_Setting_0__FMT12 |

														EP956E_Video_Interface_Setting_0__DKEN  | // DK adjust -4 step for SIi Digital Video output
														0;
		break;

		case HDMI_RX_OUTPUT_MUXED:
			EP_DEV_DBG("Rx Output Muxed Mode\n");
			EP956C_Registers.Video_Interface[0] =	//	EP956E_Video_Interface_Setting_0__DSEL  |
													//	EP956E_Video_Interface_Setting_0__BSEL  |
														EP956E_Video_Interface_Setting_0__EDGE  |
														EP956E_Video_Interface_Setting_0__FMT12 |

														EP956E_Video_Interface_Setting_0__DKEN  | // DK adjust -4 step for SIi Digital Video output
														0;
		break;
		
		case HDMI_RX_OUTPUT_EMBEDED_SYNC:
			EP_DEV_DBG("Rx Output Muxed Mode and Embeded Sync\n");
			EP956C_Registers.Video_Interface[0] =	//	EP956E_Video_Interface_Setting_0__DSEL  |
													//	EP956E_Video_Interface_Setting_0__BSEL  |
														EP956E_Video_Interface_Setting_0__EDGE  |
														EP956E_Video_Interface_Setting_0__FMT12 |

														EP956E_Video_Interface_Setting_0__DKEN  | // DK adjust -4 step for SIi Digital Video output
														0;

			EP956C_Registers.Video_Interface[1] |= EP956E_Video_Interface_Setting_1__SYNC__Embeded;
		break;
	}

	// MHL Output setting
	EP956C_Registers.Video_Input_Format[0] = inputVIC;
	EP_DEV_DBG("input VIC %02X\n",EP956C_Registers.Video_Input_Format[0]);
}

void EP955_UserLevel_AudioConfig(BOOL isAudioIIS,BYTE AudioFreq)
{
	//HDMI_Tx_Audio_Config();
	// Audio Interface Setting
	EP956C_Registers.Audio_Interface = EP956E_Audio_Interface_Setting__CHANNEL__2ch;

	// Audio Input Format Setting
	if(isAudioIIS)
	{
		EP956C_Registers.Audio_Input_Format[0] = EP956E_Audio_Output_Format__AUDIO_TYPE__IIS; // 
		EP_DEV_DBG("Audio Set to IIS %02X\n",EP956C_Registers.Audio_Input_Format[0]);
	}
	else
	{
		EP956C_Registers.Audio_Input_Format[0] = EP956E_Audio_Output_Format__AUDIO_TYPE__SPDIF; // Use SPIDIF
		EP_DEV_DBG("Audio Set to SPDIF %02X\n",EP956C_Registers.Audio_Input_Format[0]);
	}

	// Rx Audio Sampling Frequency
	switch(AudioFreq) {
		case AUD_FREQ_32000Hz:
			EP_DEV_DBG("RX: 32KHz Audio Detected\n");
			EP956C_Registers.Audio_Input_Format[0] &= ~EP956E_Audio_Output_Format__ADO_FREQ;
			EP956C_Registers.Audio_Input_Format[0] |= EP956E_Audio_Output_Format__ADO_FREQ__32000Hz;
			break;
		case AUD_FREQ_44100Hz:
			EP_DEV_DBG("RX: 44.1KHz Audio Detected\n");
			EP956C_Registers.Audio_Input_Format[0] &= ~EP956E_Audio_Output_Format__ADO_FREQ;
			EP956C_Registers.Audio_Input_Format[0] |= EP956E_Audio_Output_Format__ADO_FREQ__44100Hz;
			break;
		case AUD_FREQ_48000Hz:
			EP_DEV_DBG("RX: 48KHz Audio Detected\n");
			EP956C_Registers.Audio_Input_Format[0] &= ~EP956E_Audio_Output_Format__ADO_FREQ;
			EP956C_Registers.Audio_Input_Format[0] |= EP956E_Audio_Output_Format__ADO_FREQ__48000Hz;
			break;
		case AUD_FREQ_88200Hz:
			EP_DEV_DBG("RX: 88.2KHz Audio Detected\n");
			EP956C_Registers.Audio_Input_Format[0] &= ~EP956E_Audio_Output_Format__ADO_FREQ;
			EP956C_Registers.Audio_Input_Format[0] |= EP956E_Audio_Output_Format__ADO_FREQ__88200Hz;
			break;
		case AUD_FREQ_96000Hz:
			EP_DEV_DBG("RX: 96KHz Audio Detected\n");
			EP956C_Registers.Audio_Input_Format[0] &= ~EP956E_Audio_Output_Format__ADO_FREQ;
			EP956C_Registers.Audio_Input_Format[0] |= EP956E_Audio_Output_Format__ADO_FREQ__96000Hz;
			break;
		case AUD_FREQ_176400Hz:
			EP_DEV_DBG("RX: 176.4KHz Audio Detected\n");
			EP956C_Registers.Audio_Input_Format[0] &= ~EP956E_Audio_Output_Format__ADO_FREQ;
			EP956C_Registers.Audio_Input_Format[0] |= EP956E_Audio_Output_Format__ADO_FREQ__176400Hz;
			break;
		case AUD_FREQ_192000Hz:
			EP_DEV_DBG("RX: 192KHz Audio Detected\n");
			EP956C_Registers.Audio_Input_Format[0] &= ~EP956E_Audio_Output_Format__ADO_FREQ;
			EP956C_Registers.Audio_Input_Format[0] |= EP956E_Audio_Output_Format__ADO_FREQ__192000Hz;
			break;
		default:
			EP_DEV_DBG("RX: ? Audio Detected, 0x%02X\n", AudioFreq);
	}
	
	EP_DEV_DBG("EP956C_Registers.Audio_Input_Format %02X\n",EP956C_Registers.Audio_Input_Format[0]);
}

int EP955_UserLevel_VideoEnable(BOOL enable)
{
	return 0;
}

int EP955_UserLevel_AudioEnable(BOOL enable)
{
	return 0;
}

CBUS_LINK_STATE EP955_UserLevel_GetState(void)
{
	return BackupLink_State;
}


void HDMI_Power_Up(void)
{
	//////////////////////////////////////////////////////////////////////////////////
	// HDMI Power Up
	//

	// Power Up EP956E
	EP956C_Registers.Power_Control = 0x00;

	//
	//////////////////////////////////////////////////////////////////////////////////
}

void HDMI_Power_Down(void)
{
	//////////////////////////////////////////////////////////////////////////////////
	// HDMI Power Down
	//

	// Power Down EP956E
	EP956C_Registers.Power_Control = EP956E_Power_Control__PD_HDMI;

	//
	//////////////////////////////////////////////////////////////////////////////////
}

void On_Ext_Int_EP956E()
{
	if(EP956C_Registers.Interrupt_Flags & EP956E_Interrupt_Flags__EDID_CHG) {
		EP956C_Registers.Interrupt_Flags &= ~EP956E_Interrupt_Flags__EDID_CHG; // Clear the flag
		Event_EDID_CH = 1;
	}
	if(EP956C_Registers.Interrupt_Flags & EP956E_Interrupt_Flags__VIDEO_CHG) {
		EP956C_Registers.Interrupt_Flags &= ~EP956E_Interrupt_Flags__VIDEO_CHG; // Clear the flag
		Event_VIDEO_CH = 1;
	}
	if(EP956C_Registers.Interrupt_Flags & EP956E_Interrupt_Flags__AUDIO_CHG) {
		EP956C_Registers.Interrupt_Flags &= ~EP956E_Interrupt_Flags__AUDIO_CHG; // Clear the flag
		Event_AUDIO_CH = 1;
	}
	if(EP956C_Registers.Interrupt_Flags & EP956E_Interrupt_Flags__MSC_MSG) {
		EP956C_Registers.Interrupt_Flags &= ~EP956E_Interrupt_Flags__MSC_MSG; // Clear the flag
		Event_MSC_MSG = 1;
	}
}
//==================================================================================================
//
// Android Driver Implementation
//
//==================================================================================================


//------------------------------------------------------------------------------------------------
// Call Back and Interrupt Functions
//------------------------------------------------------------------------------------------------

static enum hrtimer_restart callback_timer(struct hrtimer *timer)
{
	schedule_work(&work_Link_Task);
	
	return HRTIMER_NORESTART;

	//hrtimer_forward(pTimer_Link_Task, ktime_set(0, LINK_TASK_TIME_BASE*1000000));
	//return HRTIMER_RESTART;
}

static enum hrtimer_restart callback_EP955_timer(struct hrtimer *timer)
{
	schedule_work(&work_Timer_Task);
	
	return HRTIMER_NORESTART;
}

irqreturn_t callback_irq(int irq, void *handle)
{
	schedule_work(&work_Int);
	return IRQ_HANDLED;
}

void callback_work_Link_Task(struct work_struct *p)
{
	if(EP955_i2c_client == NULL) return;

	cancel_delayed_work_sync(&work_Int);
	
	// EP955 Link Task to check the CBUS connection every 55 ms
	// Set a timer to run this task every 55 ms 

	Link_State = EP956Controller_Task();

	if(BackupLink_State != Link_State)
	{
		BackupLink_State = Link_State;
		if(EP955_IntCB != NULL)
			EP955_IntCB(BackupLink_State);
	}

	if(Link_State == CBUS_LINK_STATE__USB_Mode) 
	{
		// USB Mode
		//hrtimer_forward(pTimer_Link_Task, ktime_set(0, 1000*1000000)); // 1 sec
		hrtimer_start(&pTimer_Link_Task, ktime_set(0, 1000*1000000), HRTIMER_MODE_REL); // 1 sec
			
		///////////////////////////////////////////////////////////////
		// Stop the thread that run EP955Control_Propagation_Task();
//		cancel_work_sync(work_PPG);
		///////////////////////////////////////////////////////////////
	}
	else 
	{
		if(EP956C_Registers.HDCP_State == A4_Authenticated) {
			hrtimer_start(&pTimer_Link_Task, ktime_set(0, 1000*1000000), HRTIMER_MODE_REL); // 1 sec
		}
		else {
			hrtimer_start(&pTimer_Link_Task, ktime_set(0, LINK_TASK_TIME_BASE*1000000), HRTIMER_MODE_REL);
		}

		// check int first
		//On_Ext_Int_EP956E();
		// EP956E Interrupt
		if(Event_EDID_CH) {
			Event_EDID_CH = 0;
			EP_DEV_DBG("EP956E EDID Change\n");
		}
		if(Event_VIDEO_CH) {
			Event_VIDEO_CH = 0;
			EP_DEV_DBG("EP956E Video Source Change\n");
		}
		if(Event_AUDIO_CH) {
			Event_AUDIO_CH = 0;
			EP_DEV_DBG("EP956E Audio Source Change\n");
		}
		if(Event_MSC_MSG) {
			Event_MSC_MSG = 0;
			EP_DEV_DBG("EP956E Get MSC Message, 0x%02X, 0x%02X\n", EP956C_Registers.MSC_Message[0], EP956C_Registers.MSC_Message[1]);


			//////////////////////////////////////////////////////////////////////////////////
			// Handle the RCP/RAP command
			//
			if(!(EP956C_Registers.MSC_Message[1] & 0x80))
			{
				Send_RCP_Key_Code(EP956C_Registers.MSC_Message[1] & 0x7F);
			}
			//
			//////////////////////////////////////////////////////////////////////////////////
		}
	}
}

void callback_EP955_Timer_Task(struct work_struct *p)
{
	// Timmer need to trigger every 5ms
	EP956Controller_Timer();

	hrtimer_start(&Timer_EP955, ktime_set(0, 5*1000000), HRTIMER_MODE_REL);
}

void callback_work_Int(struct work_struct *p)
{
	if(EP955_i2c_client == NULL) return;

	cancel_delayed_work_sync(&work_Link_Task);

	EP_DEV_DBG("EP955_interrupt_event_work() is called\n");
	
	EP956Controller_Interrupt();

	hrtimer_start(&pTimer_Link_Task, ktime_set(0, LINK_TASK_TIME_BASE*1000000), HRTIMER_MODE_REL);
}

//------------------------------------------------------------------------------------------------
void Interpret_VIN_FMT(BYTE VIN_FMT)
{
	switch(VIN_FMT) {
		case 0x00: EP_DEV_DBG("RGB Limited Range, ITU601 detected"); break;
		case 0x04: EP_DEV_DBG("RGB Limited Range, ITU709 detected"); break;
		case 0x02:
		case 0x06: EP_DEV_DBG("RGB Full Range");break;
		case 0x08: EP_DEV_DBG("YUV444, ITU601 detected"); break;
		case 0x09: EP_DEV_DBG("YUV422, ITU601 detected"); break;
		case 0x0A: EP_DEV_DBG("YUV444, xvYCC601 detected"); break;
		case 0x0B: EP_DEV_DBG("YUV422, xvYCC601 detected"); break;
		case 0x0C: EP_DEV_DBG("YUV444, ITU709 detected"); break;
		case 0x0D: EP_DEV_DBG("YUV422, ITU709 detected"); break;
		case 0x0E: EP_DEV_DBG("YUV444, xvYCC709 detected"); break;
		case 0x0F: EP_DEV_DBG("YUV422, xvYCC709 detected"); break;
		default: EP_DEV_DBG("Unknown");
			
	}
}

// Interpret Video Code
void Interpret_VTIME(BYTE VTIME)
{
	switch(VTIME) {
		case 1: EP_DEV_DBG("CEA-861D 640 x 480 (60Hz or 59.94Hz) Progressive 4:3"); break;
		case 2: EP_DEV_DBG("CEA-861D 720 x 480 (60Hz or 59.94Hz) Progressive 4:3"); break;
		case 3: EP_DEV_DBG("CEA-861D 720 x 480 (60Hz or 59.94Hz) Progressive 16:9"); break;
		case 4: EP_DEV_DBG("CEA-861D 1280 x 720 (60Hz or 59.94Hz) Progressive 16:9"); break;
		case 5: EP_DEV_DBG("CEA-861D 1920 x 1080 (60Hz or 59.94Hz) Interlaced 16:9"); break;
		case 6: EP_DEV_DBG("CEA-861D 720(1440) x 480 (60Hz or 59.94Hz) Interlaced 4:3"); break;
		case 7: EP_DEV_DBG("CEA-861D 720(1440) x 480 (60Hz or 59.94Hz) Interlaced 16:9"); break;
		case 8: EP_DEV_DBG("CEA-861D 720(1440) x 240 (60Hz or 59.94Hz) Progressive 4:3"); break;
		case 9: EP_DEV_DBG("CEA-861D 720(1440) x 240 (60Hz or 59.94Hz) Progressive 16:9"); break;
		case 10: EP_DEV_DBG("CEA-861D 2880 x 480 (60Hz or 59.94Hz) Interlaced 4:3"); break;
		case 11: EP_DEV_DBG("CEA-861D 2880 x 480 (60Hz or 59.94Hz) Interlaced 16:9"); break;
		case 12: EP_DEV_DBG("CEA-861D 2880 x 240 (60Hz or 59.94Hz) Progressive 4:3"); break;
		case 13: EP_DEV_DBG("CEA-861D 2880 x 240 (60Hz or 59.94Hz) Progressive 16:9"); break;
		case 14: EP_DEV_DBG("CEA-861D 1440 x 480 (60Hz or 59.94Hz) Progressive 4:3"); break;
		case 15: EP_DEV_DBG("CEA-861D 1440 x 480 (60Hz or 59.94Hz) Progressive 16:9"); break;
		case 16 : EP_DEV_DBG("CEA-861D 1920 x 1080 (60Hz or 59.94Hz) Progressive 16:9"); break;
		case 17: EP_DEV_DBG("CEA-861D 720 x 576 (50Hz) Progressive 4:3"); break;
		case 18: EP_DEV_DBG("CEA-861D 720 x 576 (50Hz) Progressive 16:9"); break;
		case 19: EP_DEV_DBG("CEA-861D 1280 x 720 (50Hz) Progressive 16:9 "); break;
		case 20: EP_DEV_DBG("CEA-861D 1920 x 1080 (50Hz) Interlaced 16:9"); break;
		case 21: EP_DEV_DBG("CEA-861D 720(1440) x 576 (50Hz) Interlaced 4:3"); break;
		case 22: EP_DEV_DBG("CEA-861D 720(1440) x 576 (50Hz) Interlaced 16:9"); break;
		case 23: EP_DEV_DBG("CEA-861D 720(1440) x 288 (50Hz) Progressive 4:3"); break;
		case 24: EP_DEV_DBG("CEA-861D 720(1440) x 288 (50Hz) Progressive 16:9"); break;
		case 25: EP_DEV_DBG("CEA-861D 2880 x 576 (50Hz) Interlaced 4:3"); break;
		case 26: EP_DEV_DBG("CEA-861D 2880 x 576 (50Hz) Interlaced 16:9"); break;
		case 27: EP_DEV_DBG("CEA-861D 2880 x 288 (50Hz) Progressive 4:3"); break;
		case 28: EP_DEV_DBG("CEA-861D 2880 x 288 (50Hz) Progressive 16:9"); break;
		case 29: EP_DEV_DBG("CEA-861D 1440 x 576 (50Hz) Progressive 4:3"); break;
		case 30: EP_DEV_DBG("CEA-861D 1440 x 576 (50Hz) Progressive 16:9"); break;
		case 31: EP_DEV_DBG("CEA-861D 1920 x 1080 (50Hz) Progressive 16:9"); break;
		case 32: EP_DEV_DBG("CEA-861D 1920 x 1080 (24Hz) Progressive 16:9"); break;
		case 33: EP_DEV_DBG("CEA-861D 1920 x 1080 (25Hz) Progressive 16:9"); break;
		case 34: EP_DEV_DBG("CEA-861D 1920 x 1080 (30Hz) Progressive 16:9"); break;

		case 35: EP_DEV_DBG("CEA-861D 2880 x 480 (60Hz or 59.94Hz) Progressive 4:3"); break;
		case 36: EP_DEV_DBG("CEA-861D 2880 x 480 (60Hz or 59.94Hz) Progressive 16:9"); break;
		case 37: EP_DEV_DBG("CEA-861D 2880 x 576 (60Hz or 59.94Hz) Progressive 4:3"); break;
		case 38: EP_DEV_DBG("CEA-861D 2880 x 576 (60Hz or 59.94Hz) Progressive 16:9"); break;
		case 39: EP_DEV_DBG("CEA-861D 1920 x 1080 (50Hz, H = 31.25KHz) Interlaced 16:9"); break;
		case 40: EP_DEV_DBG("CEA-861D 1920 x 1080 (100Hz) Interlaced 16:9"); break;
		case 41: EP_DEV_DBG("CEA-861D 1280 x 720 (100Hz) Progressive 16:9"); break;
		case 42: EP_DEV_DBG("CEA-861D 720 x 576 (100Hz) Progressive 4:3"); break;
		case 43: EP_DEV_DBG("CEA-861D 720 x 576 (100Hz) Progressive 16:9"); break;
		case 44: EP_DEV_DBG("CEA-861D 720(1440) x 576 (100Hz) Interlaced 4:3"); break;
		case 45: EP_DEV_DBG("CEA-861D 720(1440) x 576 (100Hz) Interlaced 16:9"); break;
		case 46: EP_DEV_DBG("CEA-861D 1920 x 1080 (120Hz or 119.88Hz) Interlaced 16:9"); break;
		case 47: EP_DEV_DBG("CEA-861D 1280 x 720 (120Hz or 119.88Hz) Progressive 16:9"); break;
		case 48: EP_DEV_DBG("CEA-861D 720 x 480 (120Hz or 119.88Hz) Progressive 4:3"); break;
		case 49: EP_DEV_DBG("CEA-861D 720 x 480 (120Hz or 119.88Hz) Progressive 16:9"); break;
		case 50: EP_DEV_DBG("CEA-861D 720(1440) x 480 (120Hz or 119.88Hz) Interlaced 4:3"); break;
		case 51: EP_DEV_DBG("CEA-861D 720(1440) x 480 (120Hz or 119.88Hz) Interlaced 16:9"); break;
		case 52: EP_DEV_DBG("CEA-861D 720 x 576 (200Hz) Progressive 4:3"); break;
		case 53: EP_DEV_DBG("CEA-861D 720 x 576 (200Hz) Progressive 16:9"); break;
		case 54: EP_DEV_DBG("CEA-861D 720(1440) x 576 (200Hz) Interlaced 4:3"); break;
		case 55: EP_DEV_DBG("CEA-861D 720(1440) x 576 (200Hz) Interlaced 16:9"); break;
		case 56: EP_DEV_DBG("CEA-861D 720 x 480 (240Hz or 239Hz) Progressive 4:3"); break;
		case 57: EP_DEV_DBG("CEA-861D 720 x 480 (240Hz or 239Hz) Progressive 16:9"); break;
		case 58: EP_DEV_DBG("CEA-861D 720(1440) x 480 (240Hz or 239Hz) Interlaced 4:3"); break;
		case 59: EP_DEV_DBG("CEA-861D 720(1440) x 480 (240Hz or 239Hz) Interlaced 16:9"); break;

		case 128: EP_DEV_DBG("VESA-DMT10 640 x 350 Progressive"); break;
		case 132: EP_DEV_DBG("VESA-DMT10 640 x 400 Progressive"); break;
		case 136: EP_DEV_DBG("VESA-DMT10 720 x 400 Progressive"); break;
		case 140: EP_DEV_DBG("VESA-DMT10 640 x 480 Progressive"); break;
		case 144: EP_DEV_DBG("VESA-DMT10 800 x 600 Progressive"); break;
		case 148: EP_DEV_DBG("VESA-DMT10 848 x 480 Progressive"); break;
		case 152: EP_DEV_DBG("VESA-DMT10 1024 x 768 Progressive"); break;
		case 156: EP_DEV_DBG("VESA-DMT10 1152 x 864 Progressive"); break;
		case 160: EP_DEV_DBG("VESA-DMT10 1280 x 768 Progressive"); break;
		case 164: EP_DEV_DBG("VESA-DMT10 1280 x 960 Progressive"); break;
		case 168: EP_DEV_DBG("VESA-DMT10 1280 x 1024 Progressive"); break;
		case 172: EP_DEV_DBG("VESA-DMT10 1360 x 768 Progressive"); break;
		case 176: EP_DEV_DBG("VESA-DMT10 1400 x 1050 Progressive"); break;
		case 180: EP_DEV_DBG("VESA-DMT10 1440 x 900 Progressive"); break;
		case 184: EP_DEV_DBG("VESA-DMT10 1600 x 1200 Progressive"); break;
		case 188: EP_DEV_DBG("VESA-DMT10 1680 x 1050 Progressive"); break;
		case 192: EP_DEV_DBG("VESA-DMT10 1792 x 1344 Progressive"); break;
		case 196: EP_DEV_DBG("VESA-DMT10 1856 x 1392 Progressive"); break;
		case 200: EP_DEV_DBG("VESA-DMT10 1920 x 1200 Progressive"); break;
		case 204: EP_DEV_DBG("VESA-DMT10 1920 x 1440 Progressive"); break;

		default: EP_DEV_DBG("Unknown");
	}
}

void Interpret_SAMP_FREQ(BYTE SAMP_FREQ)
{
	switch(SAMP_FREQ) {
		case 0:	EP_DEV_DBG("32KHz"); break;
		case 1:	EP_DEV_DBG("44.1KHz"); break;
		case 2:	EP_DEV_DBG("48KHz"); break;
		case 3:	EP_DEV_DBG("88.2KHz") ;break;
		case 4:	EP_DEV_DBG("96KHz"); break;
		case 5:	EP_DEV_DBG("176.4KHz"); break;
		case 6:	EP_DEV_DBG("192KHz"); break;
		case 7:	EP_DEV_DBG("786KHz"); break;
		default: EP_DEV_DBG("Unknown");	
	}
}

void Interpret_ADSFREQ(BYTE ADS_FREQ)
{
	switch(ADS_FREQ) {
		case 0x03:	EP_DEV_DBG("32KHz"); break;
		case 0x00:	EP_DEV_DBG("44.1KHz"); break;
		case 0x02:	EP_DEV_DBG("48KHz"); break;
		case 0x08:	EP_DEV_DBG("88.2KHz") ;break;
		case 0x0A:	EP_DEV_DBG("96KHz"); break;
		case 0x0C:	EP_DEV_DBG("176.4KHz"); break;
		case 0x0E:	EP_DEV_DBG("192KHz"); break;
		default: EP_DEV_DBG("Unknown");	
	}
}



void HDMI_Dump_Status()
{
	WORD Temp_USHORT;
	BYTE regAddr;
	BYTE Temp_Byte[1];

	//display_sii_rx_control_registers();
	
	// System Status
	EP_DEV_DBG("======= Dump EP956 register information =======\n");
	EP_DEV_DBG("Main Address %X\n",&EP956C_Registers);
	EP_DEV_DBG("EP956C_Registers.Audio_Input_Format %X\n",EP956C_Registers.Audio_Input_Format[0]);
	
	return;
	EP_DEV_DBG("[Reg Map]");
	for(Temp_USHORT = 0; Temp_USHORT < 0x89; ++Temp_USHORT) {
		if(Temp_USHORT%16 == 0) 
		{
			EP_DEV_DBG("\n");
			EP_DEV_DBG("$%02X:", Temp_USHORT );
		}
		if(Temp_USHORT%8 == 0) EP_DEV_DBG(" ");
		
		regAddr = (BYTE) Temp_USHORT;
		EP956_Reg_Read(regAddr, Temp_Byte, 1);
		EP_DEV_DBG("0x%02X,", Temp_Byte[0] );
	}
	EP_DEV_DBG("\n");

	
	// System Status
	EP_DEV_DBG("======= Dump EP956 Controller information =======\n");

	EP_DEV_DBG("[EDID Data]");
	for(Temp_USHORT = 0; Temp_USHORT < 256; ++Temp_USHORT) {
		if(Temp_USHORT%16 == 0) EP_DEV_DBG("\n");
		if(Temp_USHORT%8 == 0) EP_DEV_DBG(" ");
		EP_DEV_DBG("0x%02X,", (int)EP956C_Registers.Readed_EDID[Temp_USHORT] );
	}
	EP_DEV_DBG("\n");
	
	EP_DEV_DBG("\n");
	EP_DEV_DBG("[Revision & Configuration]\n");
	EP_DEV_DBG("VendorID=0x%0.4X, ", EP956C_Registers.VendorID );
	EP_DEV_DBG("DeviceID=0x%0.4X, ", EP956C_Registers.DeviceID );
	EP_DEV_DBG("Version=%d.%d\n", (int)EP956C_Registers.Version_Major, (int)EP956C_Registers.Version_Minor );
	
	EP_DEV_DBG("\n");
	EP_DEV_DBG("[Interrupt Flags]\n");
	EP_DEV_DBG("EDID_CHG=%d, ", (int)((EP956C_Registers.Interrupt_Flags & EP956E_Interrupt_Flags__EDID_CHG)?1:0) );
	EP_DEV_DBG("VIDEO_CHG=%d, ", (int)((EP956C_Registers.Interrupt_Flags & EP956E_Interrupt_Flags__VIDEO_CHG)?1:0) );
	EP_DEV_DBG("AUDIO_CHG=%d\n", (int)((EP956C_Registers.Interrupt_Flags & EP956E_Interrupt_Flags__AUDIO_CHG)?1:0) );

	EP_DEV_DBG("\n");
	EP_DEV_DBG("[System Status]\n");
	EP_DEV_DBG("RSEN=%d, ", (int)((EP956C_Registers.System_Status & EP956E_System_Status__RSEN)?1:0) );
	EP_DEV_DBG("HTPLG=%d, ", (int)((EP956C_Registers.System_Status & EP956E_System_Status__HTPLG)?1:0) );
	EP_DEV_DBG("KEY_FAIL=%d, ", (int)((EP956C_Registers.System_Status & EP956E_System_Status__KEY_FAIL)?1:0) );

	EP_DEV_DBG("\n");
	EP_DEV_DBG("[EDID Status]\n");
	EP_DEV_DBG("EDID_HDMI=%d, ", (int)((EP956C_Registers.EDID_Status & EP956E_EDID_Status__HDMI)?1:0) );
	EP_DEV_DBG("DDC_STATUS=%d\n", (int)(EP956C_Registers.EDID_Status & 0x0F) );
	EP_DEV_DBG("VIDEO_DATA_ADDR=0x%02X, ", (int)EP956C_Registers.EDID_VideoDataAddr );
	EP_DEV_DBG("AUDIO_DATA_ADDR=0x%02X, ", (int)EP956C_Registers.EDID_AudioDataAddr );
	EP_DEV_DBG("SPEAKER_DATA_ADDR=0x%02X, ", (int)EP956C_Registers.EDID_SpeakerDataAddr );
	EP_DEV_DBG("VENDOR_DATA_ADDR=0x%02X\n", (int)EP956C_Registers.EDID_VendorDataAddr );
	EP_DEV_DBG("ASFREQ=0x%02X, ", (int)EP956C_Registers.EDID_ASFreq );
	EP_DEV_DBG("ACHANNEL_PCM=%d, ", (int)EP956C_Registers.EDID_AChannel_PCM );
	EP_DEV_DBG("ACHANNEL_DSD=%d\n", (int)EP956C_Registers.EDID_AChannel_DSD );

	EP_DEV_DBG("\n");
	EP_DEV_DBG("[Video Status]\n");
	EP_DEV_DBG("Interface=0x%02X, ", (int)EP956C_Registers.Video_Params_Backup.Interface );
	EP_DEV_DBG("VideoSettingIndex=%d (", (int)EP956C_Registers.Video_Params_Backup.VideoSettingIndex );
		Interpret_VTIME(EP956_VDO_Settings[EP956C_Registers.Video_Params_Backup.VideoSettingIndex].VideoCode);
		EP_DEV_DBG("), ");
	EP_DEV_DBG("HVPol=%d, ", (int)EP956C_Registers.Video_Params_Backup.HVPol );
	EP_DEV_DBG("SyncMode=%d, ", (int)EP956C_Registers.Video_Params_Backup.SyncMode );
	EP_DEV_DBG("FormatIn=%d, ", (int)EP956C_Registers.Video_Params_Backup.FormatIn );
	EP_DEV_DBG("FormatOut=%d, ", (int)EP956C_Registers.Video_Params_Backup.FormatOut );
	EP_DEV_DBG("ColorSpace=%d, ", (int)EP956C_Registers.Video_Params_Backup.ColorSpace );
	EP_DEV_DBG("AFARate=%d\n", (int)EP956C_Registers.Video_Params_Backup.AFARate );

	EP_DEV_DBG("\n");
	EP_DEV_DBG("[Audio Status]\n");
	EP_DEV_DBG("Interface=0x%02X, ", (int)EP956C_Registers.Audio_Params_Backup.Interface );
	EP_DEV_DBG("VideoSettingIndex=%d, ", (int)EP956C_Registers.Audio_Params_Backup.VideoSettingIndex );
	EP_DEV_DBG("ChannelNumber=%d, ", (int)EP956C_Registers.Audio_Params_Backup.ChannelNumber );
	EP_DEV_DBG("ADSRate=%d, ", (int)EP956C_Registers.Audio_Params_Backup.ADSRate );
	EP_DEV_DBG("InputFrequency=%d (", (int)EP956C_Registers.Audio_Params_Backup.InputFrequency );
		Interpret_ADSFREQ(EP956C_Registers.Audio_Params_Backup.InputFrequency);
		EP_DEV_DBG("), ");
	EP_DEV_DBG("VFS=%d, ", (int)EP956C_Registers.Audio_Params_Backup.VFS );
	EP_DEV_DBG("ChannelStatusControl=%d\n", (int)EP956C_Registers.Audio_Params_Backup.ChannelStatusControl );

	EP_DEV_DBG("\n");
	EP_DEV_DBG("[Power Control]\n");
	EP_DEV_DBG("PD_HDMI=%d, ", (int)((EP956C_Registers.Power_Control & EP956E_Power_Control__PD_HDMI)?1:0) );
	EP_DEV_DBG("PD_TOT=%d\n", (int)((EP956C_Registers.Power_Control & EP956E_Power_Control__PD_TOT)?1:0) );

	EP_DEV_DBG("\n");
	EP_DEV_DBG("[System Configuration]\n");
	EP_DEV_DBG("HDCP_DIS=%d, ", (int)((EP956C_Registers.System_Configuration & EP956E_System_Configuration__HDCP_DIS)?1:0) );
	EP_DEV_DBG("AUDIO_DIS=%d, ", (int)((EP956C_Registers.System_Configuration & EP956E_System_Configuration__AUDIO_DIS)?1:0) );
	EP_DEV_DBG("VIDEO_DIS=%d\n", (int)((EP956C_Registers.System_Configuration & EP956E_System_Configuration__VIDEO_DIS)?1:0) );

	EP_DEV_DBG("\n");
	EP_DEV_DBG("[Interrupt Enable]\n");
	EP_DEV_DBG("EDID_CHG=%d, ", (int)((EP956C_Registers.Interrupt_Enable & EP956E_Interrupt_Enable__EDID_CHG)?1:0) );
	EP_DEV_DBG("VS_PERIOD_CHG=%d, ", (int)((EP956C_Registers.Interrupt_Enable & EP956E_Interrupt_Enable__VIDEO_CHG)?1:0) );
	EP_DEV_DBG("AS_FREQ_CHG=%d\n", (int)((EP956C_Registers.Interrupt_Enable & EP956E_Interrupt_Enable__AUDIO_CHG)?1:0) );

	EP_DEV_DBG("\n");
	EP_DEV_DBG("[Video Interface 0]\n");
	EP_DEV_DBG("DK=%d, ", (int)((EP956C_Registers.Video_Interface[0] & EP956E_Video_Interface_Setting_0__DK)?1:0) );
	EP_DEV_DBG("DKEN=%d, ", (int)((EP956C_Registers.Video_Interface[0] & EP956E_Video_Interface_Setting_0__DKEN)?1:0) );
	EP_DEV_DBG("DSEL=%d, ", (int)((EP956C_Registers.Video_Interface[0] & EP956E_Video_Interface_Setting_0__DSEL)?1:0) );
	EP_DEV_DBG("BSEL=%d, ", (int)((EP956C_Registers.Video_Interface[0] & EP956E_Video_Interface_Setting_0__BSEL)?1:0) );
	EP_DEV_DBG("EDGE=%d, ", (int)((EP956C_Registers.Video_Interface[0] & EP956E_Video_Interface_Setting_0__EDGE)?1:0) );
	EP_DEV_DBG("FMT12=%d\n", (int)((EP956C_Registers.Video_Interface[0] & EP956E_Video_Interface_Setting_0__FMT12)?1:0) );

	EP_DEV_DBG("\n");
	EP_DEV_DBG("[Video Interface 1]\n");
	EP_DEV_DBG("COLOR=%d, ", (int)((EP956C_Registers.Video_Interface[1] & EP956E_Video_Interface_Setting_1__COLOR)>>4) );
	EP_DEV_DBG("SYNC=%d, ", (int)((EP956C_Registers.Video_Interface[1] & EP956E_Video_Interface_Setting_1__SYNC)>>2) );
	EP_DEV_DBG("VIN_FMT=%d\n", (int)((EP956C_Registers.Video_Interface[1] & EP956E_Video_Interface_Setting_1__VIN_FMT)>>0) );

	EP_DEV_DBG("\n");
	EP_DEV_DBG("[Audio Interface]\n");
	EP_DEV_DBG("CHANNEL=%d, ", (int) (EP956C_Registers.Audio_Interface & EP956E_Audio_Interface_Setting__CHANNEL)>>4 );
	EP_DEV_DBG("SD_DIR=%d, ", (int)((EP956C_Registers.Audio_Interface & EP956E_Audio_Interface_Setting__SD_DIR)?1:0) );
	EP_DEV_DBG("WS_M=%d, ", (int)((EP956C_Registers.Audio_Interface & EP956E_Audio_Interface_Setting__WS_M)?1:0) );
	EP_DEV_DBG("WS_POL=%d, ", (int)((EP956C_Registers.Audio_Interface & EP956E_Audio_Interface_Setting__WS_POL)?1:0) );
	EP_DEV_DBG("SCK_POL=%d\n", (int)((EP956C_Registers.Audio_Interface & EP956E_Audio_Interface_Setting__SCK_POL)?1:0) );	

	EP_DEV_DBG("\n");
	EP_DEV_DBG("[Video Input Format]\n");
	EP_DEV_DBG("VIC=%d, ", (int)EP956C_Registers.Video_Input_Format[0] );
	EP_DEV_DBG("AFAR_VIF=0x%02X\n", (int)EP956C_Registers.Video_Input_Format[1] );	

	EP_DEV_DBG("\n");
	EP_DEV_DBG("[Audio Input Format]\n");
	EP_DEV_DBG("Audio_Input_Format=0x%02X\n", (int)EP956C_Registers.Audio_Input_Format[0] );	
}
//------------------------------------------------------------------------------------------------

void Send_RCP_Key_Code(BYTE rcp_key_code)
{
	if (rcp_key_code > 0x7F || rcp_key_code < 0x00)
	{
		EP_DEV_DBG_ERROR("get rcp_key_code error: rcp_key_code = 0x%x\n",rcp_key_code);
		return;
	}
	
	switch(rcp_key_code) 
	{
		case RCP_Select:
			EP_DEV_DBG("RCP: Select\n");
			Send_Key_Event(KEY_ENTER);
			break;
		case RCP_Up:
			EP_DEV_DBG("RCP: Up\n");
			Send_Key_Event(KEY_UP);
			break;
		case RCP_Down:
			EP_DEV_DBG("RCP: Down\n");
			Send_Key_Event(KEY_DOWN);
			break;
		case RCP_Left:
			EP_DEV_DBG("RCP: Left\n");
			Send_Key_Event(KEY_LEFT);
			break;
		case RCP_Right:
			EP_DEV_DBG("RCP: Right\n");
			Send_Key_Event(KEY_RIGHT);
			break;
#if 0
		case RCP_Right_Up:
			EP_DEV_DBG("RCP: Right_Up\n");
			break;
		case RCP_Right_Down:
			EP_DEV_DBG("RCP: Right_Down\n");
			break;
		case RCP_Left_Up:
			EP_DEV_DBG("RCP: Left_Up\n");
			break;
		case RCP_Left_Down:
			EP_DEV_DBG("RCP: Left_Down\n");
			break;
#endif
		case RCP_Root_Menu:
			EP_DEV_DBG("RCP: Root_Menu\n");
			Send_Key_Event(KEY_HOMEPAGE);
			break;
#if 0
		case RCP_Setup_Menu:
			EP_DEV_DBG("RCP: Setup_Menu\n");
			break;
		case RCP_Contents_Menu:
			EP_DEV_DBG("RCP: Contents_Menu\n");
			break;
		case RCP_Favorite_Menu:
			EP_DEV_DBG("RCP: Favorite_Menu\n");
			break;
#endif
		case RCP_Exit:
			EP_DEV_DBG("RCP: Exit\n");
			Send_Key_Event(KEY_BACK);
			break;
			
#if 0
		case RCP_Numeric_0:
			EP_DEV_DBG("RCP: Numeric_0\n");
			break;
		case RCP_Numeric_1:
			EP_DEV_DBG("RCP: Numeric_1\n");
			break;
		case RCP_Numeric_2:
			EP_DEV_DBG("RCP: Numeric_2\n");
			break;
		case RCP_Numeric_3:
			EP_DEV_DBG("RCP: Numeric_3\n");
			break;
		case RCP_Numeric_4:
			EP_DEV_DBG("RCP: Numeric_4\n");
			break;
		case RCP_Numeric_5:
			EP_DEV_DBG("RCP: Numeric_5\n");
			break;
		case RCP_Numeric_6:
			EP_DEV_DBG("RCP: Numeric_6\n");
			break;
		case RCP_Numeric_7:
			EP_DEV_DBG("RCP: Numeric_7\n");
			break;
		case RCP_Numeric_8:
			EP_DEV_DBG("RCP: Numeric_8\n");
			break;
		case RCP_Numeric_9:
			EP_DEV_DBG("RCP: Numeric_9\n");
			break;
		case RCP_Dot:
			EP_DEV_DBG("RCP: Dot\n");
			break;
		case RCP_Enter:
			EP_DEV_DBG("RCP: Enter\n");
			break;
		case RCP_Clear:
			EP_DEV_DBG("RCP: Clear\n");
			break;
			
		case RCP_Channel_Up:
			EP_DEV_DBG("RCP: Channel_Up\n");
			break;
		case RCP_Channel_Down:
			EP_DEV_DBG("RCP: Channel_Down\n");
			break;
		case RCP_Previous_Channel:
			EP_DEV_DBG("RCP: Previous_Channel\n");
			break;
			
		case RCP_Volume_Up:
			EP_DEV_DBG("RCP: Volume_Up\n");
			break;
		case RCP_Volume_Down:
			EP_DEV_DBG("RCP: Volume_Down\n");
			break;
		case RCP_Mute:
			EP_DEV_DBG("RCP: Mute\n");
			break;
#endif
		case RCP_Play:
			EP_DEV_DBG("RCP: Play\n");
			Send_Key_Event(KEY_PLAYCD);
			break;
		case RCP_Stop:
			EP_DEV_DBG("RCP: Stop\n");
			Send_Key_Event(KEY_STOPCD);
			break;
		case RCP_Pause:
			EP_DEV_DBG("RCP: Pause\n");
			Send_Key_Event(KEY_PLAYPAUSE);
			break;
#if 0
		case RCP_Record:
			EP_DEV_DBG("RCP: Record\n");
			break;
#endif
		case RCP_Rewind:
			EP_DEV_DBG("RCP: Rewind\n");
			Send_Key_Event(KEY_REWIND);
			break;
		case RCP_Fast_Forward:
			EP_DEV_DBG("RCP: Fast_Forward\n");
			Send_Key_Event(KEY_FASTFORWARD);
			break;
#if 0
		case RCP_Eject:
			EP_DEV_DBG("RCP: Eject\n");
			break;
#endif
		case RCP_Forward:
			EP_DEV_DBG("RCP: Forward\n");
			Send_Key_Event(KEY_NEXTSONG);
			break;
		case RCP_Backward:
			EP_DEV_DBG("RCP: Backward\n");
			Send_Key_Event(KEY_PREVIOUSSONG);
			break;
#if 0
		case RCP_Angle:
			EP_DEV_DBG("RCP: Angle\n");
			break;
		case RCP_Subpicture:
			EP_DEV_DBG("RCP: Subpicture\n");
			break;
#endif
		case RCP_Play_Function:
			EP_DEV_DBG("RCP: Play_Function\n");
			Send_Key_Event(KEY_PLAYCD);
			break;
		case RCP_Pause_Play_Function:
			EP_DEV_DBG("RCP: Pause_Play_Function\n");
			Send_Key_Event(KEY_PLAYPAUSE);
			break;
#if 0
		case RCP_Record_Function:
			EP_DEV_DBG("RCP: Record_Function\n");
			break;
		case RCP_Pause_Record_Function:
			EP_DEV_DBG("RCP: Pause_Record_Function\n");
			break;
#endif
		case RCP_Stop_Function:
			EP_DEV_DBG("RCP: Stop_Function\n");
			Send_Key_Event(KEY_STOPCD);
			break;
#if 0
		case RCP_Mute_Function:
			EP_DEV_DBG("RCP: Mute_Function\n");
			break;
		case RCP_Restore_Volume_Function:
			EP_DEV_DBG("RCP: Restore_Volume_Function\n");
			break;
		case RCP_Tune_Function:
			EP_DEV_DBG("RCP: Tune_Function\n");
			break;
		case RCP_Select_Media_Function:
			EP_DEV_DBG("RCP: Select_Media_Function\n");
			break;
#endif
#ifdef SPECIFICATION_MHL_1_2
		// Color Keys are not defined in MHL 1.1
		case RCP_F1:
			EP_DEV_DBG("RCP: F1(Blue)\n");
			break;
		case RCP_F2:
			EP_DEV_DBG("RCP: F2(Red)\n");
			break;
		case RCP_F3:
			EP_DEV_DBG("RCP: F3(Green)\n");
			break;
		case RCP_F4:
			EP_DEV_DBG("RCP: F4(Yellow)\n");
			break;
#endif
#if 0
		case RCP_F5:
			EP_DEV_DBG("RCP: F5\n");
			break;
		case RCP_Vendor_Specific:
			EP_DEV_DBG("RCP: Vendor_Specific\n");
			break;
#endif
		default:
			EP_DEV_DBG_ERROR("RCP: Unknown Key Code = 0x%x\n", rcp_key_code);
			break;
	}
}

void Send_Key_Event(unsigned int key_event)
{
	if (key_event < 0)
	{
		EP_DEV_DBG_ERROR("get key_event error: key_event = %d\n",key_event);
		return;
	}
	
	EP_DEV_DBG("send key_event as input key event: key_event = %d\n",key_event);
	
	//input_report_key(dev_keyboard, key_event, 1);
	//input_sync(dev_keyboard);
	//input_report_key(dev_keyboard, key_event, 0);
	//input_sync(dev_keyboard);
}

inline int Init_Keyboard(void)
{
#if 0
    int error;

	dev_keyboard = input_allocate_device();
	if (!dev_keyboard)
	{
		EP_DEV_DBG_ERROR("dev_keyboard: Not enough memory\n");
		error = -ENOMEM;
		goto err_free_irq;
	}

	set_bit(EV_KEY, dev_keyboard->evbit);
	set_bit(EV_REP, dev_keyboard->evbit);

	dev_keyboard->phys = "atakbd/input0";
	dev_keyboard->id.bustype = BUS_HOST;

	set_bit(KEY_UP, dev_keyboard->keybit);
	set_bit(KEY_DOWN, dev_keyboard->keybit);
	set_bit(KEY_LEFT, dev_keyboard->keybit);
	set_bit(KEY_RIGHT, dev_keyboard->keybit);
	set_bit(KEY_ENTER, dev_keyboard->keybit);
	set_bit(KEY_BACK, dev_keyboard->keybit);
	set_bit(KEY_PLAYPAUSE, dev_keyboard->keybit);
	set_bit(KEY_STOP, dev_keyboard->keybit);
	set_bit(KEY_SELECT, dev_keyboard->keybit);
	set_bit(KEY_OK,dev_keyboard->keybit);
	set_bit(KEY_REPLY,dev_keyboard->keybit);
	set_bit(KEY_PLAYCD,dev_keyboard->keybit);
	set_bit(KEY_STOPCD,dev_keyboard->keybit);

	set_bit(BTN_LEFT,dev_keyboard->keybit);
	set_bit(BTN_SELECT,dev_keyboard->keybit);

	set_bit(KEY_HOMEPAGE,dev_keyboard->keybit);
	set_bit(KEY_REWIND,dev_keyboard->keybit);
	set_bit(KEY_FASTFORWARD,dev_keyboard->keybit);
	set_bit(KEY_NEXTSONG,dev_keyboard->keybit);
	set_bit(KEY_PREVIOUSSONG,dev_keyboard->keybit);
	set_bit(KEY_MENU,dev_keyboard->keybit);

	dev_keyboard->id.bustype = BUS_USB;
	dev_keyboard->id.vendor  = 0x1095;
	dev_keyboard->id.product = 0x9244;
	dev_keyboard->id.version = 0xA;				//use version to distinguish mouse from keyboard

	error = input_register_device(dev_keyboard);
	if (error) {
			EP_DEV_DBG_ERROR("dev_keyboard: Failed to register device\n");
			goto err_free_dev;
	}

	EP_DEV_DBG("dev_keyboard: driver loaded\n");

	return 0;

 err_free_dev:
         input_free_device(dev_keyboard);
 err_free_irq:
         return error;
#endif
}



