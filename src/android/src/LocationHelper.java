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

public class LocationHelper {

    public static void start(Context context) {
         LocationManager locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
         Log.d("LocatioHelper", "starting..");
         if (!locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
             Log.d("LocatioHelper", "requesting..");
             // I servizi di localizzazione non sono attivi, indirizzare l'utente alle impostazioni del GPS
             Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
             Activity a = (Activity)context;
             a.startActivity(intent);
         } else {
             // I servizi di localizzazione sono già attivi
             // Implementa qui la tua logica se i servizi di localizzazione sono attivi
         }
     }
}
