package org.cagnulen.qdomyoszwift;

import android.app.ActivityManager;
import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import android.os.Looper;
import android.os.Handler;
import org.cagnulen.qdomyoszwift.QLog;
import android.content.BroadcastReceiver;
import android.content.ContextWrapper;
import android.content.IntentFilter;
import android.widget.Toast;

import org.jetbrains.annotations.Nullable;

import java.util.HashMap;
import java.util.List;

import android.bluetooth.BluetoothManager;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseSettings;
import android.os.ParcelUuid;
import java.util.UUID;

public class BleAdvertiser {
    private static final UUID SERVICE_UUID = UUID.fromString("00001826-0000-1000-8000-00805f9b34fb");
    // PM5 Concept2 UUIDs
    private static final UUID PM5_DISCOVERY_SERVICE_UUID = UUID.fromString("CE060000-43E5-11E4-916C-0800200C9A66");
    private static final UUID PM5_ROWING_SERVICE_UUID = UUID.fromString("CE060030-43E5-11E4-916C-0800200C9A66");
    private static final byte[] SERVICE_DATA_ROWER = {0x01, 0x10, 0x00};
    private static final byte[] SERVICE_DATA_TREADMILL = {0x01, 0x01, 0x00};

    public static void startAdvertisingRower(Context context) {
        BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager != null) {
            android.bluetooth.le.BluetoothLeAdvertiser advertiser = bluetoothManager.getAdapter().getBluetoothLeAdvertiser();

            AdvertiseSettings settings = new AdvertiseSettings.Builder()
                    .setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_LOW_LATENCY)
                    .setTxPowerLevel(AdvertiseSettings.ADVERTISE_TX_POWER_HIGH)
                    .setConnectable(true)
                    .build();

            AdvertiseData advertiseData = new AdvertiseData.Builder()
                    .setIncludeDeviceName(true)
                    .addServiceUuid(new ParcelUuid(SERVICE_UUID))
                    .addServiceData(new ParcelUuid(SERVICE_UUID), SERVICE_DATA_ROWER)
                    .build();

            if (advertiser != null) {
                advertiser.startAdvertising(settings, advertiseData, advertiseCallback);
            }
        }
    }

    public static void startAdvertisingRowerPM5(Context context) {
        BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager != null) {
            android.bluetooth.le.BluetoothLeAdvertiser advertiser = bluetoothManager.getAdapter().getBluetoothLeAdvertiser();

            AdvertiseSettings settings = new AdvertiseSettings.Builder()
                    .setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_LOW_LATENCY)
                    .setTxPowerLevel(AdvertiseSettings.ADVERTISE_TX_POWER_HIGH)
                    .setConnectable(true)
                    .build();

            // PM5 advertising - only use device name, no service UUID
            // 128-bit UUIDs are too large for advertising packet (31 bytes max)
            // Apps like Mywhoosh should discover services after connection
            AdvertiseData advertiseData = new AdvertiseData.Builder()
                    .setIncludeDeviceName(true)
                    .build();

            if (advertiser != null) {
                QLog.d("BleAdvertiser", "Starting PM5 advertising (name only, no UUID)");
                advertiser.startAdvertising(settings, advertiseData, advertiseCallback);
            }
        }
    }

    public static void startAdvertisingTreadmill(Context context) {
        BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager != null) {
            android.bluetooth.le.BluetoothLeAdvertiser advertiser = bluetoothManager.getAdapter().getBluetoothLeAdvertiser();

            AdvertiseSettings settings = new AdvertiseSettings.Builder()
                    .setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_LOW_LATENCY)
                    .setTxPowerLevel(AdvertiseSettings.ADVERTISE_TX_POWER_HIGH)
                    .setConnectable(true)
                    .build();

            AdvertiseData advertiseData = new AdvertiseData.Builder()
                    .setIncludeDeviceName(true)
                    .addServiceUuid(new ParcelUuid(SERVICE_UUID))
                    .addServiceData(new ParcelUuid(SERVICE_UUID), SERVICE_DATA_TREADMILL)
                    .build();

            if (advertiser != null) {
                advertiser.startAdvertising(settings, advertiseData, advertiseCallback);
            }
        }
    }

    private static AdvertiseCallback advertiseCallback = new AdvertiseCallback() {
        @Override
        public void onStartSuccess(AdvertiseSettings settingsInEffect) {
          QLog.d("BleAdvertiser", "Advertising started successfully");
        }

        @Override
        public void onStartFailure(int errorCode) {
          QLog.e("BleAdvertiser", "Advertising failed with error code: " + errorCode);
        }
    };
}
