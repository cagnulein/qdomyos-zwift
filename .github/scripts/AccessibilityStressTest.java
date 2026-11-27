// Stress test of the accessibility layer on Android. One thread reads the whole accessibility tree
// of QZ again and again without pause, as a screen reader does while a page changes, and the main
// thread opens the tile settings page (Settings -> Tiles Options, some 90 items) and goes back, again
// and again, so that Qt builds and destroys many QML items while Android asks for their nodes. Qt
// 5.15.0 answered those questions on the Android UI thread, and QZ crashed (QTBUG-95764); main.cpp
// moves them to the Qt thread. Run through app_process after the fake bike accessibility test, with
// QZ on its home page:
//
//   CLASSPATH=/data/local/tmp/a11y-stress.jar app_process / AccessibilityStressTest pages <cycles>
//   CLASSPATH=/data/local/tmp/a11y-stress.jar app_process / AccessibilityStressTest connect
//
// "connect" only connects (which turns accessibility on in QZ), reads the tree once and leaves: run
// many times, it turns accessibility on and off while QZ runs (QTBUG-144911).
// Lines start with OK, FAIL, INFO or DUMP (the output of "dumpsys input"); the last one is DONE.
import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.app.UiAutomation;
import android.graphics.Rect;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.SystemClock;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;

public class AccessibilityStressTest {
    private static final String PACKAGE = "org.cagnulen.qdomyoszwift";
    private static final String TILES_PAGE = "Speed, order index:";
    private static UiAutomation automation;
    private static volatile boolean reading = true;
    private static volatile long reads = 0;
    private static int cacheClears = 0;
    private static int presses = 0;
    private static boolean inputDumped = false;

    public static void main(String[] args) throws Exception {
        Looper.prepareMainLooper();
        HandlerThread thread = new HandlerThread("AccessibilityStressTest");
        thread.start();
        Class<?> connectionInterface = Class.forName("android.app.IUiAutomationConnection");
        Object connection = Class.forName("android.app.UiAutomationConnection").getConstructor().newInstance();
        Constructor<UiAutomation> constructor = UiAutomation.class.getConstructor(Looper.class, connectionInterface);
        automation = constructor.newInstance(thread.getLooper(), connection);
        UiAutomation.class.getMethod("connect").invoke(automation);
        try {
            // Android 9 does not reset the active window id when the window of QZ drops its
            // accessibility connection (accessibility goes off in QZ when the previous connection
            // leaves, or QZ is restarted), and without interactive windows sets it again only from a
            // window event of QZ. But it turns accessibility on in QZ before it tells QZ which
            // events to send, so QZ may drop that event. Then getRootInActiveWindow() stays null
            // for the whole wait, without a question to QZ (on the API 28 emulator 8 of 900
            // "connect" runs, with no event from QZ in each; a "pages" run once too). With
            // interactive windows on, the active window comes from the focus of the window
            // manager. Android 10 fixed this (AOSP 6c7522aecc and 6c8a43b685).
            AccessibilityServiceInfo info = automation.getServiceInfo();
            info.flags |= AccessibilityServiceInfo.FLAG_RETRIEVE_INTERACTIVE_WINDOWS;
            automation.setServiceInfo(info);
            if (args.length > 0 && args[0].equals("connect")) {
                // the windows come to a new connection some time after it: wait up to 5 s for them
                long start = SystemClock.uptimeMillis();
                int nodes = appNodes(false).size();
                while (nodes == 0 && SystemClock.uptimeMillis() - start < 5000) {
                    Thread.sleep(100);
                    nodes = appNodes(false).size();
                }
                System.out.println("INFO " + nodes + " nodes after " + (SystemClock.uptimeMillis() - start) + " ms");
            } else if (args.length > 1 && args[0].equals("pages")) {
                pages(Integer.parseInt(args[1]));
            } else {
                System.out.println("FAIL usage: pages <cycles> | connect");
            }
        } finally {
            UiAutomation.class.getMethod("disconnect").invoke(automation);
            thread.quitSafely();
        }
        System.out.println("DONE");
    }

    private static void pages(int cycles) throws Exception {
        Thread reader = new Thread(new Runnable() {
            @Override
            public void run() {
                // only a read that got nodes of QZ asked QZ anything: an empty one (another app in
                // front, a page changing, the cache below) is not counted
                long emptySince = 0;
                while (reading) {
                    try {
                        if (!appNodes(true).isEmpty()) {
                            reads++;
                            emptySince = 0;
                        } else if (emptySince == 0) {
                            emptySince = SystemClock.uptimeMillis();
                        } else if (SystemClock.uptimeMillis() - emptySince > 1000) {
                            clearCache(false);
                            emptySince = 0;
                        }
                    } catch (RuntimeException e) {
                        // a node of a window that went away: read again
                    }
                }
            }
        });
        reader.start();
        int taps = 0;
        try {
            for (int i = 1; i <= cycles; i++) {
                // From wherever QZ is (home, menu, settings): a finger's tap on the next step to the
                // tiles page, again if a tap was lost, up to 45 s. Finger taps and not the press of
                // the accessibility action (but for taps Android gave to the input sink, below): this test is
                // about the reads.
                long end = SystemClock.uptimeMillis() + 45000;
                long emptySince = 0;
                boolean shown = false;
                // The step of the last tap, how many taps in a row before it were on the same step (two
                // can be normal: "◄" from the settings to home, then "◄" to the menu) and the time of
                // the first of them. Once on API 33 Android gave every finger tap for 33 s to the input
                // sink of the activity (InputDispatcher: "Not sending touch gesture to ...
                // ActivityRecordInputSink ... because it is not responsive") and not to the window of
                // QZ, with no crash or ANR, and the test stood on one page for 45 s. After 3 taps on
                // the same step, if the logcat has such lines for QZ since the first of them, the step
                // is pressed with the accessibility action, which does not go through the input
                // dispatcher; the first time "dumpsys input" tells which windows took the touch.
                // Taps with no such line are not pressed: they may have reached QZ and done nothing, and
                // the page that does not open fails the test. Each step Android gave to the input sink
                // (pressed or not) is a warning of the run (android_accessibility_stress_test.sh): why
                // the window of QZ did not take the touch is not known
                String lastStep = null;
                String stepSince = null;
                int repeats = 0;
                while (SystemClock.uptimeMillis() < end) {
                    // a screen of another app over QZ (Health Connect on API 35+ after a start)
                    String active = activePackage();
                    if (active != null && !PACKAGE.equals(active)) {
                        automation.performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
                        Thread.sleep(1500);
                        continue;
                    }
                    List<AccessibilityNodeInfo> nodes = appNodes(false);
                    if (nodes.isEmpty() && active != null) {
                        emptySince = emptySince == 0 ? SystemClock.uptimeMillis() : emptySince;
                        if (SystemClock.uptimeMillis() - emptySince > 5000) {
                            clearCache(true);
                            emptySince = 0;
                        }
                        Thread.sleep(300);
                        continue;
                    }
                    emptySince = 0;
                    if (find(nodes, TILES_PAGE, false) != null) {
                        shown = true;
                        break;
                    }
                    AccessibilityNodeInfo next = find(nodes, "Tiles Options", true);
                    if (next == null)
                        next = find(nodes, "Settings", false);
                    if (next == null)
                        next = find(nodes, "◄", true);
                    if (next == null) {
                        Thread.sleep(300);
                        continue;
                    }
                    String step = String.valueOf(next.getContentDescription());
                    repeats = step.equals(lastStep) ? repeats + 1 : 0;
                    lastStep = step;
                    if (repeats == 0)
                        stepSince = logcatTime();
                    boolean done;
                    if (repeats >= 3 && sinkTouches(stepSince) > 0) {
                        if (!inputDumped) {
                            inputDumped = true;
                            dumpInput();
                        }
                        // CustomQtActivity presses a node that is not marked clickable too ("Settings"
                        // of the menu, an ItemDelegate) and answers true only when main.cpp pressed it
                        done = next.performAction(AccessibilityNodeInfo.ACTION_CLICK);
                        System.out.println("INFO Android gave " + repeats + " finger taps in a row on '" + step
                                + "' to the input sink, " + (done ? "pressed it with the accessibility action"
                                : "the accessibility action pressed nothing, tapping it again"));
                        if (done)
                            presses++;
                        else
                            done = tap(next);
                        // the next taps are counted anew
                        lastStep = null;
                    } else {
                        if (repeats == 3)
                            System.out.println("INFO 3 finger taps in a row on '" + step
                                    + "' did not take QZ further, with no line of the input dispatcher about its input sink");
                        done = tap(next);
                    }
                    if (done) {
                        taps++;
                        Thread.sleep(1500);
                    } else {
                        Thread.sleep(300);
                    }
                }
                if (!shown) {
                    System.out.println("FAIL the tiles page did not show in cycle " + i + " in 45 s, the active window is "
                            + activePackage() + ", it shows: " + shownTexts() + "; the tree:" + outline());
                    dumpInput();
                    return;
                }
                automation.performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
                // the next cycle must not take the closing page for a new one
                long closeEnd = SystemClock.uptimeMillis() + 10000;
                while (find(appNodes(false), TILES_PAGE, false) != null && SystemClock.uptimeMillis() < closeEnd)
                    Thread.sleep(200);
                if (find(appNodes(false), TILES_PAGE, false) != null) {
                    System.out.println("FAIL the tiles page did not close in cycle " + i + " in 10 s");
                    return;
                }
            }
        } finally {
            reading = false;
            reader.join(10000);
        }
        // A whole read asks QZ for each of its 160 to 260 nodes; 40 cycles gave 46 to 150 whole reads on
        // the emulators of API 24-37. Far fewer means the reader did not load QZ, and a pass would prove nothing
        if (reads < cycles / 4) {
            System.out.println("FAIL the tree of QZ was read only " + reads + " times in " + cycles + " cycles, "
                    + cacheClears + " clears of the accessibility cache");
            return;
        }
        System.out.println("OK opened and closed the tiles page " + cycles + " times, " + taps + " taps (" + presses
                + " of them with the accessibility action after Android gave finger taps to the input sink), " + reads
                + " reads of the tree, " + cacheClears + " clears of the accessibility cache");
    }

    // Now, as the logcat writes its time
    private static String logcatTime() {
        return new SimpleDateFormat("MM-dd HH:mm:ss.SSS", Locale.US).format(new Date());
    }

    // The lines of the input dispatcher since the given time that tell it gave a touch to the input
    // sink of the activity of QZ and not to its window
    private static int sinkTouches(String since) {
        int count = 0;
        try {
            Process logcat = new ProcessBuilder("logcat", "-d", "-t", since, "-s", "InputDispatcher:W")
                    .redirectErrorStream(true).start();
            BufferedReader output = new BufferedReader(new InputStreamReader(logcat.getInputStream()));
            String line;
            while ((line = output.readLine()) != null) {
                if (line.contains("ActivityRecordInputSink") && line.contains(PACKAGE))
                    count++;
            }
            logcat.waitFor();
        } catch (Exception e) {
            System.out.println("INFO cannot read the logcat: " + e);
        }
        return count;
    }

    // The windows of the input dispatcher, which of them take touches and where, as DUMP lines
    private static void dumpInput() {
        try {
            Process dumpsys = new ProcessBuilder("dumpsys", "input").redirectErrorStream(true).start();
            BufferedReader output = new BufferedReader(new InputStreamReader(dumpsys.getInputStream()));
            String line;
            while ((line = output.readLine()) != null)
                System.out.println("DUMP " + line);
            dumpsys.waitFor();
        } catch (Exception e) {
            System.out.println("INFO cannot run dumpsys input: " + e);
        }
    }

    private static String activePackage() {
        AccessibilityNodeInfo root = automation.getRootInActiveWindow();
        return root == null ? null : String.valueOf(root.getPackageName());
    }

    // The first node whose description starts with text, or null; a node with no size is not
    // shown (a page under the top one keeps its items)
    private static AccessibilityNodeInfo find(List<AccessibilityNodeInfo> nodes, String text, boolean clickable) {
        Rect bounds = new Rect();
        for (AccessibilityNodeInfo node : nodes) {
            CharSequence description = node.getContentDescription();
            node.getBoundsInScreen(bounds);
            if (description != null && !bounds.isEmpty() && (!clickable || node.isClickable())
                    && description.toString().startsWith(text)) {
                return node;
            }
        }
        return null;
    }

    // The first descriptions of shown nodes, to tell in a failure where QZ was
    private static String shownTexts() {
        StringBuilder out = new StringBuilder();
        Rect bounds = new Rect();
        int count = 0;
        for (AccessibilityNodeInfo node : appNodes(false)) {
            node.getBoundsInScreen(bounds);
            if (bounds.isEmpty())
                continue;
            if (count > 0)
                out.append(" | ");
            out.append(node.getContentDescription());
            if (++count == 12)
                break;
        }
        return out.toString();
    }

    // The classes and child counts of the first nodes of the active window, to tell in a failure
    // what the tree of this process holds
    private static String outline() {
        StringBuilder out = new StringBuilder();
        outline(automation.getRootInActiveWindow(), 0, out);
        return out.toString();
    }

    private static void outline(AccessibilityNodeInfo node, int depth, StringBuilder out) {
        if (node == null || depth > 8 || out.length() > 1500)
            return;
        out.append(' ').append(depth).append(':').append(node.getClassName()).append('[').append(node.getChildCount())
                .append(']');
        for (int i = 0; i < node.getChildCount() && i < 3; i++)
            outline(node.getChild(i), depth + 1, out);
    }

    // The nodes this process keeps of other windows: on the API 24 image, once a page changed, the
    // root of QZ stayed a View$VirtualChild without children, with and without the fix of main.cpp,
    // and right after this clear the tree had all its nodes (so probably Android's cache, not QZ).
    // The reader clears after 1 s with no node, silently; the main thread after 5 s, logged, so that
    // a failure tells it was used. Hidden API, through reflection; synchronized: both threads call it.
    private static synchronized void clearCache(boolean log) {
        String before = log ? outline() : "";
        try {
            Class<?> client = Class.forName("android.view.accessibility.AccessibilityInteractionClient");
            Object instance = client.getMethod("getInstance").invoke(null);
            try {
                client.getMethod("clearCache").invoke(instance);
            } catch (NoSuchMethodException e) {
                // API 33+: a cache per connection
                Field id = UiAutomation.class.getDeclaredField("mConnectionId");
                id.setAccessible(true);
                client.getMethod("clearCache", int.class).invoke(instance, id.getInt(automation));
            }
            cacheClears++;
            if (log)
                System.out.println("INFO no node of QZ for 5 s, accessibility cache cleared; the tree before:" + before
                        + "; after: " + appNodes(false).size() + " nodes");
        } catch (Exception e) {
            if (log)
                System.out.println("INFO no node of QZ for 5 s, cannot clear the accessibility cache: " + e);
        }
    }

    // A finger's tap in the middle of the node
    private static boolean tap(AccessibilityNodeInfo node) throws Exception {
        Rect bounds = new Rect();
        node.getBoundsInScreen(bounds);
        if (bounds.isEmpty()) {
            return false;
        }
        long down = SystemClock.uptimeMillis();
        boolean injected = true;
        int[] actions = {MotionEvent.ACTION_DOWN, MotionEvent.ACTION_UP};
        for (int action : actions) {
            if (action == MotionEvent.ACTION_UP) {
                Thread.sleep(100);
            }
            MotionEvent event = MotionEvent.obtain(down, SystemClock.uptimeMillis(), action,
                    bounds.exactCenterX(), bounds.exactCenterY(), 0);
            event.setSource(InputDevice.SOURCE_TOUCHSCREEN);
            injected &= automation.injectInputEvent(event, true);
            event.recycle();
        }
        return injected;
    }

    // The nodes of QZ with a description; with refresh each node is asked of QZ anew, not taken
    // from the cache of this process, so QZ answers for every one
    private static List<AccessibilityNodeInfo> appNodes(boolean refresh) {
        List<AccessibilityNodeInfo> out = new ArrayList<>();
        AccessibilityNodeInfo root = automation.getRootInActiveWindow();
        if (root != null && PACKAGE.equals(String.valueOf(root.getPackageName()))) {
            collect(root, out, refresh);
        }
        return out;
    }

    private static void collect(AccessibilityNodeInfo node, List<AccessibilityNodeInfo> out, boolean refresh) {
        if (node == null || (refresh && !node.refresh())) {
            return;
        }
        CharSequence description = node.getContentDescription();
        if (description != null && description.length() > 0) {
            out.add(node);
        }
        for (int i = 0; i < node.getChildCount(); i++) {
            collect(node.getChild(i), out, refresh);
        }
    }
}
