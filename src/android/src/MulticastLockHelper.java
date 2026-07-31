package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.net.wifi.WifiManager;

/**
 * Holds a WifiManager.MulticastLock so that incoming multicast packets (mDNS
 * service discovery queries) are delivered to the app. Android drops them by
 * default to save power, which makes QZ invisible to apps looking for it via
 * mDNS (OpenBikeControl/MyWhoosh Link, DirCon).
 *
 * This is intentionally independent of ForegroundService: that service is only
 * started when the "android notification" setting is enabled and a device is
 * connected, while mDNS advertising starts as soon as the app launches.
 */
public class MulticastLockHelper {
    private static WifiManager.MulticastLock multicastLock;

    public static synchronized void acquire(Context context) {
        if (multicastLock != null && multicastLock.isHeld()) {
            return;
        }

        try {
            WifiManager wifiManager =
                (WifiManager) context.getApplicationContext().getSystemService(Context.WIFI_SERVICE);
            if (wifiManager == null) {
                QLog.d("MulticastLockHelper", "WifiManager unavailable; cannot acquire multicast lock");
                return;
            }

            multicastLock = wifiManager.createMulticastLock("QZ:mdns");
            multicastLock.setReferenceCounted(false);
            multicastLock.acquire();
            QLog.d("MulticastLockHelper", "Acquired multicast lock");
        } catch (Exception e) {
            QLog.e("MulticastLockHelper", "Failed to acquire multicast lock", e);
        }
    }

    public static synchronized void release() {
        try {
            if (multicastLock != null && multicastLock.isHeld()) {
                multicastLock.release();
                QLog.d("MulticastLockHelper", "Released multicast lock");
            }
        } catch (Exception e) {
            QLog.e("MulticastLockHelper", "Failed to release multicast lock", e);
        } finally {
            multicastLock = null;
        }
    }
}
