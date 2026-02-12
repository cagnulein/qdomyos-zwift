package com.cgutman.androidremotedebugger.service;


import java.util.HashMap;

import com.cgutman.adblib.AdbCrypto;
import com.cgutman.androidremotedebugger.console.ConsoleBuffer;
import com.cgutman.androidremotedebugger.devconn.DeviceConnection;
import com.cgutman.androidremotedebugger.devconn.DeviceConnectionListener;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.WifiLock;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import androidx.core.app.NotificationCompat;
import org.cagnulen.qdomyoszwift.QLog;

public class ShellService extends Service implements DeviceConnectionListener {

        private static final String EXTRA_FOREGROUND_SERVICE_TYPE = "FOREGROUND_SERVICE_TYPE";
        private static final int FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE = 0x10;
	
	private ShellServiceBinder binder = new ShellServiceBinder();
	private ShellListener listener = new ShellListener(this);
	
	private HashMap<String, DeviceConnection> currentConnectionMap =
			new HashMap<String, DeviceConnection>();
	
	private WifiLock wlanLock;
	private WakeLock wakeLock;

	private final static int FOREGROUND_PLACEHOLDER_ID = 1;
	private final static int CONN_BASE = 12131;
	private final static int FAILED_BASE = 12111;
	private final static String CHANNEL_ID = "connectionInfo";
	
	private int foregroundId;
	
	public class ShellServiceBinder extends Binder {
		public DeviceConnection createConnection(String host, int port) {
			QLog.d("ShellService", "createConnection - START: host=" + host + ", port=" + port + ", listener=" + listener);
			DeviceConnection conn = new DeviceConnection(listener, host, port);
			QLog.d("ShellService", "createConnection - CONNECTION_CREATED: conn=" + conn);
			listener.addListener(conn, ShellService.this);
			QLog.d("ShellService", "createConnection - LISTENER_ADDED: returning conn=" + conn);
			return conn;
		}
		
		public DeviceConnection findConnection(String host, int port) {
			String connStr = host+":"+port;
			QLog.d("ShellService", "findConnection - SEARCH: connStr=" + connStr + ", mapSize=" + currentConnectionMap.size());
			DeviceConnection found = currentConnectionMap.get(connStr);
			QLog.d("ShellService", "findConnection - RESULT: found=" + (found != null ? "exists" : "null"));
			return found;
		}
		
		public void notifyPausingActivity(DeviceConnection devConn) {
			devConn.setForeground(false);
		}
		
		public void notifyResumingActivity(DeviceConnection devConn) {
			devConn.setForeground(true);
		}
		
		public void notifyDestroyingActivity(DeviceConnection devConn) {
			/* If we're pausing before destruction after the connection is closed, remove the failure
			 * notification */
			if (devConn.isClosed()) {
				NotificationManager nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
				nm.cancel(getFailedNotificationId(devConn));
			}
		}
		
		public void addListener(DeviceConnection conn, DeviceConnectionListener listener) {
			QLog.d("ShellService", "addListener - START: conn=" + conn + ", listener=" + listener);
			ShellService.this.listener.addListener(conn, listener);
			QLog.d("ShellService", "addListener - END: listener added");
		}
		
		public void removeListener(DeviceConnection conn, DeviceConnectionListener listener) {
			QLog.d("ShellService", "removeListener - START: conn=" + conn + ", listener=" + listener);
			ShellService.this.listener.removeListener(conn, listener);
			QLog.d("ShellService", "removeListener - END: listener removed");
		}
	}

	@Override
	public IBinder onBind(Intent arg0) {
		QLog.d("ShellService", "onBind - START: intent=" + arg0 + ", binder=" + binder);
		QLog.d("ShellService", "onBind - END: returning binder");
		return binder;
	}

	@Override
	public boolean onUnbind(Intent intent) {
		QLog.d("ShellService", "onUnbind - START: intent=" + intent + ", connections=" + currentConnectionMap.size());
		/* Stop the service if no connections remain */
		if (currentConnectionMap.isEmpty()) {
			QLog.d("ShellService", "onUnbind - STOPPING_SERVICE: no connections remain");
			stopSelf();
		} else {
			QLog.d("ShellService", "onUnbind - KEEPING_SERVICE: " + currentConnectionMap.size() + " connections remain");
		}

		QLog.d("ShellService", "onUnbind - END: returning false");
		return false;
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		QLog.d("ShellService", "onStartCommand - START: intent=" + intent + ", flags=" + flags + ", startId=" + startId + ", foregroundId=" + foregroundId);
		if (foregroundId == 0) {
                    try {
                        int serviceType = intent.getIntExtra(EXTRA_FOREGROUND_SERVICE_TYPE, FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE);
			// If we're not already running in the foreground, use a placeholder
			// notification until a real connection is established. After connection
			// establishment, the real notification will replace this one.
			QLog.d("ShellService", "onStartCommand - FOREGROUND_START: serviceType=" + serviceType + ", SDK_INT=" + Build.VERSION.SDK_INT);
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                            QLog.d("ShellService", "onStartCommand - FOREGROUND_Q+: starting with service type");
                            startForeground(FOREGROUND_PLACEHOLDER_ID, createForegroundPlaceholderNotification(), serviceType);
                        } else {
                            QLog.d("ShellService", "onStartCommand - FOREGROUND_LEGACY: starting without service type");
                            startForeground(FOREGROUND_PLACEHOLDER_ID, createForegroundPlaceholderNotification());
                        }
                        QLog.d("ShellService", "onStartCommand - FOREGROUND_SUCCESS: foreground service started");
                    } catch (Exception e) {
                        QLog.e("ForegroundService", "Failed to start foreground service", e);
                        return START_NOT_STICKY;
                    }
		} else {
			QLog.d("ShellService", "onStartCommand - SKIP_FOREGROUND: already running in foreground with id=" + foregroundId);
		}

		// Don't restart if we've been killed. We will have already lost our connections
		// when we died, so we'll just be running doing nothing if the OS restarted us.
		QLog.d("ShellService", "onStartCommand - END: returning START_NOT_STICKY");
		return Service.START_NOT_STICKY;
	}

	@Override
	public void onCreate() {
		QLog.d("ShellService", "onCreate - START: initializing service");
		super.onCreate();

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
			QLog.d("ShellService", "onCreate - NOTIFICATION_CHANNEL: creating notification channel");
			NotificationChannel channel = new NotificationChannel(CHANNEL_ID, "Connection Info", NotificationManager.IMPORTANCE_DEFAULT);
			NotificationManager notificationManager = getSystemService(NotificationManager.class);
			notificationManager.createNotificationChannel(channel);
			QLog.d("ShellService", "onCreate - NOTIFICATION_CHANNEL: channel created");
		}

		QLog.d("ShellService", "onCreate - WIFI_LOCK: creating wifi lock");
		WifiManager wm = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
		wlanLock = wm.createWifiLock(WifiManager.WIFI_MODE_FULL, "RemoteADBShell:ShellService");
		QLog.d("ShellService", "onCreate - WIFI_LOCK: wlanLock=" + wlanLock);

		QLog.d("ShellService", "onCreate - WAKE_LOCK: creating wake lock");
		PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
		wakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "RemoteADBShell:ShellService");
		QLog.d("ShellService", "onCreate - WAKE_LOCK: wakeLock=" + wakeLock);
		QLog.d("ShellService", "onCreate - END: service initialization complete");
	}

	@Override
	public void onDestroy() {
		if (wlanLock.isHeld()) {
			wlanLock.release();
		}
		if (wakeLock.isHeld()) {
			wakeLock.release();
		}

		super.onDestroy();
	}
	
	private int getFailedNotificationId(DeviceConnection devConn) {
		return FAILED_BASE + getConnectionString(devConn).hashCode();
	}
	
	private int getConnectedNotificationId(DeviceConnection devConn) {
		return CONN_BASE + getConnectionString(devConn).hashCode();
	}

	private Notification createForegroundPlaceholderNotification() {
		return new NotificationCompat.Builder(getApplicationContext(), CHANNEL_ID)
				.setOngoing(true)
				.setSilent(true)
				.setContentTitle("Remote ADB Shell")
				.setContentText("Connecting...")
				.setForegroundServiceBehavior(NotificationCompat.FOREGROUND_SERVICE_IMMEDIATE)
				.build();
	}

	private Notification createConnectionNotification(DeviceConnection devConn, boolean connected) {
		String ticker;
		String message;
		
		if (connected) {
			ticker = "Connection Established";
			message = "Connected to "+getConnectionString(devConn);
		}
		else {
			ticker = "Connection Terminated";
			message = "Connection to "+getConnectionString(devConn)+" failed";
		}

		return new NotificationCompat.Builder(getApplicationContext(), CHANNEL_ID)
				.setTicker("Remote ADB Shell - "+ticker)
				.setOnlyAlertOnce(true)
				.setOngoing(connected)
				.setAutoCancel(!connected)
				.setSilent(connected)
				.setContentTitle("Remote ADB Shell")
				.setContentText(message)
				.setForegroundServiceBehavior(NotificationCompat.FOREGROUND_SERVICE_IMMEDIATE)
				.build();
	}
	
	private void updateNotification(DeviceConnection devConn, boolean connected) {
		NotificationManager nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		
		removeNotification(devConn);
	}
	
	private void removeNotification(DeviceConnection devConn) {
		NotificationManager nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		
		/* Removing failure notifications is easy */
		nm.cancel(getFailedNotificationId(devConn));
		
		/* Connected notifications is a bit more complex */
		if (getConnectedNotificationId(devConn) == foregroundId) {
			/* We're the foreground notification, so we need to switch in another
			 * notification to take our place */
			
			/* Search for a new device connection to promote */
			DeviceConnection newConn = null;
			for (DeviceConnection conn : currentConnectionMap.values()) {
				if (devConn == conn) {
					continue;
				}
				else {
					newConn = conn;
					break;
				}
			}
			
			if (newConn == null) {
				/* None found, so we're done in foreground */
				stopForeground(true);
				foregroundId = 0;
			}
			else {
				/* Found one, so cancel this guy's original notification
				 * and start it as foreground */
				foregroundId = getConnectedNotificationId(newConn);
				nm.cancel(foregroundId);
                                startForeground(foregroundId, createConnectionNotification(newConn, true));
			}
		}
		else {
			/* This just a normal connected notification */
			nm.cancel(getConnectedNotificationId(devConn));
		}
	}
	
	private String getConnectionString(DeviceConnection devConn) {
		return devConn.getHost()+":"+devConn.getPort();
	}
	
	private synchronized void addNewConnection(DeviceConnection devConn) {
		QLog.d("ShellService", "addNewConnection - START: devConn=" + devConn + ", currentSize=" + currentConnectionMap.size());
		if (currentConnectionMap.isEmpty()) {
			QLog.d("ShellService", "addNewConnection - ACQUIRING_LOCKS: first connection, acquiring locks");
			wakeLock.acquire();
			wlanLock.acquire();
			QLog.d("ShellService", "addNewConnection - LOCKS_ACQUIRED: wakeLock and wlanLock acquired");
		}

		String connString = getConnectionString(devConn);
		QLog.d("ShellService", "addNewConnection - ADDING: connString=" + connString);
		currentConnectionMap.put(connString, devConn);
		QLog.d("ShellService", "addNewConnection - END: connection added, newSize=" + currentConnectionMap.size());
	}
	
	private synchronized void removeConnection(DeviceConnection devConn) {
		String connString = getConnectionString(devConn);
		QLog.d("ShellService", "removeConnection - START: devConn=" + devConn + ", connString=" + connString + ", currentSize=" + currentConnectionMap.size());
		currentConnectionMap.remove(connString);
		QLog.d("ShellService", "removeConnection - REMOVED: newSize=" + currentConnectionMap.size());

		/* Stop the service if no connections remain */
		if (currentConnectionMap.isEmpty()) {
			QLog.d("ShellService", "removeConnection - STOPPING_SERVICE: no connections remain");
			stopSelf();
		} else {
			QLog.d("ShellService", "removeConnection - KEEPING_SERVICE: " + currentConnectionMap.size() + " connections remain");
		}
		QLog.d("ShellService", "removeConnection - END");
	}

	@Override
	public void notifyConnectionEstablished(DeviceConnection devConn) {
		QLog.d("ShellService", "notifyConnectionEstablished - START: devConn=" + devConn + ", host=" + (devConn != null ? devConn.getHost() : "null") + ", port=" + (devConn != null ? devConn.getPort() : "null"));
		addNewConnection(devConn);
		QLog.d("ShellService", "notifyConnectionEstablished - CONNECTION_ADDED: updating notification");
		updateNotification(devConn, true);
		QLog.d("ShellService", "notifyConnectionEstablished - END: connection established successfully");
	}

	@Override
	public void notifyConnectionFailed(DeviceConnection devConn, Exception e) {
		QLog.d("ShellService", "notifyConnectionFailed - START: devConn=" + devConn + ", host=" + (devConn != null ? devConn.getHost() : "null") + ", port=" + (devConn != null ? devConn.getPort() : "null"));
		QLog.e("ShellService", "notifyConnectionFailed - ERROR: " + (e != null ? e.getMessage() : "null exception"));
		if (e != null) {
			QLog.e("ShellService", "notifyConnectionFailed - STACK_TRACE: ", e);
		}
		/* No notification is displaying here */
		QLog.d("ShellService", "notifyConnectionFailed - END");
	}
	
	@Override
	public void notifyStreamFailed(DeviceConnection devConn, Exception e) {
		QLog.d("ShellService", "notifyStreamFailed - START: devConn=" + devConn + ", host=" + (devConn != null ? devConn.getHost() : "null") + ", port=" + (devConn != null ? devConn.getPort() : "null"));
		QLog.e("ShellService", "notifyStreamFailed - ERROR: " + (e != null ? e.getMessage() : "null exception"));
		if (e != null) {
			QLog.e("ShellService", "notifyStreamFailed - STACK_TRACE: ", e);
		}
		updateNotification(devConn, false);
		QLog.d("ShellService", "notifyStreamFailed - NOTIFICATION_UPDATED: removing connection");
		removeConnection(devConn);
		QLog.d("ShellService", "notifyStreamFailed - END");
	}

	@Override
	public void notifyStreamClosed(DeviceConnection devConn) {
		QLog.d("ShellService", "notifyStreamClosed - START: devConn=" + devConn + ", host=" + (devConn != null ? devConn.getHost() : "null") + ", port=" + (devConn != null ? devConn.getPort() : "null"));
		updateNotification(devConn, false);
		QLog.d("ShellService", "notifyStreamClosed - NOTIFICATION_UPDATED: removing connection");
		removeConnection(devConn);
		QLog.d("ShellService", "notifyStreamClosed - END");
	}

	@Override
	public AdbCrypto loadAdbCrypto(DeviceConnection devConn) {
		return null;
	}

	@Override
	public void receivedData(DeviceConnection devConn, byte[] data, int offset,
			int length) {
	}

	@Override
	public boolean canReceiveData() {
		return false;
	}

	@Override
	public boolean isConsole() {
		return false;
	}

	@Override
	public void consoleUpdated(DeviceConnection devConn,
			ConsoleBuffer console) {
	}
}
