package org.cagnulen.qdomyoszwift;

import android.util.Log;

/**
 * QLog - Wrapper for Android's Log class that redirects logs to Qt's logging system
 * Usage: import org.cagnulen.qdomyoszwift.Log;
 */
public class QLog {
    public static native void sendToQt(int level, String tag, String message);

    static {
        try {
            // Try to load the native library if needed
            System.loadLibrary("qtlogging_native");
        } catch (UnsatisfiedLinkError e) {
            // Library might be loaded elsewhere, or will be loaded later
            Log.w("QLog", "Native library not loaded yet: " + e.getMessage());
        }
    }

    // Debug level methods
    public static int d(String tag, String msg) {
        try {
            sendToQt(3, tag, msg);
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.d(tag, msg);
    }

    public static int d(String tag, String msg, Throwable tr) {
        try {
            sendToQt(3, tag, msg + '\n' + Log.getStackTraceString(tr));
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.d(tag, msg, tr);
    }

    // Error level methods
    public static int e(String tag, String msg) {
        try {
            sendToQt(6, tag, msg);
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.e(tag, msg);
    }

    public static int e(String tag, String msg, Throwable tr) {
        try {
            sendToQt(6, tag, msg + '\n' + Log.getStackTraceString(tr));
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.e(tag, msg, tr);
    }

    // Info level methods
    public static int i(String tag, String msg) {
        try {
            sendToQt(4, tag, msg);
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.i(tag, msg);
    }

    public static int i(String tag, String msg, Throwable tr) {
        try {
            sendToQt(4, tag, msg + '\n' + Log.getStackTraceString(tr));
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.i(tag, msg, tr);
    }

    // Verbose level methods
    public static int v(String tag, String msg) {
        try {
            sendToQt(2, tag, msg);
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.v(tag, msg);
    }

    public static int v(String tag, String msg, Throwable tr) {
        try {
            sendToQt(2, tag, msg + '\n' + Log.getStackTraceString(tr));
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.v(tag, msg, tr);
    }

    // Warning level methods
    public static int w(String tag, String msg) {
        try {
            sendToQt(5, tag, msg);
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.w(tag, msg);
    }

    public static int w(String tag, String msg, Throwable tr) {
        try {
            sendToQt(5, tag, msg + '\n' + Log.getStackTraceString(tr));
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.w(tag, msg, tr);
    }

    public static int w(String tag, Throwable tr) {
        try {
            sendToQt(5, tag, Log.getStackTraceString(tr));
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.w(tag, tr);
    }

    // What a Terrible Failure: Report an exception that should never happen
    public static int wtf(String tag, String msg) {
        try {
            sendToQt(7, tag, "WTF: " + msg);
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.wtf(tag, msg);
    }

    public static int wtf(String tag, Throwable tr) {
        try {
            sendToQt(7, tag, "WTF: " + Log.getStackTraceString(tr));
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
        return Log.wtf(tag, tr);
    }

    public static int wtf(String tag, String msg, Throwable tr) {
        try {
            sendToQt(7, tag, "WTF: " + msg + '\n' + Log.getStackTraceString(tr));
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
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
        try {
            sendToQt(priority, tag, msg);
        } catch (UnsatisfiedLinkError e) {
            // Native library not available, continue with Android logging only
        }
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
