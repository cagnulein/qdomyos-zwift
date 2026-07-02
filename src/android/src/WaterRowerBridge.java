package org.cagnulen.qdomyoszwift;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Build;
import androidx.core.content.ContextCompat;

import com.hoho.android.usbserial.driver.CdcAcmSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;

import java.util.HashMap;
import java.util.Iterator;
import java.util.concurrent.CountDownLatch;

import de.tbressler.waterrower.WaterRower;
import de.tbressler.waterrower.IWaterRowerConnectionListener;
import de.tbressler.waterrower.discovery.WaterRowerAutoDiscovery;
import de.tbressler.waterrower.io.transport.SerialChannel;
import de.tbressler.waterrower.io.transport.SerialDeviceAddress;
import de.tbressler.waterrower.model.ErrorCode;
import de.tbressler.waterrower.model.ModelInformation;
import de.tbressler.waterrower.model.StrokeType;
import de.tbressler.waterrower.subscriptions.values.*;

import java.io.IOException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

public class WaterRowerBridge {
    private static final String TAG = "WaterRowerBridge";
    private static WaterRower waterRower;
    private static boolean isConnected = false;
    private static double lastStrokeRate = 0;
    private static double lastDistance = 0;
    private static double lastPace = 0;
    private static double lastWatts = 0;
    private static double lastCalories = 0;
    private static long lastDataUpdate = 0;

    private static boolean isWaterRowerDevice(UsbDevice device) {
        if (device == null || device.getVendorId() != 1240) {
            return false;
        }

        int productId = device.getProductId();
        if (productId == 10 || productId == 223) {
            return true;
        }

        String productName = device.getProductName();
        return productName != null && productName.toUpperCase().contains("WR-S");
    }

    private static String describeDevice(UsbDevice device) {
        if (device == null) {
            return "null";
        }

        return device.getDeviceName() + " vendor=" + device.getVendorId() +
                " product=" + device.getProductId() +
                " name=" + device.getProductName();
    }
    
    private static final IWaterRowerConnectionListener connectionListener = new IWaterRowerConnectionListener() {
        @Override
        public void onConnected(ModelInformation modelInformation) {
            QLog.d(TAG, "WaterRower connected: " + modelInformation.getMonitorType());
            isConnected = true;
            
            // Subscribe to rowing metrics
            // Subscribe to stroke events
            waterRower.subscribe(new StrokeSubscription() {
                @Override
                protected void onStroke(StrokeType strokeType) {
                    lastDataUpdate = System.currentTimeMillis();
                    QLog.d(TAG, "Stroke: " + strokeType);
                }
            });

            // Subscribe to stroke rate
            waterRower.subscribe(new AverageStrokeRateSubscription() {
                @Override
                protected void onStrokeRateUpdated(double strokeRate) {
                    lastStrokeRate = strokeRate;
                    lastDataUpdate = System.currentTimeMillis();
                    QLog.d(TAG, "Stroke rate: " + strokeRate);
                }
            });

            // Subscribe to distance
            waterRower.subscribe(new DistanceSubscription() {
                @Override
                protected void onDistanceUpdated(double distance) {
                    lastDistance = distance;
                    lastDataUpdate = System.currentTimeMillis();
                    QLog.d(TAG, "Distance: " + distance);
                }
            });
            
            // Subscribe to total velocity (pace)
            waterRower.subscribe(new TotalVelocitySubscription() {
                @Override
                protected void onVelocityUpdated(double velocity) {
                    if (velocity > 0) {
                        lastPace = 500.0 / velocity; // Convert to seconds per 500m
                    }
                    lastDataUpdate = System.currentTimeMillis();
                    QLog.d(TAG, "Velocity: " + velocity + ", Pace: " + lastPace);
                }
            });
            
            // Subscribe to watts
            waterRower.subscribe(new WattsSubscription() {
                @Override
                protected void onWattsUpdated(int watts) {
                    lastWatts = watts;
                    lastDataUpdate = System.currentTimeMillis();
                    QLog.d(TAG, "Watts: " + watts);
                }
            });
            
            // Subscribe to calories
            waterRower.subscribe(new TotalCaloriesSubscription() {
                @Override
                protected void onCaloriesUpdated(int calories) {
                    lastCalories = calories;
                    lastDataUpdate = System.currentTimeMillis();
                    QLog.d(TAG, "Calories: " + calories);
                }
            });
        }
        
        @Override
        public void onDisconnected() {
            QLog.d(TAG, "WaterRower disconnected");
            isConnected = false;
        }
        
        @Override
        public void onError(ErrorCode errorCode) {
            QLog.e(TAG, "WaterRower error: " + errorCode);
            isConnected = false;
        }
    };
    
    public static String getDevicePath(Context context) {
        QLog.d(TAG, "getDevicePath: searching for WaterRower device");
        UsbManager manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        HashMap<String, UsbDevice> deviceList = manager.getDeviceList();
        QLog.d(TAG, "getDevicePath: UsbManager device count " + deviceList.size());
        Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
        while(deviceIterator.hasNext()){
            UsbDevice device = deviceIterator.next();
            QLog.d(TAG, "getDevicePath: found device " + describeDevice(device) +
                    " permission=" + manager.hasPermission(device));
            if (isWaterRowerDevice(device)) {
                QLog.d(TAG, "getDevicePath: found WaterRower device from UsbManager at " + device.getDeviceName());
                return device.getDeviceName();
            }
        }

        if (context instanceof Activity) {
            Intent intent = ((Activity) context).getIntent();
            if (intent != null && UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(intent.getAction())) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                QLog.d(TAG, "getDevicePath: USB attach intent device " + describeDevice(device) +
                        " permission=" + (device != null && manager.hasPermission(device)));
                if (isWaterRowerDevice(device)) {
                    QLog.d(TAG, "getDevicePath: found WaterRower device from USB attach intent at " + device.getDeviceName());
                    return device.getDeviceName();
                }
            } else {
                QLog.d(TAG, "getDevicePath: no USB attach intent available");
            }
        }

        QLog.d(TAG, "getDevicePath: WaterRower device not found");
        return "";
    }

    /* Finds the attached WaterRower UsbDevice, or null if none is currently attached. */
    private static UsbDevice findWaterRowerDevice(Context context, UsbManager manager) {
        HashMap<String, UsbDevice> deviceList = manager.getDeviceList();
        Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
        while (deviceIterator.hasNext()) {
            UsbDevice device = deviceIterator.next();
            if (isWaterRowerDevice(device)) {
                return device;
            }
        }

        if (context instanceof Activity) {
            Intent intent = ((Activity) context).getIntent();
            if (intent != null && UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(intent.getAction())) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (isWaterRowerDevice(device)) {
                    return device;
                }
            }
        }

        return null;
    }

    /* Requests runtime USB permission for the device and blocks (up to 5s) until the user
       responds or the permission is already granted (e.g. via the USB_DEVICE_ATTACHED
       intent-filter chooser). Returns true if permission is granted. */
    private static boolean requestUsbPermissionIfNeeded(Context context, UsbManager manager, UsbDevice device) {
        if (manager.hasPermission(device)) {
            return true;
        }

        QLog.d(TAG, "requestUsbPermissionIfNeeded: requesting USB permission for " + describeDevice(device));

        final CountDownLatch latch = new CountDownLatch(1);
        final boolean[] granted = {false};
        String action = "org.cagnulen.qdomyoszwift.WATERROWER_USB_PERMISSION";
        BroadcastReceiver usbReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context ctx, Intent intent) {
                granted[0] = intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false);
                latch.countDown();
            }
        };

        int flags = Build.VERSION.SDK_INT >= Build.VERSION_CODES.M ? PendingIntent.FLAG_IMMUTABLE : 0;
        PendingIntent permissionIntent = PendingIntent.getBroadcast(context, 0, new Intent(action), flags);
        IntentFilter filter = new IntentFilter(action);
        ContextCompat.registerReceiver(context, usbReceiver, filter, ContextCompat.RECEIVER_EXPORTED);

        try {
            manager.requestPermission(device, permissionIntent);
            latch.await(5, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        } finally {
            context.unregisterReceiver(usbReceiver);
        }

        QLog.d(TAG, "requestUsbPermissionIfNeeded: permission granted=" + granted[0]);
        return granted[0];
    }

    public static String connect(Context context, String devicePath) {
        QLog.d(TAG, "Connecting to WaterRower at " + devicePath);

        if (waterRower != null) {
            shutdown();
        }

        try {
            UsbManager manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
            UsbDevice device = findWaterRowerDevice(context, manager);
            if (device == null) {
                QLog.e(TAG, "connect: WaterRower USB device not found");
                return "CONNECTION_FAILED: WaterRower USB device not found";
            }

            if (!requestUsbPermissionIfNeeded(context, manager, device)) {
                QLog.e(TAG, "connect: USB permission denied for " + describeDevice(device));
                return "CONNECTION_FAILED: USB permission denied";
            }

            UsbDeviceConnection connection = manager.openDevice(device);
            if (connection == null) {
                QLog.e(TAG, "connect: could not open USB device connection");
                return "CONNECTION_FAILED: Could not open USB device connection";
            }

            // The WaterRower S4/S5 USB monitor exposes a standard USB CDC-ACM interface, but is
            // not in usb-serial-for-android's default probe table (it's identified by generic
            // Microchip CDC-ACM VID/PID), so the driver is created explicitly here rather than
            // via UsbSerialProber.
            UsbSerialPort port = new CdcAcmSerialDriver(device).getPorts().get(0);
            port.open(connection);
            port.setParameters(19200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

            // Hand the already-open port to the SerialChannel that WaterRower.connect() below
            // will instantiate; jSerialComm can not open raw Android USB device paths directly.
            SerialChannel.setUsbSerialPort(port);

            waterRower = new WaterRower();
            waterRower.addConnectionListener(connectionListener);

            SerialDeviceAddress address = new SerialDeviceAddress(devicePath);
            waterRower.connect(address);

            QLog.d(TAG, "Connection attempt returned SUCCESS");
            return "SUCCESS";

        } catch (Exception e) {
            SerialChannel.setUsbSerialPort(null);
            QLog.e(TAG, "Failed to connect to WaterRower", e);
            return "CONNECTION_FAILED: " + e.getMessage();
        }
    }
    
    public static void shutdown() {
        QLog.d(TAG, "Shutting down WaterRower connection");
        
        if (waterRower != null) {
            try {
                waterRower.disconnect();
            } catch (IOException e) {
                QLog.e(TAG, "Error disconnecting WaterRower", e);
            }
            waterRower = null;
        }
        
        isConnected = false;
        lastStrokeRate = 0;
        lastDistance = 0;
        lastPace = 0;
        lastWatts = 0;
        lastCalories = 0;
        lastDataUpdate = 0;
    }
    
    public static boolean isConnected() {
        return isConnected;
    }
    
    public static String getStrokeData() {
        if (!isConnected || waterRower == null) {
            return "NO_DATA";
        }
        
        // Check if data is recent (within last 5 seconds)
        long currentTime = System.currentTimeMillis();
        if (currentTime - lastDataUpdate > 5000) {
            return "NO_DATA";
        }
        
        // Return data in CSV format: strokeRate,distance,pace,watts,calories
        return String.format("%.1f,%.1f,%.2f,%.1f,%.1f", 
                            lastStrokeRate, lastDistance, lastPace, lastWatts, lastCalories);
    }
    
    public static double getStrokeRate() {
        return lastStrokeRate;
    }
    
    public static double getDistance() {
        return lastDistance;
    }
    
    public static double getPace() {
        return lastPace;
    }
    
    public static double getWatts() {
        return lastWatts;
    }
    
    public static double getCalories() {
        return lastCalories;
    }
}
