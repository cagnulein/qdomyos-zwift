package org.cagnulen.qdomyoszwift;

import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.view.DisplayCutout;
import org.qtproject.qt.android.bindings.QtActivity;

public class CustomQtActivity extends QtActivity {
    private static final String TAG = "CustomQtActivity";

    // Declare the native method that will be implemented in C++
    private static native void onInsetsChanged(int top, int bottom, int left, int right);

    @Override
    public void onCreate(Bundle savedInstanceState) {
        // Ensure QtLoader has the extras it expects even if Intent/meta-data are empty
        try {
            android.content.Intent intent = getIntent();
            android.os.Bundle extras = intent.getExtras();
            if (extras == null) extras = new android.os.Bundle();
            // Message strings as resource IDs
            extras.putInt("android.app.ministro_not_found_msg", org.cagnulen.qdomyoszwift.R.string.ministro_not_found_msg);
            extras.putInt("android.app.ministro_needed_msg", org.cagnulen.qdomyoszwift.R.string.ministro_needed_msg);
            extras.putInt("android.app.fatal_error_msg", org.cagnulen.qdomyoszwift.R.string.fatal_error_msg);
            extras.putInt("android.app.unsupported_android_version", org.cagnulen.qdomyoszwift.R.string.unsupported_android_version);
            // Arrays as resource IDs
            extras.putInt("android.app.qt_sources_resource_id", org.cagnulen.qdomyoszwift.R.array.qt_sources);
            extras.putInt("android.app.qt_libs_resource_id", org.cagnulen.qdomyoszwift.R.array.qt_libs);
            extras.putInt("android.app.bundled_libs_resource_id", org.cagnulen.qdomyoszwift.R.array.bundled_libs);
            extras.putInt("android.app.load_local_libs_resource_id", org.cagnulen.qdomyoszwift.R.array.load_local_libs);
            // Configuration values
            extras.putBoolean("android.app.use_local_qt_libs", false);
            extras.putString("android.app.lib_name", getString(org.cagnulen.qdomyoszwift.R.string.qt_lib_name));
            extras.putString("android.app.repository", getString(org.cagnulen.qdomyoszwift.R.string.qt_repository));
            extras.putString("android.app.libs_prefix", getString(org.cagnulen.qdomyoszwift.R.string.qt_libs_prefix));
            extras.putString("android.app.load_local_jars", getString(org.cagnulen.qdomyoszwift.R.string.qt_load_local_jars));
            extras.putString("android.app.static_init_classes", getString(org.cagnulen.qdomyoszwift.R.string.qt_static_init_classes));
            intent.replaceExtras(extras);
        } catch (Throwable t) {
            Log.e(TAG, "Failed to seed Qt extras", t);
        }

        // Log meta-data resource IDs before Qt loads to diagnose 0x0 IDs
        try {
            android.content.pm.ActivityInfo ai = getPackageManager().getActivityInfo(getComponentName(), android.content.pm.PackageManager.GET_META_DATA);
            android.os.Bundle md = ai.metaData;
            if (md == null) {
                Log.e(TAG, "Activity meta-data bundle is null");
            } else {
                String[] keys = new String[] {
                        "android.app.lib_name",
                        "android.app.repository",
                        "android.app.libs_prefix",
                        "android.app.load_local_jars",
                        "android.app.static_init_classes",
                        "android.app.qt_sources_resource_id",
                        "android.app.qt_libs_resource_id",
                        "android.app.bundled_libs_resource_id",
                        "android.app.load_local_libs_resource_id",
                        "android.app.ministro_not_found_msg",
                        "android.app.ministro_needed_msg",
                        "android.app.fatal_error_msg",
                        "android.app.unsupported_android_version"
                };
                for (String k : keys) {
                    int id = md.getInt(k);
                    Object val = md.get(k);
                    Log.i(TAG, "meta-data key=" + k + " getInt=" + id + " raw=" + val);
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Error inspecting meta-data", e);
        }

        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate: CustomQtActivity initialized");

        // This tells the OS that we want to handle the display cutout area ourselves
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        // This is the core of the new solution. We set a listener on the main view.
        // The OS will call this listener whenever the insets change (e.g., on rotation).
        final View decorView = getWindow().getDecorView();
        decorView.setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener() {
            @Override
            public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
                final float density = getResources().getDisplayMetrics().density;
                int top = 0;
                int bottom = 0;
                int left = 0;
                int right = 0;

                if (density > 0) {
                    // Use system window insets as primary source
                    top = Math.round(insets.getSystemWindowInsetTop() / density);
                    bottom = Math.round(insets.getSystemWindowInsetBottom() / density);
                    left = Math.round(insets.getSystemWindowInsetLeft() / density);
                    right = Math.round(insets.getSystemWindowInsetRight() / density);

                    // For API 28+, also check display cutout for additional padding
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                        DisplayCutout cutout = insets.getDisplayCutout();
                        if (cutout != null) {
                            // Use the maximum between system window inset and cutout safe inset
                            left = Math.max(left, Math.round(cutout.getSafeInsetLeft() / density));
                            right = Math.max(right, Math.round(cutout.getSafeInsetRight() / density));
                            top = Math.max(top, Math.round(cutout.getSafeInsetTop() / density));
                            bottom = Math.max(bottom, Math.round(cutout.getSafeInsetBottom() / density));
                        }
                    }
                }

                Log.d(TAG, "onApplyWindowInsets - Top:" + top + " Bottom:" + bottom + " Left:" + left + " Right:" + right);
                Log.d(TAG, "Raw insets - SystemTop:" + insets.getSystemWindowInsetTop() + 
                          " SystemBottom:" + insets.getSystemWindowInsetBottom() + 
                          " SystemLeft:" + insets.getSystemWindowInsetLeft() + 
                          " SystemRight:" + insets.getSystemWindowInsetRight());
                Log.d(TAG, "Stable insets - StableTop:" + insets.getStableInsetTop() + 
                          " StableBottom:" + insets.getStableInsetBottom() + 
                          " StableLeft:" + insets.getStableInsetLeft() + 
                          " StableRight:" + insets.getStableInsetRight());
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                    DisplayCutout cutout = insets.getDisplayCutout();
                    if (cutout != null) {
                        Log.d(TAG, "Cutout insets - Top:" + cutout.getSafeInsetTop() + 
                              " Bottom:" + cutout.getSafeInsetBottom() + 
                              " Left:" + cutout.getSafeInsetLeft() + 
                              " Right:" + cutout.getSafeInsetRight());
                    }
                }

                // Push the new, correct inset values to the C++ layer
                onInsetsChanged(top, bottom, left, right);

                return v.onApplyWindowInsets(insets);
            }
        });
    }

    // This method is still needed for the QML check
    public static int getApiLevel() {
        return Build.VERSION.SDK_INT;
    }
}
