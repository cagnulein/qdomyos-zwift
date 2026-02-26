package org.cagnulen.qdomyoszwift;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.garmin.android.connectiq.IQDevice;

import java.nio.BufferUnderflowException;

import org.cagnulen.qdomyoszwift.QLog;

public class IQMessageReceiverWrapper extends BroadcastReceiver {
    private final BroadcastReceiver receiver;
    private static String TAG = "IQMessageReceiverWrapper: ";

    public IQMessageReceiverWrapper(BroadcastReceiver receiver) {
        this.receiver = receiver;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        try {
            QLog.d(TAG, "=== GARMIN INTENT DEBUG START ===");
            QLog.d(TAG, "Action: " + intent.getAction());

            // Log all extras in the intent
            if (intent.getExtras() != null) {
                QLog.d(TAG, "Extras bundle: " + intent.getExtras());
                try {
                    for (String key : intent.getExtras().keySet()) {
                        Object value = intent.getExtras().get(key);
                        QLog.d(TAG, "  Extra[" + key + "] = " + value + " (type: " + (value != null ? value.getClass().getName() : "null") + ")");
                    }
                } catch (Exception e) {
                    QLog.e(TAG, "Error iterating extras: " + e.toString());
                }
            } else {
                QLog.d(TAG, "No extras in intent");
            }

            // Process known actions
            if ("com.garmin.android.connectiq.SEND_MESSAGE_STATUS".equals(intent.getAction())) {
                QLog.d(TAG, "Processing SEND_MESSAGE_STATUS");
                replaceIQDeviceById(intent, "com.garmin.android.connectiq.EXTRA_REMOTE_DEVICE");
            } else if ("com.garmin.android.connectiq.OPEN_APPLICATION".equals(intent.getAction())) {
                QLog.d(TAG, "Processing OPEN_APPLICATION");
                replaceIQDeviceById(intent, "com.garmin.android.connectiq.EXTRA_OPEN_APPLICATION_DEVICE");
            } else if ("com.garmin.android.connectiq.DEVICE_STATUS".equals(intent.getAction())) {
                QLog.d(TAG, "Processing DEVICE_STATUS");
                replaceIQDeviceById(intent, "com.garmin.android.connectiq.EXTRA_REMOTE_DEVICE");
            } else if ("com.garmin.android.connectiq.INCOMING_MESSAGE".equals(intent.getAction())) {
                QLog.d(TAG, "Processing INCOMING_MESSAGE");
                replaceIQDeviceById(intent, "com.garmin.android.connectiq.EXTRA_REMOTE_DEVICE");
            } else {
                QLog.d(TAG, "Unknown action, no processing");
            }

            QLog.d(TAG, "Calling wrapped receiver.onReceive()");
            receiver.onReceive(context, intent);
            QLog.d(TAG, "=== GARMIN INTENT DEBUG END (success) ===");

        } catch (Exception e) {
            QLog.e(TAG, "=== EXCEPTION in wrapper (BEFORE or DURING receiver call) ===");
            QLog.e(TAG, "Exception type: " + e.getClass().getName());
            QLog.e(TAG, "Exception message: " + e.getMessage());
            QLog.e(TAG, "Stack trace:", e);
            QLog.e(TAG, "=== GARMIN INTENT DEBUG END (error) ===");
        }
    }

    private static void replaceIQDeviceById(Intent intent, String extraName) {
        try {
            QLog.d(TAG, "  Attempting to get Parcelable for extra: " + extraName);
            IQDevice device = intent.getParcelableExtra(extraName);
            if (device != null) {
                QLog.d(TAG, "  Found IQDevice: " + device.getFriendlyName() + " (ID: " + device.getDeviceIdentifier() + ")");
                long deviceId = device.getDeviceIdentifier();
                intent.putExtra(extraName, deviceId);
                QLog.d(TAG, "  Replaced IQDevice with Long ID: " + deviceId);
            } else {
                QLog.d(TAG, "  Extra '" + extraName + "' is null or not an IQDevice");
            }
        } catch (ClassCastException e) {
            QLog.d(TAG, "  ClassCastException for '" + extraName + "': " + e.toString());
            QLog.d(TAG, "  (Extra is already a Long, probably on simulator)");
        } catch (Exception e) {
            QLog.e(TAG, "  Unexpected exception in replaceIQDeviceById: " + e.toString());
            QLog.e(TAG, "  Stack trace:", e);
        }
    }


}
