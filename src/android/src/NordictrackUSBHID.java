package org.cagnulen.qdomyoszwift;

import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Deque;
import java.util.EnumSet;
import java.util.concurrent.Callable;
import java.util.ArrayList;
import java.util.List;
import java.nio.charset.StandardCharsets;

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
import android.util.Log;

public class NordictrackUSBHID {

	static HidBridge hidBridge;
	static byte[] receiveData;
	static int lastReadLen = 0;

	public static void open(Context context) {
		 Log.d("QZ","NordictrackUSBHID open");
		           hidBridge = new HidBridge(context, 0x0002, 0x213c);
		 boolean ret = hidBridge.OpenDevice();
		 Log.d("QZ","hidBridge.OpenDevice " + ret);
		 hidBridge.StartReadingThread();
		 Log.d("QZ","hidBridge.StartReadingThread");
	}

   public static void write (byte[] bytes) {
		          Log.d("QZ","NordictrackUSBHID writing " + new String(bytes, StandardCharsets.ISO_8859_1));
		hidBridge.WriteData(bytes);
		}

	public static int readLen() {
		return lastReadLen;
	}

   public static byte[] read() {
		      if(hidBridge.IsThereAnyReceivedData()) {
		receiveData = hidBridge.GetReceivedDataFromQueue();
		lastReadLen = receiveData.length;
		          Log.d("QZ","NordictrackUSBHID reading " + lastReadLen + new String(receiveData, StandardCharsets.ISO_8859_1));
		return receiveData;
		      } else {
				    Log.d("QZ","NordictrackUSBHID empty data");
					 lastReadLen = 0;
					 return null;
					}
	}
}
