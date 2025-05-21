package org.cagnulen.qdomyoszwift;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.hardware.display.DisplayManager;
import android.os.Build;
import android.os.IBinder;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import android.webkit.WebView;
import android.webkit.WebSettings;
import android.webkit.WebViewClient;
import org.cagnulen.qdomyoszwift.QLog;
import android.content.SharedPreferences;

public class FloatingWindowGFG extends Service {

    // The reference variables for the
    // ViewGroup, WindowManager.LayoutParams,
    // WindowManager, Button, EditText classes are created
    private ViewGroup floatView;
    private int LAYOUT_TYPE;
    private WindowManager.LayoutParams floatWindowLayoutParam;
    private WindowManager windowManager;
    private Button maximizeBtn;

    // Retrieve the user preference node for the package com.mycompany
    SharedPreferences sharedPreferences;

    // Preference key names
    final String PREF_NAME_X = "floatWindowLayoutUpdateParamX";
    final String PREF_NAME_Y = "floatWindowLayoutUpdateParamY";

    // As FloatingWindowGFG inherits Service class,
    // it actually overrides the onBind method
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    // Define an array of window types to try in order of preference
    private int[] getWindowTypesToTry(boolean isExternalDisplay) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) { // Android 8.0+
            if (isExternalDisplay) {
                // Window types to try for external displays (DeX mode)
                return new int[] {
                    WindowManager.LayoutParams.TYPE_APPLICATION,                // Try regular application window first
                    WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY,        // Then try overlay
                    WindowManager.LayoutParams.TYPE_APPLICATION_PANEL,          // Then try panel
                    WindowManager.LayoutParams.TYPE_APPLICATION_ATTACHED_DIALOG // Finally try attached dialog
                };
            } else {
                // Window types for the primary display
                return new int[] {
                    WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY,
                    WindowManager.LayoutParams.TYPE_APPLICATION
                };
            }
        } else { // Pre-Android 8.0
            return new int[] {
                WindowManager.LayoutParams.TYPE_TOAST,
                WindowManager.LayoutParams.TYPE_PHONE,
                WindowManager.LayoutParams.TYPE_SYSTEM_ALERT
            };
        }
    }

    // Helper method to convert window type integer to descriptive string for logging
    private String windowTypeToString(int windowType) {
        switch (windowType) {
            case WindowManager.LayoutParams.TYPE_APPLICATION:
                return "TYPE_APPLICATION";
            case WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY:
                return "TYPE_APPLICATION_OVERLAY";
            case WindowManager.LayoutParams.TYPE_APPLICATION_PANEL:
                return "TYPE_APPLICATION_PANEL";
            case WindowManager.LayoutParams.TYPE_APPLICATION_ATTACHED_DIALOG:
                return "TYPE_APPLICATION_ATTACHED_DIALOG";
            case WindowManager.LayoutParams.TYPE_TOAST:
                return "TYPE_TOAST";
            case WindowManager.LayoutParams.TYPE_PHONE:
                return "TYPE_PHONE";
            case WindowManager.LayoutParams.TYPE_SYSTEM_ALERT:
                return "TYPE_SYSTEM_ALERT";
            default:
                return "TYPE_" + windowType;
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();

        // Initialize SharedPreferences for position
        sharedPreferences = getSharedPreferences("FloatingWindowGFG", MODE_PRIVATE);

        try {
            // Get DisplayManager to access all connected displays
            DisplayManager displayManager = (DisplayManager) getSystemService(Context.DISPLAY_SERVICE);

            // Get ALL displays, not just presentation displays
            Display[] displays = displayManager.getDisplays();

            // Log all available displays for debugging
            QLog.d("QZ", "Number of displays: " + displays.length);
            for (int i = 0; i < displays.length; i++) {
                QLog.d("QZ", "Display " + i + ": " + displays[i].getDisplayId() +
                       (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1 ?
                       " - Name: " + displays[i].getName() : ""));
            }

            // Choose which display to use - use external if available
            Display targetDisplay;

            if (displays.length > 1) {
                // Use the second display (index 1), which is typically the external one in DeX mode
                targetDisplay = displays[1];
                QLog.d("QZ", "Using external display (ID: " + targetDisplay.getDisplayId() + ")");
            } else {
                // No external displays, use the default display
                targetDisplay = displays[0];
                QLog.d("QZ", "Using default display (ID: " + targetDisplay.getDisplayId() + ")");
            }

            // Get display metrics for the chosen display
            DisplayMetrics metrics = new DisplayMetrics();
            targetDisplay.getMetrics(metrics);
            QLog.d("QZ", "Selected display metrics - Width: " + metrics.widthPixels +
                   ", Height: " + metrics.heightPixels);

            // Create a context specific to the chosen display
            Context displayContext = createDisplayContext(targetDisplay);

            // Get WindowManager for the specific display
            windowManager = (WindowManager) displayContext.getSystemService(Context.WINDOW_SERVICE);

            // A LayoutInflater instance is created to retrieve the
            // LayoutInflater for the floating_layout xml
            LayoutInflater inflater = (LayoutInflater) getBaseContext().getSystemService(LAYOUT_INFLATER_SERVICE);

            // inflate a new view hierarchy from the floating_layout xml
            floatView = (ViewGroup) inflater.inflate(R.layout.floating_layout, null);

            // Set up the WebView
            WebView wv = (WebView)floatView.findViewById(R.id.webview);
            wv.setWebViewClient(new WebViewClient(){
                public boolean shouldOverrideUrlLoading(WebView view, String url) {
                    view.loadUrl(url);
                    return true;
                }
            });
            WebSettings settings = wv.getSettings();
            settings.setJavaScriptEnabled(true);
            wv.loadUrl("http://localhost:" + FloatingHandler._port + "/floating/floating.htm");
            wv.clearView();
            wv.measure(100, 100);
            wv.setAlpha(Float.valueOf(FloatingHandler._alpha) / 100.0f);
            settings.setBuiltInZoomControls(true);
            settings.setUseWideViewPort(true);
            settings.setDomStorageEnabled(true);
            QLog.d("QZ", "loadurl");

            // Determine if we're using an external display
            boolean isExternalDisplay = targetDisplay.getDisplayId() != Display.DEFAULT_DISPLAY;
            QLog.d("QZ", "Is external display: " + isExternalDisplay);

            // Get array of window types to try
            int[] windowTypesToTry = getWindowTypesToTry(isExternalDisplay);

            // Try each window type until one works
            Exception lastException = null;
            boolean windowCreated = false;

            for (int windowType : windowTypesToTry) {
                try {
                    // Set the current window type
                    LAYOUT_TYPE = windowType;
                    QLog.d("QZ", "Trying window type: " + windowTypeToString(windowType));

                    // Create layout params with current window type
                    floatWindowLayoutParam = new WindowManager.LayoutParams(
                        (int) (FloatingHandler._width),
                        (int) (FloatingHandler._height),
                        LAYOUT_TYPE,
                        WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                        PixelFormat.TRANSLUCENT
                    );

                    // The Gravity of the Floating Window is set.
                    floatWindowLayoutParam.gravity = Gravity.CENTER;

                    // X and Y value of the window is set - retrieve from preferences if available
                    floatWindowLayoutParam.x = sharedPreferences.getInt(PREF_NAME_X, 0);
                    floatWindowLayoutParam.y = sharedPreferences.getInt(PREF_NAME_Y, 0);

                    // Add additional flags that might help with visibility and placement on external displays
                    if (isExternalDisplay) {
                        // Try various flags that might help with external display visibility
                        floatWindowLayoutParam.flags |= WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED;
                        floatWindowLayoutParam.flags |= WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN;
                        floatWindowLayoutParam.flags |= WindowManager.LayoutParams.FLAG_LAYOUT_INSET_DECOR;

                        // Additional flags that might help with DeX mode
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) { // Android 9.0+
                            floatWindowLayoutParam.flags |= WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS;
                        }
                    }

                    // Log before adding the view
                    QLog.d("QZ", "About to add view with window type: " + windowTypeToString(windowType));

                    // Add the view to the window manager
                    windowManager.addView(floatView, floatWindowLayoutParam);

                    // If we get here, adding the view was successful
                    QLog.d("QZ", "Successfully added view with window type: " + windowTypeToString(windowType));
                    windowCreated = true;
                    break; // Exit the loop since we found a working window type

                } catch (Exception e) {
                    // Log the exception and try the next window type
                    lastException = e;
                    QLog.d("QZ", "Failed to add view with window type " + windowTypeToString(windowType) +
                           ": " + e.getMessage());
                }
            }

            // If we couldn't create a window with any of the types, log the error
            if (!windowCreated) {
                QLog.d("QZ", "Failed to create floating window with any window type. Last error: " +
                       (lastException != null ? lastException.getMessage() : "unknown"));
                // Return from onCreate to prevent further execution
                return;
            }

            // Set up the touch listener for moving the window
            floatView.setOnTouchListener(new View.OnTouchListener() {
                final WindowManager.LayoutParams floatWindowLayoutUpdateParam = floatWindowLayoutParam;
                double x;
                double y;
                double px;
                double py;

                @Override
                public boolean onTouch(View v, MotionEvent event) {
                    QLog.d("QZ", "onTouch");

                    switch (event.getAction()) {
                        // When the window will be touched,
                        // the x and y position of that position
                        // will be retrieved
                        case MotionEvent.ACTION_DOWN:
                            x = floatWindowLayoutUpdateParam.x;
                            y = floatWindowLayoutUpdateParam.y;

                            // returns the original raw X
                            // coordinate of this event
                            px = event.getRawX();

                            // returns the original raw Y
                            // coordinate of this event
                            py = event.getRawY();
                            break;
                        // When the window will be dragged around,
                        // it will update the x, y of the Window Layout Parameter
                        case MotionEvent.ACTION_MOVE:
                            floatWindowLayoutUpdateParam.x = (int) ((x + event.getRawX()) - px);
                            floatWindowLayoutUpdateParam.y = (int) ((y + event.getRawY()) - py);

                            SharedPreferences.Editor myEdit = sharedPreferences.edit();
                            myEdit.putInt(PREF_NAME_X, floatWindowLayoutUpdateParam.x);
                            myEdit.putInt(PREF_NAME_Y, floatWindowLayoutUpdateParam.y);
                            myEdit.commit();

                            // updated parameter is applied to the WindowManager
                            windowManager.updateViewLayout(floatView, floatWindowLayoutUpdateParam);
                            break;
                    }
                    return false;
                }
            });
        } catch (Exception e) {
            // Log any exceptions that might occur at the top level
            QLog.d("QZ", "Error in onCreate: " + e.getMessage());
            e.printStackTrace();
        }
    }

    // It is called when stopService()
    // method is called in MainActivity
    @Override
    public void onDestroy() {
        super.onDestroy();
        stopSelf();
        try {
            // Window is removed from the screen
            if (windowManager != null && floatView != null) {
                windowManager.removeView(floatView);
                QLog.d("QZ", "View removed successfully");
            }
        } catch (Exception e) {
            QLog.d("QZ", "Error removing view: " + e.getMessage());
        }
    }
}
