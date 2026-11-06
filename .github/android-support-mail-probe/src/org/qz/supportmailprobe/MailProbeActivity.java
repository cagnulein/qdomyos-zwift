package org.qz.supportmailprobe;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.zip.GZIPInputStream;

/**
 * An instrumented fake mail recipient, not a real email service.
 * Verifies what a distinct app/UID actually received from Android's share chooser.
 */
public final class MailProbeActivity extends Activity {
    private static final String TAG = "QZMailProbe";

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        String result;
        try {
            Intent intent = getIntent();
            if (!Intent.ACTION_SEND.equals(intent.getAction()))
                throw new IllegalStateException("Not ACTION_SEND: " + intent.getAction());
            if (!"message/rfc822".equals(intent.getType()))
                throw new IllegalStateException("Unexpected MIME type: " + intent.getType());

            String[] recipients = intent.getStringArrayExtra(Intent.EXTRA_EMAIL);
            if (recipients == null || recipients.length != 1 ||
                    !"roberto.viola83@gmail.com".equals(recipients[0]))
                throw new IllegalStateException("Incorrect EXTRA_EMAIL");
            String subject = intent.getStringExtra(Intent.EXTRA_SUBJECT);
            if (subject == null || !subject.startsWith("QZ issue | Android CI | "))
                throw new IllegalStateException("Incorrect EXTRA_SUBJECT: " + subject);
            String scenario = subject.substring("QZ issue | Android CI | ".length());
            String body = intent.getStringExtra(Intent.EXTRA_TEXT);
            if (!("QZ support mail test body: " + scenario).equals(body))
                throw new IllegalStateException("Incorrect EXTRA_TEXT: " + body);

            Uri uri = intent.getParcelableExtra(Intent.EXTRA_STREAM);
            if ("with_log".equals(scenario)) {
                if (uri == null || !"content".equals(uri.getScheme()))
                    throw new IllegalStateException("Missing content:// EXTRA_STREAM");
                if (!"org.cagnulen.qdomyoszwift.support".equals(uri.getAuthority()))
                    throw new IllegalStateException("Unexpected FileProvider authority");
                if ((intent.getFlags() & Intent.FLAG_GRANT_READ_URI_PERMISSION) == 0)
                    throw new IllegalStateException("Missing read permission grant");
                // Proves the grant works cross-UID, not only that URI generation succeeded.
                try (InputStream stream = getContentResolver().openInputStream(uri);
                     GZIPInputStream gzip = new GZIPInputStream(stream);
                     BufferedReader reader = new BufferedReader(new InputStreamReader(
                         gzip, StandardCharsets.UTF_8))) {
                    if (!"QZ support mail test log: with_log".equals(reader.readLine()))
                        throw new IllegalStateException("Log attachment is unreadable or corrupted");
                }
            } else if ("without_log".equals(scenario) || "missing_log".equals(scenario)) {
                if (uri != null)
                    throw new IllegalStateException("Unexpected EXTRA_STREAM for " + scenario);
            } else {
                throw new IllegalStateException("Unknown scenario " + scenario);
            }
            result = "PASS " + scenario + " API=" + android.os.Build.VERSION.SDK_INT;
        } catch (Throwable error) {
            result = "FAIL " + error.getClass().getSimpleName() + " " + error.getMessage();
            Log.e(TAG, result, error);
        }

        Log.i(TAG, result);
        try (FileOutputStream out = openFileOutput("result.txt", MODE_PRIVATE)) {
            out.write(result.getBytes(StandardCharsets.UTF_8));
        } catch (Exception error) {
            Log.e(TAG, "Cannot write test result", error);
        }
        TextView message = new TextView(this);
        message.setText("QZ Mail Probe: " + result);
        message.setTextSize(18);
        setContentView(message);
    }
}
