package org.cagnulen.qdomyoszwift;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.view.KeyEvent;
import android.content.IntentFilter;
import android.util.Log;

public class MediaButtonReceiver extends BroadcastReceiver {
    private static MediaButtonReceiver instance;

    @Override
    public void onReceive(Context context, Intent intent) {
        String intentAction = intent.getAction();
        if (Intent.ACTION_MEDIA_BUTTON.equals(intentAction)) {
            KeyEvent event = intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
            if (event != null) {
                int keycode = event.getKeyCode();
                int action = event.getAction();

                Log.d("MediaButtonReceiver", "onReceive");

                nativeOnMediaButtonEvent(keycode, action);
            }
        }
    }

    private native void nativeOnMediaButtonEvent(int keycode, int action);

    public static void registerReceiver(Context context) {
        if (instance == null) {
            instance = new MediaButtonReceiver();
        }
        IntentFilter filter = new IntentFilter(Intent.ACTION_MEDIA_BUTTON);
        context.registerReceiver(instance, filter);
        Log.d("MediaButtonReceiver", "registerReceiver");
    }

    public static void unregisterReceiver(Context context) {
        if (instance != null) {
            context.unregisterReceiver(instance);
            instance = null;
        }
    }
}
