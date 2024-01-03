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

    public static void start(Context context) {
        LocationManager locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        boolean isGpsEnabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
        boolean isBluetoothEnabled = (bluetoothAdapter != null && bluetoothAdapter.isEnabled());

        Log.d("LocatioHelper", "starting..");

        if (!isGpsEnabled || !isBluetoothEnabled) {
            Log.d("LocatioHelper", "requesting..");
            // Mostra una finestra di dialogo per avvisare che i servizi di localizzazione e il Bluetooth sono necessari
            Activity a = (Activity)context;
            new AlertDialog.Builder(a)
                .setTitle("Required Services")
                .setMessage("Please enable Location Services and Bluetooth.")
                .setPositiveButton("Settings", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        // Indirizza l'utente alle impostazioni del GPS
                        if (!isGpsEnabled) {
                            Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
                            a.startActivity(intent);
                        }
                        // Indirizza l'utente alle impostazioni del Bluetooth
                        if (!isBluetoothEnabled) {
                            Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                            a.startActivity(intent);
                        }
                    }
                })/*
                .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        // Gestisci l'annullamento qui se necessario
                    }
                })*/
                .setIcon(android.R.drawable.ic_dialog_alert)
                .show();
        } else {
            // I servizi di localizzazione e il Bluetooth sono già attivi
            // Implementa qui la tua logica se i servizi di localizzazione e il Bluetooth sono attivi
        }        
     }
}
