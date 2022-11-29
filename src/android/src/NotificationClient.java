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
		  try {
			   m_notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

				if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
					 int importance = NotificationManager.IMPORTANCE_DEFAULT;
					 NotificationChannel notificationChannel = new NotificationChannel("QZ", "QZ Notifier", importance);
					 m_notificationManager.createNotificationChannel(notificationChannel);
					 m_builder = new Notification.Builder(context, notificationChannel.getId());
					} else {
					 m_builder = new Notification.Builder(context);
					}

				m_builder.setSmallIcon(R.drawable.icon)
				        .setLargeIcon(BitmapFactory.decodeResource(context.getResources(), R.drawable.icon))
						  .setContentTitle("QZ is running!")
						  .setContentText(message)
						  .setDefaults(Notification.DEFAULT_SOUND)
						  .setColor(Color.MAGENTA)
						  .setOngoing(true);

						m_notificationManager.notify(0, m_builder.build());
				} catch (Exception e) {
				e.printStackTrace();
				}
	 }
}
