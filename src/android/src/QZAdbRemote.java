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
		  ADBConnected = true;
		  QLog.i(LOG_TAG, "notifyConnectionEstablished" + lastCommand);
		}

	 @Override
	 public void notifyConnectionFailed(DeviceConnection devConn, Exception e) {
		  ADBConnected = false;
		  QLog.e(LOG_TAG, e.getMessage());
		}

	 @Override
	 public void notifyStreamFailed(DeviceConnection devConn, Exception e) {
		  ADBConnected = false;
		  QLog.e(LOG_TAG, e.getMessage());
		}

	 @Override
	 public void notifyStreamClosed(DeviceConnection devConn) {
		  ADBConnected = false;
		  QLog.e(LOG_TAG, "notifyStreamClosed");
		}

	 @Override
	 public AdbCrypto loadAdbCrypto(DeviceConnection devConn) {
		  return AdbUtils.readCryptoConfig(_context.getFilesDir());
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
		  /* Create the connection object */
		  DeviceConnection conn = binder.createConnection(host, port);

		  /* Add this activity as a connection listener */
		  binder.addListener(conn, this);

		  /* Begin the async connection process */
		  conn.startConnect();

		  return conn;
		}

	 private DeviceConnection connectOrLookupConnection(String host, int port) {
		  DeviceConnection conn = binder.findConnection(host, port);
		  if (conn == null) {
			   /* No existing connection, so start the connection process */
				conn = startConnection(host, port);
				}
			else {
			   /* Add ourselves as a new listener of this connection */
				binder.addListener(conn, this);
				}
			return conn;
		}

	 public ServiceConnection serviceConn = new ServiceConnection() {
		  @Override
		  public void onServiceConnected(ComponentName arg0, IBinder arg1) {
			   binder = (ShellService.ShellServiceBinder)arg1;
				if (connection != null) {
					 binder.removeListener(connection, QZAdbRemote.getInstance());
					}
				connection = connectOrLookupConnection(_address, 5555);
				}

			@Override
		  public void onServiceDisconnected(ComponentName arg0) {
			   binder = null;
				}
	 };

         static public void createConnection(String ip, Context context) {
		  _address = ip;
		  _context = context;

		  /* If we have old RSA keys, just use them */
		  AdbCrypto crypto = AdbUtils.readCryptoConfig(_context.getFilesDir());
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

		  if (binder == null) {
			   service = new Intent(_context, ShellService.class);
                           service.putExtra(EXTRA_FOREGROUND_SERVICE_TYPE, FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE);

				/* Bind the service if we're not bound already. After binding, the callback will
				 * perform the initial connection. */
				_context.bindService(service, QZAdbRemote.getInstance().serviceConn, Service.BIND_AUTO_CREATE);

				if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                                         _context.startForegroundService(service);
					}
				else {
					 _context.startService(service);
					}
			}
	 }

    static public void sendCommand(String command) {
                  QLog.d(LOG_TAG, "sendCommand " + ADBConnected + " " + command);
		  if(ADBConnected) {
			   StringBuilder commandBuffer = new StringBuilder();

				commandBuffer.append(command);

				/* Append a newline since it's not included in the command itself */
				commandBuffer.append('\n');

				/* Send it to the device */
				connection.queueCommand(commandBuffer.toString());
				} else {
				QLog.e(LOG_TAG, "sendCommand ADB is not connected!");
				}
	 }

}
