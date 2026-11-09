package org.cagnulen.qdomyoszwift;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.nio.charset.StandardCharsets;
import java.util.zip.GZIPOutputStream;

/**
 * Debug-only CI entry point. Exercises the actual production SupportEmail.compose()
 * with a real Activity, ACTION_CHOOSER and FileProvider, without Qt or BLE setup.
 * A separately installed fake email app checks the delivered intent and attachment.
 */
public final class SupportEmailTestActivity extends Activity {
    private static final String TAG = "QZSupportMailTest";
    private static final String RECIPIENT = "roberto.viola83@gmail.com";
    private static final String LOG_PAYLOAD = "QZ support mail test log: with_log";

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        String scenario = getIntent().getStringExtra("scenario");
        if (!"with_log".equals(scenario) && !"without_log".equals(scenario)
                && !"missing_log".equals(scenario)) {
            Log.e(TAG, "FAIL unknown scenario: " + scenario);
            finish();
            return;
        }

        try {
            String attachmentPath = "";
            if ("with_log".equals(scenario)) {
                File output = new File(getCacheDir(), "qz-support-ci-test.log.gz");
                try (GZIPOutputStream zip = new GZIPOutputStream(new FileOutputStream(output))) {
                    zip.write(LOG_PAYLOAD.getBytes(StandardCharsets.UTF_8));
                }
                attachmentPath = output.getAbsolutePath();
            } else if ("missing_log".equals(scenario)) {
                // A missing log must not turn into a broken content URI.
                attachmentPath = new File(getCacheDir(), "qz-support-missing.log.gz").getAbsolutePath();
            }

            Log.i(TAG, "START " + scenario);
            boolean launched = SupportEmail.compose(
                this, RECIPIENT, "QZ issue | Android CI | " + scenario,
                "QZ support mail test body: " + scenario, attachmentPath);
            if (!launched)
                Log.e(TAG, "FAIL compose() returned false");
        } catch (Exception error) {
            Log.e(TAG, "FAIL preparing mail test", error);
            finish();
        }
    }
}
