package com.mediatek.contacts.plugin;

import android.content.Intent;
import android.util.Log;
import com.mediatek.contacts.ext.ContactPluginDefault;
import com.mediatek.contacts.ext.DialtactsExtension;

public class OP01DialtactsExtension extends DialtactsExtension {
    private static final String TAG = "OP01DialtactsExtension";
    @Override
    public boolean checkComponentName(Intent intent, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return false;
        }
        String componentName = intent.getComponent().getClassName();
        if ("com.android.contacts.VideoCallEntryActivity".equals(componentName)) {
            Log.i(TAG, "[checkComponentName] return true");
            return true;
        }
        Log.i(TAG, "[checkComponentName] return false");
        return false;
    }

    public boolean startActivity(String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)){
            return false;
        }
        Log.i(TAG, "DialerSearchAdapter: [startActivity()]"); 
        return true;
    }
}
