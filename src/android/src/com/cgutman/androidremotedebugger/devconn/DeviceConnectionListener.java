package com.cgutman.androidremotedebugger.devconn;

import com.cgutman.adblib.AdbCrypto;
import com.cgutman.androidremotedebugger.console.ConsoleBuffer;

public interface DeviceConnectionListener {
	public void notifyConnectionEstablished(DeviceConnection devConn);
	
	public void notifyConnectionFailed(DeviceConnection devConn, Exception e);
	
	public void notifyStreamFailed(DeviceConnection devConn, Exception e);
	
	public void notifyStreamClosed(DeviceConnection devConn);
	
	public AdbCrypto loadAdbCrypto(DeviceConnection devConn);
	
	public boolean canReceiveData();
	
	public void receivedData(DeviceConnection devConn, byte[] data, int offset, int length);
	
	public boolean isConsole();
	
	public void consoleUpdated(DeviceConnection devConn, ConsoleBuffer console);
}
