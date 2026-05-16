package org.cagnulen.qdomyoszwift;

import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.util.SparseArray;
import android.util.Log;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.view.DisplayCutout;
import org.qtproject.qt5.android.bindings.QtActivity;

public class CustomQtActivity extends QtActivity {
    private static final String TAG = "CustomQtActivity";
    private static final int DOCUMENT_PICKER_PROFILE_REQUEST_CODE = 4101;
    private static final int DOCUMENT_PICKER_TRAINING_REQUEST_CODE = 4102;
    private static final int DOCUMENT_PICKER_GPX_REQUEST_CODE = 4103;
    private static final int DOCUMENT_PICKER_SETTINGS_REQUEST_CODE = 4104;
    private final SparseArray<String> pendingImportDirectories = new SparseArray<>();

    // Declare the native method that will be implemented in C++
    private static native void onInsetsChanged(int top, int bottom, int left, int right);
    private static native void nativeOnOAuthCallback(String callbackUrl);
    private static native void nativeOnDocumentPicked(int requestCode, int resultCode, String localPath);

    private void dispatchOAuthCallback(Intent intent) {
        if (intent == null) {
            return;
        }

        Uri data = intent.getData();
        if (data == null) {
            return;
        }

        String url = data.toString();
        if (url.startsWith("https://www.qzfitness.com/peloton/callback")) {
            Log.d(TAG, "dispatchOAuthCallback: https://www.qzfitness.com/peloton/callback?code=XXXX&state=XXXX");
            nativeOnOAuthCallback(url);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate: CustomQtActivity initialized");
        dispatchOAuthCallback(getIntent());

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

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);
        dispatchOAuthCallback(intent);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (isDocumentPickerRequest(requestCode)) {
            handleDocumentPickerResult(requestCode, resultCode, data);
            return;
        }

        String uriString = "";
        if (data != null && data.getData() != null) {
            uriString = data.getData().toString();
        }
        Log.d(TAG, "onActivityResult passthrough requestCode=" + requestCode + " resultCode=" + resultCode + " uri=" + uriString);
        super.onActivityResult(requestCode, resultCode, data);
    }

    public void openDocumentPicker(String mimeType, int requestCode, String destinationDir) {
        pendingImportDirectories.put(requestCode, destinationDir == null ? "" : destinationDir);
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType((mimeType == null || mimeType.isEmpty()) ? "*/*" : mimeType);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        startActivityForResult(Intent.createChooser(intent, "Select file"), requestCode);
    }

    // This method is still needed for the QML check
    public static int getApiLevel() {
        return Build.VERSION.SDK_INT;
    }

    private boolean isDocumentPickerRequest(int requestCode) {
        return requestCode == DOCUMENT_PICKER_PROFILE_REQUEST_CODE
            || requestCode == DOCUMENT_PICKER_TRAINING_REQUEST_CODE
            || requestCode == DOCUMENT_PICKER_GPX_REQUEST_CODE
            || requestCode == DOCUMENT_PICKER_SETTINGS_REQUEST_CODE;
    }

    private void handleDocumentPickerResult(int requestCode, int resultCode, Intent data) {
        String destinationDir = pendingImportDirectories.get(requestCode, "");
        pendingImportDirectories.remove(requestCode);

        String action = "";
        int flags = 0;
        int clipItemCount = 0;
        String uriString = "";
        String localPath = "";

        if (data != null) {
            action = data.getAction() == null ? "" : data.getAction();
            flags = data.getFlags();
            if (data.getClipData() != null) {
                clipItemCount = data.getClipData().getItemCount();
            }
            if (data.getData() != null) {
                Uri uri = data.getData();
                uriString = uri.toString();
                if (resultCode == RESULT_OK) {
                    localPath = ContentHelper.importContentToAppDir(this, uri, destinationDir);
                }
            }
        }

        Log.d(TAG, "handleDocumentPickerResult requestCode=" + requestCode
            + " resultCode=" + resultCode
            + " action=" + action
            + " flags=0x" + Integer.toHexString(flags)
            + " clipItems=" + clipItemCount
            + " uri=" + uriString
            + " localPath=" + localPath);
        nativeOnDocumentPicked(requestCode, resultCode, localPath);
    }
}
