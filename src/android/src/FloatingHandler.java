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

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

public class FloatingHandler {
    static Context _context;

	public static void show(Context context) {
            _context = context;
		// First it confirms whether the
		// 'Display over other apps' permission in given
		if (checkOverlayDisplayPermission()) {
			 // FloatingWindowGFG service is started
                         context.startService(new Intent(context, FloatingWindowGFG.class));
			 // The MainActivity closes here
			 //finish();
			} else {
			 // If permission is not given,
			 // it shows the AlertDialog box and
			 // redirects to the Settings
			 requestOverlayDisplayPermission();
			}
	}


private static void requestOverlayDisplayPermission() {
	     // An AlertDialog is created
                  AlertDialog.Builder builder = new AlertDialog.Builder(_context);

		  // This dialog can be closed, just by taping
		  // anywhere outside the dialog-box
		  builder.setCancelable(true);

		  // The title of the Dialog-box is set
		  builder.setTitle("Screen Overlay Permission Needed");

		  // The message of the Dialog-box is set
		  builder.setMessage("Enable 'Display over other apps' from System Settings.");

		  // The event of the Positive-Button is set
		  builder.setPositiveButton("Open Settings", new DialogInterface.OnClickListener() {
			   @Override
				public void onClick(DialogInterface dialog, int which) {
					 // The app will redirect to the 'Display over other apps' in Settings.
					 // This is an Implicit Intent. This is needed when any Action is needed
					 // to perform, here it is
					 // redirecting to an other app(Settings).
                                         Intent intent = new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION, Uri.parse("package:" + _context.getPackageName()));

					 // This method will start the intent. It takes two parameter, one is the Intent and the other is
					 // an requestCode Integer. Here it is -1.
                                         Activity a = (Activity)_context;
                                         a.startActivityForResult(intent, -1);
					}
			});
             AlertDialog dialog = builder.create();
		  // The Dialog will
		  // show in the screen
		  dialog.show();
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
