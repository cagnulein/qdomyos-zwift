package org.cagnulen.qdomyoszwift;

import org.cagnulen.qdomyoszwift.QLog;

public class AppConfiguration {
    private static final String TAG = "AppConfiguration";

    public static class TouchCoordinate {
        public final double xPercent;
        public final double yPercent;

        public TouchCoordinate(double xPercent, double yPercent) {
            this.xPercent = xPercent;
            this.yPercent = yPercent;
        }

        public int getX(int screenWidth) {
            return (int) (screenWidth * xPercent);
        }

        public int getY(int screenHeight) {
            return (int) (screenHeight * yPercent);
        }
    }

    public static class AppConfig {
        public final String appName;
        public final String packageName;
        public final TouchCoordinate shiftUp;
        public final TouchCoordinate shiftDown;

        public AppConfig(String appName, String packageName, TouchCoordinate shiftUp, TouchCoordinate shiftDown) {
            this.appName = appName;
            this.packageName = packageName;
            this.shiftUp = shiftUp;
            this.shiftDown = shiftDown;
        }
    }

    // Predefined configurations based on SwiftControl
    private static final AppConfig[] SUPPORTED_APPS = {
        // MyWhoosh - coordinates from SwiftControl repository
        new AppConfig(
            "MyWhoosh",
            "com.mywhoosh.whooshgame",
            new TouchCoordinate(0.98, 0.94),  // Shift Up - bottom right corner
            new TouchCoordinate(0.80, 0.94)   // Shift Down - more to the left
        ),

        // IndieVelo / TrainingPeaks
        new AppConfig(
            "IndieVelo",
            "com.indieVelo.client",
            new TouchCoordinate(0.66, 0.74),  // Shift Up - center right
            new TouchCoordinate(0.575, 0.74)  // Shift Down - center left
        ),

        // Biketerra.com
        new AppConfig(
            "Biketerra",
            "biketerra",
            new TouchCoordinate(0.8, 0.5),    // Generic coordinates for now
            new TouchCoordinate(0.2, 0.5)
        ),

        // Default configuration for unrecognized apps
        new AppConfig(
            "Default",
            "*",
            new TouchCoordinate(0.85, 0.9),   // Conservative coordinates
            new TouchCoordinate(0.15, 0.9)
        )
    };

    public static AppConfig getConfigForPackage(String packageName) {
        // Use custom coordinates from settings instead of hardcoded values
        return getCurrentConfig();
    }

    // Get current configuration from user settings
    public static AppConfig getCurrentConfig() {
        try {
            double shiftUpX = VirtualGearingBridge.getVirtualGearingShiftUpX();
            double shiftUpY = VirtualGearingBridge.getVirtualGearingShiftUpY();
            double shiftDownX = VirtualGearingBridge.getVirtualGearingShiftDownX();
            double shiftDownY = VirtualGearingBridge.getVirtualGearingShiftDownY();
            int appIndex = VirtualGearingBridge.getVirtualGearingApp();

            String appName = "Custom";
            if (appIndex >= 0 && appIndex < SUPPORTED_APPS.length) {
                appName = SUPPORTED_APPS[appIndex].appName;
            }

            QLog.d(TAG, "Using custom coordinates: shiftUp(" + shiftUpX + "," + shiftUpY +
                        ") shiftDown(" + shiftDownX + "," + shiftDownY + ") for " + appName);

            return new AppConfig(
                appName,
                "*", // Package name not relevant for custom config
                new TouchCoordinate(shiftUpX, shiftUpY),
                new TouchCoordinate(shiftDownX, shiftDownY)
            );
        } catch (Exception e) {
            QLog.e(TAG, "Error getting custom config, using fallback", e);
            return getDefaultConfig();
        }
    }

    public static AppConfig getDefaultConfig() {
        return SUPPORTED_APPS[SUPPORTED_APPS.length - 1]; // Last element is the default
    }

    public static AppConfig[] getAllConfigs() {
        return SUPPORTED_APPS;
    }
}