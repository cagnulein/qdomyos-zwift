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

	 public NotificationClient() {}

	 public static void notify(Context context, String message) {
		  Intent serviceIntent = new Intent(this, ForegroundService.class);
		  serviceIntent.putExtra("inputExtra", "QZ is Running");
		  ContextCompat.startForegroundService(this, serviceIntent);
	 }
}
