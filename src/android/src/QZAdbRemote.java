package org.cagnulen.qdomyoszwift;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.os.PowerManager;
import android.provider.Settings;
import org.cagnulen.qdomyoszwift.QLog;
import android.view.View;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import java.util.logging.Logger;

import static android.content.ContentValues.TAG;

import androidx.core.app.NotificationCompat;

import com.cgutman.androidremotedebugger.AdbUtils;
import com.cgutman.androidremotedebugger.console.ConsoleBuffer;
import com.cgutman.androidremotedebugger.devconn.DeviceConnection;
import com.cgutman.androidremotedebugger.devconn.DeviceConnectionListener;
import com.cgutman.androidremotedebugger.service.ShellService;
import com.cgutman.adblib.AdbCrypto;

public class QZAdbRemote implements DeviceConnectionListener {
	 private static ShellService.ShellServiceBinder binder;
	 private static DeviceConnection connection;
	 private static Intent service;
	 private static final String LOG_TAG = "QZ:AdbRemote";
	 private static String lastCommand = "";
	 private static boolean ADBConnected = false;

	 private static String _address = "127.0.0.1";
	 private static Context _context;

	 private static QZAdbRemote INSTANCE;

         private static final String EXTRA_FOREGROUND_SERVICE_TYPE = "FOREGROUND_SERVICE_TYPE";
         private static final int FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE = 0x10;

	 public static QZAdbRemote getInstance() {
		 if(INSTANCE == null) {
			 INSTANCE = new QZAdbRemote();
		 }

	    return INSTANCE;
	 }

	 @Override
	 public void notifyConnectionEstablished(DeviceConnection devConn) {
		  QLog.d(LOG_TAG, "notifyConnectionEstablished - START: devConn=" + devConn + ", host=" + (devConn != null ? devConn.getHost() : "null") + ", port=" + (devConn != null ? devConn.getPort() : "null"));
		  ADBConnected = true;
		  QLog.i(LOG_TAG, "notifyConnectionEstablished - CONNECTED=true, lastCommand=" + lastCommand);
		  QLog.d(LOG_TAG, "notifyConnectionEstablished - END: ADBConnected=" + ADBConnected);
		}

	 @Override
	 public void notifyConnectionFailed(DeviceConnection devConn, Exception e) {
		  QLog.d(LOG_TAG, "notifyConnectionFailed - START: devConn=" + devConn + ", host=" + (devConn != null ? devConn.getHost() : "null") + ", port=" + (devConn != null ? devConn.getPort() : "null"));
		  ADBConnected = false;
		  QLog.e(LOG_TAG, "notifyConnectionFailed - ERROR: " + (e != null ? e.getMessage() : "null exception") + ", ADBConnected=" + ADBConnected);
		  if (e != null) {
			  QLog.e(LOG_TAG, "notifyConnectionFailed - STACK_TRACE: ", e);
		  }
		}

	 @Override
	 public void notifyStreamFailed(DeviceConnection devConn, Exception e) {
		  QLog.d(LOG_TAG, "notifyStreamFailed - START: devConn=" + devConn + ", host=" + (devConn != null ? devConn.getHost() : "null") + ", port=" + (devConn != null ? devConn.getPort() : "null"));
		  ADBConnected = false;
		  QLog.e(LOG_TAG, "notifyStreamFailed - ERROR: " + (e != null ? e.getMessage() : "null exception") + ", ADBConnected=" + ADBConnected);
		  if (e != null) {
			  QLog.e(LOG_TAG, "notifyStreamFailed - STACK_TRACE: ", e);
		  }
		}

	 @Override
	 public void notifyStreamClosed(DeviceConnection devConn) {
		  QLog.d(LOG_TAG, "notifyStreamClosed - START: devConn=" + devConn + ", host=" + (devConn != null ? devConn.getHost() : "null") + ", port=" + (devConn != null ? devConn.getPort() : "null"));
		  ADBConnected = false;
		  QLog.e(LOG_TAG, "notifyStreamClosed - ADBConnected=" + ADBConnected);
		}

	 @Override
	 public AdbCrypto loadAdbCrypto(DeviceConnection devConn) {
		  QLog.d(LOG_TAG, "loadAdbCrypto - START: devConn=" + devConn + ", context=" + _context);
		  AdbCrypto crypto = AdbUtils.readCryptoConfig(_context.getFilesDir());
		  QLog.d(LOG_TAG, "loadAdbCrypto - RESULT: crypto=" + (crypto != null ? "valid" : "null"));
		  return crypto;
		}

	 @Override
	 public boolean canReceiveData() {
		  return true;
		}

	 @Override
	 public void receivedData(DeviceConnection devConn, byte[] data, int offset, int length) {
		  QLog.i(LOG_TAG, data.toString());
		}

	 @Override
	 public boolean isConsole() {
		  return false;
		}

	 @Override
	 public void consoleUpdated(DeviceConnection devConn, ConsoleBuffer console) {

		}


	 private DeviceConnection startConnection(String host, int port) {
		  QLog.d(LOG_TAG, "startConnection - START: host=" + host + ", port=" + port + ", binder=" + binder);
		  /* Create the connection object */
		  DeviceConnection conn = binder.createConnection(host, port);
		  QLog.d(LOG_TAG, "startConnection - CONNECTION_CREATED: conn=" + conn);

		  /* Add this activity as a connection listener */
		  binder.addListener(conn, this);
		  QLog.d(LOG_TAG, "startConnection - LISTENER_ADDED: this=" + this);

		  /* Begin the async connection process */
		  QLog.d(LOG_TAG, "startConnection - STARTING_CONNECT: about to call conn.startConnect()");
		  conn.startConnect();
		  QLog.d(LOG_TAG, "startConnection - END: startConnect() called, returning conn=" + conn);

		  return conn;
		}

	 private DeviceConnection connectOrLookupConnection(String host, int port) {
		  QLog.d(LOG_TAG, "connectOrLookupConnection - START: host=" + host + ", port=" + port + ", binder=" + binder);
		  DeviceConnection conn = binder.findConnection(host, port);
		  QLog.d(LOG_TAG, "connectOrLookupConnection - EXISTING_CONN: conn=" + (conn != null ? "found" : "null"));
		  if (conn == null) {
			   /* No existing connection, so start the connection process */
				QLog.d(LOG_TAG, "connectOrLookupConnection - NEW_CONNECTION: starting new connection");
				conn = startConnection(host, port);
				}
			else {
			   /* Add ourselves as a new listener of this connection */
				QLog.d(LOG_TAG, "connectOrLookupConnection - REUSE_CONNECTION: adding listener to existing connection");
				binder.addListener(conn, this);
				}
			QLog.d(LOG_TAG, "connectOrLookupConnection - END: returning conn=" + conn);
			return conn;
		}

	 public ServiceConnection serviceConn = new ServiceConnection() {
		  @Override
		  public void onServiceConnected(ComponentName arg0, IBinder arg1) {
			   QLog.d(LOG_TAG, "onServiceConnected - START: componentName=" + arg0 + ", binder=" + arg1 + ", _address=" + _address);
			   binder = (ShellService.ShellServiceBinder)arg1;
				QLog.d(LOG_TAG, "onServiceConnected - BINDER_SET: binder=" + binder + ", existing_connection=" + connection);
				if (connection != null) {
					 QLog.d(LOG_TAG, "onServiceConnected - REMOVING_OLD_LISTENER: connection=" + connection);
					 binder.removeListener(connection, QZAdbRemote.getInstance());
					}
				QLog.d(LOG_TAG, "onServiceConnected - CONNECTING: about to call connectOrLookupConnection");
				connection = connectOrLookupConnection(_address, 5555);
				QLog.d(LOG_TAG, "onServiceConnected - END: connection=" + connection);
				}

			@Override
		  public void onServiceDisconnected(ComponentName arg0) {
			   QLog.d(LOG_TAG, "onServiceDisconnected - START: componentName=" + arg0 + ", old_binder=" + binder);
			   binder = null;
				QLog.d(LOG_TAG, "onServiceDisconnected - END: binder set to null");
				}
	 };

         static public void createConnection(String ip, Context context) {
		  QLog.d(LOG_TAG, "createConnection - START: ip=" + ip + ", context=" + context + ", existing_binder=" + binder);
		  _address = ip;
		  _context = context;
		  QLog.d(LOG_TAG, "createConnection - PARAMS_SET: _address=" + _address + ", _context=" + _context);

		  /* If we have old RSA keys, just use them */
		  QLog.d(LOG_TAG, "createConnection - CHECKING_CRYPTO: reading existing crypto config");
		  AdbCrypto crypto = AdbUtils.readCryptoConfig(_context.getFilesDir());
		  QLog.d(LOG_TAG, "createConnection - CRYPTO_CHECK: crypto=" + (crypto != null ? "exists" : "null"));
		  if (crypto == null)
		  {
			   /* We need to make a new pair */
				QLog.i(LOG_TAG,
				        "This will only be done once.");

						new Thread(new Runnable() {
					 @Override
					 public void run() {
						  AdbCrypto crypto;

						  crypto = AdbUtils.writeNewCryptoConfig(_context.getFilesDir());

						  if (crypto == null)
						  {
							   QLog.e(LOG_TAG,
								        "Unable to generate and save RSA key pair");
										return;
								}

					 }
				}).start();
			}

		  QLog.d(LOG_TAG, "createConnection - SERVICE_CHECK: binder=" + (binder != null ? "exists" : "null"));
		  if (binder == null) {
				QLog.i(LOG_TAG, "createConnection - STARTING_SERVICE: Starting ShellService.class");

			   service = new Intent(_context, ShellService.class);
                           service.putExtra(EXTRA_FOREGROUND_SERVICE_TYPE, FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE);
				QLog.d(LOG_TAG, "createConnection - SERVICE_INTENT: service=" + service);

				/* Bind the service if we're not bound already. After binding, the callback will
				 * perform the initial connection. */
				QLog.d(LOG_TAG, "createConnection - BINDING_SERVICE: about to bind service");
				_context.bindService(service, QZAdbRemote.getInstance().serviceConn, Service.BIND_AUTO_CREATE);
				QLog.d(LOG_TAG, "createConnection - SERVICE_BOUND: bindService called");

				QLog.d(LOG_TAG, "createConnection - STARTING_SERVICE: SDK_INT=" + Build.VERSION.SDK_INT);
				if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                                         QLog.d(LOG_TAG, "createConnection - FOREGROUND_SERVICE: starting foreground service");
                                         _context.startForegroundService(service);
					}
				else {
					 QLog.d(LOG_TAG, "createConnection - REGULAR_SERVICE: starting regular service");
					 _context.startService(service);
					}
				QLog.d(LOG_TAG, "createConnection - SERVICE_STARTED: service start completed");
			} else {
				QLog.d(LOG_TAG, "createConnection - SKIP_SERVICE: binder already exists, skipping service creation");
			}
			 QLog.d(LOG_TAG, "createConnection - END: method completed");
	 }

    static public void sendCommand(String command) {
                  QLog.d(LOG_TAG, "sendCommand " + ADBConnected + " " + command);

			if (connection != null && !connection.isClosed() && connection.isConnected()) {
				StringBuilder commandBuffer = new StringBuilder();

				commandBuffer.append(command);

				/* Append a newline since it's not included in the command itself */
				commandBuffer.append('\n');

				/* Send it to the device */
				connection.queueCommand(commandBuffer.toString());
			} else {
				QLog.e(LOG_TAG, "sendCommand ADB is not connected!");
				createConnection(_address, _context);
			}
	 }

}
