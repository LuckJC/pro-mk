/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;

import com.mediatek.xlog.Xlog;

public class EngineerModeReceiver extends BroadcastReceiver {

    private static final String TAG = "EM/SECRET_CODE";
    // process *#*#3646633#*#*
	private final Uri mEmUri = Uri.parse("android_secret_code://3646633");
	
	// process *#*#66635#*#*
    private final Uri mEmUri_66635 = Uri.parse("android_secret_code://66635");
	
    // process *#*#63863555#*#*
    private final Uri mCitUri = Uri.parse("android_secret_code://63863555");
	
	// process *#*#889#*#*
	private final Uri mCitUri_889 = Uri.parse("android_secret_code://889");
	
	// process *#*#0000#*#*
	private final Uri mVersionUri = Uri.parse("android_secret_code://0000");
	
	// process *#*#07#*#*
	private final Uri mSARUri = Uri.parse("android_secret_code://07");

	// process *#*#8814#*#*
	private final Uri deputyUri = Uri.parse("android_secret_code://8814");

    @Override
    public void onReceive(Context context, Intent intent) {
        if(intent.getAction()== null){
            Xlog.i(TAG, "Null action");
            return;
        }
        if (intent.getAction().equals(
				android.provider.Telephony.Intents.SECRET_CODE_ACTION)) {
            Uri uri = intent.getData();
            //Xlog.i(TAG, "getIntent success in if");
            if (uri.equals(mEmUri)||uri.equals(mEmUri_66635)) {
                Intent intentEm = new Intent(context, EngineerMode.class);
                intentEm.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                //Xlog.i(TAG, "Before start EM activity");
                context.startActivity(intentEm);
            } else if(uri.equals(mCitUri)) {
				handleCIT(context, "*#63863555#");
            } else if(uri.equals(mCitUri_889)) {
				handleCIT(context, "*#889#");
            } else if(uri.equals(deputyUri)) {
				//	Xlog.i("cui_Log", "*#8814#");
					handleCIT(context, "*#8814#");
			} else if(uri.equals(mVersionUri)) {
				//	Xlog.i("cui_Log", "*#0000#");
					handleCIT(context, "*#0000#");
			} else if(uri.equals(mSARUri)) {
				//	Xlog.i("cui_Log", "*#07#");
					handleCIT(context, "*#07#");
			} else {
                Xlog.i(TAG, "No matched URI!");
            }
        } else {
            Xlog.i(TAG, "Not SECRET_CODE_ACTION!");
        }
    }
	private void handleCIT(Context context, String input) 
	{
		int len = input.length();

		if (input.equals("*#63863555#"))
		{
			Intent intent = new Intent("sim.android.cit", Uri.parse("cit_secret_code://63863555"));
			context.sendBroadcast(intent);
	    } else if (input.equals("*#889#")){
			//Xlog.i("cui_Log", "handleCIT():*#889#");
			Intent intent = new Intent("sim.android.cit", Uri.parse("cit_secret_code://889"));
			context.sendBroadcast(intent);
		} else if (input.equals("*#0000#")){
			//Xlog.i("cui_Log", "handleCIT():*#0000#");
			Intent intent = new Intent("sim.android.cit", Uri.parse("cit_secret_code://0000"));
			context.sendBroadcast(intent);
		} else if (input.equals("*#8814#")){
			//Xlog.i("cui_Log", "handleCIT():*#8814#");
			Intent intent = new Intent("sim.android.cit", Uri.parse("cit_secret_code://8814"));
			context.sendBroadcast(intent);
		} else if (input.equals("*#07#")){
			//Xlog.i("cui_Log", "handleCIT():*#07#");
			Intent intent = new Intent("sim.android.cit", Uri.parse("cit_secret_code://07"));
			context.sendBroadcast(intent);
		}
	}
}
