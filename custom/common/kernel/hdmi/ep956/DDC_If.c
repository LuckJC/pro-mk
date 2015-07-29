/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

  File        :  DDC_If.c

  Description :  EP955E DDC Interface

\******************************************************************************/

#include "DDC_If.h"
#include "EP956_If.h"

//--------------------------------------------------------------------------------------------------

#define HDCP_RX_ADDR            0x74     // HDCP RX Address
#define EDID_ADDR       		0xA0     // EDID Address
#define EDID_SEGMENT_PTR		0x60

#define HDCP_RX_BKSV_ADDR       0x00     // HDCP RX, BKSV Register Address
#define HDCP_RX_RI_ADDR         0x08     // HDCP RX, RI Register Address
#define HDCP_RX_AKSV_ADDR       0x10     // HDCP RX, AKSV Register Address
#define HDCP_RX_AINFO_ADDR      0x15     // HDCP RX, AINFO Register Address
#define HDCP_RX_AN_ADDR         0x18     // HDCP RX, AN Register Address
#define HDCP_RX_SHA1_HASH_ADDR  0x20     // HDCP RX, SHA-1 Hash Value Start Address
#define HDCP_RX_BCAPS_ADDR      0x40     // HDCP RX, BCAPS Register Address
#define HDCP_RX_BSTATUS_ADDR    0x41     // HDCP RX, BSTATUS Register Address
#define HDCP_RX_KSV_FIFO_ADDR   0x43     // HDCP RX, KSV FIFO Start Address

#define EDID_READ_STEP	16

//--------------------------------------------------------------------------------------------------

// Temp Data
static int i, j;
static SMBUS_STATUS status;
static BYTE DDC_Data[EDID_READ_STEP];
static BYTE TempBit;

// Private Data

BYTE DDC_Bus;			// IIC
BYTE DDC_Port;		// IIC

// Private Functions
SMBUS_STATUS DDC_Access(BYTE IICAddr, BYTE RegAddr, void *Data, WORD Size);
#define DDC_Write DDC_Access
SMBUS_STATUS DDC_Read(BYTE IICAddr, BYTE RegAddr, void *Data, WORD Size);
void DDC_Segment_Write(BYTE Segment);

//==================================================================================================
//
// Public Function Implementation
//

//--------------------------------------------------------------------------------------------------
// Hardware Interface

void DDC_If_Initial(BYTE Bus, BYTE Port)
{
	DDC_Bus = Bus;
	DDC_Port = Port;
}

//--------------------------------------------------------------------------------------------------
//
// Downstream HDCP Control
//

BOOL Downstream_Rx_read_BKSV(BYTE *pBKSV)
{
	status = DDC_Read(HDCP_RX_ADDR, HDCP_RX_BKSV_ADDR, pBKSV, 5);
	if(status != SMBUS_STATUS_Success) {
		EP_DEV_DBG("Err: BKSV read - DN DDC %bu\n", status);
		return FALSE;
	}

	i = 0;
	j = 0;
	while (i < 5) {
		TempBit = 1;
		while (TempBit) {
			if (pBKSV[i] & TempBit) j++;
			TempBit <<= 1;
		}
		i++;
	}
	if(j != 20) {
		EP_DEV_DBG("Err: BKSV read - Key Wrong\n");
		EP_DEV_DBG("Err: BKSV = 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\n", pBKSV[0], pBKSV[1], pBKSV[2], pBKSV[3], pBKSV[4]);
		return FALSE;
	}
	return TRUE;
}

BYTE Downstream_Rx_BCAPS()
{
	DDC_Read(HDCP_RX_ADDR, HDCP_RX_BCAPS_ADDR, &DDC_Data[0], 1);
	return DDC_Data[0];
}

void Downstream_Rx_write_AINFO(char ainfo) 
{
	DDC_Write(HDCP_RX_ADDR, HDCP_RX_AINFO_ADDR, &ainfo, 1);
}

BOOL Downstream_Rx_write_AN(BYTE *pAN) 
{
	if(DDC_Write(HDCP_RX_ADDR, HDCP_RX_AN_ADDR, pAN, 8)) return FALSE;
	else return TRUE;
}

BOOL Downstream_Rx_write_AKSV(BYTE *pAKSV) 
{
	if(DDC_Write(HDCP_RX_ADDR, HDCP_RX_AKSV_ADDR, pAKSV, 5)) return FALSE;
	else return TRUE;
}

BOOL Downstream_Rx_read_RI(BYTE *pRI)
{
	// Not to use the Short Read
	status = DDC_Read(HDCP_RX_ADDR, HDCP_RX_RI_ADDR, pRI, 2);
	if(status != SMBUS_STATUS_Success) {
		EP_DEV_DBG("Err: Rx Ri read\n");
		return FALSE;
	}
	return TRUE;
}

void Downstream_Rx_read_BSTATUS(BYTE *pBSTATUS)
{
	DDC_Read(HDCP_RX_ADDR, HDCP_RX_BSTATUS_ADDR, pBSTATUS, 2);
}

void Downstream_Rx_read_SHA1_HASH(BYTE *pSHA) 
{
	DDC_Read(HDCP_RX_ADDR, HDCP_RX_SHA1_HASH_ADDR, pSHA, 20);
}

// Retrive 5 bytes KSV at "Index" from FIFO
BOOL Downstream_Rx_read_KSV_FIFO(BYTE *pBKSV, BYTE Index, BYTE DevCount)
{
#ifdef MHL_CODE
	// Try not to re-read the previous KSV
	// But, when? to send STOP is can not be determined
	if(Index == 0) { // Start 
		DDC_Data[0] = HDCP_RX_KSV_FIFO_ADDR;
		MHL_Tx_DDC_Cmd(HDCP_RX_ADDR, &DDC_Data[0], 1, SMBUS_SkipStop);
		if(Index == DevCount-1) {
			MHL_Tx_DDC_Cmd(HDCP_RX_ADDR+1, pBKSV, 5, SMBUS_Normal); // Start also End
		}
		else {
			MHL_Tx_DDC_Cmd(HDCP_RX_ADDR+1, pBKSV, 5, SMBUS_SkipStop); // Start only
		}
	}
	else if(Index == DevCount-1) { // End
		MHL_Tx_DDC_Cmd(HDCP_RX_ADDR+1, pBKSV, 5, SMBUS_SkipStart);
	}
	else { // Mid
		MHL_Tx_DDC_Cmd(HDCP_RX_ADDR+1, pBKSV, 5, SMBUS_DataOnly);
	}

	if(status != SMBUS_STATUS_Success) {
		EP_DEV_DBG("Err: KSV FIFO read - DN DDC %bu\n", status );
		return FALSE;
	}

	i = 0;
	j = 0;
	while (i < 5) {
		TempBit = 1;
		while (TempBit) {
			if (pBKSV[i] & TempBit) j++;
			TempBit <<= 1;
		}
		i++;
	}
	if(j != 20) {
		EP_DEV_DBG("Err: KSV FIFO read - Key Wrong\n");
		MHL_Tx_DDC_Cmd(HDCP_RX_ADDR+1, pBKSV, 0, SMBUS_SkipStart); // Stop
		return FALSE;
	}	
	return TRUE;
#else
	// Try not to re-read the previous KSV
	// But, when? to send STOP is can not be determined
	if(Index == 0) { // Start 
		DDC_Data[0] = HDCP_RX_KSV_FIFO_ADDR;
		SMBUS_master_rw_synchronous(DDC_Bus, HDCP_RX_ADDR, &DDC_Data[0], 1, SMBUS_SkipStop);
		if(Index == DevCount-1) {
			SMBUS_master_rw_synchronous(DDC_Bus, HDCP_RX_ADDR+1, pBKSV, 5, SMBUS_Normal); // Start also End
		}
		else {
			SMBUS_master_rw_synchronous(DDC_Bus, HDCP_RX_ADDR+1, pBKSV, 5, SMBUS_SkipStop); // Start only
		}
	}
	else if(Index == DevCount-1) { // End
		SMBUS_master_rw_synchronous(DDC_Bus, HDCP_RX_ADDR+1, pBKSV, 5, SMBUS_SkipStart);
	}
	else { // Mid
		SMBUS_master_rw_synchronous(DDC_Bus, HDCP_RX_ADDR+1, pBKSV, 5, SMBUS_DataOnly);
	}

	if(status != SMBUS_STATUS_Success) {
		EP_DEV_DBG("Err: KSV FIFO read - DN DDC %bu\n", status );
		return FALSE;
	}

	i = 0;
	j = 0;
	while (i < 5) {
		TempBit = 1;
		while (TempBit) {
			if (pBKSV[i] & TempBit) j++;
			TempBit <<= 1;
		}
		i++;
	}
	if(j != 20) {
		EP_DEV_DBG("Err: KSV FIFO read - Key Wrong\n");
		SMBUS_master_rw_synchronous(DDC_Bus, HDCP_RX_ADDR+1, pBKSV, 0, SMBUS_SkipStart); // Stop
		return FALSE;
	}	
	return TRUE;
#endif
}


//--------------------------------------------------------------------------------------------------
//
// Downstream EDID Control
//

// Read the EDID test
BOOL Downstream_Rx_poll_EDID(void)
{
	// Segment Pointer Address
	DDC_Segment_Write(0);

	// Base Address and Read 1
	status = DDC_Read(EDID_ADDR, 0, DDC_Data, 1);
	if(status == SMBUS_STATUS_Success) {
		return TRUE;
	}
	return FALSE;
}

EDID_STATUS Downstream_Rx_read_EDID(BYTE *pEDID)
{
	BYTE seg_ptr, BlockCount, Block1Found, ChkSum;

	// =========================================================
	// I. Read the block 0

	for(i=0; i<128; i+=EDID_READ_STEP) {
		// Segment Pointer Address
		DDC_Segment_Write(0); 

		// Read EDID data
		status = DDC_Read(EDID_ADDR, i, &pEDID[i], EDID_READ_STEP);
		if(status != SMBUS_STATUS_Success) {
			EP_DEV_DBG("Err: EDID b0-%d read - DN DDC %bu\n", i, status);
			return status;
		}
	}

#ifdef EDID_DBG
	EP_DEV_DBG("EDID b0 read:");
	for(i=0; i<128; ++i) {
		if(i%16 == 0) EP_DEV_DBG("\n");
		if(i%8 == 0) EP_DEV_DBG(" ");
		EP_DEV_DBG("0x%02X,", pEDID[i] );
	}
	EP_DEV_DBG("\n");
#endif

	// Check EDID
	if(pEDID[126] > 8) {
		EP_DEV_DBG("Err: EDID Check failed, pEDID[126]=0x%02X > 8\n\r", pEDID[126] );
		return EDID_STATUS_ExtensionOverflow;
	}

	// =========================================================
	// II. Read other blocks and find Timing Extension Block

	BlockCount = pEDID[126];
	Block1Found = 0;
	for (seg_ptr = 1; seg_ptr <= BlockCount; ++seg_ptr) {

		// Base Address and Read 128
		if(Block1Found) {		
			for(i=0; i<128; i+=EDID_READ_STEP) {
				// Segment Pointer Address
				DDC_Segment_Write(seg_ptr >> 1); 
		
				// Read EDID data
				status = DDC_Read(EDID_ADDR, ((seg_ptr & 0x01) << 7)+i, DDC_Data, EDID_READ_STEP);
				if(status != SMBUS_STATUS_Success) {
					EP_DEV_DBG("Err: EDID b%bu-%d read\n", seg_ptr, i);
					return status;
				}
			}
		}
		else {	
			for(i=0; i<128; i+=EDID_READ_STEP) {
				// Segment Pointer Address
				DDC_Segment_Write(seg_ptr >> 1); 
		
				// Read EDID data
				status = DDC_Read(EDID_ADDR, ((seg_ptr & 0x01) << 7)+i, &pEDID[128+i], EDID_READ_STEP);
				if(status != SMBUS_STATUS_Success) {
					EP_DEV_DBG("Err: EDID b%bu-%d read\n", seg_ptr, i);
					return status;
				}
			}
		}

		if(pEDID[128] == 0x02 && Block1Found == 0) {
			Block1Found = 1;
#ifdef EDID_DBG
			EP_DEV_DBG("EDID b%bu read:", seg_ptr);
			for(i=0; i<128; ++i) {
				if(i%16 == 0) EP_DEV_DBG("\n");
				if(i%8 == 0) EP_DEV_DBG(" ");
				EP_DEV_DBG("0x%02X,", pEDID[128+i] );
			}
			EP_DEV_DBG("\n");
#endif
		}
	}
	
	// Check CheckSum
	ChkSum = 0;
	for(i=0; i<((Block1Found)?256:128); ++i) {
		ChkSum += pEDID[i];
	}
	if(ChkSum != 0) {
		return EDID_STATUS_ChecksumError;
	}
	if(Block1Found) {
		pEDID[126] = 1;
	}
	else {
		pEDID[126] = 0;
	}
	return EDID_STATUS_Success;
}

//==================================================================================================
//
// Private Functions
//

SMBUS_STATUS DDC_Access(BYTE IICAddr, BYTE RegAddr, void *Data, WORD Size)
{
#ifdef MHL_CODE
	if(IICAddr & 0x01) { // Read
		status = MHL_Tx_DDC_Cmd(IICAddr-1, &RegAddr, 1, SMBUS_SkipStop);
		if(!status)
		status |= MHL_Tx_DDC_Cmd(IICAddr, Data, Size, SMBUS_Normal);
	}
	else { // Write
		status = MHL_Tx_DDC_Cmd(IICAddr, &RegAddr, 1, SMBUS_SkipStop);
		if(!status)
		status |= MHL_Tx_DDC_Cmd(IICAddr, Data, Size, SMBUS_SkipStart);
	}
	if(status) { // failed and retry
		EP_DEV_DBG("Err: DDC failed %bu, IICAddr=0x%02X, RegAddr=0x%02X\n", status, IICAddr, RegAddr);
	
	}
	return status;
#else
	int i;
	
	// Not realy a Robust Algorithm for multi-master arbitration retry
	for(i=10, j=0; j<3; ++j) { // Time-Out to prevent dead lock
		if(IICAddr & 0x01) { // Read
			status = SMBUS_master_rw_synchronous(DDC_Bus, IICAddr-1, &RegAddr, 1, SMBUS_SkipStop);
			status |= SMBUS_master_rw_synchronous(DDC_Bus, IICAddr, Data, Size, SMBUS_Normal);
		}
		else { // Write
			status = SMBUS_master_rw_synchronous(DDC_Bus, IICAddr, &RegAddr, 1, SMBUS_SkipStop);
			status |= SMBUS_master_rw_synchronous(DDC_Bus, IICAddr, Data, Size, SMBUS_SkipStart);
		}
		if(status) { // failed and retry
			EP_DEV_DBG("Err: DDC failed %bu, IICAddr=0x%02X, RegAddr=0x%02X\n", status, IICAddr, RegAddr);
			delay_1ms(i);
			if(i>0) i-= 3; // Become faster to prevent starvation
		}
		else break;
	}
	return status;
#endif
}

SMBUS_STATUS DDC_Read(BYTE IICAddr, BYTE RegAddr, void *Data, WORD Size)
{
	if(!(IICAddr & 0x01)) IICAddr |= 1;
	return DDC_Access(IICAddr, RegAddr, Data, Size);
}

void DDC_Segment_Write(BYTE Segment)
{
#ifdef MHL_CODE
	SMBUS_STATUS status;
	status = MHL_Tx_DDC_Cmd(EDID_SEGMENT_PTR, &Segment, 0, SMBUS_SkipStop);
	if(!status) { // if no error
		MHL_Tx_DDC_Cmd(EDID_SEGMENT_PTR, &Segment, 1, SMBUS_DataOnly);
	}
#else
	SMBUS_master_rw_synchronous(DDC_Bus, EDID_SEGMENT_PTR, &Segment, 0, SMBUS_SkipStop);
	SMBUS_master_rw_synchronous(DDC_Bus, EDID_SEGMENT_PTR, &Segment, 1, SMBUS_DataOnly);
#endif
}
