package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import java.util.HashMap;

public class UsbserialDebug {

    public static void listAllUsbDevices(Context context) {
        QLog.d("QZ", "=== USB DEVICE DEBUG START ===");
        UsbManager manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        HashMap<String, UsbDevice> deviceList = manager.getDeviceList();

        QLog.d("QZ", "Total USB devices found: " + deviceList.size());

        for (UsbDevice device : deviceList.values()) {
            QLog.d("QZ", "----------------------------");
            QLog.d("QZ", "Device Name: " + device.getDeviceName());
            QLog.d("QZ", "Vendor ID: " + device.getVendorId() + " (0x" + Integer.toHexString(device.getVendorId()) + ")");
            QLog.d("QZ", "Product ID: " + device.getProductId() + " (0x" + Integer.toHexString(device.getProductId()) + ")");
            QLog.d("QZ", "Device Class: " + device.getDeviceClass());
            QLog.d("QZ", "Device Subclass: " + device.getDeviceSubclass());
            QLog.d("QZ", "Device Protocol: " + device.getDeviceProtocol());
            QLog.d("QZ", "Has Permission: " + manager.hasPermission(device));
            QLog.d("QZ", "Interface Count: " + device.getInterfaceCount());
        }

        QLog.d("QZ", "=== USB DEVICE DEBUG END ===");
    }
}
