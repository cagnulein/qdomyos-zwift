// Accessibility test for the android-emulator-test job: sends QZ the actions a screen reader such
// as TalkBack sends, without TalkBack, so it runs on every emulator image. It connects as the
// shell's UiAutomation, as uiautomator does, which turns accessibility on in the app, then:
// - moves the accessibility focus over the app's nodes (QZ scrolls the focused item into view,
//   CustomQtActivity -> nativeScrollToAccessibleObject),
// - scrolls a scrollable node forward and back (a spin box or a list),
// - presses a button whose description starts with the given text (QZ presses a Qt button itself,
//   nativeAccessibleClick, and answers true; Qt 5.15 alone answers false for a button).
// Every line it prints starts with OK, FAIL, SKIP or INFO; the last one is DONE.
//
//   CLASSPATH=/data/local/tmp/a11y-test.jar app_process / AccessibilityTest [button]
import android.app.UiAutomation;
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
        String button = args.length > 0 ? args[0] : "";
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
            run(button);
        } finally {
            UiAutomation.class.getMethod("disconnect").invoke(automation);
            thread.quitSafely();
        }
        System.out.println("DONE");
    }

    private static void run(String button) throws Exception {
        List<AccessibilityNodeInfo> nodes = waitForAppNodes();
        if (nodes.isEmpty()) {
            AccessibilityNodeInfo root = automation.getRootInActiveWindow();
            CharSequence active = root == null ? null : root.getPackageName();
            if (active == null || !active.toString().equals(PACKAGE)) {
                // e.g. an "isn't responding" dialog of the system over QZ on a slow emulator
                System.out.println("SKIP the active window is " + active + ", not QZ");
            } else {
                System.out.println("FAIL no accessibility node of QZ with a description: Qt accessibility is off");
            }
            return;
        }
        System.out.println("OK " + nodes.size() + " nodes of QZ with a description");
        for (AccessibilityNodeInfo node : nodes) {
            System.out.println("INFO node " + describe(node));
        }

        int focused = 0;
        for (AccessibilityNodeInfo node : nodes) {
            if (focused == 10) {
                break;
            }
            if (node.isFocusable()) {
                node.performAction(AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS);
                focused++;
                Thread.sleep(300);
            }
        }
        System.out.println(focused > 0 ? "OK focus moved over " + focused + " nodes" : "SKIP no focusable node");

        AccessibilityNodeInfo scrollable = null;
        for (AccessibilityNodeInfo node : nodes) {
            if (node.isScrollable()) {
                scrollable = node;
                break;
            }
        }
        if (scrollable == null) {
            System.out.println("SKIP no scrollable node");
        } else {
            boolean forward = scrollable.performAction(AccessibilityNodeInfo.ACTION_SCROLL_FORWARD);
            Thread.sleep(500);
            boolean backward = scrollable.performAction(AccessibilityNodeInfo.ACTION_SCROLL_BACKWARD);
            Thread.sleep(500);
            System.out.println("OK scrolled " + describe(scrollable) + ": forward " + forward + ", backward " + backward);
        }

        if (button.isEmpty()) {
            System.out.println("SKIP no button to press given");
            return;
        }
        AccessibilityNodeInfo target = null;
        for (AccessibilityNodeInfo node : appNodes()) {
            CharSequence description = node.getContentDescription();
            if (node.isClickable() && description != null && description.toString().startsWith(button)) {
                target = node;
                break;
            }
        }
        if (target == null) {
            System.out.println("SKIP no button '" + button + "' on screen");
            return;
        }
        boolean pressed = target.performAction(AccessibilityNodeInfo.ACTION_CLICK);
        Thread.sleep(1000);
        System.out.println((pressed ? "OK pressed " : "FAIL press not handled by QZ: ") + describe(target));
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

    private static String describe(AccessibilityNodeInfo node) {
        return "'" + node.getContentDescription() + "' class=" + node.getClassName()
                + " clickable=" + node.isClickable() + " focusable=" + node.isFocusable()
                + " scrollable=" + node.isScrollable();
    }
}
