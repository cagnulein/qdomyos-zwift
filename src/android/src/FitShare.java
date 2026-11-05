package org.cagnulen.qdomyoszwift;

import android.app.Activity;
import android.content.ClipData;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.widget.Toast;

import androidx.core.content.FileProvider;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

// Shares a saved workout file through the Android share sheet (mail, messengers, cloud drives).
// The file is copied to the cache folder first: FitFileProvider serves only that folder, so it
// works wherever QZ saved the workout (Documents/QZ or the app folder).
public class FitShare {
    // failedText: shown as a toast when the sheet can't be opened (the file is missing or not
    // readable, no room in the cache)
    public static void share(final Context context, final String path, final String title, final String failedText) {
        try {
            File source = new File(path);
            File folder = new File(context.getCacheDir(), "share");
            // only the copy of this share is kept: the copies of earlier shares are removed, also
            // one left by a share that failed
            File[] old = folder.listFiles();
            if (old != null) {
                for (File f : old) {
                    f.delete();
                }
            }
            folder.mkdirs();
            File copy = new File(folder, source.getName());
            InputStream in = new FileInputStream(source);
            try {
                OutputStream out = new FileOutputStream(copy);
                try {
                    byte[] buffer = new byte[8192];
                    int n;
                    while ((n = in.read(buffer)) > 0) {
                        out.write(buffer, 0, n);
                    }
                } finally {
                    out.close();
                }
            } finally {
                in.close();
            }

            // the authority of FitFileProvider in AndroidManifest.xml
            Uri uri = FileProvider.getUriForFile(context, context.getPackageName() + ".fitshare", copy);
            Intent send = new Intent(Intent.ACTION_SEND);
            send.setType("application/octet-stream");
            send.putExtra(Intent.EXTRA_STREAM, uri);
            send.putExtra(Intent.EXTRA_SUBJECT, source.getName());
            // the chooser passes the read permission on only through ClipData
            send.setClipData(ClipData.newRawUri(source.getName(), uri));
            send.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            final Intent chooser = Intent.createChooser(send, title);
            chooser.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            if (context instanceof Activity) {
                ((Activity) context).runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        context.startActivity(chooser);
                    }
                });
            } else {
                chooser.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(chooser);
            }
            QLog.d("FitShare", "share " + path + " -> " + uri);
        } catch (Exception e) {
            QLog.e("FitShare", "share failed for " + path + ": " + e);
            new Handler(Looper.getMainLooper()).post(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(context, failedText, Toast.LENGTH_LONG).show();
                }
            });
        }
    }
}
