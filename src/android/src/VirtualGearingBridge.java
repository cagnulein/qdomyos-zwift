package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.content.Intent;
import android.provider.Settings;
import android.text.TextUtils;
import org.cagnulen.qdomyoszwift.QLog;

public class VirtualGearingBridge {
    private static final String TAG = "VirtualGearingBridge";

    public static boolean isAccessibilityServiceEnabled(Context context) {
        String settingValue = Settings.Secure.getString(
            context.getContentResolver(),
            Settings.Secure.ENABLED_ACCESSIBILITY_SERVICES);

        if (settingValue != null) {
            TextUtils.SimpleStringSplitter splitter = new TextUtils.SimpleStringSplitter(':');
            splitter.setString(settingValue);
            while (splitter.hasNext()) {
                String service = splitter.next();
                if (service.contains("VirtualGearingService")) {
                    QLog.d(TAG, "VirtualGearingService is enabled");
                    return true;
                }
            }
        }
        QLog.d(TAG, "VirtualGearingService is not enabled");
        return false;
    }

    public static void openAccessibilitySettings(Context context) {
        try {
            Intent intent = new Intent(Settings.ACTION_ACCESSIBILITY_SETTINGS);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent);
            QLog.d(TAG, "Opened accessibility settings");
        } catch (Exception e) {
            QLog.e(TAG, "Failed to open accessibility settings", e);
        }
    }

    public static void simulateShiftUp() {
        QLog.d(TAG, "Simulating shift up");
        VirtualGearingService.shiftUp();
    }

    public static void simulateShiftDown() {
        QLog.d(TAG, "Simulating shift down");
        VirtualGearingService.shiftDown();
    }

    public static void simulateTouch(int x, int y) {
        QLog.d(TAG, "Simulating touch at (" + x + ", " + y + ")");
        VirtualGearingService.simulateKeypress(x, y);
    }

    public static boolean isServiceRunning() {
        boolean running = VirtualGearingService.isServiceEnabled();
        QLog.d(TAG, "Service running: " + running);
        return running;
    }
}