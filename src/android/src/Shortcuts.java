package org.cagnulen.qdomyoszwift;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;
import android.app.Service;
import android.media.RingtoneManager;
import android.net.Uri;
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
    public static void createShortcutsForFiles(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N_MR1) {
            ShortcutManager shortcutManager = context.getSystemService(ShortcutManager.class);

            List<ShortcutInfo> shortcuts = new ArrayList<>();

            File[] files = new File("profiles").listFiles();
            if (files != null) {
                for (File file : files) {
                    Log.d("Shortcuts", file.getAbsolutePath());
                    Intent intent = new Intent(context, org.qtproject.qt5.android.bindings.QtActivity);
                    intent.setAction(Intent.ACTION_VIEW);
                    intent.putExtra("profile_path", file.getAbsolutePath());

                    ShortcutInfo shortcut = new ShortcutInfo.Builder(context, "id" + file.getName())
                            .setShortLabel(file.getName())
                            .setLongLabel("Open " + file.getName())
                            .setIcon(Icon.createWithResource(context, R.drawable.icon))
                            .setIntent(intent)
                            .build();

                    shortcuts.add(shortcut);
                }
            }

            shortcutManager.setDynamicShortcuts(shortcuts);
        }
    }
}