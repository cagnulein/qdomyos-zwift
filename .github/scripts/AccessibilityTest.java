// Accessibility test for the android-emulator-test job: sends QZ the actions a screen reader such
// as TalkBack sends, without TalkBack, so it runs on every emulator image. It connects as the
// shell's UiAutomation, as uiautomator does, which turns accessibility on in the app, waits until
// the screen stops changing, then:
// - moves the accessibility focus to a node below the screen, if there is one, and checks that QZ
//   scrolled it into view (CustomQtActivity -> nativeScrollToAccessibleObject),
// - moves the accessibility focus over the app's nodes; each one must take it,
// - presses the given buttons in turn (QZ presses a Qt button itself, nativeAccessibleClick, and
//   answers true; Qt 5.15 alone answers false for a button) and checks what each press shows.
// Every line it prints starts with OK, FAIL or INFO; the last one is DONE.
//
//   CLASSPATH=/data/local/tmp/a11y-test.jar app_process / AccessibilityTest <press> [<press> ...]
//
// A press is the start of a button's description, optionally with "=" and the start of the
// description of a node that the press must show within 5 s and that must still be there 2 s later
// (a second press, e.g. by auto repeat, would replace it). "_" stands for a space.
// Example: Start=First-time_setup Increase_Resistance=Resistance:_1
import android.app.UiAutomation;
import android.graphics.Rect;
import android.os.HandlerThread;
import android.os.Looper;
import android.view.accessibility.AccessibilityNodeInfo;
import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.List;

public class AccessibilityTest {
    private static final String PACKAGE = "org.cagnulen.qdomyoszwift";
    private static UiAutomation automation;

    public static void main(String[] args) throws Exception {
        // AccessibilityInteractionClient makes a Handler on the main looper, which app_process lacks
        Looper.prepareMainLooper();
        HandlerThread thread = new HandlerThread("AccessibilityTest");
        thread.start();
        Class<?> connectionInterface = Class.forName("android.app.IUiAutomationConnection");
        Object connection = Class.forName("android.app.UiAutomationConnection").getConstructor().newInstance();
        Constructor<UiAutomation> constructor = UiAutomation.class.getConstructor(Looper.class, connectionInterface);
        automation = constructor.newInstance(thread.getLooper(), connection);
        UiAutomation.class.getMethod("connect").invoke(automation);
        try {
            run(args);
        } finally {
            UiAutomation.class.getMethod("disconnect").invoke(automation);
            thread.quitSafely();
        }
        System.out.println("DONE");
    }

    private static void run(String[] presses) throws Exception {
        if (presses.length == 0) {
            System.out.println("FAIL no button to press given");
            return;
        }
        List<AccessibilityNodeInfo> nodes = waitForAppNodes();
        if (nodes.isEmpty()) {
            AccessibilityNodeInfo root = automation.getRootInActiveWindow();
            CharSequence active = root == null ? null : root.getPackageName();
            String nativeText = firstText(root);
            // the images where QZ cannot show its page (no Bluetooth, API < 30) are skipped by
            // android_accessibility_test.sh before this runs
            if (active == null || !active.toString().equals(PACKAGE)) {
                System.out.println("FAIL the active window is " + active + ", not QZ");
            } else if (nativeText != null) {
                System.out.println("FAIL a native window of QZ is in front: '" + nativeText + "'");
            } else {
                System.out.println("FAIL no accessibility node of QZ with a description: Qt accessibility is off");
            }
            return;
        }
        // Qt builds the page in steps after a start or a window in front of it: a node read too
        // early may be gone by the time an action is sent to it
        nodes = waitForSettledNodes();
        if (nodes == null) {
            System.out.println("FAIL the screen of QZ did not stop changing in 30 s");
            return;
        }
        System.out.println("OK " + nodes.size() + " nodes of QZ with a description");
        for (AccessibilityNodeInfo node : nodes) {
            System.out.println("INFO node " + describe(node));
        }

        if (!focusBelowScreen(nodes)) {
            return;
        }

        int focused = 0;
        for (AccessibilityNodeInfo node : appNodes()) {
            if (focused == 10) {
                break;
            }
            if (node.isFocusable()) {
                if (!node.performAction(AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS)) {
                    System.out.println("FAIL focus not taken by " + describe(node));
                    return;
                }
                focused++;
                Thread.sleep(300);
            }
        }
        if (focused == 0) {
            System.out.println("FAIL no focusable node of QZ");
            return;
        }
        System.out.println("OK focus moved over " + focused + " nodes");

        for (String press : presses) {
            if (!press(press)) {
                return;
            }
        }
    }

    // The focusable node that ends lowest below the screen gets the focus, and must then be on it
    private static boolean focusBelowScreen(List<AccessibilityNodeInfo> nodes) throws Exception {
        AccessibilityNodeInfo root = automation.getRootInActiveWindow();
        if (root == null) {
            System.out.println("FAIL no active window");
            return false;
        }
        Rect screen = new Rect();
        root.getBoundsInScreen(screen);
        AccessibilityNodeInfo lowest = null;
        Rect bounds = new Rect();
        int bottom = screen.bottom;
        for (AccessibilityNodeInfo node : nodes) {
            node.getBoundsInScreen(bounds);
            if (node.isFocusable() && bounds.bottom > bottom) {
                lowest = node;
                bottom = bounds.bottom;
            }
        }
        if (lowest == null) {
            System.out.println("INFO every node is on the screen: no scroll to the focus to check");
            return true;
        }
        String description = lowest.getContentDescription().toString();
        if (!lowest.performAction(AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS)) {
            System.out.println("FAIL focus not taken by " + describe(lowest));
            return false;
        }
        // the scroll runs on the Qt thread, which can be busy for seconds after a start while a
        // device connects (7 s seen on the API 36 emulator): wait for it up to 15 s
        AccessibilityNodeInfo moved = null;
        for (int i = 0; i < 30; i++) {
            Thread.sleep(500);
            moved = null;
            for (AccessibilityNodeInfo node : appNodes()) {
                if (node.equals(lowest)) {
                    moved = node;
                    break;
                }
            }
            if (moved == null) {
                System.out.println("FAIL '" + description + "' is gone after the focus");
                return false;
            }
            moved.getBoundsInScreen(bounds);
            if (bounds.top >= screen.top && bounds.bottom <= screen.bottom) {
                break;
            }
        }
        if (bounds.top < screen.top || bounds.bottom > screen.bottom) {
            System.out.println("FAIL not scrolled into view: " + describe(moved) + " " + bounds.toShortString()
                    + ", screen " + screen.toShortString());
            return false;
        }
        System.out.println("OK scrolled into view on focus: '" + description + "' from bottom " + bottom
                + " to " + bounds.toShortString());
        return true;
    }

    private static boolean press(String press) throws Exception {
        press = press.replace('_', ' ');
        int sign = press.indexOf('=');
        String button = sign < 0 ? press : press.substring(0, sign);
        String expect = sign < 0 ? null : press.substring(sign + 1);
        AccessibilityNodeInfo target = find(button, true);
        if (target == null) {
            System.out.println("FAIL no button '" + button + "' on screen");
            return false;
        }
        if (expect != null && find(expect, false) != null) {
            System.out.println("FAIL '" + expect + "' is on screen before '" + button + "' is pressed");
            return false;
        }
        if (!target.performAction(AccessibilityNodeInfo.ACTION_CLICK)) {
            System.out.println("FAIL press not handled by QZ: " + describe(target));
            return false;
        }
        if (expect == null) {
            Thread.sleep(1000);
            System.out.println("OK pressed " + describe(target));
            return true;
        }
        AccessibilityNodeInfo shown = null;
        for (int i = 0; i < 10 && shown == null; i++) {
            Thread.sleep(500);
            shown = find(expect, false);
        }
        if (shown == null) {
            System.out.println("FAIL pressed '" + button + "', but no '" + expect + "' in 5 s");
            for (AccessibilityNodeInfo node : appNodes()) {
                System.out.println("INFO node " + describe(node));
            }
            return false;
        }
        Thread.sleep(2000);
        if (find(expect, false) == null) {
            System.out.println("FAIL pressed '" + button + "': '" + expect + "' showed and went away (pressed twice?)");
            return false;
        }
        System.out.println("OK pressed '" + button + "': " + describe(shown));
        return true;
    }

    // The first node whose description starts with the text, and not with a longer number
    // ("Resistance: 1" is not "Resistance: 12"); a button must be clickable
    private static AccessibilityNodeInfo find(String text, boolean clickable) {
        for (AccessibilityNodeInfo node : appNodes()) {
            CharSequence description = node.getContentDescription();
            if ((!clickable || node.isClickable()) && description != null && startsWith(description.toString(), text)) {
                return node;
            }
        }
        return null;
    }

    private static boolean startsWith(String description, String text) {
        if (!description.startsWith(text)) {
            return false;
        }
        return description.length() == text.length() || text.isEmpty()
                || !Character.isDigit(text.charAt(text.length() - 1))
                || !Character.isDigit(description.charAt(text.length()));
    }

    // Qt adds its accessibility nodes once accessibility is on: wait for them up to 20 s
    private static List<AccessibilityNodeInfo> waitForAppNodes() throws Exception {
        for (int i = 0; i < 40; i++) {
            List<AccessibilityNodeInfo> nodes = appNodes();
            if (!nodes.isEmpty()) {
                return nodes;
            }
            Thread.sleep(500);
        }
        return new ArrayList<>();
    }

    // The nodes once their buttons are the same in two reads a second apart: values change on
    // their own (a running "Elapsed", "Next search in 3 s"), buttons only while a page is built;
    // null after 30 s
    private static List<AccessibilityNodeInfo> waitForSettledNodes() throws Exception {
        String last = null;
        String now = null;
        for (int i = 0; i < 30; i++) {
            List<AccessibilityNodeInfo> nodes = appNodes();
            now = snapshot(nodes);
            if (now.equals(last)) {
                return nodes;
            }
            if (i < 29) {
                last = now;
                Thread.sleep(1000);
            }
        }
        // what kept changing: the lines of the last read that the one before did not have
        for (String line : now.split("\n")) {
            if (!last.contains(line + "\n")) {
                System.out.println("INFO still changing: " + line);
            }
        }
        return null;
    }

    private static String snapshot(List<AccessibilityNodeInfo> nodes) {
        StringBuilder out = new StringBuilder();
        Rect bounds = new Rect();
        for (AccessibilityNodeInfo node : nodes) {
            if (node.isClickable()) {
                node.getBoundsInScreen(bounds);
                out.append(node.getContentDescription()).append(' ').append(bounds.toShortString()).append('\n');
            }
        }
        return out.toString();
    }

    private static List<AccessibilityNodeInfo> appNodes() {
        List<AccessibilityNodeInfo> out = new ArrayList<>();
        AccessibilityNodeInfo root = automation.getRootInActiveWindow();
        if (root != null && root.getPackageName() != null && root.getPackageName().toString().equals(PACKAGE)) {
            collect(root, out);
        }
        return out;
    }

    private static void collect(AccessibilityNodeInfo node, List<AccessibilityNodeInfo> out) {
        if (node == null) {
            return;
        }
        CharSequence description = node.getContentDescription();
        if (description != null && description.length() > 0) {
            out.add(node);
        }
        for (int i = 0; i < node.getChildCount(); i++) {
            collect(node.getChild(i), out);
        }
    }

    // The first text of a native Android view in the tree; Qt sets descriptions, not texts
    private static String firstText(AccessibilityNodeInfo node) {
        if (node == null) {
            return null;
        }
        CharSequence text = node.getText();
        if (text != null && text.length() > 0) {
            return text.toString();
        }
        for (int i = 0; i < node.getChildCount(); i++) {
            String found = firstText(node.getChild(i));
            if (found != null) {
                return found;
            }
        }
        return null;
    }

    private static String describe(AccessibilityNodeInfo node) {
        return "'" + node.getContentDescription() + "' class=" + node.getClassName()
                + " clickable=" + node.isClickable() + " focusable=" + node.isFocusable()
                + " scrollable=" + node.isScrollable();
    }
}
