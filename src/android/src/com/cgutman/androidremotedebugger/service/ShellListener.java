package com.cgutman.androidremotedebugger.service;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.concurrent.ConcurrentHashMap;

import android.app.Service;

import com.cgutman.adblib.AdbCrypto;
import com.cgutman.androidremotedebugger.AdbUtils;
import com.cgutman.androidremotedebugger.console.ConsoleBuffer;
import com.cgutman.androidremotedebugger.devconn.DeviceConnection;
import com.cgutman.androidremotedebugger.devconn.DeviceConnectionListener;

public class ShellListener implements DeviceConnectionListener {
	private static final int TERM_LENGTH = 25000;
	
	private final HashMap<DeviceConnection, LinkedList<DeviceConnectionListener>> listenerMap =
			new HashMap<DeviceConnection, LinkedList<DeviceConnectionListener>>();
	private final ConcurrentHashMap<DeviceConnection, ConsoleBuffer> consoleMap =
			new ConcurrentHashMap<DeviceConnection, ConsoleBuffer>();
	private Service service;
	
	public ShellListener(Service service) {
		this.service = service;
	}
	
	public void addListener(DeviceConnection conn, DeviceConnectionListener listener) {
		synchronized (listenerMap) {
			LinkedList<DeviceConnectionListener> listeners = listenerMap.get(conn);
			if (listeners != null) {
				listeners.add(listener);
			}
			else {
				listeners = new LinkedList<DeviceConnectionListener>();
				listeners.add(listener);
				listenerMap.put(conn, listeners);
			}
		}
		
		/* If the listener supports console input, we'll tell them about the console buffer
		 * by firing them an initial console updated callback */
		ConsoleBuffer console = consoleMap.get(conn);
		if (console != null && listener.isConsole()) {
			listener.consoleUpdated(conn, console);
		}
	}
	
	public void removeListener(DeviceConnection conn, DeviceConnectionListener listener) {
		synchronized (listenerMap) {
			LinkedList<DeviceConnectionListener> listeners = listenerMap.get(conn);
			if (listeners != null) {
				listeners.remove(listener);
			}
		}
	}
	
	@Override
	public void notifyConnectionEstablished(DeviceConnection devConn) {
		consoleMap.put(devConn, new ConsoleBuffer(TERM_LENGTH));

		synchronized (listenerMap) {
			LinkedList<DeviceConnectionListener> listeners = listenerMap.get(devConn);
			if (listeners != null) {
				for (DeviceConnectionListener listener : listeners) {
					listener.notifyConnectionEstablished(devConn);
				}
			}
		}
	}

	@Override
	public void notifyConnectionFailed(DeviceConnection devConn, Exception e) {
		synchronized (listenerMap) {
			LinkedList<DeviceConnectionListener> listeners = listenerMap.get(devConn);
			if (listeners != null) {
				for (DeviceConnectionListener listener : listeners) {
					listener.notifyConnectionFailed(devConn, e);
				}
			}
		}
	}

	@Override
	public void notifyStreamFailed(DeviceConnection devConn, Exception e) {
		/* Return if this connection has already "failed" */
		if (consoleMap.remove(devConn) == null) {
			return;
		}

		synchronized (listenerMap) {
			LinkedList<DeviceConnectionListener> listeners = listenerMap.get(devConn);
			if (listeners != null) {
				for (DeviceConnectionListener listener : listeners) {
					listener.notifyStreamFailed(devConn, e);
				}
			}
		}
	}

	@Override
	public void notifyStreamClosed(DeviceConnection devConn) {
		/* Return if this connection has already "failed" */
		if (consoleMap.remove(devConn) == null) {
			return;
		}

		synchronized (listenerMap) {
			LinkedList<DeviceConnectionListener> listeners = listenerMap.get(devConn);
			if (listeners != null) {
				for (DeviceConnectionListener listener : listeners) {
					listener.notifyStreamClosed(devConn);
				}
			}
		}
	}

	@Override
	public AdbCrypto loadAdbCrypto(DeviceConnection devConn) {
		return AdbUtils.readCryptoConfig(service.getFilesDir());
	}

	@Override
	public void receivedData(DeviceConnection devConn, byte[] data,
			int offset, int length) {
		/* Add data to the console for this connection */
		ConsoleBuffer console = consoleMap.get(devConn);
		if (console != null) {
			/* Hack to remove the bell from the end of the prompt */
			if (data[offset+length-1] == 0x07) {
				length--;
			}
			
			console.append(data, offset, length);
			
			/* Attempt to deliver a console update notification */
			synchronized (listenerMap) {
				LinkedList<DeviceConnectionListener> listeners = listenerMap.get(devConn);
				if (listeners != null) {
					for (DeviceConnectionListener listener : listeners) {
						if (listener.isConsole()) {
							listener.consoleUpdated(devConn, console);
						}
					}
				}
			}
		}
	}

	@Override
	public boolean canReceiveData() {
		/* We can always receive data */
		return true;
	}

	@Override
	public boolean isConsole() {
		return false;
	}

	@Override
	public void consoleUpdated(DeviceConnection devConsole,
			ConsoleBuffer console) {
	}
}
