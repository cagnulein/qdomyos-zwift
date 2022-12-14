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
import android.util.Log;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

public class FloatingHandler {
    static Context _context;
    static public int _port = 0;

        public static void show(Context context, int port) {
            _context = context;
            _port = port;
		// First it confirms whether the
		// 'Display over other apps' permission in given
		if (checkOverlayDisplayPermission()) {
			 // FloatingWindowGFG service is started
                         context.startService(new Intent(context, FloatingWindowGFG.class));
			 // The MainActivity closes here
                         //finish();
			} else {
                            Intent intent = new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION, Uri.parse("package:" + _context.getPackageName()));

                            // This method will start the intent. It takes two parameter, one is the Intent and the other is
                            // an requestCode Integer. Here it is -1.
                            Activity a = (Activity)_context;
                            a.startActivityForResult(intent, -1);
			}
	}

   private static boolean checkOverlayDisplayPermission() {
		  // Android Version is lesser than Marshmallow
		    // or the API is lesser than 23
			// doesn't need 'Display over other apps' permission enabling.
		  if (Build.VERSION.SDK_INT > Build.VERSION_CODES.M) {
			   // If 'Display over other apps' is not enabled it
				  // will return false or else true
                                if (!Settings.canDrawOverlays(_context)) {
                                         return false;
					} else {
					 return true;
					}
			} else {
			   return true;
				}
	 }
}
