package org.cagnulen.qdomyoszwift;

import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.Intent;
import android.location.LocationManager;
import android.provider.Settings;
import android.util.Log;

public class LocationHelper {
    private static final String TAG = "LocationHelper";

    public static void start(Context context) {
        Log.d(TAG, "Starting LocationHelper check...");

        boolean isLocationEnabled = isLocationEnabled(context);
        boolean isBluetoothEnabled = isBluetoothEnabled();

        if (!isLocationEnabled || !isBluetoothEnabled) {
            Log.d(TAG, "Some services are disabled. Prompting user...");
            if (!isLocationEnabled) {
                promptEnableLocation(context);
            }
            if (!isBluetoothEnabled) {
                promptEnableBluetooth(context);
            }
        } else {
            Log.d(TAG, "All services are already enabled.");
        }

        // Internal callback
        onServicesCheck(isLocationEnabled, isBluetoothEnabled);
    }

    public static boolean check(Context context) {
        return isLocationEnabled(context) && isBluetoothEnabled();
    }

    private static boolean isLocationEnabled(Context context) {
        LocationManager locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        return locationManager != null && locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
    }

    private static boolean isBluetoothEnabled() {
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        return bluetoothAdapter != null && bluetoothAdapter.isEnabled();
    }

    private static void promptEnableLocation(Context context) {
        Log.d(TAG, "Prompting to enable Location...");
        Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    private static void promptEnableBluetooth(Context context) {
        Log.d(TAG, "Prompting to enable Bluetooth...");
        Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    private static void onServicesCheck(boolean isLocationEnabled, boolean isBluetoothEnabled) {
        if (isLocationEnabled && isBluetoothEnabled) {
            Log.d(TAG, "Both Location and Bluetooth are enabled");
            // You can add any additional logic here when both services are enabled
        } else {
            Log.d(TAG, "Some services are still disabled");
            if (!isLocationEnabled) {
                Log.d(TAG, "Location is disabled");
            }
            if (!isBluetoothEnabled) {
                Log.d(TAG, "Bluetooth is disabled");
            }
            // You can add any additional logic here when some services are disabled
        }
    }
}
