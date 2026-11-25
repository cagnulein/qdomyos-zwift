package org.cagnulen.qdomyoszwift;

import android.Manifest;
import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;
import android.content.pm.PackageManager;
import android.location.LocationManager;
import android.os.Build;
import android.os.Bundle;
import java.lang.reflect.Method;

/** Debug-only regression test of the actual Qt helper in an installed split package. */
public class SplitBluetoothTestInstrumentation extends Instrumentation {
    @Override
    public void onCreate(Bundle arguments) {
        super.onCreate(arguments);
        start();
    }

    @Override
    public void onStart() {
        Bundle result = new Bundle();
        try {
            Context context = getTargetContext();
            int sdk = Build.VERSION.SDK_INT;
            if (sdk < 30) throw new AssertionError("Test requires API 30+");
            String[] splits = context.getApplicationInfo().splitSourceDirs;
            if (splits == null || splits.length == 0)
                throw new AssertionError("Not a split APK installation");
            result.putInt("split_count", splits.length);

            if (context.checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION)
                    == PackageManager.PERMISSION_GRANTED
                    || context.checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION)
                    == PackageManager.PERMISSION_GRANTED)
                throw new AssertionError("Location must not be granted");
            LocationManager location = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
            if (location == null || location.isLocationEnabled())
                throw new AssertionError("System Location must be disabled");
            if (sdk >= 31) {
                for (String permission : new String[]{"android.permission.BLUETOOTH_SCAN",
                        "android.permission.BLUETOOTH_CONNECT"}) {
                    if (context.checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED)
                        throw new AssertionError("Missing Nearby permission: " + permission);
                }
            }

            // Load the exact Java helper packaged with Qt, rather than a copied test implementation.
            Class<?> utility = Class.forName("org.qtproject.qt5.android.bluetooth.QtBluetoothUtility",
                    true, context.getClassLoader());
            Method requiresLocation = utility.getDeclaredMethod("bluetoothScanRequiresLocation", Context.class);
            requiresLocation.setAccessible(true);
            boolean expected = sdk < 31;
            boolean actual = (Boolean) requiresLocation.invoke(null, context);
            result.putBoolean("location_required", actual);
            if (actual != expected)
                throw new AssertionError("API " + sdk + ": expected Location requirement "
                        + expected + ", got " + actual);
            if ((Boolean) requiresLocation.invoke(null, context) != expected)
                throw new AssertionError("Cached result changed");

            result.putString("qz_split_bluetooth", "PASS");
            finish(Activity.RESULT_OK, result);
        } catch (Throwable error) {
            result.putString("qz_split_bluetooth", "FAIL");
            result.putString("error", android.util.Log.getStackTraceString(error));
            finish(Activity.RESULT_CANCELED, result);
        }
    }
}
