/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

  File        :  VBUS_If.h 

  Description :  Head file of VBUS Interface  

\******************************************************************************/

#ifndef VBUS_IF_H
#define VBUS_IF_H
#include "CommonInclude.h"
//==================================================================================================
//
// Protected Data Member
//
#define GPIO_PORT	BYTE
//==================================================================================================
//
// Public Functions
//

//--------------------------------------------------------------------------------------------------
//
// General
//

// All Interface Inital
void VBUS_If_Initial(GPIO_PORT P5VControlPort, BYTE P5VControlBitMask, GPIO_PORT ControlPort, BYTE ControlBitMask, GPIO_PORT DetectPort, BYTE DetectBitMask);
void VBUS_If_Reset(void);


//--------------------------------------------------------------------------------------------------
//
// VBUS Control Interface
//

BOOL VBUS_Power(BOOL Enable);
BOOL MHL_Tx_VBUS_Power(void);

#endif // VBUS_IF_H


