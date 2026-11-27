// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt5.android.bluetooth;

import android.os.Build;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.util.Log;

class QtBluetoothUtility {

    private static final String TAG = "QtBluetoothUtility";
    private static final String BLUETOOTH_SCAN_PERMISSION = "android.permission.BLUETOOTH_SCAN";
    // PackageInfo.REQUESTED_PERMISSION_NEVER_FOR_LOCATION was added in API 31.
    private static final int REQUESTED_PERMISSION_NEVER_FOR_LOCATION = 0x10000;

    // The merged manifest is stable for the life of the process, so cache this check.
    private static boolean isScanRequiresLocationChecked = false;
    private static boolean scanRequiresLocation = true;

    // API < 31 always requires Location. On API >= 31, Location is required unless
    // BLUETOOTH_SCAN declares neverForLocation. On unexpected failures, allow scanning
    // without Location so a package query failure cannot reintroduce a location prompt.
    public static synchronized boolean bluetoothScanRequiresLocation(final Context qtContext)
    {
        Log.d(TAG, "Checking if Location is required for bluetooth scan");
        if (isScanRequiresLocationChecked) {
            Log.d(TAG, "Using cached result for scan needing location: " + scanRequiresLocation);
            return scanRequiresLocation;
        }

        if (Build.VERSION.SDK_INT < 31) {
            scanRequiresLocation = true;
            isScanRequiresLocationChecked = true;
            return scanRequiresLocation;
        }
        if (qtContext == null) {
            Log.w(TAG, "No context object provided");
            return false;
        }

        // QTBUG-133975: AssetManager may open a split APK's manifest instead of
        // the base manifest. Query the installed package's permissions instead.
        try {
            PackageInfo pi = qtContext.getPackageManager().getPackageInfo(
                    qtContext.getPackageName(), PackageManager.GET_PERMISSIONS);
            String[] permissions = pi.requestedPermissions;
            int[] flags = pi.requestedPermissionsFlags;
            if (permissions != null && flags != null) {
                for (int i = 0; i < permissions.length && i < flags.length; ++i) {
                    if (BLUETOOTH_SCAN_PERMISSION.equals(permissions[i])) {
                        scanRequiresLocation =
                                (flags[i] & REQUESTED_PERMISSION_NEVER_FOR_LOCATION) == 0;
                        isScanRequiresLocationChecked = true;
                        Log.d(TAG, "BLUETOOTH_SCAN requires Location: " + scanRequiresLocation);
                        return scanRequiresLocation;
                    }
                }
            }
        } catch (Exception ex) {
            Log.w(TAG, "An error occurred while checking Bluetooth's Location need: " + ex);
            scanRequiresLocation = false;
        }

        Log.d(TAG, "BLUETOOTH_SCAN permission not found");
        isScanRequiresLocationChecked = true;
        return scanRequiresLocation;
    }
}
