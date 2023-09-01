package org.cagnulen.qdomyoszwift;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.PendingResult;
import com.google.android.gms.common.api.ResultCallback;
import com.google.android.gms.wearable.MessageClient;
import com.google.android.gms.wearable.MessageEvent;
import com.google.android.gms.wearable.Wearable;
import android.util.Log;

public class WearableMessageListenerService extends Service implements
        MessageClient.OnMessageReceivedListener {

    private GoogleApiClient googleApiClient;

    @Override
    public void onCreate() {
        super.onCreate();

        googleApiClient = new GoogleApiClient.Builder(this)
                .addApi(Wearable.API)
                .build();

        googleApiClient.connect();

        // Register the MessageClient.OnMessageReceivedListener
        Wearable.getMessageClient(this).addListener(this);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // Your service logic here

        // Return START_STICKY to restart the service if it's killed by the system
        return START_STICKY;
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
