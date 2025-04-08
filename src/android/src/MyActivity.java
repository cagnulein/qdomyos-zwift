package org.cagnulen.qdomyoszwift;
import org.cagnulen.qdomyoszwift.qlog as Log;

public class MyActivity extends org.qtproject.qt5.android.bindings.QtActivity {

 private final String TAG = "MyActivity";

 public static MyActivity activity_;

 @Override
 public void onCreate(android.os.Bundle savedInstanceState){
	 super.onCreate(savedInstanceState);
	 this.getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
	 activity_ = this;
	 Log.v(TAG, "onCreate");
 }
}
