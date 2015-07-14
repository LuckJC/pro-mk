/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005 
                           ALL RIGHTS RESERVED 
 
--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  HDCP.h

  Description :  Header file of HDCP.c

  Codeing     :  Shihken

\******************************************************************************/

#ifndef HDCP_H
#define HDCP_H

//#define EXT_RICMP_TRIGGER
#define HDCP_TIMER_PERIOD 					5		//   

// HDCP Transmiter Link State
typedef enum {
	A0_Wait_for_Active_Rx,
	A1_Exchange_KSVs,
	A2_Computations,
	A3_Validate_Receiver,
	A4_Authenticated,
	A5_Link_Integrity_Check,
	A6_Test_for_Repeater,
	A8_Wait_for_READY,
	A9_Read_KSV_List
} HDCP_STATE;

#define HDCP_ERROR_BKSV									0x80
#define HDCP_ERROR_AKSV									0x40
#define HDCP_ERROR_R0									0x20
#define HDCP_ERROR_Ri									0x10
#define HDCP_ERROR_RepeaterRdy							0x08
#define HDCP_ERROR_RepeaterSHA							0x04
#define HDCP_ERROR_RSEN									0x02
#define HDCP_ERROR_RepeaterMax							0x01

HDCP_STATE HDCP_Authentication_Task(void);
void HDCP_Stop(void);
unsigned char HDCP_Get_Status(void);
void HDCP_Timer(void);
void HDCP_Ext_Ri_Trigger(void);

// Special Functions
void HDCP_Assign_RKSV_List(unsigned char *pRevocationList, unsigned char ListNumber);
void HDCP_Fake(unsigned char Enable);
void HDCP_Extract_BKSV(unsigned char *pBKSV);
void HDCP_Extract_BCAPS3(unsigned char *pBCaps3);
void HDCP_Extract_FIFO(unsigned char *pFIFO, unsigned char ListNumber);

#endif // HDCP_H
