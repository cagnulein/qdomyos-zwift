package org.cagnulen.qdomyoszwift;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.os.ParcelUuid;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Native Android BLE scanner to discover all advertised services.
 * Qt Bluetooth has a limitation on Android/iOS where it doesn't report
 * services discovered progressively. This scanner uses native Android APIs
 * to collect all advertised service UUIDs for each device.
 */
public class BleServiceScanner {
    private static final String TAG = "BleServiceScanner";

    // Store discovered services for each device (keyed by MAC address)
    private static final Map<String, List<String>> deviceServices = new HashMap<>();

    private static BluetoothLeScanner scanner = null;
    private static ScanCallback scanCallback = null;
    private static boolean isScanning = false;

    /**
     * Start native BLE scanning in the background.
     * This runs in parallel with Qt's Bluetooth scanning.
     */
    public static void startScan(Context context) {
        Log.d(TAG, "Starting native BLE scan");

        BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager == null) {
            Log.e(TAG, "BluetoothManager is null");
            return;
        }

        BluetoothAdapter adapter = bluetoothManager.getAdapter();
        if (adapter == null || !adapter.isEnabled()) {
            Log.e(TAG, "Bluetooth adapter is null or not enabled");
            return;
        }

        scanner = adapter.getBluetoothLeScanner();
        if (scanner == null) {
            Log.e(TAG, "BluetoothLeScanner is null");
            return;
        }

        // Clear previous scan results
        deviceServices.clear();

        // Create scan callback
        scanCallback = new ScanCallback() {
            @Override
            public void onScanResult(int callbackType, ScanResult result) {
                super.onScanResult(callbackType, result);
                processScanResult(result);
            }

            @Override
            public void onBatchScanResults(List<ScanResult> results) {
                super.onBatchScanResults(results);
                for (ScanResult result : results) {
                    processScanResult(result);
                }
            }

            @Override
            public void onScanFailed(int errorCode) {
                super.onScanFailed(errorCode);
                Log.e(TAG, "Scan failed with error code: " + errorCode);
            }
        };

        // Scan settings for maximum performance
        ScanSettings settings = new ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                .setReportDelay(0) // Report immediately
                .build();

        // Start scanning (no filters = all devices)
        try {
            scanner.startScan(null, settings, scanCallback);
            isScanning = true;
            Log.d(TAG, "Native BLE scan started successfully");
        } catch (SecurityException e) {
            Log.e(TAG, "SecurityException starting scan: " + e.getMessage());
        }
    }

    /**
     * Stop native BLE scanning.
     */
    public static void stopScan() {
        if (scanner != null && scanCallback != null && isScanning) {
            Log.d(TAG, "Stopping native BLE scan");
            try {
                scanner.stopScan(scanCallback);
                isScanning = false;
                Log.d(TAG, "Native BLE scan stopped");
            } catch (SecurityException e) {
                Log.e(TAG, "SecurityException stopping scan: " + e.getMessage());
            }
        }
    }

    /**
     * Process a scan result and extract all advertised service UUIDs.
     */
    private static void processScanResult(ScanResult result) {
        BluetoothDevice device = result.getDevice();
        String address = device.getAddress();

        // Get service UUIDs from scan record
        List<ParcelUuid> serviceUuids = result.getScanRecord() != null ?
                                        result.getScanRecord().getServiceUuids() : null;

        if (serviceUuids != null && !serviceUuids.isEmpty()) {
            List<String> uuidStrings = new ArrayList<>();
            for (ParcelUuid uuid : serviceUuids) {
                uuidStrings.add(uuid.getUuid().toString());
            }

            // Store or update services for this device
            List<String> existingServices = deviceServices.get(address);
            if (existingServices == null) {
                deviceServices.put(address, uuidStrings);
                Log.d(TAG, "New device " + address + " with " + uuidStrings.size() + " services: " + uuidStrings);
            } else {
                // Merge services (some might be discovered progressively)
                for (String uuid : uuidStrings) {
                    if (!existingServices.contains(uuid)) {
                        existingServices.add(uuid);
                        Log.d(TAG, "Device " + address + " added service: " + uuid);
                    }
                }
            }
        }
    }

    /**
     * Get all discovered service UUIDs for a specific device.
     * Called from Qt C++ via JNI.
     *
     * @param macAddress Device MAC address (e.g., "AA:BB:CC:DD:EE:FF")
     * @return Comma-separated list of service UUIDs, or empty string if not found
     */
    public static String getDeviceServices(String macAddress) {
        List<String> services = deviceServices.get(macAddress);
        if (services == null || services.isEmpty()) {
            Log.d(TAG, "No services found for device: " + macAddress);
            return "";
        }

        // Return as comma-separated string for easy JNI transfer
        String result = String.join(",", services);
        Log.d(TAG, "Returning services for " + macAddress + ": " + result);
        return result;
    }

    /**
     * Get all discovered device addresses.
     * Useful for debugging.
     */
    public static String getAllDevices() {
        return String.join(",", deviceServices.keySet());
    }
}
