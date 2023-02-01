package org.cagnulen.qdomyoszwift;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.media.projection.MediaProjectionManager;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.util.DisplayMetrics;

import com.rvalerio.fgchecker.AppChecker;

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

 public boolean isLandscape(Context context){
	  bool landscape = false;
	  DisplayMetrics displaymetrics = new DisplayMetrics();
	  context.getWindowManager().getDefaultDisplay().getMetrics(displaymetrics);
	  int width = displaymetrics.widthPixels;
	  int height = displaymetrics.heightPixels;

	  if(width<height){
		   landscape = false;
	  } else {
	      landscape = true;
	  }

      return landscape;

 }

 void requestUsageStatsPermission() {
	 if(android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP
	 && !hasUsageStatsPermission(this)) {
		 startActivity(new Intent(Settings.ACTION_USAGE_ACCESS_SETTINGS));
		 }
	 }

 @TargetApi(Build.VERSION_CODES.KITKAT)
 boolean hasUsageStatsPermission(Context context) {
	 AppOpsManager appOps = (AppOpsManager) context.getSystemService(Context.APP_OPS_SERVICE);
	 int mode = appOps.checkOpNoThrow("android:get_usage_stats",
	 android.os.Process.myUid(), context.getPackageName());
	 boolean granted = mode == AppOpsManager.MODE_ALLOWED;
	 return granted;
	 }

    public static void startService(Context context, int resultCode, Intent data) {
		  final Context _context = context;
		  requestUsageStatsPermission();
        context.startService(org.cagnulen.qdomyoszwift.ScreenCaptureService.getStartIntent(context, resultCode, data));

		  AppChecker appChecker = new AppChecker();
		  appChecker
		      .whenAny(new AppChecker.Listener() {
					 @Override
					 public void onForeground(String packageName) {
						  Log.e("MediaProjection", packageName);
						  if(isLandscape(_context))
						     Log.e("MediaProjection", "Landscape");
						  else
						     Log.e("MediaProjection", "Portrait");
						}
				)
				.timeout(1000)
				.start(this);
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
