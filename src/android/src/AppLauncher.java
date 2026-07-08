package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.widget.Toast;
import org.cagnulen.qdomyoszwift.QLog;

public class AppLauncher {
    private static final String TAG = "AppLauncher";

    public static void launchApp(Context context, String packageName) {
        try {
            PackageManager pm = context.getPackageManager();
            Intent launchIntent = pm.getLaunchIntentForPackage(packageName);

            if (launchIntent != null) {
                launchIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(launchIntent);
                QLog.d(TAG, "Successfully launched app: " + packageName);
            } else {
                QLog.e(TAG, "App not found: " + packageName);
                Toast.makeText(context, "App not found: " + packageName, Toast.LENGTH_SHORT).show();
            }
        } catch (Exception e) {
            QLog.e(TAG, "Error launching app: " + packageName + " - " + e.getMessage());
            Toast.makeText(context, "Error launching app", Toast.LENGTH_SHORT).show();
        }
    }

    public static void launchIFitApp(Context context) {
        launchApp(context, "com.ifit.gandalf");
    }
}
