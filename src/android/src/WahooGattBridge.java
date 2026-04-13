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

import java.util.ArrayDeque;
import java.util.Queue;
import java.util.UUID;

public class WahooGattBridge {
    private static final String TAG = "WahooGattBridge";

    private static final UUID WAHOO_SERVICE_UUID = UUID.fromString("a026ee01-0a7d-4ab3-97fa-f1500f9feb8b");
    private static final UUID WAHOO_WRITE_UUID = UUID.fromString("a026e005-0a7d-4ab3-97fa-f1500f9feb8b");
    private static final UUID CYCLING_POWER_MEASUREMENT_UUID =
            UUID.fromString("00002a63-0000-1000-8000-00805f9b34fb");
    private static final UUID CCCD_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    private static BluetoothGatt bluetoothGatt;
    private static BluetoothGattCharacteristic writeCharacteristic;
    private static BluetoothGattCharacteristic cyclingPowerMeasurementCharacteristic;
    private static final Queue<GattOperation> operationQueue = new ArrayDeque<>();
    private static boolean operationInFlight = false;

    public static native void nativeOnConnectionStateChanged(boolean connected, int status);
    public static native void nativeOnServicesDiscovered(boolean hasWriteCharacteristic, boolean hasCyclingPowerMeasurement);
    public static native void nativeOnSubscriptionCompleted(boolean success, int status);
    public static native void nativeOnCharacteristicChanged(String uuid, byte[] value);
    public static native void nativeOnCharacteristicWrite(String uuid, byte[] value, int status);

    private interface GattOperation {
        boolean start();
    }

    public static synchronized boolean connect(Context context, String address) {
        disconnect();

        if (context == null) {
            QLog.e(TAG, "connect: null context");
            return false;
        }

        BluetoothManager manager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (manager == null) {
            QLog.e(TAG, "connect: BluetoothManager not available");
            return false;
        }

        BluetoothAdapter adapter = manager.getAdapter();
        if (adapter == null) {
            QLog.e(TAG, "connect: BluetoothAdapter not available");
            return false;
        }

        BluetoothDevice device;
        try {
            device = adapter.getRemoteDevice(address);
        } catch (IllegalArgumentException ex) {
            QLog.e(TAG, "connect: invalid address " + address, ex);
            return false;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            bluetoothGatt = device.connectGatt(context, false, callback, BluetoothDevice.TRANSPORT_LE);
        } else {
            bluetoothGatt = device.connectGatt(context, false, callback);
        }

        QLog.i(TAG, "connect: started for " + address);
        return bluetoothGatt != null;
    }

    public static synchronized void disconnect() {
        operationQueue.clear();
        operationInFlight = false;
        writeCharacteristic = null;
        cyclingPowerMeasurementCharacteristic = null;

        if (bluetoothGatt != null) {
            try {
                bluetoothGatt.disconnect();
            } catch (Exception ex) {
                QLog.w(TAG, "disconnect: disconnect failed", ex);
            }
            try {
                bluetoothGatt.close();
            } catch (Exception ex) {
                QLog.w(TAG, "disconnect: close failed", ex);
            }
            bluetoothGatt = null;
        }
    }

    public static synchronized boolean writeControl(final byte[] value) {
        if (bluetoothGatt == null || writeCharacteristic == null) {
            QLog.w(TAG, "writeControl: bridge not ready");
            return false;
        }

        operationQueue.add(new GattOperation() {
            @Override
            public boolean start() {
                writeCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                writeCharacteristic.setValue(value);
                boolean started = bluetoothGatt.writeCharacteristic(writeCharacteristic);
                QLog.i(TAG, "writeControl: started=" + started + " payload=" + bytesToHex(value));
                return started;
            }
        });

        processNextOperation();
        return true;
    }

    private static synchronized void processNextOperation() {
        if (operationInFlight || bluetoothGatt == null) {
            return;
        }

        GattOperation op = operationQueue.peek();
        if (op == null) {
            return;
        }

        operationInFlight = op.start();
        if (!operationInFlight) {
            operationQueue.poll();
            processNextOperation();
        }
    }

    private static synchronized void completeCurrentOperation() {
        operationQueue.poll();
        operationInFlight = false;
        processNextOperation();
    }

    private static synchronized void enqueueCyclingPowerSubscription() {
        if (bluetoothGatt == null || cyclingPowerMeasurementCharacteristic == null) {
            nativeOnSubscriptionCompleted(false, -1);
            return;
        }

        operationQueue.add(new GattOperation() {
            @Override
            public boolean start() {
                BluetoothGattDescriptor cccd = cyclingPowerMeasurementCharacteristic.getDescriptor(CCCD_UUID);
                if (cccd == null) {
                    QLog.w(TAG, "enqueueCyclingPowerSubscription: missing CCCD");
                    nativeOnSubscriptionCompleted(false, -2);
                    return false;
                }

                boolean notificationSet =
                        bluetoothGatt.setCharacteristicNotification(cyclingPowerMeasurementCharacteristic, true);
                if (!notificationSet) {
                    QLog.w(TAG, "enqueueCyclingPowerSubscription: setCharacteristicNotification failed");
                    nativeOnSubscriptionCompleted(false, -3);
                    return false;
                }

                byte[] descriptorValue =
                        (cyclingPowerMeasurementCharacteristic.getProperties() & BluetoothGattCharacteristic.PROPERTY_INDICATE) != 0
                                ? BluetoothGattDescriptor.ENABLE_INDICATION_VALUE
                                : BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE;
                cccd.setValue(descriptorValue);
                boolean started = bluetoothGatt.writeDescriptor(cccd);
                QLog.i(TAG, "enqueueCyclingPowerSubscription: started=" + started);
                return started;
            }
        });

        processNextOperation();
    }

    private static synchronized void resolveServices() {
        writeCharacteristic = null;
        cyclingPowerMeasurementCharacteristic = null;

        if (bluetoothGatt == null) {
            nativeOnServicesDiscovered(false, false);
            nativeOnSubscriptionCompleted(false, -4);
            return;
        }

        for (BluetoothGattService service : bluetoothGatt.getServices()) {
            if (WAHOO_SERVICE_UUID.equals(service.getUuid())) {
                writeCharacteristic = service.getCharacteristic(WAHOO_WRITE_UUID);
            }

            BluetoothGattCharacteristic cpm = service.getCharacteristic(CYCLING_POWER_MEASUREMENT_UUID);
            if (cpm != null) {
                cyclingPowerMeasurementCharacteristic = cpm;
            }
        }

        boolean hasWriteCharacteristic = writeCharacteristic != null;
        boolean hasCyclingPowerMeasurement = cyclingPowerMeasurementCharacteristic != null;
        QLog.i(TAG, "resolveServices: write=" + hasWriteCharacteristic + " power=" + hasCyclingPowerMeasurement);
        nativeOnServicesDiscovered(hasWriteCharacteristic, hasCyclingPowerMeasurement);

        if (hasCyclingPowerMeasurement) {
            enqueueCyclingPowerSubscription();
        } else {
            nativeOnSubscriptionCompleted(false, -5);
        }
    }

    private static final BluetoothGattCallback callback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            boolean connected = newState == BluetoothProfile.STATE_CONNECTED;
            QLog.i(TAG, "onConnectionStateChange status=" + status + " newState=" + newState);
            nativeOnConnectionStateChanged(connected, status);

            if (connected) {
                gatt.discoverServices();
            } else {
                synchronized (WahooGattBridge.class) {
                    operationQueue.clear();
                    operationInFlight = false;
                    writeCharacteristic = null;
                    cyclingPowerMeasurementCharacteristic = null;
                }
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            QLog.i(TAG, "onServicesDiscovered status=" + status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                resolveServices();
            } else {
                nativeOnServicesDiscovered(false, false);
                nativeOnSubscriptionCompleted(false, status);
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            byte[] value = characteristic.getValue();
            if (value == null) {
                value = new byte[0];
            }
            nativeOnCharacteristicChanged(characteristic.getUuid().toString(), value);
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            byte[] value = characteristic.getValue();
            if (value == null) {
                value = new byte[0];
            }
            nativeOnCharacteristicWrite(characteristic.getUuid().toString(), value, status);
            completeCurrentOperation();
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            QLog.i(TAG, "onDescriptorWrite status=" + status + " descriptor=" + descriptor.getUuid());
            if (CCCD_UUID.equals(descriptor.getUuid()) && cyclingPowerMeasurementCharacteristic != null &&
                    descriptor.getCharacteristic().getUuid().equals(cyclingPowerMeasurementCharacteristic.getUuid())) {
                nativeOnSubscriptionCompleted(status == BluetoothGatt.GATT_SUCCESS, status);
            }
            completeCurrentOperation();
        }
    };

    private static String bytesToHex(byte[] value) {
        if (value == null || value.length == 0) {
            return "";
        }

        StringBuilder builder = new StringBuilder(value.length * 2);
        for (byte b : value) {
            builder.append(String.format("%02x", b));
        }
        return builder.toString();
    }
}
