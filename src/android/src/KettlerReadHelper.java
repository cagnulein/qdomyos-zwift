package org.cagnulen.qdomyoszwift;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.List;
import java.util.UUID;

public class KettlerReadHelper {
    private static final String TAG = "KettlerReadHelper";

    /**
     * Attempts to find and use the active BluetoothGatt connection to read a characteristic.
     * Strategy: Use reflection on BluetoothManager to access internal GATT connections.
     *
     * @param context Android context
     * @param deviceAddress Bluetooth device MAC address (e.g., "00:A0:50:0E:1F:26")
     * @param serviceUuid The service UUID string
     * @param characteristicUuid The characteristic UUID string
     * @return true if read was initiated successfully
     */
    public static boolean readCharacteristicDirect(Context context,
                                                   String deviceAddress,
                                                   String serviceUuid,
                                                   String characteristicUuid) {
        try {
            QLog.d(TAG, "=== KettlerReadHelper: Starting direct characteristic read ===");
            QLog.d(TAG, "Device: " + deviceAddress);
            QLog.d(TAG, "Service: " + serviceUuid);
            QLog.d(TAG, "Characteristic: " + characteristicUuid);

            // Get BluetoothManager
            BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
            if (bluetoothManager == null) {
                QLog.e(TAG, "BluetoothManager is null");
                return false;
            }

            // Get the target device
            BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
            if (adapter == null) {
                QLog.e(TAG, "BluetoothAdapter is null");
                return false;
            }

            BluetoothDevice targetDevice = adapter.getRemoteDevice(deviceAddress);
            if (targetDevice == null) {
                QLog.e(TAG, "Could not get BluetoothDevice for address: " + deviceAddress);
                return false;
            }

            // Check if device is connected
            int connectionState = bluetoothManager.getConnectionState(targetDevice, BluetoothProfile.GATT);
            QLog.d(TAG, "Device connection state: " + connectionState + " (2=CONNECTED)");

            if (connectionState != BluetoothProfile.STATE_CONNECTED) {
                QLog.e(TAG, "Device is not connected");
                return false;
            }

            // Try to access BluetoothManager's internal mBluetoothGatt field or connection list
            QLog.d(TAG, "Attempting to access BluetoothManager internals via reflection");

            try {
                // Try to find the mService field in BluetoothManager (BluetoothGatt service proxy)
                Field serviceField = BluetoothManager.class.getDeclaredField("mService");
                serviceField.setAccessible(true);
                Object bluetoothGattService = serviceField.get(bluetoothManager);

                if (bluetoothGattService != null) {
                    QLog.d(TAG, "Found mService: " + bluetoothGattService.getClass().getName());

                    // Try to get client connections
                    Class<?> serviceClass = bluetoothGattService.getClass();
                    Method[] methods = serviceClass.getDeclaredMethods();
                    for (Method m : methods) {
                        if (m.getName().contains("Client") || m.getName().contains("Connection")) {
                            QLog.d(TAG, "Service method: " + m.getName());
                        }
                    }
                }
            } catch (Exception e) {
                QLog.w(TAG, "Could not access BluetoothManager internals: " + e.getMessage());
            }

            // Alternative approach: Use reflection to find ALL BluetoothGatt instances in memory
            // and match by device address
            QLog.d(TAG, "Attempting to find BluetoothGatt instances via class reflection");

            try {
                // Search for mBluetoothGatt field in any loaded classes
                Class<?> qtBluetoothClass = Class.forName("org.qtproject.qt5.android.bluetooth.QtBluetoothLE");
                QLog.d(TAG, "Found QtBluetoothLE class");

                // We need the instance, not just the class
                // Try to access static fields or instance registry
                Field[] staticFields = qtBluetoothClass.getDeclaredFields();
                for (Field field : staticFields) {
                    QLog.d(TAG, "QtBluetoothLE field: " + field.getName() + " static=" + java.lang.reflect.Modifier.isStatic(field.getModifiers()));
                }

            } catch (ClassNotFoundException e) {
                QLog.e(TAG, "QtBluetoothLE class not found: " + e.getMessage());
            }

            // Last resort: Try to use reflection on BluetoothDevice to get active GATT
            QLog.d(TAG, "Attempting reflection on BluetoothDevice");
            Field[] deviceFields = BluetoothDevice.class.getDeclaredFields();
            for (Field field : deviceFields) {
                field.setAccessible(true);
                try {
                    Object value = field.get(targetDevice);
                    if (value != null && value.toString().contains("Gatt")) {
                        QLog.d(TAG, "BluetoothDevice field: " + field.getName() + " = " + value.getClass().getName());
                    }
                } catch (Exception e) {
                    // Ignore
                }
            }

            QLog.e(TAG, "=== Could not find active BluetoothGatt instance ===");
            QLog.e(TAG, "This approach requires either:");
            QLog.e(TAG, "1. Passing the QtBluetoothLE instance from C++ via JNI");
            QLog.e(TAG, "2. Modifying QtBluetoothLE to expose mBluetoothGatt");
            QLog.e(TAG, "3. Uncommenting the executeReadJob code in QtBluetoothLE.java");

            return false;

        } catch (Exception e) {
            QLog.e(TAG, "Unexpected error: " + e.getMessage());
            e.printStackTrace();
            return false;
        }
    }
}
