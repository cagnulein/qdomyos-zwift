package org.cagnulen.qdomyoszwift;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.app.PendingIntent;
import android.graphics.Color;
import android.graphics.BitmapFactory;
import android.app.NotificationChannel;

public class NotificationClient
{
	 private static NotificationManager m_notificationManager;
	 private static Notification.Builder m_builder;
         private static Context _context;
         private static Intent serviceIntent = null;

         private static final String EXTRA_FOREGROUND_SERVICE_TYPE = "FOREGROUND_SERVICE_TYPE";
         private static final int FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE = 0x10;

	 public NotificationClient() {}

	 public static void notify(Context context, String message) {
                  _context = context;
                  serviceIntent = new Intent(context, ForegroundService.class);
		  serviceIntent.putExtra("inputExtra", "QZ is Running");
                  serviceIntent.putExtra(EXTRA_FOREGROUND_SERVICE_TYPE,
                                         FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE);

		  if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
			   context.startForegroundService(serviceIntent);
		  } else {
		      context.startService(serviceIntent);
		  }
	 }

         public static void hide() {
             if(serviceIntent != null)
                _context.stopService(serviceIntent);
         }
}
