package org.cagnulen.qdomyoszwift;

import android.location.LocationManager;
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
import android.util.Log;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import android.bluetooth.BluetoothAdapter;

public class LocationHelper {

    public static int check(Context context) {
        LocationManager locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        boolean isGpsEnabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
        boolean isBluetoothEnabled = (bluetoothAdapter != null && bluetoothAdapter.isEnabled());

        return (isGpsEnabled && isBluetoothEnabled ? 1 : 0);
     }

    public static void start(Context context) {
        LocationManager locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        boolean isGpsEnabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
        boolean isBluetoothEnabled = (bluetoothAdapter != null && bluetoothAdapter.isEnabled());

        Log.d("LocatioHelper", "starting..");

        if (!isGpsEnabled || !isBluetoothEnabled) {
            Log.d("LocatioHelper", "requesting..");
                if (!isGpsEnabled) {
                    Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
                    a.startActivity(intent);
                }
                // Indirizza l'utente alle impostazioni del Bluetooth
                if (!isBluetoothEnabled) {
                    Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                    a.startActivity(intent);
                }
        } else {
        }        
     }
}
