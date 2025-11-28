package org.cagnulen.qdomyoszwift;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import org.cagnulen.qdomyoszwift.QLog;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;

public class MediaButtonReceiver extends BroadcastReceiver {
    private static MediaButtonReceiver instance;
    private static final int TARGET_VOLUME = 7;  // Middle volume value for infinite gear changes
    private static boolean restoringVolume = false;  // Flag to prevent recursion

    @Override
    public void onReceive(Context context, Intent intent) {
        QLog.d("MediaButtonReceiver", "Received intent: " + intent.toString());
        String intentAction = intent.getAction();
        if ("android.media.VOLUME_CHANGED_ACTION".equals(intentAction)) {
            AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
            int maxVolume = audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
            int currentVolume = intent.getIntExtra("android.media.EXTRA_VOLUME_STREAM_VALUE", -1);
            int previousVolume = intent.getIntExtra("android.media.EXTRA_PREV_VOLUME_STREAM_VALUE", -1);

            QLog.d("MediaButtonReceiver", "Volume changed. Current: " + currentVolume + ", Previous: " + previousVolume + ", Max: " + maxVolume + ", Restoring: " + restoringVolume);

            // If we're restoring volume, skip processing and reset flag
            if (restoringVolume) {
                QLog.d("MediaButtonReceiver", "Volume restore completed");
                restoringVolume = false;
                return;
            }

            // Process the gear change (guard against missing native lib)
            try {
                nativeOnMediaButtonEvent(previousVolume, currentVolume, maxVolume);
            } catch (UnsatisfiedLinkError e) {
                QLog.w("MediaButtonReceiver", "Native not ready: " + e.getMessage());
                return;
            }

            // Auto-restore volume to middle value after a short delay to enable infinite gear changes
            if (currentVolume != TARGET_VOLUME) {
                final AudioManager am = audioManager;
                new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        QLog.d("MediaButtonReceiver", "Auto-restoring volume to: " + TARGET_VOLUME);
                        restoringVolume = true;
                        am.setStreamVolume(AudioManager.STREAM_MUSIC, TARGET_VOLUME, 0);
                    }
                }, 100);  // 100ms delay to ensure gear change is processed first
            }
        }
    }

    private native void nativeOnMediaButtonEvent(int prev, int current, int max);

    public static void registerReceiver(Context context) {
        try {
            if (instance == null) {
                instance = new MediaButtonReceiver();
            }
            IntentFilter filter = new IntentFilter("android.media.VOLUME_CHANGED_ACTION");
            
            if (context == null) {
                QLog.e("MediaButtonReceiver", "Context is null, cannot register receiver");
                return;
            }
    
            if (Build.VERSION.SDK_INT >= 34) {
                try {
                    context.registerReceiver(instance, filter, Context.RECEIVER_EXPORTED);
                } catch (SecurityException se) {
                    QLog.e("MediaButtonReceiver", "Security exception while registering receiver: " + se.getMessage());
                }
            } else {
                try {
                    context.registerReceiver(instance, filter);
                } catch (SecurityException se) {
                    QLog.e("MediaButtonReceiver", "Security exception while registering receiver: " + se.getMessage());
                }
            }
            QLog.d("MediaButtonReceiver", "Receiver registered successfully");

            // Initialize volume to target value for gear control
            AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
            if (audioManager != null) {
                int currentVolume = audioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
                if (currentVolume != TARGET_VOLUME) {
                    QLog.d("MediaButtonReceiver", "Initializing volume to: " + TARGET_VOLUME);
                    restoringVolume = true;
                    audioManager.setStreamVolume(AudioManager.STREAM_MUSIC, TARGET_VOLUME, 0);
                    // Reset flag after initialization
                    new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            restoringVolume = false;
                        }
                    }, 200);
                }
            }

        } catch (IllegalArgumentException e) {
            QLog.e("MediaButtonReceiver", "Invalid arguments for receiver registration: " + e.getMessage());
        } catch (Exception e) {
            QLog.e("MediaButtonReceiver", "Unexpected error while registering receiver: " + e.getMessage());
        }
    }
    
    public static void unregisterReceiver(Context context) {
        if (instance != null) {
            context.unregisterReceiver(instance);
            instance = null;
        }
    }
}
