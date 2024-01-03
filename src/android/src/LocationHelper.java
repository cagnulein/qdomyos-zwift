package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.location.LocationManager;

public class LocationHelper {

    public static void start(Context context) {
         LocationManager locationManager = (LocationManager) getSystemService(context.LOCATION_SERVICE);
         if (!locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
             // I servizi di localizzazione non sono attivi, indirizzare l'utente alle impostazioni del GPS
             Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
             startActivity(intent);
         } else {
             // I servizi di localizzazione sono già attivi
             // Implementa qui la tua logica se i servizi di localizzazione sono attivi
         }
     }
}
