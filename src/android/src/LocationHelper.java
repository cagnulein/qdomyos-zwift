package org.cagnulen.qdomyoszwift;

import android.Manifest;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.location.LocationManager;
import android.os.Build;
import android.provider.Settings;
import android.util.Log;
import androidx.appcompat.app.AlertDialog;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public class LocationHelper {
    private static final String TAG = "LocationHelper";
    private static final int REQUEST_ENABLE_BT = 1;
    private static final int REQUEST_ENABLE_LOCATION = 2;
    private static final int PERMISSION_REQUEST_CODE = 3;

    public static boolean check(Context context) {
        LocationManager locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        boolean isGpsEnabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
        boolean isBluetoothEnabled = (bluetoothAdapter != null && bluetoothAdapter.isEnabled());
        return isGpsEnabled && isBluetoothEnabled;
    }

    public static void start(final Activity activity) {
        if (!(activity instanceof Activity)) {
            Log.e(TAG, "Context must be an instance of Activity");
            return;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (ContextCompat.checkSelfPermission(activity, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED ||
                ContextCompat.checkSelfPermission(activity, Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(activity,
                    new String[]{Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.BLUETOOTH},
                    PERMISSION_REQUEST_CODE);
                return;
            }
        }

        if (!check(activity)) {
            showEnableServicesDialog(activity);
        }
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

    private static void showEnableServicesDialog(final Activity activity) {
        new AlertDialog.Builder(activity)
            .setTitle("Enable Services")
            .setMessage("Location Service and Bluetooth are required for this app. Location services is not required for GPS, but it's the way how Google ask for permissions to search devices around you. Would you like to enable them?")
            .setPositiveButton("Yes", (dialog, which) -> enableServices(activity))
            .setNegativeButton("No", (dialog, which) -> onServicesCheck(false, false))
            .create()
            .show();
    }

    private static void enableServices(Activity activity) {
        LocationManager locationManager = (LocationManager) activity.getSystemService(Context.LOCATION_SERVICE);
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        if (!locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
            Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
            activity.startActivityForResult(intent, REQUEST_ENABLE_LOCATION);
        }

        if (bluetoothAdapter != null && !bluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            activity.startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

        new AlertDialog.Builder(activity)
            .setTitle("Restart the app")
            .setMessage("Do you want to restart the app to apply the changes?")
            .setPositiveButton("Yes", (dialog, which) -> activity.finish())
            .setNegativeButton("No", (dialog, which) -> onServicesCheck(false, false))
            .create()
            .show();

    }
}
