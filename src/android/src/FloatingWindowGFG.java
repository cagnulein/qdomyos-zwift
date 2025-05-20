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

    @Override
    public void onCreate() {
        super.onCreate();

        // Initialize SharedPreferences for position
        sharedPreferences = getSharedPreferences("FloatingWindowGFG", MODE_PRIVATE);

        // Get DisplayManager to access all connected displays
        DisplayManager displayManager = (DisplayManager) getSystemService(Context.DISPLAY_SERVICE);
        
        // Get ALL displays, not just presentation displays
        Display[] displays = displayManager.getDisplays();
        
        // Log all available displays for debugging
        for (int i = 0; i < displays.length; i++) {
            QLog.d("QZ", "Display " + i + ": " + displays[i].getName() + 
                   " (ID: " + displays[i].getDisplayId() + ")");
        }
        
        // Choose which display to use - use external if available
        Display targetDisplay;
        
        if (displays.length > 1) {
            // Use the second display (index 1), which is typically the external one
            targetDisplay = displays[1];
            QLog.d("QZ", "Using external display: " + targetDisplay.getName());
        } else {
            // No external displays, use the default display
            targetDisplay = displays[0];
            QLog.d("QZ", "Using default display: " + targetDisplay.getName());
        }
        
        // Get display metrics for the chosen display
        DisplayMetrics metrics = new DisplayMetrics();
        targetDisplay.getMetrics(metrics);
        QLog.d("QZ", "Selected display metrics - Width: " + metrics.widthPixels + 
               ", Height: " + metrics.heightPixels);
        
        try {
            // Create a context specific to the chosen display
            Context displayContext = createDisplayContext(targetDisplay);
            
            // Get WindowManager for the specific display
            windowManager = (WindowManager) displayContext.getSystemService(Context.WINDOW_SERVICE);
            
            // A LayoutInflater instance is created to retrieve the
            // LayoutInflater for the floating_layout xml
            LayoutInflater inflater = (LayoutInflater) getBaseContext().getSystemService(LAYOUT_INFLATER_SERVICE);
    
            // inflate a new view hierarchy from the floating_layout xml
            floatView = (ViewGroup) inflater.inflate(R.layout.floating_layout, null);
    
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
    
            // WindowManager.LayoutParams takes a lot of parameters to set the
            // the parameters of the layout. One of them is Layout_type.
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                // Try different window types for API 26+ depending on the display
                if (targetDisplay.getDisplayId() == Display.DEFAULT_DISPLAY) {
                    // For primary display, use APPLICATION_OVERLAY
                    LAYOUT_TYPE = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
                } else {
                    // For external displays, try PRESENTATION
                    LAYOUT_TYPE = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
                    // Fallback options if needed:
                    // LAYOUT_TYPE = WindowManager.LayoutParams.TYPE_APPLICATION;
                    // LAYOUT_TYPE = WindowManager.LayoutParams.TYPE_SYSTEM_ALERT;
                }
            } else {
                // For older Android versions
                LAYOUT_TYPE = WindowManager.LayoutParams.TYPE_TOAST;
            }
    
            // Now the Parameter of the floating-window layout is set.
            floatWindowLayoutParam = new WindowManager.LayoutParams(
                (int) (FloatingHandler._width),
                (int) (FloatingHandler._height),
                LAYOUT_TYPE,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                PixelFormat.TRANSLUCENT
            );
    
            // The Gravity of the Floating Window is set.
            // The Window will appear in the center of the screen
            floatWindowLayoutParam.gravity = Gravity.CENTER;
    
            // X and Y value of the window is set - retrieve from preferences if available
            floatWindowLayoutParam.x = sharedPreferences.getInt(PREF_NAME_X, 0);
            floatWindowLayoutParam.y = sharedPreferences.getInt(PREF_NAME_Y, 0);
    
            // Add flags needed for external displays
            if (targetDisplay.getDisplayId() != Display.DEFAULT_DISPLAY) {
                floatWindowLayoutParam.flags |= WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED;
                // More flags that might help:
                // floatWindowLayoutParam.flags |= WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON;
                // floatWindowLayoutParam.flags |= WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON;
            }
            
            // Log before adding the view
            QLog.d("QZ", "About to add view to display: " + targetDisplay.getName());
    
            // The ViewGroup that inflates the floating_layout.xml is
            // added to the WindowManager with all the parameters
            windowManager.addView(floatView, floatWindowLayoutParam);
            
            // Log after adding the view
            QLog.d("QZ", "View added successfully");
    
            // Another feature of the floating window is, the window is movable.
            // The window can be moved at any position on the screen.
            floatView.setOnTouchListener(new View.OnTouchListener() {
                final WindowManager.LayoutParams floatWindowLayoutUpdateParam = floatWindowLayoutParam;
                double x;
                double y;
                double px;
                double py;
    
                @Override
                public boolean onTouch(View v, MotionEvent event) {
                    QLog.d("QZ","onTouch");
    
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
            // Log any exceptions that might occur
            QLog.d("QZ", "Error creating floating window: " + e.getMessage());
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
