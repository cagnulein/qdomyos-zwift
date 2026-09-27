package org.cagnulen.qdomyoszwift;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.util.SparseArray;
import android.util.Log;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.view.DisplayCutout;
import android.graphics.Insets;
import org.qtproject.qt5.android.bindings.QtActivity;

public class CustomQtActivity extends QtActivity {
    private static final String TAG = "CustomQtActivity";
    private static final int DOCUMENT_PICKER_PROFILE_REQUEST_CODE = 4101;
    private static final int DOCUMENT_PICKER_TRAINING_REQUEST_CODE = 4102;
    private static final int DOCUMENT_PICKER_GPX_REQUEST_CODE = 4103;
    private static final int DOCUMENT_PICKER_SETTINGS_REQUEST_CODE = 4104;
    private final SparseArray<String> pendingImportDirectories = new SparseArray<>();

    // Declare the native method that will be implemented in C++
    private static native void onInsetsChanged(int top, int bottom, int left, int right,
                                               int waterfallTop, int waterfallBottom,
                                               int waterfallLeft, int waterfallRight);
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
            try {
                nativeOnOAuthCallback(url);
            } catch (UnsatisfiedLinkError e) {
                Log.w(TAG, "Qt not ready yet for OAuth callback, ignoring: " + e.getMessage());
            }
        }
    }

    // Traverse the view hierarchy to find Qt's rendering surface (SurfaceView or TextureView)
    // and return its Y position in pixels within the window.
    // Qt 5.15 on some emulator configurations internally shifts its viewport by the top inset
    // in landscape, even after setDecorFitsSystemWindows(false). Detecting this offset lets us
    // avoid double-counting the top inset in QML.
    private int findQtSurfaceYOffset(View view) {
        if (view instanceof SurfaceView || view instanceof TextureView) {
            int[] loc = new int[2];
            view.getLocationInWindow(loc);
            return loc[1];
        }
        if (view instanceof ViewGroup) {
            ViewGroup group = (ViewGroup) view;
            for (int i = 0; i < group.getChildCount(); i++) {
                int result = findQtSurfaceYOffset(group.getChildAt(i));
                if (result > 0) return result;
            }
        }
        return 0;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate: CustomQtActivity initialized");
        dispatchOAuthCallback(getIntent());
        AgeSignalsHelper.requestAgeSignals(this);
        HealthConnectHelper.initialize(this);

        // Make the window truly edge-to-edge so the app renders into ALL screen areas
        // including the display cutout (punch-hole/notch) on all 4 rotation variants.
        // setDecorFitsSystemWindows(false) (API 30+) prevents the system from shrinking
        // the content area around insets — we handle insets ourselves via onApplyWindowInsets.
        // LAYOUT_IN_DISPLAY_CUTOUT_MODE_ALWAYS extends into the cutout on every edge.
        // Dark window background ensures no white strip is visible in areas Qt may not render.
        getWindow().setBackgroundDrawable(new ColorDrawable(Color.BLACK));
        if (Build.VERSION.SDK_INT >= 30) {
            getWindow().setDecorFitsSystemWindows(false);
            getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_ALWAYS;
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
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
                int waterfallTop = 0;
                int waterfallBottom = 0;
                int waterfallLeft = 0;
                int waterfallRight = 0;

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

                            // Android 11+ exposes curved waterfall display areas separately from cutouts.
                            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                                Insets waterfallInsets = cutout.getWaterfallInsets();
                                waterfallLeft = Math.round(waterfallInsets.left / density);
                                waterfallRight = Math.round(waterfallInsets.right / density);
                                waterfallTop = Math.round(waterfallInsets.top / density);
                                waterfallBottom = Math.round(waterfallInsets.bottom / density);
                            }
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
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                            Insets waterfallInsets = cutout.getWaterfallInsets();
                            Log.d(TAG, "Waterfall insets - Top:" + waterfallInsets.top +
                                  " Bottom:" + waterfallInsets.bottom +
                                  " Left:" + waterfallInsets.left +
                                  " Right:" + waterfallInsets.right);
                        }
                    }
                }

                // Push the new, correct inset values to the C++ layer immediately for
                // responsiveness. Guard against the race where Qt's native library hasn't
                // finished loading yet when Android fires onApplyWindowInsets early.
                try {
                    onInsetsChanged(top, bottom, left, right, waterfallTop, waterfallBottom, waterfallLeft, waterfallRight);
                } catch (UnsatisfiedLinkError ignored) {
                    // Qt not ready yet; insets will be re-applied once Qt initializes.
                }

                // Deferred correction: Qt 5.15 on some emulator configurations internally
                // shifts its rendering viewport by the top inset in landscape mode even
                // after setDecorFitsSystemWindows(false). On real devices this does not
                // happen. We detect the offset by measuring the Qt surface view position
                // after the layout pass completes, then send corrected values so QML
                // topPadding is not double-counted.
                final int fTop = top, fBottom = bottom, fLeft = left, fRight = right;
                final float fDensity = density;
                v.post(() -> {
                    int surfaceYPx = findQtSurfaceYOffset(v);
                    Log.d(TAG, "deferred surfaceYPx=" + surfaceYPx + " fTop=" + fTop);
                    if (surfaceYPx > 0 && fDensity > 0) {
                        int adjustedTop = Math.max(0, fTop - Math.round(surfaceYPx / fDensity));
                        Log.d(TAG, "adjustedTop=" + adjustedTop);
                        if (adjustedTop != fTop) {
                            try {
                                onInsetsChanged(adjustedTop, fBottom, fLeft, fRight);
                            } catch (UnsatisfiedLinkError ignored) {}
                        }
                    }
                });

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
