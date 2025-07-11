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
        
        // Normal window mode - no fullscreen flags
    }
}