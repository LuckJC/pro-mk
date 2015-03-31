package com.mediatek.contacts.plugin;

import android.util.Log;

import com.mediatek.contacts.ext.DialPadExtension;
import com.mediatek.contacts.ext.ContactPluginDefault;

public class OP01DialPadExtension extends DialPadExtension {
    private static final String TAG = "OP01DialPadExtension";
    @Override
    public String changeChar(String string, String string2, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return null;
        }
        Log.i(TAG, "[changeChar] string : " + string + " | string2 : " + string2);
        return string2;
    }

}
