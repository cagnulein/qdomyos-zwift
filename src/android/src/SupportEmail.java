package org.cagnulen.qdomyoszwift;

import android.app.Activity;
import android.content.ClipData;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.widget.Toast;

import androidx.core.content.FileProvider;

import java.io.File;

/**
 * Launches an installed mail application with a pre-filled support request.
 * No message is sent without the rider reviewing and submitting it.
 */
public final class SupportEmail {
    private static final String TAG = "QZSupportEmail";

    private SupportEmail() { }

    public static boolean compose(Activity activity, String recipient, String subject,
                                  String body, String attachmentPath) {
        if (activity == null) {
            return false;
        }
        activity.runOnUiThread(() -> {
            try {
                Intent send = new Intent(Intent.ACTION_SEND);
                send.setType("message/rfc822");
                send.putExtra(Intent.EXTRA_EMAIL, new String[] {recipient});
                send.putExtra(Intent.EXTRA_SUBJECT, subject);
                send.putExtra(Intent.EXTRA_TEXT, body);
                if (attachmentPath != null && !attachmentPath.isEmpty()) {
                    File attachment = new File(attachmentPath);
                    if (attachment.isFile()) {
                        // Android 7+ prohibits file:// URI sharing. A FileProvider
                        // scoped to the app cache gives the chosen mail app read access.
                        Uri uri = FileProvider.getUriForFile(
                            activity, activity.getPackageName() + ".support", attachment);
                        send.putExtra(Intent.EXTRA_STREAM, uri);
                        send.setClipData(ClipData.newRawUri("QZ debug log", uri));
                        send.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    }
                }
                activity.startActivity(Intent.createChooser(send, "Send QZ support email"));
            } catch (Exception e) {
                Log.e(TAG, "Could not open email application", e);
                Toast.makeText(activity, "Could not open an email application", Toast.LENGTH_LONG).show();
            }
        });
        return true;
    }
}
