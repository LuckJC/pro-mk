/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

  File        :  Edid.h 

  Description :  Head file of Edid IO Interface 

\******************************************************************************/

#ifndef EDID_H
#define EDID_H
#include "CommonInclude.h"
#define EDID_BLOCK_SIZE  128

//
// Structure Definitions
//

BYTE EDID_GetHDMICap(BYTE *pTarget);
BYTE EDID_GetPCMFreqCap(BYTE *pTarget);
BYTE EDID_GetChannelCap(BYTE *pTarget, BYTE AFormat);
BYTE EDID_GetDataBlockAddr(BYTE *pTarget, BYTE Tag);

#endif // EDID_H


