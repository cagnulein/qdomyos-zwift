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
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import java.lang.reflect.Method;
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
    private static final UUID CLIENT_CHARACTERISTIC_CONFIG_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    // CSC Service UUID
    private static final UUID CSC_SERVICE_UUID = UUID.fromString("00001816-0000-1000-8000-00805f9b34fb");
    private static final UUID CSC_MEASUREMENT_CHAR_UUID = UUID.fromString("00002a5b-0000-1000-8000-00805f9b34fb");

    private static BluetoothGatt bluetoothGatt;
    private static Context appContext;
    private static String deviceAddress;
    private static Handler mainHandler = new Handler(Looper.getMainLooper());

    private static boolean isConnected = false;
    private static boolean handshakeCompleted = false;
    private static int pendingDescriptorWrites = 0;
    private static BluetoothGattCharacteristic pendingHandshakeCharacteristic = null;
    private static boolean handshakeReadInProgress = false;
    private static int handshakeReadAttempts = 0;
    private static final int MAX_HANDSHAKE_READ_ATTEMPTS = 6;
    private static final long HANDSHAKE_READ_RETRY_DELAY_MS = 200;
    private static final int HANDSHAKE_SEED_EXPECTED_LENGTH = 6;
    private static byte[] handshakeSeedBuffer = new byte[HANDSHAKE_SEED_EXPECTED_LENGTH];
    private static int handshakeSeedBytesRead = 0;
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
        resetHandshakeState();

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

                pendingHandshakeCharacteristic = handshakeChar;
                handshakeReadInProgress = false;
                handshakeReadAttempts = 0;
                pendingDescriptorWrites = 0;

                // Setup CSC notifications
                BluetoothGattService cscService = gatt.getService(CSC_SERVICE_UUID);
                if (cscService != null) {
                    BluetoothGattCharacteristic cscChar = cscService.getCharacteristic(CSC_MEASUREMENT_CHAR_UUID);
                    pendingDescriptorWrites += enableNotification(gatt, cscChar);
                }

                // Setup Kettler RPM notifications
                BluetoothGattCharacteristic rpmChar = kettlerService.getCharacteristic(RPM_CHAR_UUID);
                pendingDescriptorWrites += enableNotification(gatt, rpmChar);

                attemptHandshakeRead(gatt);

            } else {
                QLog.e(TAG, "Service discovery failed with status: " + status);
                onHandshakeReadError("Service discovery failed with status: " + status);
                cleanup();
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            QLog.d(TAG, "onCharacteristicRead: status=" + status);

            if (HANDSHAKE_READ_CHAR_UUID.equals(characteristic.getUuid())) {
                handshakeReadInProgress = false;

                if (status == BluetoothGatt.GATT_SUCCESS) {
                    byte[] data = characteristic.getValue();
                    if (data != null && data.length > 0) {
                        appendHandshakeChunk(data);

                        if (handshakeSeedBytesRead < HANDSHAKE_SEED_EXPECTED_LENGTH) {
                            if (!requestNextHandshakeChunk(gatt)) {
                                QLog.e(TAG, "Failed to request additional handshake bytes");
                                onHandshakeReadError("Failed to read complete handshake seed");
                                cleanup();
                            }
                        } else {
                            handshakeReadAttempts = 0;
                            byte[] finalSeed = new byte[HANDSHAKE_SEED_EXPECTED_LENGTH];
                            System.arraycopy(handshakeSeedBuffer, 0, finalSeed, 0, HANDSHAKE_SEED_EXPECTED_LENGTH);

                            StringBuilder hex = new StringBuilder();
                            for (byte b : finalSeed) {
                                hex.append(String.format("%02X ", b));
                            }
                            QLog.d(TAG, "Handshake seed assembled, length: " + finalSeed.length + ", data: " + hex);

                            pendingHandshakeCharacteristic = null;
                            handshakeSeedBuffer = new byte[HANDSHAKE_SEED_EXPECTED_LENGTH];
                            handshakeSeedBytesRead = 0;

                            onHandshakeSeedReceived(finalSeed);
                        }
                    } else {
                        QLog.e(TAG, "Characteristic read returned null/empty data");
                        onHandshakeReadError("Characteristic read returned null data");
                        cleanup();
                    }
                } else {
                    QLog.e(TAG, "Characteristic read failed with status: " + status);
                    onHandshakeReadError("Characteristic read failed with status: " + status);
                    cleanup();
                }
            } else {
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    QLog.w(TAG, "Read callback for unexpected characteristic: " + characteristic.getUuid());
                } else {
                    QLog.e(TAG, "Characteristic read failed with status: " + status + " for " + characteristic.getUuid());
                }
            }
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            if (pendingDescriptorWrites > 0) {
                pendingDescriptorWrites--;
            }

            UUID descriptorUuid = descriptor != null ? descriptor.getUuid() : null;
            QLog.d(TAG, "onDescriptorWrite: " + descriptorUuid + ", status: " + status + ", remaining: " + pendingDescriptorWrites);

            if (status != BluetoothGatt.GATT_SUCCESS) {
                QLog.e(TAG, "Descriptor write failed with status: " + status);
            }

            attemptHandshakeRead(gatt);
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
        resetHandshakeState();
    }

    public static void forceCleanup() {
        QLog.d(TAG, "Force cleanup called");
        cleanup();
    }

    private static void resetHandshakeState() {
        pendingDescriptorWrites = 0;
        pendingHandshakeCharacteristic = null;
        handshakeReadInProgress = false;
        handshakeReadAttempts = 0;
        handshakeSeedBuffer = new byte[HANDSHAKE_SEED_EXPECTED_LENGTH];
        handshakeSeedBytesRead = 0;
    }

    private static void attemptHandshakeRead(BluetoothGatt gatt) {
        if (pendingHandshakeCharacteristic == null) {
            return;
        }

        if (pendingDescriptorWrites > 0) {
            QLog.d(TAG, "Handshake read waiting for pending descriptor writes: " + pendingDescriptorWrites);
            return;
        }

        if (handshakeReadInProgress) {
            QLog.d(TAG, "Handshake read already in progress");
            return;
        }

        if (handshakeReadAttempts >= MAX_HANDSHAKE_READ_ATTEMPTS) {
            QLog.e(TAG, "Exceeded maximum handshake read attempts");
            onHandshakeReadError("Failed to initiate characteristic read");
            cleanup();
            return;
        }

        handshakeReadAttempts++;
        QLog.d(TAG, "Reading handshake characteristic (attempt " + handshakeReadAttempts + ")...");
        boolean readSuccess = gatt.readCharacteristic(pendingHandshakeCharacteristic);
        if (readSuccess) {
            handshakeReadInProgress = true;
        } else {
            QLog.e(TAG, "Failed to initiate characteristic read");
            mainHandler.postDelayed(() -> {
                if (bluetoothGatt == gatt && pendingHandshakeCharacteristic != null) {
                    handshakeReadInProgress = false;
                    attemptHandshakeRead(gatt);
                }
            }, HANDSHAKE_READ_RETRY_DELAY_MS);
        }
    }

    private static int enableNotification(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
        if (characteristic == null) {
            return 0;
        }

        boolean notificationSet = gatt.setCharacteristicNotification(characteristic, true);
        if (!notificationSet) {
            QLog.e(TAG, "Failed to enable notifications for characteristic: " + characteristic.getUuid());
        }

        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
        if (descriptor == null) {
            QLog.e(TAG, "Client configuration descriptor not found for characteristic: " + characteristic.getUuid());
            return 0;
        }

        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        boolean writeInitiated = gatt.writeDescriptor(descriptor);
        if (writeInitiated) {
            return 1;
        }

        QLog.e(TAG, "Failed to initiate descriptor write for characteristic: " + characteristic.getUuid());
        return 0;
    }

    private static void appendHandshakeChunk(byte[] data) {
        if (handshakeSeedBytesRead >= HANDSHAKE_SEED_EXPECTED_LENGTH) {
            return;
        }

        int bytesToCopy = Math.min(data.length, HANDSHAKE_SEED_EXPECTED_LENGTH - handshakeSeedBytesRead);
        System.arraycopy(data, 0, handshakeSeedBuffer, handshakeSeedBytesRead, bytesToCopy);
        handshakeSeedBytesRead += bytesToCopy;

        StringBuilder hex = new StringBuilder();
        for (int i = 0; i < bytesToCopy; ++i) {
            hex.append(String.format("%02X ", data[i]));
        }
        QLog.d(TAG, "Handshake chunk appended (" + bytesToCopy + " bytes): " + hex + " total=" + handshakeSeedBytesRead);
    }

    private static boolean requestNextHandshakeChunk(BluetoothGatt gatt) {
        if (pendingHandshakeCharacteristic == null) {
            return false;
        }

        int offset = handshakeSeedBytesRead;
        if (offset >= HANDSHAKE_SEED_EXPECTED_LENGTH) {
            return false;
        }

        QLog.d(TAG, "Requesting additional handshake bytes at offset " + offset);

        try {
            Method method = BluetoothGatt.class.getDeclaredMethod("readCharacteristic", BluetoothGattCharacteristic.class, int.class);
            method.setAccessible(true);
            Object result = method.invoke(gatt, pendingHandshakeCharacteristic, offset);
            boolean success;
            if (result instanceof Integer) {
                success = ((Integer) result) == BluetoothGatt.GATT_SUCCESS;
            } else if (result instanceof Boolean) {
                success = (Boolean) result;
            } else {
                success = false;
            }

            if (success) {
                handshakeReadInProgress = true;
                return true;
            }

            QLog.e(TAG, "readCharacteristic(offset) reflection returned failure: " + result);
        } catch (NoSuchMethodException e) {
            QLog.e(TAG, "readCharacteristic with offset not available: " + e.getMessage());
        } catch (Exception e) {
            QLog.e(TAG, "Error invoking readCharacteristic with offset: " + e.getMessage());
        }

        if (Build.VERSION.SDK_INT >= 33) {
            QLog.e(TAG, "Offset read failed even though API level supports it");
        } else {
            QLog.w(TAG, "Offset read not supported on this Android version");
        }

        return false;
    }

}
