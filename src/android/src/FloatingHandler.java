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
import org.cagnulen.qdomyoszwift.qlog as Log;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

public class FloatingHandler {
    static Context _context;
    static public int _port = 0;
    static Intent _intent = null;
    static public int _width;
    static public int _height;
    static public int _alpha;

        public static void show(Context context, int port, int width, int height, int transparency) {
            _context = context;
            _port = port;
            _width = width;
            _height = height;
            _alpha = transparency;

		// First it confirms whether the
		// 'Display over other apps' permission in given
		if (checkOverlayDisplayPermission()) {
                         if(_intent == null)
                            _intent = new Intent(context, FloatingWindowGFG.class);
			 // FloatingWindowGFG service is started
                         context.startService(_intent);
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

    public static void hide() {
        if(_intent != null)
            _context.stopService(_intent);
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
