/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

  File        :  Edid.c 

  Description :  Implement the interfaces fo Edid

\******************************************************************************/

#include "Edid.h"

static int i, j;

//--------------------------------------------------------------------------------------------------

BYTE EDID_GetHDMICap(BYTE *pTarget)
{
	if(pTarget[126] == 0x01) {
		for(i=4; i<pTarget[EDID_BLOCK_SIZE+2]; ++i) {
			if((pTarget[EDID_BLOCK_SIZE+i] & 0xE0) == 0x60) { // find tag code - Vendor Specific Block
				if( (pTarget[EDID_BLOCK_SIZE+1+i] == 0x03) && (pTarget[EDID_BLOCK_SIZE+2+i] == 0x0C) && (pTarget[EDID_BLOCK_SIZE+3+i] == 0x00) ) {
					return 1;
				}
			}
			else {
				i += (pTarget[EDID_BLOCK_SIZE+i] & 0x1F);
			}
		}
		if(i>=pTarget[EDID_BLOCK_SIZE+2]) { // Error, can not find the Vendor Specific Block
			return 0;
		}
	}
	return 0;
}

BYTE EDID_GetPCMFreqCap(BYTE *pTarget)
{
	if(pTarget[126] >= 0x01) {
		for(i=4; i<pTarget[EDID_BLOCK_SIZE+2]; ++i) {
			if((pTarget[EDID_BLOCK_SIZE+i] & 0xE0) == 0x20) { // find tag code - Audio Data Block
				for(j=1; j<(pTarget[EDID_BLOCK_SIZE+i] & 0x1F); j+=3) {
					if((pTarget[EDID_BLOCK_SIZE+i+j] >> 3) == 1) {
						return pTarget[EDID_BLOCK_SIZE+i+j+1];
					}
				}
			}
			else {
				i += (pTarget[EDID_BLOCK_SIZE+i] & 0x1F);
			}
		}
		if(i>=pTarget[EDID_BLOCK_SIZE+2]) { // Error, can not find the Audio Data Block
			return 0x07;
		}
	}
	return 0x00;
}

BYTE EDID_GetChannelCap(BYTE *pTarget, BYTE AFormat)
{
	BYTE MaxChannel = 1;
	if(pTarget[126] >= 0x01) {
		for(i=4; i<pTarget[EDID_BLOCK_SIZE+2]; ++i) {
			if((pTarget[EDID_BLOCK_SIZE+i] & 0xE0) == 0x20) { // find tag code - Audio Data Block
				for(j=1; j<(pTarget[EDID_BLOCK_SIZE+i] & 0x1F); j+=3) {
					if((pTarget[EDID_BLOCK_SIZE+i+j] >> 3) == AFormat) {
						//return pTarget[EDID_BLOCK_SIZE+i+j] & 0x07;
						MaxChannel = max(MaxChannel, pTarget[EDID_BLOCK_SIZE+i+j] & 0x07);
					}
				}
			}
			else {
				i += (pTarget[EDID_BLOCK_SIZE+i] & 0x1F);
			}
		}
		return MaxChannel;
		//if(i>=pTarget[EDID_BLOCK_SIZE+2]) { // Error, can not find the Audio Data Block
		//	return 1;
		//}
	}
	return 0;
}

BYTE EDID_GetDataBlockAddr(BYTE *pTarget, BYTE Tag)
{
	if(pTarget[126] >= 0x01) {
		for(i=4; i<pTarget[EDID_BLOCK_SIZE+2]; ++i) {
			if((pTarget[EDID_BLOCK_SIZE+i] & 0xE0) == Tag) { // find tag code 
				return i+128;
			}
			else {
				i += (pTarget[EDID_BLOCK_SIZE+i] & 0x1F);
			}
		}
		if(i>=pTarget[EDID_BLOCK_SIZE+2]) { // Error, can not find
			return 0;
		}
	}
	return 0;
}

