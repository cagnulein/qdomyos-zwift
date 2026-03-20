package org.cagnulen.qdomyoszwift;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import org.cagnulen.qdomyoszwift.QLog;
import android.app.Service;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Build;
import androidx.core.content.ContextCompat;

import com.hoho.android.usbserial.driver.CdcAcmSerialDriver;
import com.hoho.android.usbserial.driver.Ch34xSerialDriver;
import com.hoho.android.usbserial.driver.CommonUsbSerialPort;
import com.hoho.android.usbserial.driver.Cp21xxSerialDriver;
import com.hoho.android.usbserial.driver.FtdiSerialDriver;
import com.hoho.android.usbserial.driver.ProlificSerialDriver;
import com.hoho.android.usbserial.driver.UsbId;
import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Deque;
import java.util.EnumSet;
import java.util.LinkedList;
import java.util.concurrent.Callable;
import java.util.ArrayList;
import java.util.List;
import java.nio.charset.StandardCharsets;

public class Usbserial {

    static UsbSerialPort port = null;
    static byte[] receiveData = new byte[4096];
    static int lastReadLen = 0;

    public static void open(Context context) {
        open(context, 2400); // Default baud rate for Computrainer
    }

    public static void open(Context context, int baudRate) {
        QLog.d("QZ","UsbSerial open with baud rate: " + baudRate);
        // Find all available drivers from attached devices.
        UsbManager manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        List<UsbSerialDriver> availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(manager);
        if (availableDrivers.isEmpty()) {
            QLog.d("QZ","UsbSerial no available drivers");
            return;
        }

        // Open a connection to the first available driver.
        UsbSerialDriver driver = availableDrivers.get(0);
        if (!manager.hasPermission(driver.getDevice())) {
            Uri notification = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
            RingtoneManager.getRingtone(context, notification).play();

            QLog.d("QZ","USB permission ...");
            final Boolean[] granted = {null};
            BroadcastReceiver usbReceiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    granted[0] = intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false);
                }
            };
            int flags = Build.VERSION.SDK_INT >= Build.VERSION_CODES.M ? PendingIntent.FLAG_IMMUTABLE : 0;
            PendingIntent permissionIntent = PendingIntent.getBroadcast(context, 0, new Intent("org.cagnulen.qdomyoszwift.USB_PERMISSION"), flags);
            IntentFilter filter = new IntentFilter("org.cagnulen.qdomyoszwift.USB_PERMISSION");
            ContextCompat.registerReceiver(
                    context,
                    usbReceiver,
                    filter,
                    ContextCompat.RECEIVER_EXPORTED
                );
            manager.requestPermission(driver.getDevice(), permissionIntent);
            for(int i=0; i<5000; i++) {
                if(granted[0] != null) break;
                try {
                    Thread.sleep(1);
                }
                catch (InterruptedException e) {
                    // Do something here
                }
            }
            QLog.d("QZ","USB permission "+granted[0]);
        }

        UsbDeviceConnection connection = manager.openDevice(driver.getDevice());
        if (connection == null) {
            QLog.d("QZ","UsbSerial no permissions");
            // add UsbManager.requestPermission(driver.getDevice(), ..) handling here
            return;
        }

        port = driver.getPorts().get(0); // Most devices have just one port (port 0)
        try {
            port.open(connection);
            port.setParameters(baudRate, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);
            QLog.d("QZ","UsbSerial port opened successfully at " + baudRate + " baud");
        }
        catch (IOException e) {
            QLog.d("QZ","UsbSerial port open failed: " + e.getMessage());
        }
    }

    public static void write (byte[] bytes) {
        if(port == null)
           return;

        QLog.d("QZ","UsbSerial writing " + new String(bytes, StandardCharsets.UTF_8));
        try {
            port.write(bytes, 2000);
        }
        catch (IOException e) {
            // Do something here
        }
    }

    public static int readLen() {
        return lastReadLen;
    }

    public static byte[] read() {
        if(port == null) {
           lastReadLen = 0;
           return receiveData;
        }

        try {
            lastReadLen = port.read(receiveData, 2000);
            QLog.d("QZ","UsbSerial reading " + lastReadLen + new String(receiveData, StandardCharsets.UTF_8));
        }
        catch (IOException e) {
            // Do something here
        }
        return receiveData;
    }
}
