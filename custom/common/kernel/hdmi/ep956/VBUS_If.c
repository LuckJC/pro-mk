/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

  File        :  VBUS_If.c

  Description :  EP936E DDC Interface

\******************************************************************************/

#include "VBUS_If.h"

//--------------------------------------------------------------------------------------------------

#define VBUS_STABLE_COUNT 3

//--------------------------------------------------------------------------------------------------

// Private Data

GPIO_PORT P5V_Control_Port; 
BYTE P5V_Control_Bit;
GPIO_PORT VBUS_Control_Port; 
BYTE VBUS_Control_Bit;
GPIO_PORT VBUS_Detect_Port; 
BYTE VBUS_Detect_Bit;


static int i;

//==================================================================================================
//
// Public Function Implementation
//

//--------------------------------------------------------------------------------------------------
// Hardware Interface

void VBUS_Ouput_Enable(void)
{
	// VBUS low enable, set to close
	//GPIO_clear(VBUS_Control_Port, VBUS_Control_Bit);	
	// P5V high enable, set to open
	//GPIO_clear(P5V_Control_Port, P5V_Control_Bit);
}
void VBUS_Ouput_Disable(void)
{
	// VBUS low enable, set to close
	//GPIO_set(VBUS_Control_Port, VBUS_Control_Bit);	
	// P5V high enable, set to open
	//GPIO_clear(P5V_Control_Port, P5V_Control_Bit);
}

void VBUS_If_Initial(GPIO_PORT P5VControlPort, BYTE P5VControlBitMask, GPIO_PORT ControlPort, BYTE ControlBitMask, GPIO_PORT DetectPort, BYTE DetectBitMask)
{
	P5V_Control_Port = P5VControlPort;
	P5V_Control_Bit = P5VControlBitMask;
	VBUS_Control_Port = ControlPort;
	VBUS_Control_Bit = ControlBitMask;
	VBUS_Detect_Port = DetectPort;
	VBUS_Detect_Bit = DetectBitMask;

#ifdef MHL_CODE
	VBUS_Ouput_Disable();
#else
	VBUS_Ouput_Enable();
#endif
//	GPIO_init(P5V_Control_Port, P5V_Control_Bit, GPIO_TYPE_Out_OpenDrain);
//	GPIO_init(VBUS_Control_Port, VBUS_Control_Bit, GPIO_TYPE_Out_OpenDrain);
//	GPIO_init(VBUS_Detect_Port, VBUS_Detect_Bit, GPIO_TYPE_In);
}

void VBUS_If_Reset(void)
{
#ifdef MHL_CODE
	VBUS_Ouput_Disable();		
#else
	VBUS_Ouput_Enable();
#endif
}

//--------------------------------------------------------------------------------------------------
// VBUS Control Interface

BOOL VBUS_Power(BOOL Enable)
{
	// Disable VBUS
	VBUS_Ouput_Disable();			

	if(Enable) {
		// Check VBUS status
		for(i=0; i<VBUS_STABLE_COUNT; ++i) {
			if(0)//if( GPIO_get(VBUS_Detect_Port, VBUS_Detect_Bit) ) 
			{
				// VBUS High detected
				break;
			}
		}
		if(i<VBUS_STABLE_COUNT) {
			// Skip
			EP_DEV_DBG("VBus Power On => fail\n");
			return FALSE; 
		}
		else {
			// Enable VBUS
			VBUS_Ouput_Enable();			
			
			EP_DEV_DBG("VBus Power On => OK\n");
			return TRUE;
		}
	}
	else {
		EP_DEV_DBG("VBus Power Off\n");
		// Check VBUS status
//		for(i=0; i<VBUS_STABLE_COUNT; ++i) {
//			delay_1ms(10);
//			if(!GPIO_get(VBUS_Detect_Port, VBUS_Detect_Bit) ) {
//				// VBUS High detected
//				EP_DEV_DBG("VBus Power Already Off!!\n");
//				break;
//			}
//			else
//				EP_DEV_DBG(".PU ");
//		}
		return FALSE; 
	}
}

BOOL MHL_Tx_VBUS_Power(void)
{
	//return GPIO_get(VBUS_Detect_Port, VBUS_Detect_Bit);
	return 1;	
}
