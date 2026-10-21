package org.cagnulen.qdomyoszwift;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.system.ErrnoException;
import android.system.Os;
import android.system.OsConstants;
import android.system.StructPollfd;
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

public class Usbserial {

    private static final char[] HEX_DIGITS = "0123456789ABCDEF".toCharArray();

    static UsbSerialPort port = null;
    static java.io.FileDescriptor localSerialFd = null;
    static boolean localSerialMode = false;
    static byte[] receiveData = new byte[4096];
    static int lastReadLen = 0;
    static final String[] localSerialCandidates = {
        "/dev/ttyS4",
        "/dev/ttyS0",
        "/dev/ttyS1",
        "/dev/ttyS2",
        "/dev/ttyS3"
    };

    private static String bytesToHex(byte[] data, int length) {
        int count = Math.min(Math.max(length, 0), data.length);
        StringBuilder hex = new StringBuilder(count == 0 ? 0 : count * 3 - 1);
        for (int i = 0; i < count; i++) {
            if (i > 0)
                hex.append(' ');
            int value = data[i] & 0xFF;
            hex.append(HEX_DIGITS[value >>> 4]);
            hex.append(HEX_DIGITS[value & 0x0F]);
        }
        return hex.toString();
    }

    public static void open(Context context) {
        open(context, 2400); // Default baud rate for Computrainer
    }

    public static boolean open(Context context, int baudRate) {
        return open(context, baudRate, "", false);
    }

    public static boolean openKettler(Context context, int baudRate) {
        return open(context, baudRate, "", true);
    }

    public static boolean open(Context context, int baudRate, String devicePath) {
        return open(context, baudRate, devicePath, false);
    }

    private static boolean open(Context context, int baudRate, String devicePath, boolean configureControlLines) {
        close();

        if (devicePath != null && (devicePath.startsWith("/dev/") || devicePath.equalsIgnoreCase("auto"))) {
            return openLocalSerial(devicePath, baudRate);
        }

        QLog.d("QZ","UsbSerial open with baud rate: " + baudRate);
        localSerialMode = false;
        localSerialFd = null;
        // Find all available drivers from attached devices.
        UsbManager manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        List<UsbSerialDriver> availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(manager);
        if (availableDrivers.isEmpty()) {
            QLog.d("QZ","UsbSerial no available drivers");
            return false;
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
            return false;
        }

        port = driver.getPorts().get(0); // Most devices have just one port (port 0)
        try {
            port.open(connection);
            port.setParameters(baudRate, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);
            if (configureControlLines) {
                port.setDTR(false);
                port.setRTS(false);
            }
            QLog.d("QZ","UsbSerial port opened successfully at " + baudRate + " baud");
            return true;
        }
        catch (Exception e) {
            QLog.d("QZ","UsbSerial port open failed: " + e.getMessage());
            try {
                port.close();
            }
            catch (Exception closeException) {
                QLog.d("QZ", "UsbSerial port cleanup failed: " + closeException.getMessage());
            }
            port = null;
            return false;
        }
    }

    private static boolean openLocalSerial(String devicePath, int baudRate) {
        QLog.d("QZ","UsbSerial local serial open " + devicePath + " with baud rate: " + baudRate);
        port = null;
        localSerialFd = null;
        localSerialMode = false;
        lastReadLen = 0;

        if (devicePath.equalsIgnoreCase("auto")) {
            for (String candidate : localSerialCandidates) {
                if (openLocalSerialPath(candidate, baudRate)) {
                    return true;
                }
            }
            QLog.d("QZ","UsbSerial local serial auto open failed");
            return false;
        }

        return openLocalSerialPath(devicePath, baudRate);
    }

    private static boolean openLocalSerialPath(String devicePath, int baudRate) {
        try {
            configureLocalSerial(devicePath, baudRate);
            localSerialFd = Os.open(devicePath, OsConstants.O_RDWR | OsConstants.O_NOCTTY | OsConstants.O_NONBLOCK, 0);
            localSerialMode = true;
            QLog.d("QZ","UsbSerial local serial opened successfully: " + devicePath);
            return true;
        }
        catch (ErrnoException e) {
            QLog.d("QZ","UsbSerial local serial open failed: " + e.getMessage());
            return false;
        }
    }

    private static void configureLocalSerial(String devicePath, int baudRate) {
        try {
            String[] command = {
                "stty",
                "-F",
                devicePath,
                String.valueOf(baudRate),
                "cs8",
                "-cstopb",
                "-parenb",
                "raw",
                "-echo"
            };
            Process process = Runtime.getRuntime().exec(command);
            int rc = process.waitFor();
            QLog.d("QZ","UsbSerial local serial stty exit code: " + rc);
        }
        catch (Exception e) {
            QLog.d("QZ","UsbSerial local serial stty failed: " + e.getMessage());
        }
    }

    private static int writeInternal(byte[] bytes) {
        if (bytes == null)
            return -1;

        if(localSerialMode) {
            if(localSerialFd == null)
               return -1;

            try {
                int written = 0;
                while (written < bytes.length) {
                    int chunk = Os.write(localSerialFd, bytes, written, bytes.length - written);
                    if (chunk <= 0)
                        return -1;
                    written += chunk;
                }
                return written;
            }
            catch (ErrnoException | IOException e) {
                QLog.d("QZ","UsbSerial local serial write failed: " + e.getMessage());
                return -1;
            }
        }

        if(port == null)
           return -1;

        QLog.d("QZ", "UsbSerial writing " + bytes.length + " bytes: " +
                bytesToHex(bytes, bytes.length));
        try {
            port.write(bytes, 2000);
            return bytes.length;
        }
        catch (IOException e) {
            QLog.d("QZ", "UsbSerial write failed: " + e.getMessage());
            return -1;
        }
    }

    public static void write (byte[] bytes) {
        writeInternal(bytes);
    }

    public static int writeKettler(byte[] bytes) {
        return writeInternal(bytes);
    }

    public static boolean close() {
        boolean success = true;

        if (localSerialFd != null) {
            try {
                Os.close(localSerialFd);
            }
            catch (ErrnoException e) {
                QLog.d("QZ", "UsbSerial local serial close failed: " + e.getMessage());
                success = false;
            }
        }

        localSerialFd = null;
        localSerialMode = false;

        if (port != null) {
            try {
                port.close();
            }
            catch (IOException e) {
                QLog.d("QZ", "UsbSerial port close failed: " + e.getMessage());
                success = false;
            }
            port = null;
        }

        lastReadLen = 0;
        return success;
    }

    public static int readLen() {
        return lastReadLen;
    }

    public static byte[] read() {
        if(localSerialMode) {
            if(localSerialFd == null) {
               lastReadLen = 0;
               return receiveData;
            }

            try {
                StructPollfd pollFd = new StructPollfd();
                pollFd.fd = localSerialFd;
                pollFd.events = (short)OsConstants.POLLIN;
                int ready = Os.poll(new StructPollfd[]{pollFd}, 0);
                if(ready <= 0) {
                    lastReadLen = 0;
                    return receiveData;
                }

                lastReadLen = Os.read(localSerialFd, receiveData, 0, receiveData.length);
                QLog.d("QZ","UsbSerial local serial reading " + lastReadLen);
            }
            catch (ErrnoException | IOException e) {
                lastReadLen = 0;
                QLog.d("QZ","UsbSerial local serial read failed: " + e.getMessage());
            }
            return receiveData;
        }

        if(port == null) {
           lastReadLen = 0;
           return receiveData;
        }

        try {
            lastReadLen = port.read(receiveData, 2000);
            QLog.d("QZ", "UsbSerial reading " + lastReadLen + " bytes: " +
                    bytesToHex(receiveData, lastReadLen));
        }
        catch (IOException e) {
            lastReadLen = 0;
            QLog.d("QZ", "UsbSerial read failed: " + e.getMessage());
        }
        return receiveData;
    }
}
