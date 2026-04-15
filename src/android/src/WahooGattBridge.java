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
import java.util.ArrayList;
import java.util.List;
import java.util.Queue;
import java.util.UUID;

public class WahooGattBridge {
    private static final String TAG = "WahooGattBridge";

    private static final UUID WAHOO_WRITE_UUID = UUID.fromString("a026e005-0a7d-4ab3-97fa-f1500f9feb8b");
    private static final UUID CYCLING_POWER_MEASUREMENT_UUID =
            UUID.fromString("00002a63-0000-1000-8000-00805f9b34fb");
    private static final UUID FITNESS_MACHINE_CONTROL_POINT_UUID =
            UUID.fromString("00002ad9-0000-1000-8000-00805f9b34fb");
    private static final UUID FITNESS_MACHINE_STATUS_UUID =
            UUID.fromString("00002ada-0000-1000-8000-00805f9b34fb");
    private static final UUID INDOOR_BIKE_DATA_UUID =
            UUID.fromString("00002ad2-0000-1000-8000-00805f9b34fb");
    private static final UUID WAHOO_EXTRA_NOTIFY_UUID =
            UUID.fromString("a026e037-0a7d-4ab3-97fa-f1500f9feb8b");
    private static final UUID CCCD_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    private static BluetoothGatt bluetoothGatt;
    private static BluetoothGattCharacteristic writeCharacteristic;
    private static BluetoothGattCharacteristic cyclingPowerMeasurementCharacteristic;
    private static final List<SubscriptionTarget> subscriptionTargets = new ArrayList<>();
    private static final Queue<GattOperation> operationQueue = new ArrayDeque<>();
    private static boolean operationInFlight = false;
    private static int pendingSubscriptionOperations = 0;
    private static int successfulSubscriptionOperations = 0;
    private static int firstSubscriptionFailureStatus = BluetoothGatt.GATT_SUCCESS;

    public static native void nativeOnConnectionStateChanged(boolean connected, int status);
    public static native void nativeOnServicesDiscovered(boolean hasWriteCharacteristic, boolean hasCyclingPowerMeasurement);
    public static native void nativeOnSubscriptionCompleted(boolean success, int status);
    public static native void nativeOnCharacteristicChanged(String uuid, byte[] value);
    public static native void nativeOnCharacteristicWrite(String uuid, byte[] value, int status);

    private interface GattOperation {
        boolean start();
    }

    private static final class SubscriptionTarget {
        final BluetoothGattCharacteristic characteristic;
        final boolean useIndications;

        SubscriptionTarget(BluetoothGattCharacteristic characteristic, boolean useIndications) {
            this.characteristic = characteristic;
            this.useIndications = useIndications;
        }
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
        subscriptionTargets.clear();
        pendingSubscriptionOperations = 0;
        successfulSubscriptionOperations = 0;
        firstSubscriptionFailureStatus = BluetoothGatt.GATT_SUCCESS;

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

    private static synchronized void finishSubscriptionSequenceIfDone() {
        if (pendingSubscriptionOperations != 0) {
            return;
        }

        boolean success = successfulSubscriptionOperations > 0;
        int status = success ? BluetoothGatt.GATT_SUCCESS : firstSubscriptionFailureStatus;
        QLog.i(TAG, "finishSubscriptionSequenceIfDone: success=" + success
                + " successful=" + successfulSubscriptionOperations
                + " status=" + status);
        nativeOnSubscriptionCompleted(success, status);
    }

    private static synchronized void recordSubscriptionStartFailure(String uuid, int status) {
        pendingSubscriptionOperations--;
        if (firstSubscriptionFailureStatus == BluetoothGatt.GATT_SUCCESS) {
            firstSubscriptionFailureStatus = status;
        }
        QLog.w(TAG, "subscription start failed for " + uuid + " status=" + status);
        finishSubscriptionSequenceIfDone();
    }

    private static synchronized void enqueueSubscription(final SubscriptionTarget target) {
        operationQueue.add(new GattOperation() {
            @Override
            public boolean start() {
                BluetoothGattDescriptor cccd = target.characteristic.getDescriptor(CCCD_UUID);
                if (cccd == null) {
                    recordSubscriptionStartFailure(target.characteristic.getUuid().toString(), -2);
                    return false;
                }

                boolean notificationSet =
                        bluetoothGatt.setCharacteristicNotification(target.characteristic, true);
                if (!notificationSet) {
                    recordSubscriptionStartFailure(target.characteristic.getUuid().toString(), -3);
                    return false;
                }

                byte[] descriptorValue = target.useIndications
                        ? BluetoothGattDescriptor.ENABLE_INDICATION_VALUE
                        : BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE;
                cccd.setValue(descriptorValue);
                boolean started = bluetoothGatt.writeDescriptor(cccd);
                QLog.i(TAG, "enqueueSubscription: started=" + started
                        + " uuid=" + target.characteristic.getUuid()
                        + " mode=" + (target.useIndications ? "indicate" : "notify"));
                if (!started) {
                    recordSubscriptionStartFailure(target.characteristic.getUuid().toString(), -4);
                }
                return started;
            }
        });
    }

    private static synchronized void enqueueResolvedSubscriptions() {
        if (bluetoothGatt == null || subscriptionTargets.isEmpty()) {
            nativeOnSubscriptionCompleted(false, -1);
            return;
        }

        pendingSubscriptionOperations = subscriptionTargets.size();
        successfulSubscriptionOperations = 0;
        firstSubscriptionFailureStatus = BluetoothGatt.GATT_SUCCESS;

        for (SubscriptionTarget target : subscriptionTargets) {
            enqueueSubscription(target);
        }

        processNextOperation();
    }

    private static void maybeAddSubscriptionTarget(BluetoothGattCharacteristic characteristic,
                                                   UUID uuid,
                                                   boolean useIndications) {
        if (characteristic == null) {
            return;
        }

        for (SubscriptionTarget target : subscriptionTargets) {
            if (target.characteristic.getUuid().equals(uuid)) {
                return;
            }
        }

        subscriptionTargets.add(new SubscriptionTarget(characteristic, useIndications));
        QLog.i(TAG, "resolveServices: scheduling "
                + (useIndications ? "indication" : "notification")
                + " for " + uuid);
    }

    private static synchronized void resolveServices() {
        writeCharacteristic = null;
        cyclingPowerMeasurementCharacteristic = null;
        subscriptionTargets.clear();
        pendingSubscriptionOperations = 0;
        successfulSubscriptionOperations = 0;
        firstSubscriptionFailureStatus = BluetoothGatt.GATT_SUCCESS;

        if (bluetoothGatt == null) {
            nativeOnServicesDiscovered(false, false);
            nativeOnSubscriptionCompleted(false, -4);
            return;
        }

        BluetoothGattCharacteristic fitnessMachineControlPointCharacteristic = null;
        BluetoothGattCharacteristic fitnessMachineStatusCharacteristic = null;
        BluetoothGattCharacteristic indoorBikeDataCharacteristic = null;
        BluetoothGattCharacteristic wahooExtraNotifyCharacteristic = null;

        for (BluetoothGattService service : bluetoothGatt.getServices()) {
            if (writeCharacteristic == null) {
                BluetoothGattCharacteristic candidateWrite = service.getCharacteristic(WAHOO_WRITE_UUID);
                if (candidateWrite != null) {
                    writeCharacteristic = candidateWrite;
                }
            }

            if (cyclingPowerMeasurementCharacteristic == null) {
                cyclingPowerMeasurementCharacteristic = service.getCharacteristic(CYCLING_POWER_MEASUREMENT_UUID);
            }
            if (fitnessMachineControlPointCharacteristic == null) {
                fitnessMachineControlPointCharacteristic = service.getCharacteristic(FITNESS_MACHINE_CONTROL_POINT_UUID);
            }
            if (fitnessMachineStatusCharacteristic == null) {
                fitnessMachineStatusCharacteristic = service.getCharacteristic(FITNESS_MACHINE_STATUS_UUID);
            }
            if (indoorBikeDataCharacteristic == null) {
                indoorBikeDataCharacteristic = service.getCharacteristic(INDOOR_BIKE_DATA_UUID);
            }
            if (wahooExtraNotifyCharacteristic == null) {
                wahooExtraNotifyCharacteristic = service.getCharacteristic(WAHOO_EXTRA_NOTIFY_UUID);
            }
        }

        maybeAddSubscriptionTarget(cyclingPowerMeasurementCharacteristic, CYCLING_POWER_MEASUREMENT_UUID, false);
        maybeAddSubscriptionTarget(writeCharacteristic, WAHOO_WRITE_UUID, true);
        maybeAddSubscriptionTarget(fitnessMachineControlPointCharacteristic, FITNESS_MACHINE_CONTROL_POINT_UUID, true);
        maybeAddSubscriptionTarget(fitnessMachineStatusCharacteristic, FITNESS_MACHINE_STATUS_UUID, false);
        maybeAddSubscriptionTarget(indoorBikeDataCharacteristic, INDOOR_BIKE_DATA_UUID, false);
        maybeAddSubscriptionTarget(wahooExtraNotifyCharacteristic, WAHOO_EXTRA_NOTIFY_UUID, false);

        boolean hasWriteCharacteristic = writeCharacteristic != null;
        boolean hasCyclingPowerMeasurement = cyclingPowerMeasurementCharacteristic != null
                || indoorBikeDataCharacteristic != null
                || fitnessMachineStatusCharacteristic != null;
        QLog.i(TAG, "resolveServices: write=" + hasWriteCharacteristic
                + " power=" + hasCyclingPowerMeasurement
                + " subscriptions=" + subscriptionTargets.size());
        nativeOnServicesDiscovered(hasWriteCharacteristic, hasCyclingPowerMeasurement);

        if (!subscriptionTargets.isEmpty()) {
            enqueueResolvedSubscriptions();
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
                    subscriptionTargets.clear();
                    pendingSubscriptionOperations = 0;
                    successfulSubscriptionOperations = 0;
                    firstSubscriptionFailureStatus = BluetoothGatt.GATT_SUCCESS;
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
            if (CCCD_UUID.equals(descriptor.getUuid())) {
                BluetoothGattCharacteristic characteristic = descriptor.getCharacteristic();
                synchronized (WahooGattBridge.class) {
                    if (pendingSubscriptionOperations > 0) {
                        pendingSubscriptionOperations--;
                        if (status == BluetoothGatt.GATT_SUCCESS) {
                            successfulSubscriptionOperations++;
                        } else if (firstSubscriptionFailureStatus == BluetoothGatt.GATT_SUCCESS) {
                            firstSubscriptionFailureStatus = status;
                        }
                        QLog.i(TAG, "subscription result uuid="
                                + (characteristic != null ? characteristic.getUuid() : null)
                                + " status=" + status
                                + " remaining=" + pendingSubscriptionOperations);
                        finishSubscriptionSequenceIfDone();
                    }
                }
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
