package org.cagnulen.qdomyoszwift;

import android.app.ActivityManager;
import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import android.os.Looper;
import android.os.Handler;
import org.cagnulen.qdomyoszwift.QLog as Log;
import android.content.BroadcastReceiver;
import android.content.ContextWrapper;
import android.content.IntentFilter;
import android.widget.Toast;
import com.che.zap.play.ZwiftPlayDevice;

import org.jetbrains.annotations.Nullable;

import com.google.protobuf.InvalidProtocolBufferException;

import java.util.HashMap;
import java.util.List;

public class ZapClickLayer {

    private static Context context;

    private static final String TAG = "ZapClickLayer: ";
    private static ZwiftPlayDevice device = initDevice();

    private static ZwiftPlayDevice initDevice() {
        ZwiftPlayDevice d = new ZwiftPlayDevice();
        return d;
    }

    public static int processCharacteristic(byte[] value) {
        Log.d(TAG, "processCharacteristic");
        return device.processCharacteristic("QZ", value);
    }

    public static byte[] buildHandshakeStart() {
        Log.d(TAG, "buildHandshakeStart");
        return device.buildHandshakeStart();
    }
}
