package org.cagnulen.qdomyoszwift;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.content.IntentFilter;
import org.cagnulen.qdomyoszwift.QLog as Log;
import android.app.Service;
import android.media.RingtoneManager;
import android.net.Uri;
import android.app.Activity;
import android.os.Build;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Deque;
import java.util.EnumSet;
import java.util.LinkedList;
import java.util.concurrent.Callable;
import java.util.ArrayList;
import java.util.List;
import java.nio.charset.StandardCharsets;
import android.content.pm.ShortcutManager;
import android.content.pm.ShortcutInfo;
import java.io.File;
import android.graphics.drawable.Icon;


public class Shortcuts {
    public static void createShortcutsForFiles(String folder, Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N_MR1) {
            ShortcutManager shortcutManager = context.getSystemService(ShortcutManager.class);

            List<ShortcutInfo> shortcuts = new ArrayList<>();

            Log.d("Shortcuts", folder);
            File[] files = new File(folder, "profiles").listFiles();
            if (files != null) {
                for (int i = 0; i < files.length && i < 5; i++) { // Limit to 5 shortcuts
                    File file = files[i];
                    String fileNameWithoutExtension = file.getName();
                    int dotIndex = fileNameWithoutExtension.lastIndexOf('.');
                    if (dotIndex > 0) { // Check if there is a dot, indicating an extension exists
                        fileNameWithoutExtension = fileNameWithoutExtension.substring(0, dotIndex);
                    }
                    Log.d("Shortcuts", file.getAbsolutePath());
                    Intent intent = new Intent(context, context.getClass());
                    intent.setAction(Intent.ACTION_VIEW);
                    intent.putExtra("profile_path", file.getAbsolutePath());

                    ShortcutInfo shortcut = new ShortcutInfo.Builder(context, "id" + fileNameWithoutExtension)
                            .setShortLabel(fileNameWithoutExtension)
                            .setLongLabel("Open " + fileNameWithoutExtension)
                            .setIcon(Icon.createWithResource(context, R.drawable.icon))
                            .setIntent(intent)
                            .build();

                    shortcuts.add(shortcut);
                }
            }

        
            shortcutManager.setDynamicShortcuts(shortcuts);
            getAllExtras(context);
        }
    }

    public static String getProfileExtras(Context context) {
        Intent intent = ((Activity)context).getIntent(); // Ottieni l'Intent che ha avviato l'attività
        Bundle extras = intent.getExtras();
        if (extras != null) {
            for (String key : extras.keySet()) {
                Object value = extras.get(key);
                if("profile_path".equals(key)) {
                    Log.d("Shortcuts", "profile_path: " + value.toString());
                    return value.toString();
                }
            }
        }
        return "";
    }

    public static void getAllExtras(Context context) {
        Intent intent = ((Activity)context).getIntent(); // Ottieni l'Intent che ha avviato l'attività
        Bundle extras = intent.getExtras();
        if (extras != null) {
            for (String key : extras.keySet()) {
                Object value = extras.get(key);
                Log.d("Shortcuts", "Key: " + key + ", Value: " + value.toString());
            }
        }
    }
}
