package org.cagnulen.qdomyoszwift;

import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import org.qtproject.qt5.android.bindings.QtActivity;

public class CustomQtActivity extends QtActivity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // Handle Android 16 API 36 WindowInsetsController for fullscreen support
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            // Android 11 (API 30) and above - use WindowInsetsController
            getWindow().setDecorFitsSystemWindows(false);
            WindowInsetsController controller = getWindow().getDecorView().getWindowInsetsController();
            if (controller != null) {
                // Hide only navigation bar, keep status bar visible for notifications access
                controller.hide(WindowInsets.Type.navigationBars());
                // Allow swipe to show navigation bar temporarily
                controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else {
            // Fallback for older Android versions (API < 30)
            getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                // Remove SYSTEM_UI_FLAG_FULLSCREEN to keep status bar visible
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            );
        }
    }
}