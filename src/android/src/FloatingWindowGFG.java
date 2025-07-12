// https://www.geeksforgeeks.org/how-to-make-a-floating-window-application-in-android/

package org.cagnulen.qdomyoszwift;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.IBinder;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.DisplayMetrics;
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

         // Preference key name
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

		  // The screen height and width are calculated, cause
		  // the height and width of the floating window is set depending on this
                  /*DisplayMetrics metrics = getApplicationContext().getResources().getDisplayMetrics();
		  int width = metrics.widthPixels;
                  int height = metrics.heightPixels;*/

		  // To obtain a WindowManager of a different Display,
		  // we need a Context for that display, so WINDOW_SERVICE is used
		  windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);

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
                  wv.loadUrl("http://localhost:" + FloatingHandler._port + "/floating/" + FloatingHandler._htmlPage);
                  wv.clearView();
                  wv.measure(100, 100);
                  wv.setAlpha(Float.valueOf(FloatingHandler._alpha) / 100.0f);
                  settings.setBuiltInZoomControls(true);
                  settings.setUseWideViewPort(true);
                  settings.setDomStorageEnabled(true);
                  QLog.d("QZ","loadurl");


		  // WindowManager.LayoutParams takes a lot of parameters to set the
		  // the parameters of the layout. One of them is Layout_type.
		  if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
			   // If API Level is more than 26, we need TYPE_APPLICATION_OVERLAY
				LAYOUT_TYPE = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
				} else {
				// If API Level is lesser than 26, then we can
				// use TYPE_SYSTEM_ERROR,
				// TYPE_SYSTEM_OVERLAY, TYPE_PHONE, TYPE_PRIORITY_PHONE.
				// But these are all
				// deprecated in API 26 and later. Here TYPE_TOAST works best.
				LAYOUT_TYPE = WindowManager.LayoutParams.TYPE_TOAST;
				}

			// Now the Parameter of the floating-window layout is set.
		  // 1) The Width of the window will be 55% of the phone width.
		  // 2) The Height of the window will be 58% of the phone height.
		  // 3) Layout_Type is already set.
		  // 4) Next Parameter is Window_Flag. Here FLAG_NOT_FOCUSABLE is used. But
		  // problem with this flag is key inputs can't be given to the EditText.
		  // This problem is solved later.
		  // 5) Next parameter is Layout_Format. System chooses a format that supports
		  // translucency by PixelFormat.TRANSLUCENT

		  floatWindowLayoutParam = new WindowManager.LayoutParams(
                          (int) (FloatingHandler._width ),
                                         (int) (FloatingHandler._height ),
					 LAYOUT_TYPE,
					 WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
					 PixelFormat.TRANSLUCENT
					);

		  // The Gravity of the Floating Window is set.
		  // Use TOP | LEFT for free positioning without constraints
		  floatWindowLayoutParam.gravity = Gravity.TOP | Gravity.LEFT;

		  // X and Y value of the window is set
		  floatWindowLayoutParam.x = 0;
		  floatWindowLayoutParam.y = 0;

                  sharedPreferences = getSharedPreferences("FloatingWindowGFG",MODE_PRIVATE);
                  floatWindowLayoutParam.x = sharedPreferences.getInt(PREF_NAME_X, floatWindowLayoutParam.x);
                  floatWindowLayoutParam.y = sharedPreferences.getInt(PREF_NAME_Y, floatWindowLayoutParam.y);

		  // The ViewGroup that inflates the floating_layout.xml is
		  // added to the WindowManager with all the parameters
		  windowManager.addView(floatView, floatWindowLayoutParam);


		  // Another feature of the floating window is, the window is movable.
		  // The window can be moved at any position on the screen.
		  floatView.setOnTouchListener(new View.OnTouchListener() {
			   final WindowManager.LayoutParams floatWindowLayoutUpdateParam = floatWindowLayoutParam;
				int initialX;
				int initialY;
				float initialTouchX;
				float initialTouchY;
				boolean isDragging = false;
				final int TOUCH_THRESHOLD = 10; // Threshold for distinguishing tap vs drag

				@Override
				public boolean onTouch(View v, MotionEvent event) {

                                         QLog.d("QZ","onTouch action: " + event.getAction());

					 switch (event.getAction()) {
						  case MotionEvent.ACTION_DOWN:
						      // Store initial positions
						      initialX = floatWindowLayoutUpdateParam.x;
								initialY = floatWindowLayoutUpdateParam.y;
								initialTouchX = event.getRawX();
								initialTouchY = event.getRawY();
								isDragging = false;
								break;

						  case MotionEvent.ACTION_MOVE:
						      // Calculate distance moved
						      float deltaX = event.getRawX() - initialTouchX;
								float deltaY = event.getRawY() - initialTouchY;
								
								// Check if we've moved enough to consider this a drag
								if (!isDragging && (Math.abs(deltaX) > TOUCH_THRESHOLD || Math.abs(deltaY) > TOUCH_THRESHOLD)) {
								    isDragging = true;
								}
								
								if (isDragging) {
								    // Get screen dimensions for boundary checking
								    DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
								    int screenWidth = displayMetrics.widthPixels;
								    int screenHeight = displayMetrics.heightPixels;
								    
								    // Calculate new position
								    int newX = initialX + (int) deltaX;
								    int newY = initialY + (int) deltaY;
								    
								    // Apply boundary constraints
								    // Keep window within screen bounds
								    int windowWidth = FloatingHandler._width;
								    int windowHeight = FloatingHandler._height;
								    
								    if (newX < 0) newX = 0;
								    if (newY < 0) newY = 0;
								    if (newX + windowWidth > screenWidth) newX = screenWidth - windowWidth;
								    if (newY + windowHeight > screenHeight) newY = screenHeight - windowHeight;
								    
								    // Update position
								    floatWindowLayoutUpdateParam.x = newX;
								    floatWindowLayoutUpdateParam.y = newY;

                                                                    // Save position to preferences
                                                                    SharedPreferences.Editor myEdit = sharedPreferences.edit();
                                                                    myEdit.putInt(PREF_NAME_X, floatWindowLayoutUpdateParam.x);
                                                                    myEdit.putInt(PREF_NAME_Y, floatWindowLayoutUpdateParam.y);
                                                                    myEdit.apply(); // Use apply() instead of commit() for better performance

								    // Apply updated parameter to the WindowManager
								    windowManager.updateViewLayout(floatView, floatWindowLayoutUpdateParam);
								}
								break;

						  case MotionEvent.ACTION_UP:
						      // If it wasn't a drag, it's a tap - let the WebView handle it
						      if (!isDragging) {
						          return false; // Let the event propagate to WebView
						      }
						      isDragging = false;
						      break;
								}
							return isDragging; // Consume the event only if we're dragging
					}
			});
	 }

    // It is called when stopService()
	 // method is called in MainActivity
	 @Override
	 public void onDestroy() {
		  super.onDestroy();
		  stopSelf();
		  // Window is removed from the screen
		  windowManager.removeView(floatView);
		}
}
