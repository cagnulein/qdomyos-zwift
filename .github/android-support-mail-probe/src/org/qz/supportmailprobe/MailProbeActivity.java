package org.qz.supportmailprobe;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.graphics.Color;
import android.graphics.Typeface;
import android.view.Gravity;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.ScrollView;
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

        // First draw a recognizable, readable compose preview. In older
        // emulator images the initial screenshot could otherwise show only
        // the former single-line "PASS ..." diagnostic text or a blank screen.
        setContentView(buildComposePreview(getIntent(), result));
        Log.i(TAG, result);
        try (FileOutputStream out = openFileOutput("result.txt", MODE_PRIVATE)) {
            out.write(result.getBytes(StandardCharsets.UTF_8));
        } catch (Exception error) {
            Log.e(TAG, "Cannot write test result", error);
        }
    }
    private int dp(int value) {
        return (int) (value * getResources().getDisplayMetrics().density + 0.5f);
    }

    private TextView label(String text, int sp, int color, boolean bold) {
        TextView view = new TextView(this);
        view.setText(text);
        view.setTextColor(color);
        view.setTextSize(sp);
        if (bold)
            view.setTypeface(null, Typeface.BOLD);
        view.setPadding(0, dp(5), 0, dp(5));
        return view;
    }

    private void field(LinearLayout content, String name, String value) {
        content.addView(label(name.toUpperCase(java.util.Locale.US), 11,
                Color.rgb(95, 105, 125), true));
        TextView detail = label(value, 16, Color.rgb(33, 42, 60), false);
        detail.setTextIsSelectable(true);
        content.addView(detail);
        View divider = new View(this);
        divider.setBackgroundColor(Color.rgb(230, 233, 238));
        LinearLayout.LayoutParams line = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, dp(1));
        line.setMargins(0, dp(10), 0, dp(12));
        content.addView(divider, line);
    }

    /**
     * This is an intentionally labeled TEST receiver, not a Gmail UI.
     * Unlike the former one-line PASS screen, it displays exactly the
     * compose payload delivered by Android to a separate application UID.
     */
    private View buildComposePreview(Intent intent, String result) {
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setBackgroundColor(Color.rgb(245, 247, 251));

        ScrollView scroll = new ScrollView(this);
        scroll.setFillViewport(true);
        root.addView(scroll, new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.MATCH_PARENT));

        LinearLayout page = new LinearLayout(this);
        page.setOrientation(LinearLayout.VERTICAL);
        page.setPadding(dp(18), dp(16), dp(18), dp(20));
        scroll.addView(page);

        TextView brand = label("QZ Mail Probe", 21, Color.rgb(29, 54, 99), true);
        page.addView(brand);
        page.addView(label("EMAIL COMPOSE PREVIEW - TEST RECEIVER (NOT GMAIL)",
                11, Color.rgb(102, 107, 124), true));

        TextView status = label(result, 13,
                result.startsWith("PASS") ? Color.rgb(22, 108, 55) : Color.rgb(174, 29, 46),
                true);
        status.setPadding(dp(10), dp(8), dp(10), dp(8));
        status.setBackgroundColor(result.startsWith("PASS")
                ? Color.rgb(221, 245, 226) : Color.rgb(255, 226, 226));
        LinearLayout.LayoutParams statusLayout = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT);
        statusLayout.setMargins(0, dp(12), 0, dp(16));
        page.addView(status, statusLayout);

        LinearLayout card = new LinearLayout(this);
        card.setOrientation(LinearLayout.VERTICAL);
        card.setPadding(dp(16), dp(14), dp(16), dp(18));
        card.setBackgroundColor(Color.WHITE);
        page.addView(card, new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT));

        String[] recipients = intent.getStringArrayExtra(Intent.EXTRA_EMAIL);
        field(card, "To", recipients != null
                ? android.text.TextUtils.join(", ", recipients) : "(missing)");
        field(card, "Subject", String.valueOf(intent.getStringExtra(Intent.EXTRA_SUBJECT)));
        field(card, "Message", String.valueOf(intent.getStringExtra(Intent.EXTRA_TEXT)));

        Uri attachment = intent.getParcelableExtra(Intent.EXTRA_STREAM);
        String attachmentLabel = attachment == null ? "No attachment"
                : attachment.getLastPathSegment() + "\\ncontent:// URI, gzip read verified";
        field(card, "Attachment", attachmentLabel);

        page.addView(label("Nothing is sent. This screen proves what the Android "
                + "email intent delivered to an independent app. Gmail/other installed "
                + "email clients are not part of this automated test.",
                12, Color.rgb(90, 99, 114), false));
        return root;
    }

}
