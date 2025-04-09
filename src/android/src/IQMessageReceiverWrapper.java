package org.cagnulen.qdomyoszwift;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.garmin.android.connectiq.IQDevice;

import java.nio.BufferUnderflowException;

import org.cagnulen.qdomyoszwift.QLog as Log;

public class IQMessageReceiverWrapper extends BroadcastReceiver {
    private final BroadcastReceiver receiver;
    private static String TAG = "IQMessageReceiverWrapper: ";

    public IQMessageReceiverWrapper(BroadcastReceiver receiver) {
        this.receiver = receiver;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceive intent " + intent.getAction());
        if ("com.garmin.android.connectiq.SEND_MESSAGE_STATUS".equals(intent.getAction())) {
            replaceIQDeviceById(intent, "com.garmin.android.connectiq.EXTRA_REMOTE_DEVICE");
        } else if ("com.garmin.android.connectiq.OPEN_APPLICATION".equals(intent.getAction())) {
            replaceIQDeviceById(intent, "com.garmin.android.connectiq.EXTRA_OPEN_APPLICATION_DEVICE");
        } else if ("com.garmin.android.connectiq.DEVICE_STATUS".equals(intent.getAction())) {
            replaceIQDeviceById(intent, "com.garmin.android.connectiq.EXTRA_REMOTE_DEVICE");
        }

        try {
            receiver.onReceive(context, intent);
        } catch (IllegalArgumentException | BufferUnderflowException e) {
            Log.d(TAG, e.toString());
        }
    }

    private static void replaceIQDeviceById(Intent intent, String extraName) {
        try {
            IQDevice device = intent.getParcelableExtra(extraName);
            if (device != null) {
//                Logger.logDebug("Replacing " + device.describeContents() + " " + device.getFriendlyName() + " by " + device.getDeviceIdentifier() );
                intent.putExtra(extraName, device.getDeviceIdentifier());
            }
        } catch (ClassCastException e) {
            Log.d(TAG, e.toString());
            // It's already a long, i.e. on the simulator.
        }
    }


}
