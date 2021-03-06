/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 ** Copyright 2003-2010, VisualOn, Inc.
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */
/*******************************************************************************
	File:		cmnMemory.h

	Content:	memory operator implementation header file

*******************************************************************************/

#ifndef __cmnMemory_H__
#define __cmnMemory_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <voMem.h>

//extern VO_MEM_OPERATOR	g_memOP;

/**
 * Allocate memory
 * \param uID [in] module ID
 * \param uSize [in] size of memory
 * \return value is the allocated memory address. NULL is failed.
 */
VO_U32	cmnMemAlloc (VO_S32 uID,  VO_MEM_INFO * pMemInfo);

/**
 * Free up memory
 * \param uID [in] module ID
 * \param pMem [in] address of memory
 * \return value 0, if succeeded.
 */
VO_U32	cmnMemFree (VO_S32 uID, VO_PTR pBuffer);

/**
 * memory set function
 * \param uID [in] module ID
 * \param pBuff [in/out] address of memory
 * \param uValue [in] the value to be set
 * \param uSize [in] the size to be set
 * \return value 0, if succeeded.
 */
VO_U32	cmnMemSet (VO_S32 uID, VO_PTR pBuff, VO_U8 uValue, VO_U32 uSize);

/**
 * memory copy function
 * \param uID [in] module ID
 * \param pDest [in/out] address of destination memory
 * \param pSource [in] address of source memory
 * \param uSize [in] the size to be copied
 * \return value 0, if succeeded.
 */
VO_U32	cmnMemCopy (VO_S32 uID, VO_PTR pDest, VO_PTR pSource, VO_U32 uSize);

/**
 * memory check function
 * \param uID [in] module ID
 * \param pBuff [in] address of buffer to be checked
 * \param uSize [in] the size to be checked
 * \return value 0, if succeeded.
 */
VO_U32	cmnMemCheck (VO_S32 uID, VO_PTR pBuffer, VO_U32 uSize);

/**
 * memory compare function
 * \param uID [in] module ID
 * \param pBuffer1 [in] address of buffer 1 to be compared
 * \param pBuffer2 [in] address of buffer 2 to be compared
 * \param uSize [in] the size to be compared
 * \return value: same as standard C run-time memcmp() function.
 */
VO_S32	cmnMemCompare (VO_S32 uID, VO_PTR pBuffer1, VO_PTR pBuffer2, VO_U32 uSize);

/**
 * memory move function
 * \param uID [in] module ID
 * \param pDest [in/out] address of destination memory
 * \param pSource [in] address of source memory
 * \param uSize [in] the size to be moved
 * \return value 0, if succeeded.
 */
VO_U32	cmnMemMove (VO_S32 uID, VO_PTR pDest, VO_PTR pSource, VO_U32 uSize);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __cmnMemory_H__


