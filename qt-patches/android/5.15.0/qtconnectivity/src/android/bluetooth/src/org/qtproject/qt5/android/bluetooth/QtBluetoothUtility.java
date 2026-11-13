// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt5.android.bluetooth;

import android.os.Build;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.res.AssetManager;
import android.content.res.XmlResourceParser;
import android.util.Log;

import org.xmlpull.v1.XmlPullParser;

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
    // without Location so a parser problem cannot reintroduce a location prompt.
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

        XmlResourceParser xmlParser = null;
        try {
            final AssetManager assetManager =
                    qtContext.createPackageContext(qtContext.getPackageName(), 0).getAssets();
            xmlParser = assetManager.openXmlResourceParser(0, "AndroidManifest.xml");

            int eventType = xmlParser.getEventType();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                if (eventType == XmlPullParser.START_TAG
                        && xmlParser.getName().equals("uses-permission")) {
                    String permissionName = null;
                    int usesPermissionFlags = 0;

                    for (int i = 0; i < xmlParser.getAttributeCount(); i++) {
                        String attributeName = xmlParser.getAttributeName(i);
                        if (attributeName.equals("name")) {
                            permissionName = xmlParser.getAttributeValue(i);
                        } else if (attributeName.equals("usesPermissionFlags")) {
                            String flagValue = xmlParser.getAttributeValue(i);
                            if (flagValue.startsWith("0x")) {
                                usesPermissionFlags = Integer.parseInt(flagValue.substring(2), 16);
                            } else {
                                usesPermissionFlags = Integer.parseInt(flagValue);
                            }
                        }
                    }
                    if (BLUETOOTH_SCAN_PERMISSION.equals(permissionName)) {
                        scanRequiresLocation =
                                (usesPermissionFlags & REQUESTED_PERMISSION_NEVER_FOR_LOCATION) == 0;
                        isScanRequiresLocationChecked = true;
                        Log.d(TAG, "BLUETOOTH_SCAN requires Location: " + scanRequiresLocation);
                        return scanRequiresLocation;
                    }
                }
                eventType = xmlParser.nextToken();
            }
        } catch (Exception ex) {
            Log.w(TAG, "An error occurred while checking Bluetooth's Location need: " + ex);
            scanRequiresLocation = false;
        } finally {
            if (xmlParser != null)
                xmlParser.close();
        }

        Log.d(TAG, "BLUETOOTH_SCAN permission not found");
        isScanRequiresLocationChecked = true;
        return scanRequiresLocation;
    }
}
