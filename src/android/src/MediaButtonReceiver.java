package org.cagnulen.qdomyoszwift;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.util.Log;

public class MediaButtonReceiver extends BroadcastReceiver {
    private static MediaButtonReceiver instance;

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d("MediaButtonReceiver", "Received intent: " + intent.toString());
        String intentAction = intent.getAction();
        if ("android.media.VOLUME_CHANGED_ACTION".equals(intentAction)) {
            AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
            int maxVolume = audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
            int currentVolume = intent.getIntExtra("android.media.EXTRA_VOLUME_STREAM_VALUE", -1);
            int previousVolume = intent.getIntExtra("android.media.EXTRA_PREV_VOLUME_STREAM_VALUE", -1);

            Log.d("MediaButtonReceiver", "Volume changed. Current: " + currentVolume + ", Max: " + maxVolume);
            nativeOnMediaButtonEvent(previousVolume, currentVolume, maxVolume);
        }
    }

    private native void nativeOnMediaButtonEvent(int prev, int current, int max);

    public static void registerReceiver(Context context) {
        if (instance == null) {
            instance = new MediaButtonReceiver();
        }
        IntentFilter filter = new IntentFilter("android.media.VOLUME_CHANGED_ACTION");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
            context.registerReceiver(instance, filter, Context.RECEIVER_EXPORTED);
        } else {
            context.registerReceiver(instance, filter);
        }
        Log.d("MediaButtonReceiver", "registerReceiver");
    }

    public static void unregisterReceiver(Context context) {
        if (instance != null) {
            context.unregisterReceiver(instance);
            instance = null;
        }
    }
}
