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
        // We don't need to respond to accessibility events for keypress simulation
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

    // Predefined touch coordinates for popular cycling apps
    public static void shiftUp() {
        // Default coordinates for shift up - apps can customize these
        simulateKeypress(100, 200);
    }

    public static void shiftDown() {
        // Default coordinates for shift down - apps can customize these
        simulateKeypress(100, 300);
    }
}