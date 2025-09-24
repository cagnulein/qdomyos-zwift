package org.cagnulen.qdomyoszwift;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.view.WindowManager;
import org.cagnulen.qdomyoszwift.QLog;

public class VirtualGearingBridge {
    private static final String TAG = "VirtualGearingBridge";

    public static boolean isAccessibilityServiceEnabled(Context context) {
        String settingValue = Settings.Secure.getString(
            context.getContentResolver(),
            Settings.Secure.ENABLED_ACCESSIBILITY_SERVICES);

        QLog.d(TAG, "Enabled accessibility services: " + settingValue);

        if (settingValue != null) {
            TextUtils.SimpleStringSplitter splitter = new TextUtils.SimpleStringSplitter(':');
            splitter.setString(settingValue);
            while (splitter.hasNext()) {
                String service = splitter.next();
                QLog.d(TAG, "Checking service: " + service);
                if (service.contains("org.cagnulen.qdomyoszwift/.VirtualGearingService") ||
                    service.contains("VirtualGearingService")) {
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
        QLog.d(TAG, "Simulating shift up with app-specific coordinates");
        VirtualGearingService.shiftUpSmart();
    }

    public static void simulateShiftDown() {
        QLog.d(TAG, "Simulating shift down with app-specific coordinates");
        VirtualGearingService.shiftDownSmart();
    }

    public static String getCurrentAppPackageName(Context context) {
        try {
            ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
            if (activityManager != null) {
                ActivityManager.RunningAppProcessInfo myProcess = new ActivityManager.RunningAppProcessInfo();
                ActivityManager.getMyMemoryState(myProcess);

                // For Android 5.0+ we should use UsageStatsManager, but for simplicity
                // we use a more direct approach via current foreground process
                // In a complete implementation we should use UsageStatsManager

                // For now return null and let the service detect the app
                return null;
            }
        } catch (Exception e) {
            QLog.e(TAG, "Error getting current app package name", e);
        }
        return null;
    }

    public static int[] getScreenSize(Context context) {
        try {
            WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            DisplayMetrics displayMetrics = new DisplayMetrics();
            windowManager.getDefaultDisplay().getMetrics(displayMetrics);
            return new int[]{displayMetrics.widthPixels, displayMetrics.heightPixels};
        } catch (Exception e) {
            QLog.e(TAG, "Error getting screen size", e);
            return new int[]{1080, 1920}; // Default fallback
        }
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