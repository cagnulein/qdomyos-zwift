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
import java.util.Arrays;

public class KettlerHandshakeReader {
    private static final String TAG = "KettlerHandshakeReader";

    // Kettler Service and Characteristic UUIDs
    private static final UUID KETTLER_SERVICE_UUID = UUID.fromString("638af000-7bde-3e25-ffc5-9de9b2a0197a");
    private static final UUID HANDSHAKE_READ_CHAR_UUID = UUID.fromString("638a1104-7bde-3e25-ffc5-9de9b2a0197a");
    private static final UUID HANDSHAKE_WRITE_CHAR_UUID = UUID.fromString("638a1105-7bde-3e25-ffc5-9de9b2a0197a");
    private static final UUID POWER_CONTROL_CHAR_UUID = UUID.fromString("638a100e-7bde-3e25-ffc5-9de9b2a0197a");
    private static final UUID RPM_CHAR_UUID = UUID.fromString("638a1002-7bde-3e25-ffc5-9de9b2a0197a");
    private static final UUID POWER_DATA_CHAR_UUID = UUID.fromString("638a1003-7bde-3e25-ffc5-9de9b2a0197a");
    private static final UUID CYCLING_POWER_SERVICE_UUID = UUID.fromString("00001818-0000-1000-8000-00805f9b34fb");
    private static final UUID CYCLING_POWER_MEASUREMENT_CHAR_UUID = UUID.fromString("00002a63-0000-1000-8000-00805f9b34fb");
    private static final UUID CLIENT_CHARACTERISTIC_CONFIG_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    // CSC Service UUID
    private static final UUID CSC_SERVICE_UUID = UUID.fromString("00001816-0000-1000-8000-00805f9b34fb");
    private static final UUID CSC_MEASUREMENT_CHAR_UUID = UUID.fromString("00002a5b-0000-1000-8000-00805f9b34fb");

    private static BluetoothGatt bluetoothGatt;
    private static Context appContext;
    private static String deviceAddress;
    private static Handler mainHandler = new Handler(Looper.getMainLooper());

    // Simple single-op GATT queue to serialize operations
    private interface GattOperation { boolean execute(BluetoothGatt g); String name(); }
    private static final java.util.ArrayDeque<GattOperation> gattQueue = new java.util.ArrayDeque<>();
    private static boolean gattOpInProgress = false;
    private static final int GATT_OP_RETRY_DELAY_MS = 200;
    private static final java.util.HashSet<UUID> notificationsSet = new java.util.HashSet<>();

    private static void enqueue(GattOperation op) {
        synchronized (gattQueue) {
            gattQueue.add(op);
        }
        // Kick the runner on the main thread
        mainHandler.post(KettlerHandshakeReader::runNextGattOp);
    }

    private static void runNextGattOp() {
        if (bluetoothGatt == null) return;
        if (gattOpInProgress) return;
        final GattOperation next;
        synchronized (gattQueue) {
            next = gattQueue.poll();
        }
        if (next == null) return;

        boolean started = false;
        try {
            started = next.execute(bluetoothGatt);
        } catch (Throwable t) {
            QLog.e(TAG, "Exception starting GATT op '" + next.name() + "': " + t.getMessage());
        }
        if (started) {
            gattOpInProgress = true;
        } else {
            // Could not start now; retry shortly and move on to avoid deadlocks
            QLog.e(TAG, "Failed to start GATT op '" + next.name() + "', retrying soon");
            mainHandler.postDelayed(() -> {
                synchronized (gattQueue) { gattQueue.addFirst(next); }
                gattOpInProgress = false;
                runNextGattOp();
            }, GATT_OP_RETRY_DELAY_MS);
        }
    }

    private static void onGattOperationComplete() {
        gattOpInProgress = false;
        mainHandler.post(KettlerHandshakeReader::runNextGattOp);
    }

    private static boolean isConnected = false;
    private static boolean handshakeCompleted = false;
    private static int pendingDescriptorWrites = 0;
    private static BluetoothGattCharacteristic pendingHandshakeCharacteristic = null;
    private static boolean handshakeReadInProgress = false;
    private static int handshakeReadAttempts = 0;
    private static final int MAX_HANDSHAKE_READ_ATTEMPTS = 6;
    private static final long HANDSHAKE_READ_RETRY_DELAY_MS = 200;
    private static final int HANDSHAKE_SEED_EXPECTED_LENGTH = 2;
    private static boolean postHandshakeNotificationsEnabled = false;
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

                // Enqueue handshake read after notification setup; the queue will serialize correctly
                if (pendingHandshakeCharacteristic != null) {
                    enqueue(new GattOperation() {
                        @Override public boolean execute(BluetoothGatt g) {
                            QLog.d(TAG, "Reading handshake characteristic (queued attempt " + (handshakeReadAttempts + 1) + ")...");
                            handshakeReadAttempts++;
                            handshakeReadInProgress = true;
                            return g.readCharacteristic(pendingHandshakeCharacteristic);
                        }
                        @Override public String name() { return "readHandshakeSeed"; }
                    });
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

            if (HANDSHAKE_READ_CHAR_UUID.equals(characteristic.getUuid())) {
                handshakeReadInProgress = false;

                if (status == BluetoothGatt.GATT_SUCCESS) {
                    byte[] data = characteristic.getValue();
                    if (processHandshakeSeed(data)) {
                        handshakeReadAttempts = 0;
                        pendingHandshakeCharacteristic = null;
                    }
                } else {
                    QLog.e(TAG, "Characteristic read failed with status: " + status);
                    onHandshakeReadError("Characteristic read failed with status: " + status);
                    cleanup();
                }
            } else {
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    if (POWER_DATA_CHAR_UUID.equals(characteristic.getUuid()) ||
                        CYCLING_POWER_MEASUREMENT_CHAR_UUID.equals(characteristic.getUuid())) {
                        byte[] value = characteristic.getValue();
                        if (value != null) {
                            onDataReceived(characteristic.getUuid().toString(), value);
                        }
                    } else {
                        QLog.w(TAG, "Read callback for unexpected characteristic: " + characteristic.getUuid());
                    }
                } else {
                    QLog.e(TAG, "Characteristic read failed with status: " + status + " for " + characteristic.getUuid());
        }
        // Mark current queued op complete (for both handshake and any other reads)
        onGattOperationComplete();
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

            // Continue with next queued op (e.g., handshake read)
            onGattOperationComplete();
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            QLog.d(TAG, "onCharacteristicWrite: " + characteristic.getUuid() + ", status: " + status);

            if (HANDSHAKE_WRITE_CHAR_UUID.equals(characteristic.getUuid()) && status == BluetoothGatt.GATT_SUCCESS) {
                QLog.d(TAG, "Handshake write completed successfully");
                handshakeCompleted = true;
                enablePostHandshakeNotifications(gatt);
            }
            // Complete op in case it was queued elsewhere
            onGattOperationComplete();
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
        postHandshakeNotificationsEnabled = false;
        notificationsSet.clear();
        synchronized (gattQueue) { gattQueue.clear(); }
        gattOpInProgress = false;
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

        final UUID cuuid = characteristic.getUuid();
        final int props = characteristic.getProperties();
        final boolean supportsNotify = (props & BluetoothGattCharacteristic.PROPERTY_NOTIFY) != 0;
        final boolean supportsIndicate = (props & BluetoothGattCharacteristic.PROPERTY_INDICATE) != 0;
        if (!supportsNotify && !supportsIndicate) {
            QLog.e(TAG, "Characteristic does not support notify/indicate: " + cuuid);
            return 0;
        }

        if (!notificationsSet.contains(cuuid)) {
            boolean notificationSet = gatt.setCharacteristicNotification(characteristic, true);
            if (!notificationSet) {
                QLog.e(TAG, "Failed to enable notifications for characteristic: " + cuuid);
            }
            notificationsSet.add(cuuid);
        }

        final BluetoothGattDescriptor descriptor = characteristic.getDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
        if (descriptor == null) {
            QLog.e(TAG, "Client configuration descriptor not found for characteristic: " + cuuid);
            return 0;
        }

        final byte[] cccdValue = supportsNotify
                ? BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
                : BluetoothGattDescriptor.ENABLE_INDICATION_VALUE;

        enqueue(new GattOperation() {
            @Override public boolean execute(BluetoothGatt g) {
                descriptor.setValue(cccdValue);
                QLog.d(TAG, "Writing CCCD for " + cuuid + " value=" + (cccdValue == BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE ? "notify" : "indicate"));
                return g.writeDescriptor(descriptor);
            }
            @Override public String name() { return "writeDescriptor(" + cuuid + ")"; }
        });

        return 1;
    }

    private static void enablePostHandshakeNotifications(BluetoothGatt gatt) {
        if (gatt == null || postHandshakeNotificationsEnabled) {
            return;
        }

        BluetoothGattService kettlerService = gatt.getService(KETTLER_SERVICE_UUID);
        if (kettlerService != null) {
            pendingDescriptorWrites += enableNotification(gatt, kettlerService.getCharacteristic(RPM_CHAR_UUID));
            pendingDescriptorWrites += enableNotification(gatt, kettlerService.getCharacteristic(POWER_DATA_CHAR_UUID));
        }

        BluetoothGattService powerService = gatt.getService(CYCLING_POWER_SERVICE_UUID);
        if (powerService != null) {
            pendingDescriptorWrites +=
                enableNotification(gatt, powerService.getCharacteristic(CYCLING_POWER_MEASUREMENT_CHAR_UUID));
        }

        postHandshakeNotificationsEnabled = true;
    }

    private static boolean processHandshakeSeed(byte[] data) {
        if (data == null || data.length == 0) {
            QLog.e(TAG, "Characteristic read returned null/empty data");
            onHandshakeReadError("Characteristic read returned null data");
            cleanup();
            return false;
        }

        int length = Math.min(data.length, HANDSHAKE_SEED_EXPECTED_LENGTH);
        byte[] seed = Arrays.copyOf(data, length);

        StringBuilder hex = new StringBuilder();
        for (int i = 0; i < length; ++i) {
            hex.append(String.format("%02X ", seed[i]));
        }
        QLog.d(TAG, "Handshake seed assembled, length: " + length + ", data: " + hex);

        onHandshakeSeedReceived(seed);
        return true;
    }

}
