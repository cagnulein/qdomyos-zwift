package org.cagnulen.qdomyoszwift;

import android.app.ActivityManager;
import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import android.os.Looper;
import android.os.Handler;
import org.cagnulen.qdomyoszwift.QLog;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

public class WearableController {
    static Context _context;
    static Intent _intent = null;

        public static void start(Context context) {            
            _context = context;

             if(_intent == null)
                _intent = new Intent(context, WearableMessageListenerService.class);
             // FloatingWindowGFG service is started
             context.startService(_intent);
             QLog.v("WearableController", "started");
        }

        public static int getHeart() {
            return WearableMessageListenerService.getHeart();
        }

        // Watches whose QZ app sleeps between workouts declare this capability (res/values/wear.xml
        // of QZ_WearOS). The phone declares qz_hr_wake_sleep in its own wear.xml, which the watch
        // checks at run time.
        static final String WATCH_CAPABILITY = "qz_hr_sleep_between_workouts";
        // Watches with that capability, kept up to date by one capability listener, so that
        // setWatchAwake makes no Wear OS call at all when there is no such watch
        static final java.util.Set<String> capableNodes = java.util.concurrent.ConcurrentHashMap.newKeySet();
        static boolean capabilityWatched = false;

        static synchronized void watchCapability(Context ctx) {
            if (capabilityWatched)
                return;
            capabilityWatched = true;
            com.google.android.gms.wearable.CapabilityClient client =
                com.google.android.gms.wearable.Wearable.getCapabilityClient(ctx);
            client.addListener(info -> setCapableNodes(info.getNodes()), WATCH_CAPABILITY);
            client.getCapability(WATCH_CAPABILITY, com.google.android.gms.wearable.CapabilityClient.FILTER_REACHABLE)
                .addOnSuccessListener(info -> setCapableNodes(info.getNodes()))
                .addOnFailureListener(e -> QLog.w("WearableController", "getCapability failed: " + e));
        }

        static void setCapableNodes(java.util.Set<com.google.android.gms.wearable.Node> nodes) {
            capableNodes.clear();
            for (com.google.android.gms.wearable.Node node : nodes)
                capableNodes.add(node.getId());
            QLog.v("WearableController", WATCH_CAPABILITY + ": " + capableNodes.size() + " watch(es)");
        }

        // Wakes the QZ watch app's heart-rate service (awake=true) or puts it to sleep. Sent only
        // to watches whose QZ app understands it; older watch apps and phones without a watch get
        // nothing.
        public static void setWatchAwake(Context context, final boolean awake) {
            final Context ctx = context.getApplicationContext();
            watchCapability(ctx);
            if (capableNodes.isEmpty())
                return;
            final String path = awake ? "/qz/hr/wake" : "/qz/hr/sleep";
            for (String nodeId : capableNodes) {
                com.google.android.gms.wearable.Wearable.getMessageClient(ctx)
                    .sendMessage(nodeId, path, new byte[0])
                    .addOnFailureListener(e -> QLog.w("WearableController", path + " failed: " + e));
            }
            QLog.v("WearableController", path + " sent to " + capableNodes.size() + " watch(es)");
        }
}
