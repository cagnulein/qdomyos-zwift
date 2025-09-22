package org.cagnulen.qdomyoszwift;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import org.cagnulen.qdomyoszwift.QLog;

import java.util.UUID;

public class KettlerHandshakeReader {
    private static final String TAG = "KettlerHandshakeReader";

    // Kettler Service and Characteristic UUIDs
    private static final UUID KETTLER_SERVICE_UUID = UUID.fromString("638af000-7bde-3e25-ffc5-9de9b2a0197a");
    private static final UUID HANDSHAKE_READ_CHAR_UUID = UUID.fromString("638a1104-7bde-3e25-ffc5-9de9b2a0197a");
    private static final UUID HANDSHAKE_WRITE_CHAR_UUID = UUID.fromString("638a1105-7bde-3e25-ffc5-9de9b2a0197a");
    private static final UUID POWER_CONTROL_CHAR_UUID = UUID.fromString("638a100e-7bde-3e25-ffc5-9de9b2a0197a");
    private static final UUID RPM_CHAR_UUID = UUID.fromString("638a1002-7bde-3e25-ffc5-9de9b2a0197a");

    // CSC Service UUID
    private static final UUID CSC_SERVICE_UUID = UUID.fromString("00001816-0000-1000-8000-00805f9b34fb");
    private static final UUID CSC_MEASUREMENT_CHAR_UUID = UUID.fromString("00002a5b-0000-1000-8000-00805f9b34fb");

    private static BluetoothGatt bluetoothGatt;
    private static Context appContext;
    private static String deviceAddress;
    private static Handler mainHandler = new Handler(Looper.getMainLooper());

    private static boolean isConnected = false;
    private static boolean handshakeCompleted = false;

    // Native callback methods - will be implemented in C++
    public static native void onHandshakeSeedReceived(byte[] seedData);
    public static native void onHandshakeReadError(String error);
    public static native void onDeviceConnected();
    public static native void onDeviceDisconnected();
    public static native void onDataReceived(String characteristicUuid, byte[] data);

    public static void connectToDevice(String address, Context context) {
        QLog.d(TAG, "connectToDevice called for device: " + address);

        appContext = context;
        deviceAddress = address;
        isConnected = false;
        handshakeCompleted = false;

        BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager == null) {
            QLog.e(TAG, "BluetoothManager is null");
            onHandshakeReadError("BluetoothManager is null");
            return;
        }

        BluetoothAdapter bluetoothAdapter = bluetoothManager.getAdapter();
        if (bluetoothAdapter == null || !bluetoothAdapter.isEnabled()) {
            QLog.e(TAG, "Bluetooth not available or not enabled");
            onHandshakeReadError("Bluetooth not available or not enabled");
            return;
        }

        try {
            BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
            if (device == null) {
                QLog.e(TAG, "Device not found: " + address);
                onHandshakeReadError("Device not found: " + address);
                return;
            }

            // Connect to device
            QLog.d(TAG, "Connecting to device...");
            bluetoothGatt = device.connectGatt(context, false, gattCallback);

        } catch (Exception e) {
            QLog.e(TAG, "Error connecting to device: " + e.getMessage());
            onHandshakeReadError("Error connecting to device: " + e.getMessage());
        }
    }

    public static void sendHandshakeResponse(byte[] handshakeData) {
        QLog.d(TAG, "sendHandshakeResponse called");

        if (bluetoothGatt == null || !isConnected) {
            QLog.e(TAG, "Device not connected");
            return;
        }

        BluetoothGattService kettlerService = bluetoothGatt.getService(KETTLER_SERVICE_UUID);
        if (kettlerService == null) {
            QLog.e(TAG, "Kettler service not found");
            return;
        }

        BluetoothGattCharacteristic handshakeWriteChar = kettlerService.getCharacteristic(HANDSHAKE_WRITE_CHAR_UUID);
        if (handshakeWriteChar == null) {
            QLog.e(TAG, "Handshake write characteristic not found");
            return;
        }

        handshakeWriteChar.setValue(handshakeData);
        boolean writeSuccess = bluetoothGatt.writeCharacteristic(handshakeWriteChar);
        QLog.d(TAG, "Handshake write initiated: " + writeSuccess);
    }

    public static void setPower(int power) {
        QLog.d(TAG, "setPower called: " + power);

        if (bluetoothGatt == null || !isConnected || !handshakeCompleted) {
            QLog.e(TAG, "Device not ready for power commands");
            return;
        }

        BluetoothGattService kettlerService = bluetoothGatt.getService(KETTLER_SERVICE_UUID);
        if (kettlerService == null) {
            QLog.e(TAG, "Kettler service not found");
            return;
        }

        BluetoothGattCharacteristic powerChar = kettlerService.getCharacteristic(POWER_CONTROL_CHAR_UUID);
        if (powerChar == null) {
            QLog.e(TAG, "Power control characteristic not found");
            return;
        }

        // Convert power to 2-byte little-endian
        byte[] powerData = new byte[2];
        powerData[0] = (byte)(power & 0xFF);
        powerData[1] = (byte)((power >> 8) & 0xFF);

        powerChar.setValue(powerData);
        boolean writeSuccess = bluetoothGatt.writeCharacteristic(powerChar);
        QLog.d(TAG, "Power write initiated: " + writeSuccess);
    }

    public static void readHandshakeSeed(String address, Context context) {
        QLog.d(TAG, "readHandshakeSeed called for device: " + address);
        connectToDevice(address, context);

        appContext = context;
        deviceAddress = address;

        BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager == null) {
            QLog.e(TAG, "BluetoothManager is null");
            onHandshakeReadError("BluetoothManager is null");
            return;
        }

        BluetoothAdapter bluetoothAdapter = bluetoothManager.getAdapter();
        if (bluetoothAdapter == null || !bluetoothAdapter.isEnabled()) {
            QLog.e(TAG, "Bluetooth not available or not enabled");
            onHandshakeReadError("Bluetooth not available or not enabled");
            return;
        }

        try {
            BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
            if (device == null) {
                QLog.e(TAG, "Device not found: " + address);
                onHandshakeReadError("Device not found: " + address);
                return;
            }

            // Connect to device
            QLog.d(TAG, "Connecting to device...");
            bluetoothGatt = device.connectGatt(context, false, gattCallback);

        } catch (Exception e) {
            QLog.e(TAG, "Error connecting to device: " + e.getMessage());
            onHandshakeReadError("Error connecting to device: " + e.getMessage());
        }
    }

    private static final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            QLog.d(TAG, "onConnectionStateChange: status=" + status + ", newState=" + newState);

            if (newState == BluetoothProfile.STATE_CONNECTED) {
                QLog.d(TAG, "Connected to GATT server, discovering services...");
                isConnected = true;
                onDeviceConnected();

                // Small delay before discovering services
                mainHandler.postDelayed(() -> {
                    if (bluetoothGatt != null) {
                        bluetoothGatt.discoverServices();
                    }
                }, 1000);

            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                QLog.d(TAG, "Disconnected from GATT server");
                isConnected = false;
                handshakeCompleted = false;
                onDeviceDisconnected();
                cleanup();
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            QLog.d(TAG, "onServicesDiscovered: status=" + status);

            if (status == BluetoothGatt.GATT_SUCCESS) {
                // Setup Kettler service
                BluetoothGattService kettlerService = gatt.getService(KETTLER_SERVICE_UUID);
                if (kettlerService == null) {
                    QLog.e(TAG, "Kettler service not found");
                    onHandshakeReadError("Kettler service not found");
                    cleanup();
                    return;
                }

                // Setup CSC service
                BluetoothGattService cscService = gatt.getService(CSC_SERVICE_UUID);
                if (cscService != null) {
                    BluetoothGattCharacteristic cscChar = cscService.getCharacteristic(CSC_MEASUREMENT_CHAR_UUID);
                    if (cscChar != null) {
                        gatt.setCharacteristicNotification(cscChar, true);
                        BluetoothGattDescriptor descriptor = cscChar.getDescriptor(UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"));
                        if (descriptor != null) {
                            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                            gatt.writeDescriptor(descriptor);
                        }
                    }
                }

                // Setup Kettler RPM notifications
                BluetoothGattCharacteristic rpmChar = kettlerService.getCharacteristic(RPM_CHAR_UUID);
                if (rpmChar != null) {
                    gatt.setCharacteristicNotification(rpmChar, true);
                    BluetoothGattDescriptor descriptor = rpmChar.getDescriptor(UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"));
                    if (descriptor != null) {
                        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                        gatt.writeDescriptor(descriptor);
                    }
                }

                // Start handshake process
                BluetoothGattCharacteristic handshakeChar = kettlerService.getCharacteristic(HANDSHAKE_READ_CHAR_UUID);
                if (handshakeChar == null) {
                    QLog.e(TAG, "Handshake characteristic not found");
                    onHandshakeReadError("Handshake characteristic not found");
                    cleanup();
                    return;
                }

                // Check if characteristic supports read
                int properties = handshakeChar.getProperties();
                QLog.d(TAG, "Handshake characteristic properties: " + properties);
                if ((properties & BluetoothGattCharacteristic.PROPERTY_READ) == 0) {
                    QLog.e(TAG, "Handshake characteristic does not support read");
                    onHandshakeReadError("Handshake characteristic does not support read");
                    cleanup();
                    return;
                }

                // Attempt to read the characteristic
                QLog.d(TAG, "Reading handshake characteristic...");
                boolean readSuccess = gatt.readCharacteristic(handshakeChar);
                if (!readSuccess) {
                    QLog.e(TAG, "Failed to initiate characteristic read");
                    onHandshakeReadError("Failed to initiate characteristic read");
                    cleanup();
                }

            } else {
                QLog.e(TAG, "Service discovery failed with status: " + status);
                onHandshakeReadError("Service discovery failed with status: " + status);
                cleanup();
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            QLog.d(TAG, "onCharacteristicRead: status=" + status);

            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (HANDSHAKE_READ_CHAR_UUID.equals(characteristic.getUuid())) {
                    byte[] data = characteristic.getValue();
                    if (data != null) {
                        QLog.d(TAG, "Handshake seed received, length: " + data.length);
                        StringBuilder hex = new StringBuilder();
                        for (byte b : data) {
                            hex.append(String.format("%02X ", b));
                        }
                        QLog.d(TAG, "Handshake seed data: " + hex.toString());

                        // Send data back to C++
                        onHandshakeSeedReceived(data);
                    } else {
                        QLog.e(TAG, "Characteristic read returned null data");
                        onHandshakeReadError("Characteristic read returned null data");
                    }
                } else {
                    QLog.w(TAG, "Read callback for unexpected characteristic: " + characteristic.getUuid());
                }
            } else {
                QLog.e(TAG, "Characteristic read failed with status: " + status);
                onHandshakeReadError("Characteristic read failed with status: " + status);
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            QLog.d(TAG, "onCharacteristicWrite: " + characteristic.getUuid() + ", status: " + status);

            if (HANDSHAKE_WRITE_CHAR_UUID.equals(characteristic.getUuid()) && status == BluetoothGatt.GATT_SUCCESS) {
                QLog.d(TAG, "Handshake write completed successfully");
                handshakeCompleted = true;
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            byte[] data = characteristic.getValue();
            if (data != null) {
                StringBuilder hex = new StringBuilder();
                for (byte b : data) {
                    hex.append(String.format("%02X ", b));
                }
                QLog.d(TAG, "Characteristic changed: " + characteristic.getUuid() + ", data: " + hex.toString());

                // Send all characteristic data back to C++
                onDataReceived(characteristic.getUuid().toString(), data);
            }
        }
    };

    private static void cleanup() {
        QLog.d(TAG, "Cleaning up GATT connection");
        if (bluetoothGatt != null) {
            bluetoothGatt.disconnect();
            bluetoothGatt.close();
            bluetoothGatt = null;
        }
    }

    public static void forceCleanup() {
        QLog.d(TAG, "Force cleanup called");
        cleanup();
    }
}