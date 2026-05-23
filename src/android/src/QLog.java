package org.cagnulen.qdomyoszwift;

import android.util.Log;

/**
 * QLog - Wrapper for Android's Log class that redirects logs to Qt's logging system
 * Usage: import org.cagnulen.qdomyoszwift.Log;
 */
public class QLog {
    private static volatile boolean nativeLoggingAvailable = false;
    private static volatile boolean nativeLoggingDisabled = false;

    public static native void sendToQt(int level, String tag, String message);

    static {
        try {
            // Try to load the native library if needed
            System.loadLibrary("qtlogging_native");
            nativeLoggingAvailable = true;
        } catch (UnsatisfiedLinkError e) {
            // Library might be loaded elsewhere, or will be loaded later
            nativeLoggingAvailable = false;
            nativeLoggingDisabled = true;
            Log.w("QLog", "Native library not loaded yet: " + e.getMessage());
        }
    }

    private static void forwardToQt(int level, String tag, String message) {
        if (!nativeLoggingAvailable || nativeLoggingDisabled) {
            return;
        }

        try {
            sendToQt(level, tag, message);
        } catch (UnsatisfiedLinkError e) {
            nativeLoggingDisabled = true;
            nativeLoggingAvailable = false;
            Log.w("QLog", "Disabling native logging bridge: " + e.getMessage());
        }
    }

    // Debug level methods
    public static int d(String tag, String msg) {
        forwardToQt(3, tag, msg);
        return Log.d(tag, msg);
    }

    public static int d(String tag, String msg, Throwable tr) {
        forwardToQt(3, tag, msg + '\n' + Log.getStackTraceString(tr));
        return Log.d(tag, msg, tr);
    }

    // Error level methods
    public static int e(String tag, String msg) {
        forwardToQt(6, tag, msg);
        return Log.e(tag, msg);
    }

    public static int e(String tag, String msg, Throwable tr) {
        forwardToQt(6, tag, msg + '\n' + Log.getStackTraceString(tr));
        return Log.e(tag, msg, tr);
    }

    // Info level methods
    public static int i(String tag, String msg) {
        forwardToQt(4, tag, msg);
        return Log.i(tag, msg);
    }

    public static int i(String tag, String msg, Throwable tr) {
        forwardToQt(4, tag, msg + '\n' + Log.getStackTraceString(tr));
        return Log.i(tag, msg, tr);
    }

    // Verbose level methods
    public static int v(String tag, String msg) {
        forwardToQt(2, tag, msg);
        return Log.v(tag, msg);
    }

    public static int v(String tag, String msg, Throwable tr) {
        forwardToQt(2, tag, msg + '\n' + Log.getStackTraceString(tr));
        return Log.v(tag, msg, tr);
    }

    // Warning level methods
    public static int w(String tag, String msg) {
        forwardToQt(5, tag, msg);
        return Log.w(tag, msg);
    }

    public static int w(String tag, String msg, Throwable tr) {
        forwardToQt(5, tag, msg + '\n' + Log.getStackTraceString(tr));
        return Log.w(tag, msg, tr);
    }

    public static int w(String tag, Throwable tr) {
        forwardToQt(5, tag, Log.getStackTraceString(tr));
        return Log.w(tag, tr);
    }

    // What a Terrible Failure: Report an exception that should never happen
    public static int wtf(String tag, String msg) {
        forwardToQt(7, tag, "WTF: " + msg);
        return Log.wtf(tag, msg);
    }

    public static int wtf(String tag, Throwable tr) {
        forwardToQt(7, tag, "WTF: " + Log.getStackTraceString(tr));
        return Log.wtf(tag, tr);
    }

    public static int wtf(String tag, String msg, Throwable tr) {
        forwardToQt(7, tag, "WTF: " + msg + '\n' + Log.getStackTraceString(tr));
        return Log.wtf(tag, msg, tr);
    }

    // Utility methods
    public static String getStackTraceString(Throwable tr) {
        return Log.getStackTraceString(tr);
    }

    public static boolean isLoggable(String tag, int level) {
        return Log.isLoggable(tag, level);
    }

    // Additional utility methods
    public static int println(int priority, String tag, String msg) {
        forwardToQt(priority, tag, msg);
        return Log.println(priority, tag, msg);
    }

    // API Level 28+ (Android 9+) methods
    public static RuntimeException getStackTraceElement() {
        try {
            return (RuntimeException) Log.class.getMethod("getStackTraceElement").invoke(null);
        } catch (Exception e) {
            return new RuntimeException("QLog: Failed to get stack trace element");
        }
    }
}
