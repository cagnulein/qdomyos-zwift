package org.cagnulen.qdomyoszwift;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.media.projection.MediaProjectionManager;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class MediaProjection  {
    private static final int REQUEST_CODE = 100;
    private static Context _context;
    /*private static MediaProjection m_instance;

    public MediaProjection() {
        m_instance = this;
        startProjection();
    }*/

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_CODE) {
            if (resultCode != Activity.RESULT_OK) {
                return;
            }
            //startService(org.cagnulen.qdomyoszwift.ScreenCaptureService.getStartIntent(this, resultCode, data));
        }
    }

    public static void startService(Context context, int resultCode, Intent data) {
        context.startService(org.cagnulen.qdomyoszwift.ScreenCaptureService.getStartIntent(context, resultCode, data));
    }

    public void startProjection(Context context) {
        _context = context;
        MediaProjectionManager mProjectionManager =
                (MediaProjectionManager) context.getSystemService(Context.MEDIA_PROJECTION_SERVICE);
        //Activity a = (Activity)_context;
        //this.startActivityForResult(mProjectionManager.createScreenCaptureIntent(), REQUEST_CODE);
    }

    public void stopProjection() {
        //startService(com.mtsahakis.mediaprojectiondemo.ScreenCaptureService.getStopIntent(this));
    }
}
