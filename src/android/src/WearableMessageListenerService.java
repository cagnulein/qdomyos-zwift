package org.cagnulen.qdomyoszwift;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.PendingResult;
import com.google.android.gms.common.api.ResultCallback;
import com.google.android.gms.wearable.MessageClient;
import com.google.android.gms.wearable.DataClient;
import com.google.android.gms.wearable.DataEvent;
import com.google.android.gms.wearable.DataEventBuffer;
import com.google.android.gms.wearable.MessageEvent;
import com.google.android.gms.wearable.Wearable;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.wearable.DataItemBuffer;
import com.google.android.gms.wearable.DataMap;
import org.cagnulen.qdomyoszwift.Log;
import android.os.Bundle;
import com.google.android.gms.common.api.Status;
import java.io.InputStream;

public class WearableMessageListenerService extends Service implements
        MessageClient.OnMessageReceivedListener, GoogleApiClient.ConnectionCallbacks,GoogleApiClient.OnConnectionFailedListener,DataClient.OnDataChangedListener {

    private GoogleApiClient googleApiClient;
    private MessageClient mWearableClient;
    private String TAG = "WearableMessageListenerService";
    private static int heart_rate = 0;

    @Override
    public void onCreate() {
       super.onCreate();
       Log.v("WearableMessageListenerService","onCreate");
    }

    public static int getHeart() {
        return heart_rate;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // Your service logic here

        googleApiClient = new GoogleApiClient.Builder(this)
                .addApi(Wearable.API)
                .addConnectionCallbacks (this)
                .addOnConnectionFailedListener(this)
                .build();

        googleApiClient.connect();

        // Register the MessageClient.OnMessageReceivedListener
        mWearableClient = Wearable.getMessageClient(this);
        mWearableClient.addListener(this);
        Wearable.getDataClient(this).addListener(this);

        Log.v("WearableMessageListenerService","onStartCommand");

        // Return START_STICKY to restart the service if it's killed by the system
        return START_STICKY;
    }

    @Override
    public void onDataChanged(DataEventBuffer dataEvents) {
        for (DataEvent event : dataEvents) {
            if (event.getType() == DataEvent.TYPE_DELETED) {
                Log.d(TAG, "DataItem deleted: " + event.getDataItem().getUri());
            } else if (event.getType() == DataEvent.TYPE_CHANGED) {
                Log.d(TAG, "DataItem changed: " + event.getDataItem().getUri() + " " + event.getDataItem().getUri().getPath());
                if(event.getDataItem().getUri().getPath().equals("/qz")) {
                    new Thread(new Runnable() {
                            @Override
                            public void run() {
                                DataItemBuffer result = Wearable.DataApi.getDataItems(googleApiClient).await();
                                   if (result.getStatus().isSuccess()) {
                                       if (result.getCount() == 1) {
                                           heart_rate = DataMap.fromByteArray(result.get(0).getData())
                                                   .getInt("heart_rate", 0);
                                       } else {
                                           Log.e(TAG, "Unexpected number of DataItems found.\n"
                                                    + "\tExpected: 1\n"
                                                    + "\tActual: " + result.getCount());
                                       }
                                   } else if (Log.isLoggable(TAG, Log.DEBUG)) {
                                       Log.d(TAG, "onHandleIntent: failed to get current alarm state");
                                   }
                                Log.d(TAG, "Heart: " + heart_rate);
                            }
                        }).start();
                }
            }
        }
    }


    @Override
    public void onConnected(Bundle bundle) {
        Log.v("WearableMessageListenerService","onConnected");
    }

    @Override
    public void onConnectionSuspended(int i) {
        Log.v("WearableMessageListenerService","onConnectionSuspended");
    }

    @Override
    public void onConnectionFailed(ConnectionResult connectionResult) {
        Log.v("WearableMessageListenerService","onConnectionFailed");
    }

    @Override
    public void onMessageReceived(final MessageEvent messageEvent) {
        String path = messageEvent.getPath();
        byte[] data = messageEvent.getData();

        // Handle the received message data here
        String messageData = new String(data); // Assuming it's a simple string message

        Log.v("Wearable", path);
        Log.v("Wearable", messageData);

        // You can then perform actions or update data in your service based on the received message
    }

    @Override
    public IBinder onBind(Intent intent) {
        // This service does not support binding
        return null;
    }
}
