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
import org.cagnulen.qdomyoszwift.qlog as Log;
import com.garmin.android.connectiq.ConnectIQ;
import com.garmin.android.connectiq.ConnectIQAdbStrategy;
import com.garmin.android.connectiq.IQApp;
import com.garmin.android.connectiq.IQDevice;
import com.garmin.android.connectiq.exception.InvalidStateException;
import com.garmin.android.connectiq.exception.ServiceUnavailableException;
import android.content.BroadcastReceiver;
import android.content.ContextWrapper;
import android.content.IntentFilter;
import android.widget.Toast;

import org.jetbrains.annotations.Nullable;

import com.google.protobuf.InvalidProtocolBufferException;

import java.util.HashMap;
import java.util.List;

public class ZwiftAPI {

    private static Context context;

    private static final String TAG = "ZwiftAPI: ";
    private static ZwiftMessages.PlayerState playerState;

    public static void zwift_api_decodemessage_player(byte[] value) {
        try {
            playerState = ZwiftMessages.PlayerState.parseFrom(value);
            // Ora puoi usare 'message' come un oggetto normale
        } catch (InvalidProtocolBufferException e) {
            // Gestisci l'eccezione se il messaggio non può essere parsato
            Log.e(TAG, e.toString());
        }
    }

    public static float getAltitude() {
        Log.d(TAG, "getAltitude " + playerState.getAltitude());
        return playerState.getAltitude();
    }

    public static float getDistance() {
        Log.d(TAG, "getDistance " + playerState.getDistance());
        return playerState.getDistance();
    }
}
