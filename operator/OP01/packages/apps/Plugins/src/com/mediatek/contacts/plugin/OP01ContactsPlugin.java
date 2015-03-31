
package com.mediatek.contacts.plugin;

import com.mediatek.contacts.ext.CallDetailExtension;
import com.mediatek.contacts.ext.ContactAccountExtension;
import com.mediatek.contacts.ext.ContactDetailExtension;
import com.mediatek.contacts.ext.ContactListExtension;
import com.mediatek.contacts.ext.ContactPluginDefault;
import com.mediatek.contacts.ext.DialPadExtension;
import com.mediatek.contacts.ext.DialtactsExtension;
import com.mediatek.contacts.ext.QuickContactExtension;
import com.mediatek.contacts.ext.SpeedDialExtension;

public class OP01ContactsPlugin extends ContactPluginDefault {
    public CallDetailExtension createCallDetailExtension() {
        return new OP01CallDetailExtension();
    }

    public ContactAccountExtension createContactAccountExtension() {
        return new OP01ContactAccountExtension(); 
    }

    public ContactDetailExtension createContactDetailExtension() {
        return new OP01ContactDetailExtension();
    }

    public ContactListExtension createContactListExtension() {
        return new OP01ContactListExtension();
    }

    public DialPadExtension createDialPadExtension() {
        return new OP01DialPadExtension();
    }

    public DialtactsExtension createDialtactsExtension() {
        return new OP01DialtactsExtension();
    }

    public SpeedDialExtension createSpeedDialExtension() {
        return new OP01SpeedDialExtension();
    }    

    public QuickContactExtension createQuickContactExtension() {
        return new OP01QuickContactExtension();
    }
}
