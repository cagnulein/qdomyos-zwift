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
import org.cagnulen.qdomyoszwift.QLog;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

public class WearableController {
    static Context _context;
    static Intent _intent = null;

        public static void start(Context context) {            
            _context = context;

             if (GoogleApiAvailability.getInstance().isGooglePlayServicesAvailable(context)
                     != ConnectionResult.SUCCESS) {
                QLog.w("WearableController", "Google Play Services unavailable, wearable service not started");
                return;
             }

             if(_intent == null)
                _intent = new Intent(context, WearableMessageListenerService.class);
             // FloatingWindowGFG service is started
             context.startService(_intent);
             QLog.v("WearableController", "started");
        }

        public static int getHeart() {
            return WearableMessageListenerService.getHeart();
        }
}
