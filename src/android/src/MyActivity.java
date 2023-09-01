package org.cagnulen.qdomyoszwift;
import android.util.Log;
import android.os.Bundle;
import android.support.wearable.activity.WearableActivity;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.PendingResult;
import com.google.android.gms.common.api.ResultCallback;
import com.google.android.gms.wearable.DataApi;
import com.google.android.gms.wearable.DataEvent;
import com.google.android.gms.wearable.DataEventBuffer;
import com.google.android.gms.wearable.DataMap;
import com.google.android.gms.wearable.DataMapItem;
import com.google.android.gms.wearable.PutDataMapRequest;
import com.google.android.gms.wearable.Wearable;

public class MyActivity extends org.qtproject.qt5.android.bindings.QtActivity, WearableActivity implements DataApi.DataListener {

 private final String TAG = "MyActivity";

 public static MyActivity activity_;

 @Override
 public void onCreate(android.os.Bundle savedInstanceState){
	 super.onCreate(savedInstanceState);
	 this.getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
	 activity_ = this;
	 Log.v(TAG, "onCreate");

         googleApiClient = new GoogleApiClient.Builder(this)
                 .addApi(Wearable.API)
                 .build();

         googleApiClient.connect();
         Wearable.DataApi.addListener(googleApiClient, this);
 }

@Override
 public void onDataChanged(DataEventBuffer dataEvents) {
     for (DataEvent event : dataEvents) {
         if (event.getType() == DataEvent.TYPE_CHANGED) {
             String path = event.getDataItem().getUri().getPath();
             if (path.equals("/qz")) {
                 DataMap dataMap = DataMapItem.fromDataItem(event.getDataItem()).getDataMap();
                 int heartRate = dataMap.getInt("heart_rate");

                 Log.v(TAG, heartRate);
                 // Handle the received heart rate data here
             }
         }
     }
 }
}
