package org.cagnulen.qdomyoszwift;

import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;

public class ShellRuntime {
    private static final String LOG_TAG = "QZ:Shell";

    private static final Runtime runtime = Runtime.getRuntime();
    private static String sh = null;

    static String lastLine;

    public static String lastOutput() {
        return lastLine;
    }

    public static void execAndGetOutput(String command) throws IOException {
        Process proc = exec(command);
        BufferedReader is = new BufferedReader(new InputStreamReader(proc.getInputStream()));
        String line;
        lastLine = "";
        while ((line = is.readLine()) != null) {
            //Log.d(LOG_TAG, line);
            lastLine = lastLine + line;
        }
    }

    public static Process exec(String command) throws IOException {
        String[] cmd = {getSh(), "-c", " " + command};
        return runtime.exec(cmd);
    }

    private static String getSh() {
        if(sh == null) {
            sh = "/bin/sh";

            try {
                execAndGetOutput("ls");
            } catch (final Exception ex) {
                Log.w(LOG_TAG, "Calling " + sh + " failed", ex);
                sh = "/system/bin/sh";
            }

            Log.d(LOG_TAG, "Using sh: " + sh);
        }

        return sh;
    }
}
