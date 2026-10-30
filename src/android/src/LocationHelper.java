package org.cagnulen.qdomyoszwift;

import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.Intent;
import android.location.LocationManager;
import android.os.Build;
import android.provider.Settings;
import org.cagnulen.qdomyoszwift.QLog;

public class LocationHelper {
    private static final String TAG = "LocationHelper";
    private static boolean isLocationEnabled = false;
    private static boolean isBluetoothEnabled = false;

    public static boolean start(Context context) {
        QLog.d(TAG, "Starting LocationHelper check...");

        isLocationEnabled = isLocationEnabled(context);
        isBluetoothEnabled = isBluetoothEnabled();

        return isLocationEnabled && isBluetoothEnabled;
    }

    public static void requestPermissions(Context context) {
        // Refresh status rather than relying on a stale snapshot from app startup.
        isLocationEnabled = isLocationEnabled(context);
        isBluetoothEnabled = isBluetoothEnabled();
        if (!isLocationEnabled || !isBluetoothEnabled) {
            QLog.d(TAG, "Some services are disabled. Prompting user...");
            if (!isLocationEnabled) {
                promptEnableLocation(context);
            }
            if (!isBluetoothEnabled) {
                promptEnableBluetooth(context);
            }
        } else {
            QLog.d(TAG, "All services are already enabled.");
        }
    }

    public static boolean check(Context context) {
        return isLocationEnabled(context) && isBluetoothEnabled();
    }

    private static boolean isLocationEnabled(Context context) {
        // Qt 5.15's Android Bluetooth backend still checks the system-wide
        // Location toggle on every Android version, including API 31+.
        // Match that legacy check until we update the bundled Qt library.
        LocationManager locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        if (locationManager == null)
            return false;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
            return locationManager.isLocationEnabled();
        return locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
    }

    private static boolean isBluetoothEnabled() {
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        return bluetoothAdapter != null && bluetoothAdapter.isEnabled();
    }

    private static void promptEnableLocation(Context context) {
        QLog.d(TAG, "Prompting to enable Location...");
        Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    private static void promptEnableBluetooth(Context context) {
        QLog.d(TAG, "Prompting to enable Bluetooth...");
        Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }
}
