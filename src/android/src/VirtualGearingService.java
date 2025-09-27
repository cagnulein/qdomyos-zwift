package org.cagnulen.qdomyoszwift;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.GestureDescription;
import android.graphics.Path;
import android.view.ViewConfiguration;
import android.view.accessibility.AccessibilityEvent;
import org.cagnulen.qdomyoszwift.QLog;

public class VirtualGearingService extends AccessibilityService {
    private static final String TAG = "VirtualGearingService";
    private static VirtualGearingService instance;

    @Override
    public void onCreate() {
        super.onCreate();
        instance = this;
        QLog.d(TAG, "VirtualGearingService created");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        instance = null;
        QLog.d(TAG, "VirtualGearingService destroyed");
    }

    @Override
    public void onAccessibilityEvent(AccessibilityEvent event) {
        // Capture foreground app package name for smart coordinates
        if (event != null && event.getPackageName() != null) {
            String packageName = event.getPackageName().toString();
            if (!packageName.equals(currentPackageName)) {
                currentPackageName = packageName;
                QLog.d(TAG, "App changed to: " + packageName);
            }
        }
    }

    @Override
    public void onInterrupt() {
        QLog.d(TAG, "VirtualGearingService interrupted");
    }

    public static boolean isServiceEnabled() {
        return instance != null;
    }

    public static void simulateKeypress(int x, int y) {
        if (instance == null) {
            QLog.w(TAG, "Service not enabled, cannot simulate keypress");
            return;
        }

        try {
            GestureDescription.Builder gestureBuilder = new GestureDescription.Builder();
            Path path = new Path();
            path.moveTo(x, y);
            path.lineTo(x + 1, y);

            GestureDescription.StrokeDescription stroke = new GestureDescription.StrokeDescription(
                path, 0, ViewConfiguration.getTapTimeout(), false);
            gestureBuilder.addStroke(stroke);

            instance.dispatchGesture(gestureBuilder.build(), null, null);
            QLog.d(TAG, "Simulated keypress at (" + x + ", " + y + ")");
        } catch (Exception e) {
            QLog.e(TAG, "Error simulating keypress", e);
        }
    }

    // Legacy methods for backward compatibility
    public static void shiftUp() {
        QLog.d(TAG, "Using legacy shiftUp - consider using shiftUpSmart()");
        simulateKeypress(100, 200);
    }

    public static void shiftDown() {
        QLog.d(TAG, "Using legacy shiftDown - consider using shiftDownSmart()");
        simulateKeypress(100, 300);
    }

    // New smart methods with app-specific coordinates
    public static void shiftUpSmart() {
        if (instance == null) {
            QLog.w(TAG, "Service not enabled, cannot simulate smart shift up");
            return;
        }

        try {
            // Try to detect app from package name of last AccessibilityEvent
            String currentPackage = getCurrentPackageName();
            AppConfiguration.AppConfig config = AppConfiguration.getConfigForPackage(currentPackage);

            // Calculate coordinates based on screen dimensions
            int[] screenSize = getScreenSize();
            int x = config.shiftUp.getX(screenSize[0]);
            int y = config.shiftUp.getY(screenSize[1]);

            QLog.d(TAG, "Smart shift up for " + config.appName + " at (" + x + ", " + y + ")");
            simulateKeypress(x, y);
        } catch (Exception e) {
            QLog.e(TAG, "Error in shiftUpSmart, falling back to legacy", e);
            shiftUp();
        }
    }

    public static void shiftDownSmart() {
        if (instance == null) {
            QLog.w(TAG, "Service not enabled, cannot simulate smart shift down");
            return;
        }

        try {
            String currentPackage = getCurrentPackageName();
            AppConfiguration.AppConfig config = AppConfiguration.getConfigForPackage(currentPackage);

            int[] screenSize = getScreenSize();
            int x = config.shiftDown.getX(screenSize[0]);
            int y = config.shiftDown.getY(screenSize[1]);

            QLog.d(TAG, "Smart shift down for " + config.appName + " at (" + x + ", " + y + ")");
            simulateKeypress(x, y);
        } catch (Exception e) {
            QLog.e(TAG, "Error in shiftDownSmart, falling back to legacy", e);
            shiftDown();
        }
    }

    private static String currentPackageName = null;

    private static String getCurrentPackageName() {
        return currentPackageName != null ? currentPackageName : "unknown";
    }

    public static int[] getScreenSize() {
        if (instance != null) {
            try {
                android.content.res.Resources resources = instance.getResources();
                android.util.DisplayMetrics displayMetrics = resources.getDisplayMetrics();
                int width = displayMetrics.widthPixels;
                int height = displayMetrics.heightPixels;
                QLog.d(TAG, "Screen size: " + width + "x" + height + " (density=" + displayMetrics.density + ")");
                return new int[]{width, height};
            } catch (Exception e) {
                QLog.e(TAG, "Error getting screen size from service", e);
            }
        }
        QLog.w(TAG, "Using fallback screen size");
        return new int[]{1080, 1920}; // Default fallback
    }
}