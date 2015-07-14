/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

  File        :  DDC_If.h 

  Description :  Head file of DDC Interface  

\******************************************************************************/

#ifndef DDC_IF_H
#define DDC_IF_H

#include "CommonInclude.h"
//==================================================================================================
//
// Protected Data Member
//

// EDID status error code
typedef enum {
	// Master
	EDID_STATUS_Success = 0x00,
	EDID_STATUS_Pending,//	SMBUS_STATUS_Abort,
	EDID_STATUS_NoAct = 0x02,
	EDID_STATUS_TimeOut,
	EDID_STATUS_ArbitrationLoss = 0x04,
	EDID_STATUS_ExtensionOverflow,
	EDID_STATUS_ChecksumError
} EDID_STATUS;


#define SMBUS_ID BYTE
#define SMBUS_PORT BYTE
//==================================================================================================
//
// Public Functions
//

//--------------------------------------------------------------------------------------------------
//
// General
//

// All Interface Inital
void DDC_If_Initial(BYTE Bus, BYTE Port);


//--------------------------------------------------------------------------------------------------
//
// Downstream HDCP Control Interface
//

BOOL Downstream_Rx_read_BKSV(BYTE *pBKSV);
BYTE Downstream_Rx_BCAPS(void);
void Downstream_Rx_write_AINFO(char ainfo);
BOOL Downstream_Rx_write_AN(BYTE *pAN);
BOOL Downstream_Rx_write_AKSV(BYTE *pAKSV);
BOOL Downstream_Rx_read_RI(BYTE *pRI);
void Downstream_Rx_read_BSTATUS(BYTE *pBSTATUS);
void Downstream_Rx_read_SHA1_HASH(BYTE *pSHA);
BOOL Downstream_Rx_read_KSV_FIFO(BYTE *pBKSV, BYTE Index, BYTE DevCount);


//--------------------------------------------------------------------------------------------------
//
// Downstream EDID Control Interface
//

BOOL Downstream_Rx_poll_EDID(void);
EDID_STATUS Downstream_Rx_read_EDID(BYTE *pEDID);


#endif // DDC_IF_H


