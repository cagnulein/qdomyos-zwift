package org.cagnulen.qdomyoszwift;
 
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;
 
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import org.cagnulen.qdomyoszwift.QLog as Log;
import android.os.Build;
import androidx.core.content.ContextCompat;
 
/**
 * This class is used for talking to hid of the dongle, connecting, disconnencting and enumerating the devices.
 * @author gai
 */
public class HidBridge {
	private Context _context;
	private int _productId;
	private int _vendorId;
	
	// Can be used for debugging.
	@SuppressWarnings("unused")
	private static final String ACTION_USB_PERMISSION =
	       "org.cagnulen.qdomyoszwift.app.testhid.USB_PERMISSION";
	
	// Locker object that is responsible for locking read/write thread.
	private Object _locker = new Object();
	private Thread _readingThread = null;
	private String _deviceName;
	
	private UsbManager _usbManager;
	private UsbDevice _usbDevice;
	
	// The queue that contains the read data.
	private Queue<byte[]> _receivedQueue;
	
	/**
	 * Creates a hid bridge to the dongle. Should be created once.
	 * @param context is the UI context of Android.
	 * @param productId of the device.
	 * @param vendorId of the device.
	 */
	public HidBridge(Context context, int productId, int vendorId) {
		_context = context;
		_productId = productId;
		_vendorId = vendorId;
		_receivedQueue = new LinkedList<byte[]>();
	}
	
	/**
	 * Searches for the device and opens it if successful
	 * @return true, if connection was successful
	 */
	public boolean OpenDevice() {
		_usbManager = (UsbManager) _context.getSystemService(Context.USB_SERVICE);
		
		HashMap<String, UsbDevice> deviceList = _usbManager.getDeviceList();
 
		Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
		_usbDevice = null;
		
		// Iterate all the available devices and find ours.
		while(deviceIterator.hasNext()){
		    UsbDevice device = deviceIterator.next();
		    if (device.getProductId() == _productId && device.getVendorId() == _vendorId) {
		    	_usbDevice = device;
		    	_deviceName = _usbDevice.getDeviceName();
		    }
		}
		
		if (_usbDevice == null) {
			Log("Cannot find the device. Did you forgot to plug it?");
			Log(String.format("\t I search for VendorId: %s and ProductId: %s", _vendorId, _productId));
			return false;
		}
		
		// Create and intent and request a permission.
                int flags = Build.VERSION.SDK_INT >= Build.VERSION_CODES.M ? PendingIntent.FLAG_IMMUTABLE : 0;
                PendingIntent mPermissionIntent = PendingIntent.getBroadcast(_context, 0, new Intent(ACTION_USB_PERMISSION), flags);
		IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
				ContextCompat.registerReceiver(
                    _context,
                    mUsbReceiver,
                    filter,
                    ContextCompat.RECEIVER_EXPORTED
                );
 
		_usbManager.requestPermission(_usbDevice, mPermissionIntent);
		Log("Found the device");
		return true;
	}
	
	/**
	 * Closes the reading thread of the device.
	 */
	public void CloseTheDevice() {
		StopReadingThread();
	}
	
	/**
	 * Starts the thread that continuously reads the data from the device. 
	 * Should be called in order to be able to talk with the device.
	 */
	public void StartReadingThread() {
		if (_readingThread == null) {
			_readingThread = new Thread(readerReceiver);
			_readingThread.start();
		} else {
			Log("Reading thread already started");
		}
	}
	
	/**
	 * Stops the thread that continuously reads the data from the device.
	 * If it is stopped - talking to the device would be impossible.
	 */
	@SuppressWarnings("deprecation")
	public void StopReadingThread() {
		if (_readingThread != null) {
			// Just kill the thread. It is better to do that fast if we need that asap.
			_readingThread.stop();
			_readingThread = null;
		} else {
			Log("No reading thread to stop");
		}
	}
	
	/**
	 * Write data to the usb hid. Data is written as-is, so calling method is responsible for adding header data.
	 * @param bytes is the data to be written.
	 * @return true if succeed.
	 */
	public boolean WriteData(byte[] bytes) {
		try
		{
			// Lock that is common for read/write methods.
			synchronized (_locker) {
				UsbInterface writeIntf = _usbDevice.getInterface(0);
				UsbEndpoint writeEp = writeIntf.getEndpoint(1);
				UsbDeviceConnection writeConnection = _usbManager.openDevice(_usbDevice); 
				
				// Lock the usb interface.
				writeConnection.claimInterface(writeIntf, true);
				
				// Write the data as a bulk transfer with defined data length.
				int r = writeConnection.bulkTransfer(writeEp, bytes, bytes.length, 0);
				if (r != -1) {	
					Log(String.format("Written %s bytes to the dongle. Data written: %s", r, composeString(bytes)));
				} else {
					Log("Error happened while writing data. No ACK");
				}
 
				// Release the usb interface.
				writeConnection.releaseInterface(writeIntf);
				writeConnection.close();
			}
			
		} catch(NullPointerException e)
		{
			Log("Error happened while writing. Could not connect to the device or interface is busy?");
			Log.e("HidBridge", Log.getStackTraceString(e));
			return false;
		}
		return true;
	}
	
	/**
	 * @return true if there are any data in the queue to be read.
	 */
	public boolean IsThereAnyReceivedData() {
		synchronized(_locker) {
			return !_receivedQueue.isEmpty();
		}
	}
	
	/**
	 * Queue the data from the read queue.
	 * @return queued data.
	 */
	public byte[] GetReceivedDataFromQueue() {
		synchronized(_locker) {
			return _receivedQueue.poll();
		}
	}
	
	// The thread that continuously receives data from the dongle and put it to the queue.
	private Runnable readerReceiver = new Runnable() {
	    public void run() {
	    	if (_usbDevice == null) {
	    		Log("No device to read from");
	    		return;
	    	}
 
	    	UsbEndpoint readEp;
	    	UsbDeviceConnection readConnection = null;
	    	UsbInterface readIntf = null;
	    	boolean readerStartedMsgWasShown = false;
	    	
	    	// We will continuously ask for the data from the device and store it in the queue.
			while (true) {
				// Lock that is common for read/write methods.
				synchronized (_locker) {
					try
					{
						if (_usbDevice == null) {
							OpenDevice();
							Log("No device. Recheking in 10 sec...");
							
							Sleep(10000);
							continue;
						}
						
				    	readIntf = _usbDevice.getInterface(0);
						readEp = readIntf.getEndpoint(0);
						if (!_usbManager.getDeviceList().containsKey(_deviceName)) {
							Log("Failed to connect to the device. Retrying to acquire it.");
							OpenDevice();
							if (!_usbManager.getDeviceList().containsKey(_deviceName)) {
								Log("No device. Recheking in 10 sec...");
								
								Sleep(10000);
								continue;
							}
						}
						
						try
						{
							
							readConnection = _usbManager.openDevice(_usbDevice); 
							
							if (readConnection == null) {
								Log("Cannot start reader because the user didn't gave me permissions or the device is not present. Retrying in 2 sec...");
								Sleep(2000);
								continue;
							}
							
							// Claim and lock the interface in the android system.
							readConnection.claimInterface(readIntf, true);
						}
						catch (SecurityException e) {
							Log("Cannot start reader because the user didn't gave me permissions. Retrying in 2 sec...");
							
							Sleep(2000);
							continue;
						}
						
						// Show the reader started message once.
						if (!readerStartedMsgWasShown) {
							Log("!!! Reader was started !!!");
							readerStartedMsgWasShown = true;
						}
						
						// Read the data as a bulk transfer with the size = MaxPacketSize
						int packetSize = readEp.getMaxPacketSize();
						byte[] bytes = new byte[packetSize];
						int r = readConnection.bulkTransfer(readEp, bytes, packetSize, 50);
						if (r >= 0) {
							byte[] trancatedBytes = new byte[r]; // Truncate bytes in the honor of r
							
							int i=0; 
							for (byte b : bytes) {
								if(i < r) {
									trancatedBytes[i] = b;
								} else {
								   Log(String.format("Buffer overflow %s %s %s", r, i, packetSize));
								}
								i++;
							}
							
							_receivedQueue.add(trancatedBytes); // Store received data
							Log(String.format("Message received of lengths %s and content: %s", r, composeString(bytes)));
						}
						
						// Release the interface lock.
						readConnection.releaseInterface(readIntf);
						readConnection.close();
						} 
					
					catch (NullPointerException e) {
						Log("Error happened while reading. No device or the connection is busy");
						Log.e("HidBridge", Log.getStackTraceString(e));
					}
					catch (ThreadDeath e) {
						if (readConnection != null) {
							readConnection.releaseInterface(readIntf);
							readConnection.close();
						}
						
						throw e;
					}
				}
				
				// Sleep for 10 ms to pause, so other thread can write data or anything. 
				// As both read and write data methods lock each other - they cannot be run in parallel.
				// Looks like Android is not so smart in planning the threads, so we need to give it a small time
				// to switch the thread context.
				Sleep(10);
			}
	    }
	};
	
	private void Sleep(int milliseconds) {
		try {
			Thread.sleep(milliseconds);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	
	private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
 
	    public void onReceive(Context context, Intent intent) {
	        String action = intent.getAction();
	        if (ACTION_USB_PERMISSION.equals(action)) {
	            synchronized (this) {
	                UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
 
	                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
	                    if(device != null){
	                      //call method to set up device communication
	                   }
	                } 
	                else {
	                    Log.d("TAG", "permission denied for the device " + device);
	                }
	            }
	        }
	    }
	};
	
	/**
	 * Logs the message from HidBridge.
	 * @param message to log.
	 */
	private void Log(String message) {
		Log.d("HidBridge", message);
	}
	
	/**
	 * Composes a string from byte array.
	 */
	private String composeString(byte[] bytes) {
		StringBuilder builder = new StringBuilder();
		for (byte b: bytes) {
			builder.append(b);
			builder.append(" ");
		}
		
		return builder.toString();
	}
}
