package org.cagnulen.qdomyoszwift;
import android.app.Activity;
import org.cagnulen.qdomyoszwift.QLog;

public class MyActivity extends Activity {

 private final String TAG = "MyActivity";

 public static MyActivity activity_;

 @Override
 public void onCreate(android.os.Bundle savedInstanceState){
	 super.onCreate(savedInstanceState);
	 this.getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
	 activity_ = this;
	 QLog.v(TAG, "onCreate");
 }
}
