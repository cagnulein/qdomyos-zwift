package org.cagnulen.qdomyoszwift;

import android.content.res.Resources;
import android.os.Bundle;
import android.util.Log;
import org.qtproject.qt5.android.bindings.QtActivity;

public class CustomQtActivity extends QtActivity {
    private static final String TAG = "CustomQtActivity";
    private static CustomQtActivity instance;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        instance = this;
        
        Log.d(TAG, "onCreate: CustomQtActivity initialized");
        
        // Get and log the real status bar height
        int statusBarHeight = getStatusBarHeight();
        Log.d(TAG, "Real status bar height: " + statusBarHeight + "px");
    }
    
    // Native method that can be called from C++/Qt
    public static int getStatusBarHeight() {
        try {
            if (instance == null) {
                Log.e("CustomQtActivity", "Activity instance not available");
                return 72; // fallback value
            }
            
            Resources resources = instance.getResources();
            android.util.DisplayMetrics metrics = resources.getDisplayMetrics();
            
            // Log display metrics for analysis
            Log.d("CustomQtActivity", "Display metrics:");
            Log.d("CustomQtActivity", "  density: " + metrics.density);
            Log.d("CustomQtActivity", "  densityDpi: " + metrics.densityDpi);
            Log.d("CustomQtActivity", "  scaledDensity: " + metrics.scaledDensity);
            Log.d("CustomQtActivity", "  xdpi: " + metrics.xdpi);
            Log.d("CustomQtActivity", "  ydpi: " + metrics.ydpi);
            
            int resourceId = resources.getIdentifier("status_bar_height", "dimen", "android");
            if (resourceId > 0) {
                int heightPx = resources.getDimensionPixelSize(resourceId);
                float heightInDp = heightPx / metrics.density;
                
                Log.d("CustomQtActivity", "Status bar height analysis:");
                Log.d("CustomQtActivity", "  getDimensionPixelSize: " + heightPx + "px");
                Log.d("CustomQtActivity", "  Calculated DP: " + heightInDp + "dp");
                Log.d("CustomQtActivity", "  Returning DP value: " + Math.round(heightInDp));
                
                // Return DP value instead of pixel value to let Qt handle scaling
                return Math.round(heightInDp);
            }
        } catch (Exception e) {
            Log.e("CustomQtActivity", "Error getting status bar height", e);
        }
        return 72; // fallback value ~24dp
    }
}