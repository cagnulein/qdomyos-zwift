package org.cagnulein.qdomyoszwift;

import android.app.Activity;
import android.graphics.Rect;
import android.os.Build;
import android.util.Log;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.Window;
import android.view.WindowInsets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class SafeArea {
    private static final String TAG = "SafeArea";
    
    private int top = 0;
    private int right = 0;
    private int bottom = 0;
    private int left = 0;
    
    private Activity activity;
    
    public SafeArea(Activity activity) {
        this.activity = activity;
        calculateSafeArea();
        setupListener();
    }
    
    private void calculateSafeArea() {
        if (activity == null) return;
        
        View decorView = activity.getWindow().getDecorView();
        
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            // Android 11+ (API 30+) - Use WindowInsets
            WindowInsets insets = decorView.getRootWindowInsets();
            if (insets != null) {
                android.graphics.Insets systemBars = insets.getInsets(WindowInsets.Type.systemBars());
                android.graphics.Insets displayCutout = insets.getInsets(WindowInsets.Type.displayCutout());
                
                top = Math.max(systemBars.top, displayCutout.top);
                right = Math.max(systemBars.right, displayCutout.right);
                bottom = Math.max(systemBars.bottom, displayCutout.bottom);
                left = Math.max(systemBars.left, displayCutout.left);
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            // Android 6+ (API 23+) - Use WindowInsetsCompat
            WindowInsetsCompat insets = ViewCompat.getRootWindowInsets(decorView);
            if (insets != null) {
                top = insets.getSystemWindowInsetTop();
                right = insets.getSystemWindowInsetRight();
                bottom = insets.getSystemWindowInsetBottom();
                left = insets.getSystemWindowInsetLeft();
                
                // Handle display cutout for API 28+
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                    androidx.core.graphics.Insets cutout = insets.getInsets(WindowInsetsCompat.Type.displayCutout());
                    top = Math.max(top, cutout.top);
                    right = Math.max(right, cutout.right);
                    bottom = Math.max(bottom, cutout.bottom);
                    left = Math.max(left, cutout.left);
                }
            }
        } else {
            // Fallback for older Android versions
            Rect rect = new Rect();
            decorView.getWindowVisibleDisplayFrame(rect);
            
            top = rect.top;
            left = rect.left;
            
            // For bottom and right, we need to calculate based on screen size
            android.util.DisplayMetrics metrics = new android.util.DisplayMetrics();
            activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
            
            right = metrics.widthPixels - rect.right;
            bottom = metrics.heightPixels - rect.bottom;
        }
        
        Log.d(TAG, "SafeArea calculated: top=" + top + ", right=" + right + ", bottom=" + bottom + ", left=" + left);
    }
    
    private void setupListener() {
        if (activity == null) return;
        
        View decorView = activity.getWindow().getDecorView();
        
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            decorView.setOnApplyWindowInsetsListener((v, insets) -> {
                calculateSafeArea();
                return insets;
            });
        } else {
            decorView.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
                @Override
                public void onGlobalLayout() {
                    calculateSafeArea();
                }
            });
        }
    }
    
    public int getTop() {
        return top;
    }
    
    public int getRight() {
        return right;
    }
    
    public int getBottom() {
        return bottom;
    }
    
    public int getLeft() {
        return left;
    }
}