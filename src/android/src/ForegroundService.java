package org.cagnulen.qdomyoszwift;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.IBinder;
import androidx.core.app.NotificationCompat;
import android.content.pm.ServiceInfo;
import org.cagnulen.qdomyoszwift.QLog;

public class ForegroundService extends Service {
	 public static final String CHANNEL_ID = "ForegroundServiceChannel";
         private static final String EXTRA_FOREGROUND_SERVICE_TYPE = "FOREGROUND_SERVICE_TYPE";
         private static final int FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE = 0x10;
         private WifiManager.MulticastLock multicastLock;

         private void acquireMulticastLock() {
             if (multicastLock != null && multicastLock.isHeld()) {
                 return;
             }

             try {
                 WifiManager wifiManager = (WifiManager) getApplicationContext().getSystemService(WIFI_SERVICE);
                 if (wifiManager == null) {
                     QLog.d("ForegroundService", "WifiManager unavailable; cannot acquire multicast lock");
                     return;
                 }

                 multicastLock = wifiManager.createMulticastLock("QZ:ForegroundService");
                 multicastLock.setReferenceCounted(false);
                 multicastLock.acquire();
                 QLog.d("ForegroundService", "Acquired multicast lock");
             } catch (Exception e) {
                 QLog.e("ForegroundService", "Failed to acquire multicast lock", e);
             }
         }

         private void releaseMulticastLock() {
             try {
                 if (multicastLock != null && multicastLock.isHeld()) {
                     multicastLock.release();
                     QLog.d("ForegroundService", "Released multicast lock");
                 }
             } catch (Exception e) {
                 QLog.e("ForegroundService", "Failed to release multicast lock", e);
             } finally {
                 multicastLock = null;
             }
         }

	 @Override
	 public void onCreate() {
		  super.onCreate();
                  acquireMulticastLock();
		}
	 @Override
	 public int onStartCommand(Intent intent, int flags, int startId) {
                  acquireMulticastLock();
		  String input = intent.getStringExtra("inputExtra");
		  createNotificationChannel();
                  Intent notificationIntent = new Intent();
		  PendingIntent pendingIntent = PendingIntent.getActivity(this,
		          0, notificationIntent, PendingIntent.FLAG_IMMUTABLE);
					Notification notification = new NotificationCompat.Builder(this, CHANNEL_ID)
					 .setContentTitle("QZ is Running")
					 .setContentText(input)
					 .setSmallIcon(R.drawable.icon)
					 .setContentIntent(pendingIntent)
					 .build();

                                        try {
                                             int serviceType = intent.getIntExtra(EXTRA_FOREGROUND_SERVICE_TYPE, FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE);
                                             if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                                                startForeground(1, notification, serviceType);
                                             } else {
                                                startForeground(1, notification);
                                             }
                                         } catch (Exception e) {
                                             QLog.e("ForegroundService", "Failed to start foreground service", e);
                                             return START_NOT_STICKY;
                                         }
		  //do heavy work on a background thread
		  //stopSelf();
		  return START_NOT_STICKY;
		}
	 @Override
	 public void onDestroy() {
                  releaseMulticastLock();
		  super.onDestroy();
		}

	 @Override
	 public IBinder onBind(Intent intent) {
		  return null;
		}
	 private void createNotificationChannel() {
		  if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
			   NotificationChannel serviceChannel = new NotificationChannel(
				        CHANNEL_ID,
						  "Foreground Service Channel",
						  NotificationManager.IMPORTANCE_DEFAULT
						);
				NotificationManager manager = getSystemService(NotificationManager.class);
				manager.createNotificationChannel(serviceChannel);
				}
	 }
}
