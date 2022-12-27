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
		new Thread(new Runnable() {
			@Override
			public void run() {
				boolean connected = false;
				Socket socket = new Socket();
				AdbCrypto crypto;
				
				/* Load the crypto config */
				crypto = listener.loadAdbCrypto(DeviceConnection.this);
				if (crypto == null) {
					return;
				}
				
				try {
					/* Establish a connect to the remote host */
					socket.connect(new InetSocketAddress(host, port), CONN_TIMEOUT);
				} catch (IOException e) {
					listener.notifyConnectionFailed(DeviceConnection.this, e);
					return;
				}

				try {
					/* Establish the application layer connection */
					connection = AdbConnection.create(socket, crypto);
					connection.connect();
					
					/* Open the shell stream */
					shellStream = connection.open("shell:");
					connected = true;
				} catch (IOException e) {
					listener.notifyConnectionFailed(DeviceConnection.this, e);
				} catch (InterruptedException e) {
					listener.notifyConnectionFailed(DeviceConnection.this, e);
				} finally {
					/* Cleanup if the connection failed */
					if (!connected) {
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
				listener.notifyConnectionEstablished(DeviceConnection.this);
				
				/* Start the receive thread */
				startReceiveThread();
				
				/* Enter the blocking send loop */
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
		new Thread(new Runnable() {
			@Override
			public void run() {
				try {
					while (!shellStream.isClosed()) {
						byte[] data = shellStream.read();
						listener.receivedData(DeviceConnection.this, data, 0, data.length);
					}
					listener.notifyStreamClosed(DeviceConnection.this);
				} catch (IOException e) {
					listener.notifyStreamFailed(DeviceConnection.this, e);
				} catch (InterruptedException e) {
				} finally {
					AdbUtils.safeClose(DeviceConnection.this);
				}
			}
		}).start();
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
