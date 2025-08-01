package com.cgutman.androidremotedebugger.devconn;

import java.io.Closeable;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.concurrent.LinkedBlockingQueue;

import com.cgutman.adblib.AdbConnection;
import com.cgutman.adblib.AdbCrypto;
import com.cgutman.adblib.AdbStream;
import com.cgutman.androidremotedebugger.AdbUtils;
import org.cagnulen.qdomyoszwift.QLog;

public class DeviceConnection implements Closeable {
	private static final int CONN_TIMEOUT = 5000;

	private String host;
	private int port;
	private DeviceConnectionListener listener;
	
	private AdbConnection connection;
	private AdbStream shellStream;
	
	private boolean closed;
	private boolean foreground;
	
	private LinkedBlockingQueue<byte[]> commandQueue = new LinkedBlockingQueue<byte[]>();
	
	public DeviceConnection(DeviceConnectionListener listener, String host, int port) {
		this.host = host;
		this.port = port;
		this.listener = listener;
		this.foreground = true; /* Connections start in the foreground */
	}
	
	public String getHost() {
		return host;
	}
	
	public int getPort() {
		return port;
	}
	
	public boolean queueCommand(String command) {
		try {
			/* Queue it up for sending to the device */
			commandQueue.add(command.getBytes("UTF-8"));
			return true;
		} catch (UnsupportedEncodingException e) {
			return false;
		}
	}
	
	public boolean queueBytes(byte[] buffer) {
		/* Queue it up for sending to the device */
		commandQueue.add(buffer);
		return true;
	}
	
	public void startConnect() {
		QLog.d("DeviceConnection", "startConnect - START: host=" + host + ", port=" + port + ", listener=" + listener);
		new Thread(new Runnable() {
			@Override
			public void run() {
				QLog.d("DeviceConnection", "startConnect.run - THREAD_START: host=" + host + ", port=" + port);
				boolean connected = false;
				Socket socket = new Socket();
				AdbCrypto crypto;
				
				/* Load the crypto config */
				QLog.d("DeviceConnection", "startConnect.run - LOADING_CRYPTO: calling loadAdbCrypto");
				crypto = listener.loadAdbCrypto(DeviceConnection.this);
				if (crypto == null) {
					QLog.e("DeviceConnection", "startConnect.run - CRYPTO_FAILED: crypto is null, returning");
					return;
				}
				QLog.d("DeviceConnection", "startConnect.run - CRYPTO_LOADED: crypto=" + crypto);
				
				try {
					/* Establish a connect to the remote host */
					QLog.d("DeviceConnection", "startConnect.run - SOCKET_CONNECT: connecting to " + host + ":" + port + " with timeout=" + CONN_TIMEOUT);
					socket.connect(new InetSocketAddress(host, port), CONN_TIMEOUT);
					QLog.d("DeviceConnection", "startConnect.run - SOCKET_CONNECTED: socket connected successfully");
				} catch (IOException e) {
					QLog.e("DeviceConnection", "startConnect.run - SOCKET_FAILED: connection failed", e);
					listener.notifyConnectionFailed(DeviceConnection.this, e);
					return;
				}

				try {
					/* Establish the application layer connection */
					QLog.d("DeviceConnection", "startConnect.run - ADB_CONNECTION: creating AdbConnection");
					connection = AdbConnection.create(socket, crypto);
					QLog.d("DeviceConnection", "startConnect.run - ADB_CONNECT: calling connection.connect()");
					connection.connect();
					QLog.d("DeviceConnection", "startConnect.run - ADB_CONNECTED: ADB connection established");
					
					/* Open the shell stream */
					QLog.d("DeviceConnection", "startConnect.run - SHELL_STREAM: opening shell stream");
					shellStream = connection.open("shell:");
					QLog.d("DeviceConnection", "startConnect.run - SHELL_OPENED: shell stream opened successfully");
					connected = true;
				} catch (IOException e) {
					QLog.e("DeviceConnection", "startConnect.run - ADB_IO_ERROR: IOException during ADB connection", e);
					listener.notifyConnectionFailed(DeviceConnection.this, e);
				} catch (InterruptedException e) {
					QLog.e("DeviceConnection", "startConnect.run - ADB_INTERRUPTED: InterruptedException during ADB connection", e);
					listener.notifyConnectionFailed(DeviceConnection.this, e);
				} finally {
					/* Cleanup if the connection failed */
					if (!connected) {
						QLog.d("DeviceConnection", "startConnect.run - CLEANUP: connection failed, cleaning up");
						AdbUtils.safeClose(shellStream);
						
						/* The AdbConnection object will close the underlying socket
						 * but we need to close it ourselves if the AdbConnection object
						 * wasn't successfully constructed.
						 */
						if (!AdbUtils.safeClose(connection)) {
							try {
								socket.close();
							} catch (IOException e) {}
						}
						
						return;
					}
				}
				
				/* Notify the listener that the connection is complete */
				QLog.d("DeviceConnection", "startConnect.run - NOTIFY_SUCCESS: calling listener.notifyConnectionEstablished");
				listener.notifyConnectionEstablished(DeviceConnection.this);
				QLog.d("DeviceConnection", "startConnect.run - NOTIFIED: notifyConnectionEstablished called");
				
				/* Start the receive thread */
				QLog.d("DeviceConnection", "startConnect.run - START_RECEIVE: starting receive thread");
				startReceiveThread();
				
				/* Enter the blocking send loop */
				QLog.d("DeviceConnection", "startConnect.run - SEND_LOOP: entering send loop");
				sendLoop();
			}
		}).start();
	}
	
	private void sendLoop() {
		/* We become the send thread */
		try {
			for (;;) {
				/* Get the next command */
				byte[] command = commandQueue.take();
				
				/* This may be a close indication */
				if (shellStream.isClosed()) {
					listener.notifyStreamClosed(DeviceConnection.this);
					break;
				}
				
				/* Issue it to the device */
				shellStream.write(command);
			}
		} catch (IOException e) {
			listener.notifyStreamFailed(DeviceConnection.this, e);
		} catch (InterruptedException e) {
		} finally {
			AdbUtils.safeClose(DeviceConnection.this);
		}
	}
	
	private void startReceiveThread() {
		QLog.d("DeviceConnection", "startReceiveThread - START: creating receive thread");
		new Thread(new Runnable() {
			@Override
			public void run() {
				QLog.d("DeviceConnection", "startReceiveThread.run - THREAD_START: receive thread started");
				try {
					while (!shellStream.isClosed()) {
						QLog.d("DeviceConnection", "startReceiveThread.run - READING: waiting for data from shellStream");
						byte[] data = shellStream.read();
						QLog.d("DeviceConnection", "startReceiveThread.run - DATA_RECEIVED: " + data.length + " bytes received");
						listener.receivedData(DeviceConnection.this, data, 0, data.length);
					}
					QLog.d("DeviceConnection", "startReceiveThread.run - STREAM_CLOSED: shellStream is closed");
					listener.notifyStreamClosed(DeviceConnection.this);
				} catch (IOException e) {
					QLog.e("DeviceConnection", "startReceiveThread.run - IO_ERROR: IOException in receive thread", e);
					listener.notifyStreamFailed(DeviceConnection.this, e);
				} catch (InterruptedException e) {
					QLog.d("DeviceConnection", "startReceiveThread.run - INTERRUPTED: receive thread interrupted");
				} finally {
					QLog.d("DeviceConnection", "startReceiveThread.run - CLEANUP: cleaning up receive thread");
					AdbUtils.safeClose(DeviceConnection.this);
				}
			}
		}).start();
		QLog.d("DeviceConnection", "startReceiveThread - END: receive thread started");
	}
	
	public boolean isClosed() {
		return closed;
	}

	@Override
	public void close() throws IOException {
		if (isClosed()) {
			return;
		}
		else {
			closed = true;
		}
		
		/* Close the stream first */
		AdbUtils.safeClose(shellStream);
		
		/* Now the connection (and underlying socket) */
		AdbUtils.safeClose(connection);
		
		/* Finally signal the command queue to allow the send thread to terminate */
		commandQueue.add(new byte[0]);
	}

	public boolean isForeground() {
		return foreground;
	}

	public void setForeground(boolean foreground) {
		this.foreground = foreground;
	}
}
