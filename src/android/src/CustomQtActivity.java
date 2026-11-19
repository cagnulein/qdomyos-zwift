package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.SparseArray;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.view.WindowInsets;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityManager;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.accessibility.AccessibilityNodeProvider;
import android.view.ViewTreeObserver;
import android.webkit.WebView;
import android.window.OnBackInvokedCallback;
import android.window.OnBackInvokedDispatcher;
import android.view.WindowManager;
import android.view.Window;
import android.view.WindowInsetsController;
import android.view.DisplayCutout;
import android.graphics.Insets;
import org.qtproject.qt5.android.QtActivityDelegate;
import org.qtproject.qt5.android.QtNative;
import org.qtproject.qt5.android.bindings.QtActivity;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Set;
import java.util.WeakHashMap;

public class CustomQtActivity extends QtActivity {
    private static final String TAG = "CustomQtActivity";
    private static final int DOCUMENT_PICKER_PROFILE_REQUEST_CODE = 4101;
    private static final int DOCUMENT_PICKER_TRAINING_REQUEST_CODE = 4102;
    private static final int DOCUMENT_PICKER_GPX_REQUEST_CODE = 4103;
    private static final int DOCUMENT_PICKER_SETTINGS_REQUEST_CODE = 4104;
    private static final int DOCUMENT_PICKER_FIT_REQUEST_CODE = 4105;
    private static final int DOCUMENT_PICKER_FIT_FOLDER_REQUEST_CODE = 4106;
    private static final int DOCUMENT_PICKER_RESTORE_FOLDER_REQUEST_CODE = 4107;
    private final SparseArray<String> pendingImportDirectories = new SparseArray<>();

    // Declare the native method that will be implemented in C++
    private static native void onInsetsChanged(int top, int bottom, int left, int right,
                                               int waterfallTop, int waterfallBottom,
                                               int waterfallLeft, int waterfallRight);
    // Left and right system bar insets without the display cutout, so QML can let the
    // content go under the cutout when the user turns the cutout margin off.
    private static native void onSystemBarSideInsetsChanged(int left, int right);
    private static native void nativeOnOAuthCallback(String callbackUrl);
    private static native void nativeOnDocumentPicked(int requestCode, int resultCode, String localPath);
    // a workout import starts copying: the history page shows that it is running
    private static native void nativeOnFitImportStarted();
    // a workout import copied done of total files out of the picked folder; 0, 0 when the copy is
    // over: the history page shows how far it is
    private static native void nativeOnFitImportProgress(int done, int total);
    // a restore of the QZ folder copied done of total files: the list pages show how far it is
    private static native void nativeOnFolderRestoreProgress(int done, int total);
    // TalkBack put its focus on this accessible object: scroll it into view
    private static native void nativeScrollToAccessibleObject(int id);
    // TalkBack's double tap on this accessible object: true when QZ pressed it (a Qt button with
    // the space key, anything else with its press or toggle action)
    private static native boolean nativeAccessibleClick(int id);
    // the configuration changed (night mode among others, uiMode is in configChanges): the modern
    // look reads the system theme again
    private static native void onSystemThemeChanged();

    @Override
    public void onConfigurationChanged(android.content.res.Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        try {
            onSystemThemeChanged();
        } catch (UnsatisfiedLinkError ignored) {
            // Qt not loaded yet: QML reads the theme when it starts
        }
    }

    private void dispatchOAuthCallback(Intent intent) {
        if (intent == null) {
            return;
        }

        Uri data = intent.getData();
        if (data == null) {
            return;
        }

        String url = data.toString();
        if (url.startsWith("https://www.qzfitness.com/peloton/callback")) {
            Log.d(TAG, "dispatchOAuthCallback: https://www.qzfitness.com/peloton/callback?code=XXXX&state=XXXX");
            try {
                nativeOnOAuthCallback(url);
            } catch (UnsatisfiedLinkError e) {
                Log.w(TAG, "Qt not ready yet for OAuth callback, ignoring: " + e.getMessage());
            }
        }
    }

    private boolean injectingBackKey = false;
    // A system KEYCODE_BACK ACTION_DOWN went to the Qt delegate and its ACTION_UP has not arrived yet
    private boolean backDownSentToQt = false;
    // The callback already released that ACTION_DOWN in Qt, so the system ACTION_UP is a duplicate
    private boolean dropNextSystemBackUp = false;

    // The manifest opts out of predictive back (enableOnBackInvokedCallback="false"), so Android
    // keeps sending KEYCODE_BACK and Qt turns an unhandled one into a close event that
    // main.qml handles in onClosing. While the opt-out is honored, the system refuses this
    // callback (it only logs "OnBackInvokedCallback is not enabled") and nothing changes.
    // When the system ignores the opt-out, back goes to the callback instead, and the callback
    // feeds Qt the same back key, so main.qml sees no difference. In the API 33-35 emulators
    // no KEYCODE_BACK reaches the activity then; in the API 36 emulator ACTION_DOWN still comes
    // before the callback and a canceled ACTION_UP after it. The callback completes such an
    // ACTION_DOWN, and dispatchKeyEvent() drops the system ACTION_UP that follows (canceled or
    // not), so Qt sees exactly one press either way. A canceled ACTION_UP without the callback
    // (a back gesture or press the user aborted) is dropped too, so it no longer goes back.
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (event.getKeyCode() == KeyEvent.KEYCODE_BACK && event.getAction() == KeyEvent.ACTION_DOWN) {
            // A keyboard on the screen takes Back before the activity; here it is gone, unless the
            // insets still show it. Tell Qt before it decides whether this Back only hides it
            // (see the keyboard listener in onCreate)
            keyboardHiddenByInsets();
        }
        if (event.getKeyCode() == KeyEvent.KEYCODE_BACK && !injectingBackKey) {
            if (event.getAction() == KeyEvent.ACTION_DOWN && event.getRepeatCount() == 0) {
                backDownSentToQt = true;
                dropNextSystemBackUp = false;
            } else if (event.getAction() == KeyEvent.ACTION_UP) {
                boolean drop = dropNextSystemBackUp || event.isCanceled();
                backDownSentToQt = false;
                dropNextSystemBackUp = false;
                if (drop) {
                    return true;
                }
            }
        }
        return super.dispatchKeyEvent(event);
    }

    // API 30+: when the window insets show no keyboard, Qt is told it is hidden
    private void keyboardHiddenByInsets() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
            return;
        }
        WindowInsets insets = getWindow().getDecorView().getRootWindowInsets();
        if (insets == null || insets.isVisible(WindowInsets.Type.ime())) {
            return;
        }
        QtActivityDelegate delegate = QtNative.activityDelegate();
        if (delegate != null) {
            delegate.setKeyboardVisibility(false, System.nanoTime());
        }
    }

    private void registerBackCallback() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU) {
            return;
        }
        getOnBackInvokedDispatcher().registerOnBackInvokedCallback(
                OnBackInvokedDispatcher.PRIORITY_DEFAULT,
                new OnBackInvokedCallback() {
                    @Override
                    public void onBackInvoked() {
                        // A back key that Qt leaves unhandled can come back here through
                        // Activity.onBackPressed(); stop instead of looping.
                        if (injectingBackKey) {
                            return;
                        }
                        Log.d(TAG, "onBackInvoked: sending KEYCODE_BACK to Qt, down already sent: " + backDownSentToQt);
                        injectingBackKey = true;
                        boolean downAlreadySent = backDownSentToQt;
                        try {
                            if (!downAlreadySent) {
                                dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_BACK));
                            }
                            dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_BACK));
                        } finally {
                            injectingBackKey = false;
                            backDownSentToQt = false;
                            dropNextSystemBackUp = downAlreadySent;
                        }
                    }
                });
    }

    // Qt 5.15.0 sends a screen reader no accessibility events at all (on Android its
    // notifyAccessibilityUpdate() is a FIXME until 5.15.3, scrolling is reported from 5.15.11),
    // so TalkBack keeps the nodes it cached: after a page is pushed it still walks the page
    // under it, and after a scroll it still has the rows that came into view where they were.
    // Telling Android that the subtree of Qt's accessibility view changed drops that cache:
    // main.cpp asks for it (accessibilityTreeChanged) when a page or a popup shows or hides and
    // when a list stops scrolling.
    private final Handler accessibilityHandler = new Handler(Looper.getMainLooper());
    private View qtAccessibilityView = null;

    private boolean accessibilityEnabled() {
        AccessibilityManager manager = (AccessibilityManager) getSystemService(ACCESSIBILITY_SERVICE);
        return manager != null && manager.isEnabled();
    }

    // A screen reader (TalkBack) is on: it turns touch exploration on. isEnabled() is true with any
    // accessibility service (a password manager, a launcher, a screen time app), which moves no focus
    // and says nothing
    private boolean screenReaderOn() {
        AccessibilityManager manager = (AccessibilityManager) getSystemService(ACCESSIBILITY_SERVICE);
        return manager != null && manager.isEnabled() && manager.isTouchExplorationEnabled();
    }

    private void sendAccessibilityTreeChanged() {
        if (!accessibilityEnabled()) {
            return;
        }
        View view = qtAccessibilityView();
        ViewParent parent = view != null ? view.getParent() : null;
        if (parent == null) {
            return;
        }
        AccessibilityEvent event = AccessibilityEvent.obtain(AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED);
        event.setContentChangeTypes(AccessibilityEvent.CONTENT_CHANGE_TYPE_SUBTREE);
        view.onInitializeAccessibilityEvent(event);
        parent.requestSendAccessibilityEvent(view, event);
    }

    // Qt's accessibility view, with its node provider wrapped in ScrollToFocusProvider. Qt sets
    // its own delegate again whenever accessibility is turned back on, so this is checked anew.
    private View qtAccessibilityView() {
        View view = qtAccessibilityView;
        if (view == null || !view.isAttachedToWindow() || view.getAccessibilityNodeProvider() == null) {
            view = findQtAccessibilityView(getWindow().getDecorView());
            qtAccessibilityView = view;
        }
        if (view != null) {
            AccessibilityNodeProvider provider = view.getAccessibilityNodeProvider();
            if (!(provider instanceof ScrollToFocusProvider)) {
                final ScrollToFocusProvider wrapped = new ScrollToFocusProvider(provider, view);
                view.setAccessibilityDelegate(new View.AccessibilityDelegate() {
                    @Override
                    public AccessibilityNodeProvider getAccessibilityNodeProvider(View host) {
                        return wrapped;
                    }
                });
            }
        }
        return view;
    }

    // Android scrolls its own lists to the item TalkBack moves to, Qt 5.15 does not: TalkBack
    // stops on an item below the screen, its frame is drawn off screen and a double tap misses it.
    // On such a focus Qt is asked to scroll the item into view (nativeScrollToAccessibleObject),
    // and once it did, accessibilityTreeChanged() lets TalkBack read the new positions.
    // A double tap on a node is answered by nativeAccessibleClick, not by Qt: Qt 5.15.0 answers
    // false after the press of an item that is not a button and does nothing for a button, and on
    // false TalkBack taps the middle of the node too. When it pressed the node, TalkBack gets true,
    // a clicked event (when the node is still there) and a tree refresh for what the press opened
    // or added (TalkBack 17 does not say a new switch state from the event: it says the state when
    // the switch is touched again);
    // the event is sent later, as Qt reading the node for it right away would wait for the press,
    // past the 5 s Android waits for the answer. When it did not, false is answered and TalkBack's
    // tap is the press. Other actions go to Qt.
    // A value control of Qt (a spin box, a slider: an item with scroll actions and no children)
    // is a View to TalkBack, which lets only a seek bar change its value with a swipe up or down;
    // it is called one, the swipe sends the scroll actions Qt handles, and the new name, which
    // ends with the value, is announced, as Qt 5.15 sends nothing. The rest goes to Qt unchanged.
    // A popup that closes takes TalkBack's focus with it (a double tap on its Cancel): Qt 5.15.0 neither
    // tells TalkBack the focused node is gone nor gives the focus back, so the focus falls to the whole
    // window and the place on the screen is lost. Later Qt clears the focus of a hidden node (qtbase
    // f6ddb3e921, QTBUG-93393) and moves TalkBack's focus with the keyboard focus (fd20bc2277,
    // QTBUG-79611); that would not bring it back here: a double tap presses a button with a key event
    // sent to it (main.cpp, pressWithSpace()), which gives it no keyboard focus, so the focus would go
    // wherever the keyboard focus is when the popup closes. The node TalkBack had focused when a popup
    // showed is kept, and once the popup hid, if the focused node went with it, the focus is put back
    // there (when it is still on the screen), as a dialog of Android does. A modal popup that opens
    // takes the focus to its first stop, as TalkBack does with a dialog of Android. Both only with a
    // screen reader on (onAccessibilityProvider).
    private static final class ScrollToFocusProvider extends AccessibilityNodeProvider {
        private static final long VALUE_ANNOUNCE_DELAY_MS = 150;
        // Qt never gives an object this id (-1: QAccessibleCache::acquireId skips it,
        // qaccessiblecache.cpp:89-95)
        private static final int NO_FOCUS = View.NO_ID;
        private final AccessibilityNodeProvider qt;
        private final View host;
        // the node with TalkBack's focus, as TalkBack asked for it through this provider
        private int focusedId = NO_FOCUS;
        // the focused node when each open popup showed, by the key of the popup (main.cpp,
        // androidAccessibilityUpdate)
        private final HashMap<Integer, Integer> popupOpeners = new HashMap<>();

        ScrollToFocusProvider(AccessibilityNodeProvider qt, View host) {
            this.qt = qt;
            this.host = host;
        }

        @Override
        public AccessibilityNodeInfo createAccessibilityNodeInfo(int virtualViewId) {
            AccessibilityNodeInfo node = qt.createAccessibilityNodeInfo(virtualViewId);
            if (node != null && virtualViewId != View.NO_ID && node.isScrollable() && node.getChildCount() == 0) {
                node.setClassName("android.widget.SeekBar");
            }
            return node;
        }

        @Override
        public List<AccessibilityNodeInfo> findAccessibilityNodeInfosByText(String text, int virtualViewId) {
            return qt.findAccessibilityNodeInfosByText(text, virtualViewId);
        }

        @Override
        public AccessibilityNodeInfo findFocus(int focus) {
            return qt.findFocus(focus);
        }

        @Override
        public boolean performAction(int virtualViewId, int action, Bundle arguments) {
            if (action == AccessibilityNodeInfo.ACTION_CLICK && virtualViewId != View.NO_ID) {
                boolean pressed;
                try {
                    pressed = nativeAccessibleClick(virtualViewId);
                } catch (UnsatisfiedLinkError e) {
                    // QZ's library not loaded yet: Qt answers as before
                    return qt.performAction(virtualViewId, action, arguments);
                }
                if (pressed) {
                    afterClick(virtualViewId);
                }
                return pressed;
            }
            boolean handled = qt.performAction(virtualViewId, action, arguments);
            if (action == AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS) {
                focusedId = virtualViewId;
            } else if (action == AccessibilityNodeInfo.ACTION_CLEAR_ACCESSIBILITY_FOCUS && focusedId == virtualViewId) {
                focusedId = NO_FOCUS;
            }
            if (action == AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS && virtualViewId != View.NO_ID) {
                try {
                    nativeScrollToAccessibleObject(virtualViewId);
                } catch (UnsatisfiedLinkError ignored) {
                    // Qt not loaded yet: nothing on screen to scroll
                }
            }
            if (handled && virtualViewId != View.NO_ID && (action == AccessibilityNodeInfo.ACTION_SCROLL_FORWARD
                    || action == AccessibilityNodeInfo.ACTION_SCROLL_BACKWARD)) {
                announceValue(virtualViewId);
            }
            return handled;
        }

        // Qt's node for the id, asked after a delay, or null when accessibility was turned off in the
        // meantime (TalkBack switched off right after a double tap or a swipe on a value): Qt 5.15.0
        // then removes this view from its layout and drops its reference to it, and any node asked for
        // afterwards throws a NullPointerException (QtAccessibilityDelegate.getNodeForVirtualViewId;
        // Qt 6 returns an empty node there, qtbase af73c7a99e). Only delayed calls can come after that:
        // Android asks the provider itself on the main thread, where Qt removes the view, and only while
        // it is attached.
        private AccessibilityNodeInfo delayedNode(int virtualViewId) {
            if (host.getParent() == null) {
                return null;
            }
            return qt.createAccessibilityNodeInfo(virtualViewId);
        }

        // A focus move of a popup (into it, back to its opener) waits for the clicked events of the presses
        // before it (afterClick), so TalkBack says what was pressed first, then where the focus went;
        // with none waiting it is made at once
        private int clicksWaiting = 0;
        private final ArrayList<Runnable> afterClicks = new ArrayList<>();

        private void afterTheClicks(Runnable move) {
            if (clicksWaiting > 0) {
                afterClicks.add(move);
            } else {
                move.run();
            }
        }

        private void clickSent() {
            if (clicksWaiting > 0) {
                clicksWaiting--;
            }
            if (clicksWaiting == 0) {
                ArrayList<Runnable> moves = new ArrayList<>(afterClicks);
                afterClicks.clear();
                for (Runnable move : moves) {
                    move.run();
                }
            }
        }

        private void afterClick(final int virtualViewId) {
            clicksWaiting++;
            host.postDelayed(new Runnable() {
                @Override
                public void run() {
                    ViewParent parent = host.getParent();
                    AccessibilityNodeInfo node = delayedNode(virtualViewId);
                    // a press that closed the page leaves Qt an empty node for the id
                    if (node != null && parent != null && (node.isClickable() || node.isCheckable())) {
                        AccessibilityEvent event = AccessibilityEvent.obtain(AccessibilityEvent.TYPE_VIEW_CLICKED);
                        event.setSource(host, virtualViewId);
                        event.setPackageName(host.getContext().getPackageName());
                        event.setClassName(node.getClassName());
                        event.setContentDescription(node.getContentDescription());
                        event.setChecked(node.isChecked());
                        event.setEnabled(node.isEnabled());
                        parent.requestSendAccessibilityEvent(host, event);
                    }
                    if (node != null) {
                        node.recycle();
                    }
                    accessibilityTreeChanged();
                    clickSent();
                }
            }, VALUE_ANNOUNCE_DELAY_MS);
        }

        void popupShown(int popup) {
            popupOpeners.put(popup, focusedId);
        }

        // A modal popup opened (main.cpp, popupsOpened): the focus to first, its title or its first stop
        void popupOpened(int popup, final int first) {
            afterTheClicks(new Runnable() {
                @Override
                public void run() {
                    if (onScreen(first)) {
                        performAction(first, AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS, null);
                    }
                }
            });
        }

        void popupHidden(int popup) {
            final Integer opener = popupOpeners.remove(popup);
            if (opener == null || opener == NO_FOCUS) {
                return;
            }
            afterTheClicks(new Runnable() {
                @Override
                public void run() {
                    // the focus is on a node still on the screen (outside the popup, or moved there by
                    // the user meanwhile): left where it is
                    if (focusedId != NO_FOCUS && onScreen(focusedId)) {
                        return;
                    }
                    if (onScreen(opener)) {
                        performAction(opener, AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS, null);
                    }
                }
            });
        }

        private boolean onScreen(int virtualViewId) {
            AccessibilityNodeInfo node = delayedNode(virtualViewId);
            if (node == null) {
                return false;
            }
            boolean shown = node.isVisibleToUser();
            node.recycle();
            return shown;
        }

        private void announceValue(final int virtualViewId) {
            host.postDelayed(new Runnable() {
                @Override
                public void run() {
                    AccessibilityNodeInfo node = delayedNode(virtualViewId);
                    if (node == null) {
                        return;
                    }
                    CharSequence name = node.getContentDescription();
                    if (name == null || name.length() == 0) {
                        name = node.getText();
                    }
                    node.recycle();
                    if (name != null && name.length() > 0) {
                        host.announceForAccessibility(name);
                    }
                }
            }, VALUE_ANNOUNCE_DELAY_MS);
        }
    }

    private static volatile CustomQtActivity accessibilityActivity = null;

    // Called by Qt from its own thread after it scrolled to the focused item, after a press, when a
    // page or a popup shows or hides and when a list stops scrolling
    public static void accessibilityTreeChanged() {
        final CustomQtActivity activity = accessibilityActivity;
        if (activity == null) {
            return;
        }
        activity.accessibilityHandler.post(new Runnable() {
            @Override
            public void run() {
                activity.sendAccessibilityTreeChanged();
            }
        });
    }

    // Called by Qt from its own thread when a popup shows, a modal one opened (its enter transition
    // finished) and a popup hides (ScrollToFocusProvider: TalkBack's focus moved into a modal popup and
    // given back to its opener); popup - a key of the popup, the same for its show, its opening and its
    // hide; first - the node the focus goes to
    public static void accessibilityPopupShown(final int popup) {
        onAccessibilityProvider(new ProviderCall() {
            @Override
            public void run(ScrollToFocusProvider provider) {
                provider.popupShown(popup);
            }
        });
    }

    public static void accessibilityPopupOpened(final int popup, final int first) {
        onAccessibilityProvider(new ProviderCall() {
            @Override
            public void run(ScrollToFocusProvider provider) {
                provider.popupOpened(popup, first);
            }
        });
    }

    public static void accessibilityPopupHidden(final int popup) {
        onAccessibilityProvider(new ProviderCall() {
            @Override
            public void run(ScrollToFocusProvider provider) {
                provider.popupHidden(popup);
            }
        });
    }

    private interface ProviderCall {
        void run(ScrollToFocusProvider provider);
    }

    // The call on the Android UI thread, with the provider that wraps Qt's (none while accessibility is off),
    // and only with a screen reader on: with another service the focus moved into a popup still scrolled
    // the item into view (nativeScrollToAccessibleObject), and a long list opened at its current item
    // jumped back to its top a moment later
    private static void onAccessibilityProvider(final ProviderCall call) {
        final CustomQtActivity activity = accessibilityActivity;
        if (activity == null) {
            return;
        }
        activity.accessibilityHandler.post(new Runnable() {
            @Override
            public void run() {
                if (!activity.screenReaderOn()) {
                    return;
                }
                View view = activity.qtAccessibilityView();
                AccessibilityNodeProvider provider = view != null ? view.getAccessibilityNodeProvider() : null;
                if (provider instanceof ScrollToFocusProvider) {
                    call.run((ScrollToFocusProvider) provider);
                }
            }
        });
    }

    // The plain View that Qt adds over its surface while accessibility is on: the one whose node
    // provider is Qt's. Only plain Views are asked, so a WebView does not start its own provider.
    private static View findQtAccessibilityView(View view) {
        if (view.getClass() == View.class) {
            AccessibilityNodeProvider provider = view.getAccessibilityNodeProvider();
            if (provider instanceof ScrollToFocusProvider
                    || (provider != null && provider.getClass().getName().startsWith("org.qtproject."))) {
                return view;
            }
        }
        if (view instanceof ViewGroup) {
            ViewGroup group = (ViewGroup) view;
            for (int i = 0; i < group.getChildCount(); i++) {
                View found = findQtAccessibilityView(group.getChildAt(i));
                if (found != null) return found;
            }
        }
        return null;
    }

    // Traverse the view hierarchy to find Qt's rendering surface (SurfaceView or TextureView)
    // and return its Y position in pixels within the window.
    // Qt 5.15 on some emulator configurations internally shifts its viewport by the top inset
    // in landscape, even after setDecorFitsSystemWindows(false). Detecting this offset lets us
    // avoid double-counting the top inset in QML.
    private int findQtSurfaceYOffset(View view) {
        if (view instanceof SurfaceView || view instanceof TextureView) {
            int[] loc = new int[2];
            view.getLocationInWindow(loc);
            return loc[1];
        }
        if (view instanceof ViewGroup) {
            ViewGroup group = (ViewGroup) view;
            for (int i = 0; i < group.getChildCount(); i++) {
                int result = findQtSurfaceYOffset(group.getChildAt(i));
                if (result > 0) return result;
            }
        }
        return 0;
    }

    // WORKAROUND: back from the background a page with a web view (the workout editor) can stay
    // empty until the page is touched (OnePlus 12, Android 16): the
    // WebView is in place and visible, but Chromium makes no new frame. On return Qt 5.15 takes the
    // native WebView of Qt WebView off its layout and adds it back
    // (QAndroidPlatformForeignWindow::applicationStateChanged -> destroySurface / insertNativeView),
    // and resumes it with WebView.onResume() (QAndroidWebViewPrivate::onApplicationStateChanged).
    // Both go to the UI thread from Qt's thread; most likely, when onResume() comes before the
    // WebView is attached again, the page stays empty. Qt 6.8 still does both the same way.
    // So each WebView Qt adds is paused with the activity and resumed after the activity resumes;
    // a WebView attached to the window again is paused and resumed once more, because onResume()
    // does nothing on a WebView that Qt has already resumed. No reload: the page keeps its state
    // and scroll position. Redrawing the view from outside (invalidate, clip, layout) did not
    // reliably make Chromium draw; onPause() + onResume() did.
    private final Set<WebView> watchedWebViews = Collections.newSetFromMap(new WeakHashMap<WebView, Boolean>());
    private boolean activityResumed = false;

    private void watchWebViews(View view) {
        if (view instanceof WebView) {
            final WebView webView = (WebView) view;
            if (watchedWebViews.add(webView)) {
                webView.addOnAttachStateChangeListener(new View.OnAttachStateChangeListener() {
                    @Override
                    public void onViewAttachedToWindow(View v) {
                        if (activityResumed) {
                            Log.d(TAG, "WebView attached again: onPause + onResume");
                            webView.onPause();
                            webView.onResume();
                        }
                    }

                    @Override
                    public void onViewDetachedFromWindow(View v) {
                    }
                });
            }
            return;
        }
        if (view instanceof ViewGroup) {
            ViewGroup group = (ViewGroup) view;
            for (int i = 0; i < group.getChildCount(); i++) {
                watchWebViews(group.getChildAt(i));
            }
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        activityResumed = false;
        for (WebView webView : watchedWebViews.toArray(new WebView[0])) {
            webView.onPause();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        activityResumed = true;
        for (final WebView webView : watchedWebViews.toArray(new WebView[0])) {
            // posted: from API 24 a view Qt has taken off the window runs it when it is attached
            // again, before that on the next traversal; the attach listener covers both
            webView.post(new Runnable() {
                @Override
                public void run() {
                    if (activityResumed) {
                        webView.onResume();
                    }
                }
            });
        }
    }

    // Called by Qt from its own thread at start: whether Android has accessibility on (main.cpp
    // turns Qt's accessibility on again when Qt lost that report)
    public static boolean accessibilityOn(Context context) {
        AccessibilityManager manager = (AccessibilityManager) context.getSystemService(ACCESSIBILITY_SERVICE);
        return manager != null && manager.isEnabled();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate: CustomQtActivity initialized");
        dispatchOAuthCallback(getIntent());
        AgeSignalsHelper.requestAgeSignals(this);
        HealthConnectHelper.initialize(this);
        registerBackCallback();

        // Wrap Qt's accessibility view as soon as Qt adds it, before TalkBack moves over it, and
        // watch each WebView Qt adds (paused and resumed with the activity)
        accessibilityActivity = this;
        getWindow().getDecorView().getViewTreeObserver().addOnGlobalLayoutListener(
                new ViewTreeObserver.OnGlobalLayoutListener() {
                    @Override
                    public void onGlobalLayout() {
                        if (accessibilityEnabled()) {
                            qtAccessibilityView();
                        }
                        watchWebViews(getWindow().getDecorView());
                    }
                });

        // Qt 5.15 learns that the user closed the keyboard (its own hide key, or the back key, which
        // the keyboard takes) only from a guess: 100 ms after finishComposingText, which Qt takes as
        // the connection closing, the screen height minus the visible frame must be under 100 px
        // (HideKeyboardRunnable, QtInputConnection.java). The guess misses on Android 15 and 16 and
        // on some older devices; then Qt keeps the keyboard "visible" and spends the next Back on
        // hiding it (QtActivityDelegate onKeyDown/onKeyUp): the user has to press Back twice.
        // Qt 6 follows the keyboard by WindowInsets.isVisible(ime()) on API 30+ (qtbase 33cf82c13d,
        // QTBUG-98984), from a global layout listener (f3a51f8b38, QTBUG-141908); the same here, in
        // both directions. Qt 5.15's own updates stay (they cannot be switched off as in Qt 6,
        // 21616e1447) and the time stamps keep the latest; a late one of them saying "visible" is
        // undone on the next Back (keyboardHiddenByInsets() in dispatchKeyEvent).
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            final View imeRoot = getWindow().getDecorView();
            imeRoot.getViewTreeObserver().addOnGlobalLayoutListener(
                    new ViewTreeObserver.OnGlobalLayoutListener() {
                        private boolean imeWasVisible = false;

                        @Override
                        public void onGlobalLayout() {
                            WindowInsets insets = imeRoot.getRootWindowInsets();
                            if (insets == null) {
                                return;
                            }
                            boolean imeVisible = insets.isVisible(WindowInsets.Type.ime());
                            if (imeVisible == imeWasVisible) {
                                return;
                            }
                            imeWasVisible = imeVisible;
                            QtActivityDelegate delegate = QtNative.activityDelegate();
                            if (delegate != null) {
                                delegate.setKeyboardVisibility(imeVisible, System.nanoTime());
                            }
                        }
                    });
        }

        // Make the window truly edge-to-edge so the app renders into ALL screen areas
        // including the display cutout (punch-hole/notch) on all 4 rotation variants.
        // setDecorFitsSystemWindows(false) prevents the system from shrinking
        // the content area around insets — we handle insets ourselves via onApplyWindowInsets.
        // LAYOUT_IN_DISPLAY_CUTOUT_MODE_ALWAYS extends into the cutout on every edge.
        // Dark window background ensures no white strip is visible in areas Qt may not render.
        // API 31+ only, like the QML paddings (main.qml getTopPadding() and the others):
        // below 31 they are 0, so on Android 11 (API 30) the system bars covered the app (#5484).
        getWindow().setBackgroundDrawable(new ColorDrawable(Color.BLACK));
        if (Build.VERSION.SDK_INT >= 31) {
            getWindow().setDecorFitsSystemWindows(false);
            getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_ALWAYS;
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        // This is the core of the new solution. We set a listener on the main view.
        // The OS will call this listener whenever the insets change (e.g., on rotation).
        final View decorView = getWindow().getDecorView();
        decorView.setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener() {
            @Override
            public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
                final float density = getResources().getDisplayMetrics().density;
                int top = 0;
                int bottom = 0;
                int left = 0;
                int right = 0;
                int waterfallTop = 0;
                int waterfallBottom = 0;
                int waterfallLeft = 0;
                int waterfallRight = 0;
                int systemBarLeft = 0;
                int systemBarRight = 0;

                if (density > 0) {
                    // Use system window insets as primary source
                    top = Math.round(insets.getSystemWindowInsetTop() / density);
                    bottom = Math.round(insets.getSystemWindowInsetBottom() / density);
                    left = Math.round(insets.getSystemWindowInsetLeft() / density);
                    right = Math.round(insets.getSystemWindowInsetRight() / density);
                    // On API 30+ getSystemWindowInsets() also includes the display cutout,
                    // so ask for the system bars alone.
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                        Insets bars = insets.getInsets(WindowInsets.Type.systemBars());
                        systemBarLeft = Math.round(bars.left / density);
                        systemBarRight = Math.round(bars.right / density);
                    } else {
                        systemBarLeft = left;
                        systemBarRight = right;
                    }

                    // For API 28+, also check display cutout for additional padding
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                        DisplayCutout cutout = insets.getDisplayCutout();
                        if (cutout != null) {
                            // Use the maximum between system window inset and cutout safe inset
                            left = Math.max(left, Math.round(cutout.getSafeInsetLeft() / density));
                            right = Math.max(right, Math.round(cutout.getSafeInsetRight() / density));
                            top = Math.max(top, Math.round(cutout.getSafeInsetTop() / density));
                            bottom = Math.max(bottom, Math.round(cutout.getSafeInsetBottom() / density));

                            // Android 11+ exposes curved waterfall display areas separately from cutouts.
                            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                                Insets waterfallInsets = cutout.getWaterfallInsets();
                                waterfallLeft = Math.round(waterfallInsets.left / density);
                                waterfallRight = Math.round(waterfallInsets.right / density);
                                waterfallTop = Math.round(waterfallInsets.top / density);
                                waterfallBottom = Math.round(waterfallInsets.bottom / density);
                            }
                        }
                    }
                }

                Log.d(TAG, "onApplyWindowInsets - Top:" + top + " Bottom:" + bottom + " Left:" + left + " Right:" + right);
                Log.d(TAG, "Raw insets - SystemTop:" + insets.getSystemWindowInsetTop() +
                          " SystemBottom:" + insets.getSystemWindowInsetBottom() +
                          " SystemLeft:" + insets.getSystemWindowInsetLeft() +
                          " SystemRight:" + insets.getSystemWindowInsetRight());
                Log.d(TAG, "Stable insets - StableTop:" + insets.getStableInsetTop() +
                          " StableBottom:" + insets.getStableInsetBottom() +
                          " StableLeft:" + insets.getStableInsetLeft() +
                          " StableRight:" + insets.getStableInsetRight());
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                    DisplayCutout cutout = insets.getDisplayCutout();
                    if (cutout != null) {
                        Log.d(TAG, "Cutout insets - Top:" + cutout.getSafeInsetTop() +
                              " Bottom:" + cutout.getSafeInsetBottom() +
                              " Left:" + cutout.getSafeInsetLeft() +
                              " Right:" + cutout.getSafeInsetRight());
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                            Insets waterfallInsets = cutout.getWaterfallInsets();
                            Log.d(TAG, "Waterfall insets - Top:" + waterfallInsets.top +
                                  " Bottom:" + waterfallInsets.bottom +
                                  " Left:" + waterfallInsets.left +
                                  " Right:" + waterfallInsets.right);
                        }
                    }
                }

                // Push the new, correct inset values to the C++ layer immediately for
                // responsiveness. Guard against the race where Qt's native library hasn't
                // finished loading yet when Android fires onApplyWindowInsets early.
                try {
                    onInsetsChanged(top, bottom, left, right, waterfallTop, waterfallBottom, waterfallLeft, waterfallRight);
                    onSystemBarSideInsetsChanged(systemBarLeft, systemBarRight);
                } catch (UnsatisfiedLinkError ignored) {
                    // Qt not ready yet; insets will be re-applied once Qt initializes.
                }

                // Deferred correction: Qt 5.15 on some emulator configurations internally
                // shifts its rendering viewport by the top inset in landscape mode even
                // after setDecorFitsSystemWindows(false). On real devices this does not
                // happen. We detect the offset by measuring the Qt surface view position
                // after the layout pass completes, then send corrected values so QML
                // topPadding is not double-counted.
                final int fTop = top, fBottom = bottom, fLeft = left, fRight = right;
                final int fWT = waterfallTop, fWB = waterfallBottom, fWL = waterfallLeft, fWR = waterfallRight;
                final float fDensity = density;
                v.post(() -> {
                    int surfaceYPx = findQtSurfaceYOffset(v);
                    Log.d(TAG, "deferred surfaceYPx=" + surfaceYPx + " fTop=" + fTop);
                    if (surfaceYPx > 0 && fDensity > 0) {
                        int adjustedTop = Math.max(0, fTop - Math.round(surfaceYPx / fDensity));
                        Log.d(TAG, "adjustedTop=" + adjustedTop);
                        if (adjustedTop != fTop) {
                            try {
                                onInsetsChanged(adjustedTop, fBottom, fLeft, fRight, fWT, fWB, fWL, fWR);
                            } catch (UnsatisfiedLinkError ignored) {}
                        }
                    }
                });

                return v.onApplyWindowInsets(insets);
            }
        });
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);
        dispatchOAuthCallback(intent);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (isDocumentPickerRequest(requestCode)) {
            handleDocumentPickerResult(requestCode, resultCode, data);
            return;
        }

        String uriString = "";
        if (data != null && data.getData() != null) {
            uriString = data.getData().toString();
        }
        Log.d(TAG, "onActivityResult passthrough requestCode=" + requestCode + " resultCode=" + resultCode + " uri=" + uriString);
        super.onActivityResult(requestCode, resultCode, data);
    }

    public void openDocumentPicker(String mimeType, int requestCode, String destinationDir) {
        pendingImportDirectories.put(requestCode, destinationDir == null ? "" : destinationDir);
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType((mimeType == null || mimeType.isEmpty()) ? "*/*" : mimeType);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        startActivityForResult(Intent.createChooser(intent, "Select file"), requestCode);
    }

    // This method is still needed for the QML check
    public static int getApiLevel() {
        return Build.VERSION.SDK_INT;
    }

    // Modern look: the status and navigation bars without a background of their own, as Qt 6 does for
    // a window under them (QtDisplayManager.setSystemUiVisibility in 6.8: transparent bar colours).
    // Qt 5.15 puts the activity in Theme_Holo_Light on API 21+ (QtActivity, QtActivityLoader.setTheme),
    // whose window does not draw the system bar backgrounds, so the system paints a black bar under the
    // status bar over the app - on Android 15+ as well, where the bars are otherwise transparent. From
    // API 31 the window reaches under the bars (onCreate): they become transparent and the page shows
    // through (the drawer sheet too, with its rounded top corner). API 23-30 keep the window below the
    // bars, so they take the page colour (the navigation bar from API 26, the first with dark buttons).
    // The icons follow the page: dark on a light one. API 21-22 have no dark icons: nothing changes.
    // Classic look (modern false): the flags, colours and icons from before the first call come back.
    // Called from QML (applySystemBars) at start-up, on every return to the foreground and when the
    // look or its theme changes, so a reset by the system is put right on the next return.
    private static boolean systemBarsSaved = false;
    private static boolean savedDrawsBars;
    private static int savedStatusBarColor;
    private static int savedNavigationBarColor;
    private static boolean savedLightStatus;
    private static boolean savedLightNavigation;

    public static void setSystemBars(final int color, final boolean modern, final boolean dark) {
        final CustomQtActivity activity = accessibilityActivity;
        if (activity == null || Build.VERSION.SDK_INT < 23)
            return;
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Window window = activity.getWindow();
                final int drawsBars = WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS;
                // Android 15+ (the app targets 35+, edge to edge enforced): the system keeps the bars
                // transparent and ignores their colours, the flag alone takes the black away; the colour
                // calls are deprecated there and not made, as in Qt 6 (QtWindowInsetsController, 5472ad6adf)
                final boolean colors = Build.VERSION.SDK_INT < 35;
                if (!systemBarsSaved) {
                    if (!modern)
                        return;
                    savedDrawsBars = (window.getAttributes().flags & drawsBars) != 0;
                    if (colors) {
                        savedStatusBarColor = window.getStatusBarColor();
                        savedNavigationBarColor = window.getNavigationBarColor();
                    }
                    savedLightStatus = lightBar(window, true);
                    savedLightNavigation = lightBar(window, false);
                    systemBarsSaved = true;
                }
                if (modern) {
                    int bars = Build.VERSION.SDK_INT >= 31 ? Color.TRANSPARENT : (color | 0xFF000000);
                    window.addFlags(drawsBars);
                    if (colors)
                        window.setStatusBarColor(bars);
                    setLightBar(window, true, !dark);
                    if (Build.VERSION.SDK_INT >= 26) {
                        if (colors)
                            window.setNavigationBarColor(bars);
                        setLightBar(window, false, !dark);
                    }
                } else {
                    if (!savedDrawsBars)
                        window.clearFlags(drawsBars);
                    if (colors) {
                        window.setStatusBarColor(savedStatusBarColor);
                        window.setNavigationBarColor(savedNavigationBarColor);
                    }
                    setLightBar(window, true, savedLightStatus);
                    if (Build.VERSION.SDK_INT >= 26)
                        setLightBar(window, false, savedLightNavigation);
                }
            }
        });
    }

    // Dark icons on the status bar (status true) or dark buttons on the navigation bar
    private static boolean lightBar(Window window, boolean status) {
        if (Build.VERSION.SDK_INT >= 30) {
            WindowInsetsController controller = window.getInsetsController();
            int flag = status ? WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS
                              : WindowInsetsController.APPEARANCE_LIGHT_NAVIGATION_BARS;
            return controller != null && (controller.getSystemBarsAppearance() & flag) != 0;
        }
        int flag = status ? View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR : View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR;
        return (window.getDecorView().getSystemUiVisibility() & flag) != 0;
    }

    // API 30-34 take both ways: on API 30 the appearance of the controller alone left the icons light
    // on a light page (emulator, T-544), and androidx WindowInsetsControllerCompat sets the old flag
    // there as well (b/180881870); API 35+ the controller alone, the old flags are deprecated
    private static void setLightBar(Window window, boolean status, boolean light) {
        if (Build.VERSION.SDK_INT < 35) {
            View decor = window.getDecorView();
            int flag = status ? View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR : View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR;
            int visibility = decor.getSystemUiVisibility();
            decor.setSystemUiVisibility(light ? (visibility | flag) : (visibility & ~flag));
        }
        if (Build.VERSION.SDK_INT >= 30) {
            WindowInsetsController controller = window.getInsetsController();
            if (controller == null)
                return;
            int flag = status ? WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS
                              : WindowInsetsController.APPEARANCE_LIGHT_NAVIGATION_BARS;
            controller.setSystemBarsAppearance(light ? flag : 0, flag);
        }
    }

    private boolean isDocumentPickerRequest(int requestCode) {
        return requestCode == DOCUMENT_PICKER_PROFILE_REQUEST_CODE
            || requestCode == DOCUMENT_PICKER_TRAINING_REQUEST_CODE
            || requestCode == DOCUMENT_PICKER_GPX_REQUEST_CODE
            || requestCode == DOCUMENT_PICKER_SETTINGS_REQUEST_CODE
            || requestCode == DOCUMENT_PICKER_FIT_REQUEST_CODE
            || requestCode == DOCUMENT_PICKER_FIT_FOLDER_REQUEST_CODE
            || requestCode == DOCUMENT_PICKER_RESTORE_FOLDER_REQUEST_CODE;
    }

    // Folder of the QZ files on Android 14+ (Documents/QZ) as a document of the external storage
    // provider, so the folder picker opens there.
    private static final String QZ_DOCUMENTS_TREE_ID = "primary:Documents/QZ";

    // Opens the system folder picker to import every .fit file of a folder. After the app is
    // reinstalled, the workouts left in Documents/QZ by the previous install are hidden from the
    // file API (scoped storage); a folder granted through the picker can still be read. The grant
    // is kept, so later imports of the same folder skip the picker.
    // removeOldDatabase: also delete ddb.sqlite of the previous install from the folder (the app
    // can remove it only through the grant), with the spare ddb-N.sqlite left by earlier installs;
    // keepDatabase names the spare in use (empty when the database is elsewhere). allowPicker
    // false (automatic import at start): use a saved grant only, never open the picker; returns
    // false when there is no saved grant.
    public boolean openFitFolderPicker(int requestCode, String destinationDir, boolean removeOldDatabase,
                                       String keepDatabase, boolean allowPicker) {
        pendingImportDirectories.put(requestCode, destinationDir == null ? "" : destinationDir);
        fitFolderRemoveOldDatabase = removeOldDatabase;
        fitFolderKeepDatabase = keepDatabase == null ? "" : keepDatabase;
        fitFolderAllowPicker = allowPicker;
        for (android.content.UriPermission permission : getContentResolver().getPersistedUriPermissions()) {
            Uri treeUri = permission.getUri();
            String treeId = "";
            try {
                treeId = android.provider.DocumentsContract.getTreeDocumentId(treeUri);
            } catch (Exception ignored) {
                // a granted single document, not a folder
            }
            // removing the old database needs a grant with write access
            if (permission.isReadPermission() && QZ_DOCUMENTS_TREE_ID.equals(treeId)
                && (!removeOldDatabase || permission.isWritePermission())) {
                Intent data = new Intent();
                data.setData(treeUri);
                handleFitFolderResult(requestCode, RESULT_OK, data, true);
                return true;
            }
        }
        if (!allowPicker) {
            pendingImportDirectories.remove(requestCode);
            return false;
        }
        launchFitFolderPicker(requestCode);
        return true;
    }

    private boolean fitFolderRemoveOldDatabase = false;
    private String fitFolderKeepDatabase = "";
    private boolean fitFolderAllowPicker = true;

    private void launchFitFolderPicker(int requestCode) {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
                        | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            intent.putExtra(android.provider.DocumentsContract.EXTRA_INITIAL_URI,
                android.provider.DocumentsContract.buildDocumentUri("com.android.externalstorage.documents",
                                                                    QZ_DOCUMENTS_TREE_ID));
        }
        startActivityForResult(intent, requestCode);
    }

    // Copies the .fit files of the picked folder and of its "fit" subfolder into destinationDir
    // and passes their local paths, one per line, to the native side. An empty string still goes
    // through, so the app can say that no workout was found. The copy runs off the UI thread:
    // a folder can hold hundreds of workouts.
    // A saved grant that finds nothing may be stale (folder removed or renamed): it is dropped
    // and the picker opens, so the user can always choose the folder again.
    private void handleFitFolderResult(final int requestCode, final int resultCode, Intent data,
                                       final boolean fromSavedGrant) {
        final String destinationDir = pendingImportDirectories.get(requestCode, "");
        pendingImportDirectories.remove(requestCode);
        if (resultCode != RESULT_OK || data == null || data.getData() == null) {
            nativeOnDocumentPicked(requestCode, resultCode, "");
            return;
        }
        final Uri treeUri = data.getData();
        final boolean removeOldDatabase = fitFolderRemoveOldDatabase;
        final String keepDatabase = fitFolderKeepDatabase;
        final boolean allowPicker = fitFolderAllowPicker;
        try {
            getContentResolver().takePersistableUriPermission(treeUri,
                Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        } catch (Exception e) {
            try {
                getContentResolver().takePersistableUriPermission(treeUri, Intent.FLAG_GRANT_READ_URI_PERMISSION);
            } catch (Exception e2) {
                Log.d(TAG, "takePersistableUriPermission failed " + e2);
            }
        }
        nativeOnFitImportStarted();
        new Thread(new Runnable() {
            @Override
            public void run() {
                StringBuilder localPaths = new StringBuilder();
                // the file under way and the files to copy, counted before the first copy
                int[] progress = new int[] {0, 0};
                try {
                    String rootId = android.provider.DocumentsContract.getTreeDocumentId(treeUri);
                    progress[1] = countFitFilesInTree(treeUri, rootId, true);
                    if (progress[1] > 0) {
                        nativeOnFitImportProgress(0, progress[1]);
                    }
                    importFitFilesFromTree(treeUri, rootId, destinationDir, localPaths, true, progress);
                    // only in the QZ folder itself, never in another folder the user picked
                    if (removeOldDatabase && localPaths.length() > 0 && QZ_DOCUMENTS_TREE_ID.equals(rootId)) {
                        deleteOldHistoryDatabase(treeUri, rootId, keepDatabase);
                    }
                } catch (Exception e) {
                    Log.d(TAG, "handleFitFolderResult failed " + e);
                }
                // the copy is over; queued before the result, so the history page goes on with adding
                if (progress[1] > 0) {
                    nativeOnFitImportProgress(0, 0);
                }
                Log.d(TAG, "handleFitFolderResult requestCode=" + requestCode + " savedGrant=" + fromSavedGrant
                    + " files=" + (localPaths.length() == 0 ? 0 : localPaths.toString().split("\n").length));
                if (fromSavedGrant && localPaths.length() == 0) {
                    releaseFolderGrant(treeUri);
                    if (allowPicker) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                pendingImportDirectories.put(requestCode, destinationDir);
                                launchFitFolderPicker(requestCode);
                            }
                        });
                        return;
                    }
                }
                nativeOnDocumentPicked(requestCode, RESULT_OK, localPaths.toString());
            }
        }, "FitFolderImport").start();
    }

    // Subfolders of the QZ folder whose files the app lists (training programs, GPX routes,
    // settings files, profiles) and so restores after a reinstall.
    private static final String[] RESTORE_FOLDERS = {"training", "gpx", "settings", "profiles"};

    // Files left in these subfolders of Documents/QZ by a previous install: this install does
    // not see them in the folder and cannot replace them, so the lists are empty and the
    // bundled programs and routes are not copied again. Through a grant on the QZ folder each
    // such file is copied as this install's own one under the same name, and the old file is
    // removed. destinationDir: the QZ folder. allowPicker false: use a saved grant only, never
    // open the picker; returns false when there is no saved grant with write access. allowPicker
    // true (the user agreed): always open the picker, a saved grant may be the one that restored
    // nothing.
    public boolean openRestoreFolderPicker(int requestCode, String destinationDir, boolean allowPicker) {
        // one pass at a time: two passes over the same files share the temporary copies
        if (folderRestoreRunning) {
            Log.d(TAG, "openRestoreFolderPicker: a restore is running");
            return false;
        }
        pendingImportDirectories.put(requestCode, destinationDir == null ? "" : destinationDir);
        if (allowPicker) {
            launchFitFolderPicker(requestCode);
            return true;
        }
        Uri treeUri = savedQzFolderGrant();
        if (treeUri != null) {
            Intent data = new Intent();
            data.setData(treeUri);
            handleRestoreFolderResult(requestCode, RESULT_OK, data);
            return true;
        }
        pendingImportDirectories.remove(requestCode);
        return false;
    }

    // A saved grant on the whole QZ folder with read and write access, the one a restore pass
    // needs; null when there is none (never asked, or a subfolder or another folder was picked).
    private Uri savedQzFolderGrant() {
        for (android.content.UriPermission permission : getContentResolver().getPersistedUriPermissions()) {
            String treeId = "";
            try {
                treeId = android.provider.DocumentsContract.getTreeDocumentId(permission.getUri());
            } catch (Exception ignored) {
                // a granted single document, not a folder
            }
            if (permission.isReadPermission() && permission.isWritePermission()
                && QZ_DOCUMENTS_TREE_ID.equals(treeId)) {
                return permission.getUri();
            }
        }
        return null;
    }

    // true: the lists restore files of a previous install by themselves on the first visit of
    // each run, so they do not need a button for it.
    public boolean hasQzFolderGrant() {
        return savedQzFolderGrant() != null;
    }

    // Passes the restored files, one path per line, to the native side; an empty string when
    // nothing was restored (another folder picked, or no file of a previous install). The QZ
    // folder restores all RESTORE_FOLDERS; one of them picked alone restores only itself.
    private void handleRestoreFolderResult(final int requestCode, final int resultCode, Intent data) {
        final String destinationDir = pendingImportDirectories.get(requestCode, "");
        pendingImportDirectories.remove(requestCode);
        if (resultCode != RESULT_OK || data == null || data.getData() == null) {
            nativeOnDocumentPicked(requestCode, resultCode, "");
            return;
        }
        final Uri treeUri = data.getData();
        try {
            getContentResolver().takePersistableUriPermission(treeUri,
                Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        } catch (Exception e) {
            Log.d(TAG, "takePersistableUriPermission failed " + e);
        }
        folderRestoreRunning = true;
        new Thread(new Runnable() {
            @Override
            public void run() {
                StringBuilder restored = new StringBuilder();
                // files copied so far and files to copy: the pages show how far the restore is
                int[] progress = {0, 0};
                try {
                    String rootId = android.provider.DocumentsContract.getTreeDocumentId(treeUri);
                    String[] folderIds = new String[RESTORE_FOLDERS.length];
                    for (int i = 0; i < RESTORE_FOLDERS.length; i++) {
                        String folder = RESTORE_FOLDERS[i];
                        if (QZ_DOCUMENTS_TREE_ID.equals(rootId)) {
                            folderIds[i] = findChildDocument(treeUri, rootId, folder);
                        } else if ((QZ_DOCUMENTS_TREE_ID + "/" + folder).equals(rootId)) {
                            folderIds[i] = rootId;
                        }
                        Log.d(TAG, "handleRestoreFolderResult root=" + rootId + " " + folder + "=" + folderIds[i]);
                    }
                    // counted before the first copy, so the count does not grow while it runs
                    for (int i = 0; i < RESTORE_FOLDERS.length; i++) {
                        if (folderIds[i] != null && !destinationDir.isEmpty()) {
                            progress[1] += countRestoreFiles(treeUri, folderIds[i],
                                                             new java.io.File(destinationDir, RESTORE_FOLDERS[i]), restored);
                        }
                    }
                    Log.d(TAG, "handleRestoreFolderResult files to restore " + progress[1]);
                    if (progress[1] > 0) {
                        nativeOnFolderRestoreProgress(0, progress[1]);
                    }
                    for (int i = 0; i < RESTORE_FOLDERS.length; i++) {
                        if (folderIds[i] != null && !destinationDir.isEmpty()) {
                            restoreFolderFiles(treeUri, folderIds[i], new java.io.File(destinationDir, RESTORE_FOLDERS[i]),
                                               restored, progress);
                        }
                    }
                } catch (Exception e) {
                    Log.d(TAG, "handleRestoreFolderResult failed " + e);
                }
                folderRestoreRunning = false;
                nativeOnDocumentPicked(requestCode, RESULT_OK, restored.toString());
            }
        }, "QzFolderRestore").start();
    }

    private String findChildDocument(Uri treeUri, String parentId, String childName) {
        // The external storage provider names a document by its path under the parent, so the
        // child is looked up first as one document: listing the parent costs seconds when the QZ
        // folder holds a thousand charts and logs. The listing stays for other providers.
        String directId = findDocumentByPath(treeUri, parentId + "/" + childName, childName);
        if (directId != null) {
            return directId;
        }
        Uri childrenUri = android.provider.DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, parentId);
        android.database.Cursor cursor = null;
        try {
            cursor = getContentResolver().query(childrenUri, new String[] {
                android.provider.DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                android.provider.DocumentsContract.Document.COLUMN_DISPLAY_NAME}, null, null, null);
            while (cursor != null && cursor.moveToNext()) {
                if (childName.equals(cursor.getString(1))) {
                    return cursor.getString(0);
                }
            }
        } catch (Exception e) {
            Log.d(TAG, "findChildDocument failed " + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return null;
    }

    // documentId when the tree has a document with this id and this name, otherwise null
    private String findDocumentByPath(Uri treeUri, String documentId, String name) {
        android.database.Cursor cursor = null;
        try {
            Uri documentUri = android.provider.DocumentsContract.buildDocumentUriUsingTree(treeUri, documentId);
            cursor = getContentResolver().query(documentUri, new String[] {
                android.provider.DocumentsContract.Document.COLUMN_DISPLAY_NAME}, null, null, null);
            if (cursor != null && cursor.moveToFirst() && name.equals(cursor.getString(0))) {
                return documentId;
            }
        } catch (Exception e) {
            // no such document, or a provider with other ids: the caller lists the parent
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return null;
    }

    // Walks the folder and its subfolders; a file the app can already read is its own and stays.
    // The copy is written first under a hidden temporary name (.restore_<name>) and compared
    // with the old file byte for byte; only then is the old file removed and the copy takes the
    // real name. A failure before the removal leaves the old file in place; a copy left under
    // the temporary name after it is finished by recoverRestoreCopies() on the next pass or
    // the next visit of the list.
    // progress: the file under way and the files to copy (countRestoreFiles()); each file of a
    // previous install, copied or not, moves it on by one and is passed to the native side.
    private void restoreFolderFiles(Uri treeUri, String parentId, java.io.File localDir, StringBuilder restored,
                                    int[] progress) {
        recoverRestoreCopies(localDir, false, restored);
        Uri childrenUri = android.provider.DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, parentId);
        android.database.Cursor cursor = null;
        try {
            cursor = getContentResolver().query(childrenUri, new String[] {
                android.provider.DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                android.provider.DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                android.provider.DocumentsContract.Document.COLUMN_MIME_TYPE}, null, null, null);
            while (cursor != null && cursor.moveToNext()) {
                String documentId = cursor.getString(0);
                String name = cursor.getString(1);
                // a .restore_<name> of this install is readable and skipped below; one of a previous
                // install is restored like any file and finished by recoverRestoreCopies()
                if (documentId == null || name == null || name.isEmpty()) {
                    continue;
                }
                java.io.File local = new java.io.File(localDir, name);
                if (android.provider.DocumentsContract.Document.MIME_TYPE_DIR.equals(cursor.getString(2))) {
                    restoreFolderFiles(treeUri, documentId, local, restored, progress);
                    continue;
                }
                if (local.exists() && ContentHelper.isReadable(local)) {
                    continue;
                }
                // never past the count: a file may have come into the folder since it was counted
                progress[0] = Math.min(progress[0] + 1, progress[1]);
                nativeOnFolderRestoreProgress(progress[0], progress[1]);
                if (!localDir.exists() && !localDir.mkdirs()) {
                    Log.d(TAG, "restoreFolderFiles could not create " + localDir);
                    return;
                }
                java.io.File temporary = new java.io.File(localDir, RESTORE_PREFIX + name);
                Uri documentUri = android.provider.DocumentsContract.buildDocumentUriUsingTree(treeUri, documentId);
                if (!ContentHelper.copyContentToFile(this, documentUri, temporary.getAbsolutePath())) {
                    temporary.delete();
                    continue;
                }
                // the old file goes only once its copy is known to be whole
                if (!ContentHelper.sameContent(this, documentUri, temporary)) {
                    Log.d(TAG, "restoreFolderFiles copy of " + name + " differs from the original, kept the original");
                    temporary.delete();
                    continue;
                }
                if (local.exists() && !deleteTreeDocument(treeUri, documentId)) {
                    temporary.delete();
                    continue;
                }
                if (!moveRestoreCopy(temporary, local)) {
                    // the old file is already gone: the copy stays under the temporary name, and
                    // recoverRestoreCopies() gives it the real name later
                    Log.d(TAG, "restoreFolderFiles kept " + temporary + ", could not rename it");
                    continue;
                }
                if (restored.length() > 0) {
                    restored.append('\n');
                }
                restored.append(local.getAbsolutePath());
            }
        } catch (Exception e) {
            Log.d(TAG, "restoreFolderFiles failed " + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    // The files restoreFolderFiles() would copy from the folder and its subfolders: the ones the
    // app cannot read under the same name. Only the folder listing is read, no file content.
    // Copies left under the temporary name are finished first (as restoreFolderFiles() does), so
    // a file they bring back is not counted.
    private int countRestoreFiles(Uri treeUri, String parentId, java.io.File localDir, StringBuilder restored) {
        recoverRestoreCopies(localDir, false, restored);
        int count = 0;
        Uri childrenUri = android.provider.DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, parentId);
        android.database.Cursor cursor = null;
        try {
            cursor = getContentResolver().query(childrenUri, new String[] {
                android.provider.DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                android.provider.DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                android.provider.DocumentsContract.Document.COLUMN_MIME_TYPE}, null, null, null);
            while (cursor != null && cursor.moveToNext()) {
                String documentId = cursor.getString(0);
                String name = cursor.getString(1);
                if (documentId == null || name == null || name.isEmpty()) {
                    continue;
                }
                java.io.File local = new java.io.File(localDir, name);
                if (android.provider.DocumentsContract.Document.MIME_TYPE_DIR.equals(cursor.getString(2))) {
                    count += countRestoreFiles(treeUri, documentId, local, restored);
                } else if (!local.exists() || !ContentHelper.isReadable(local)) {
                    count++;
                }
            }
        } catch (Exception e) {
            Log.d(TAG, "countRestoreFiles failed " + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return count;
    }

    private static final String RESTORE_PREFIX = ".restore_";
    // a folder pass is running: its temporary copies are not left over yet
    private volatile boolean folderRestoreRunning = false;

    // Copies a restore left under the temporary name in RESTORE_FOLDERS of qzDirectory and their
    // subfolders (the old file was already removed when the rename failed or the app stopped)
    // take their real name. They are this install's own files, so no folder grant is needed.
    // Returns their paths, one per line; empty while a folder pass is running.
    public String recoverFolderRestoreCopies(String qzDirectory) {
        StringBuilder recovered = new StringBuilder();
        if (qzDirectory != null && !qzDirectory.isEmpty() && !folderRestoreRunning) {
            for (String folder : RESTORE_FOLDERS) {
                recoverRestoreCopies(new java.io.File(qzDirectory, folder), true, recovered);
            }
        }
        return recovered.toString();
    }

    // For each .restore_<name> in dir: the real name is free – the copy takes it; taken by a
    // readable file with the same bytes – the copy is dropped; with other bytes – the copy goes
    // under a free <name>_N, so neither is lost; taken by a file of a previous install – the copy
    // stays, and the folder pass copies that file again.
    private void recoverRestoreCopies(java.io.File dir, boolean recursive, StringBuilder recovered) {
        java.io.File[] files = dir.listFiles();
        if (files == null) {
            return;
        }
        for (java.io.File file : files) {
            String name = file.getName();
            if (file.isDirectory()) {
                if (recursive) {
                    recoverRestoreCopies(file, true, recovered);
                }
                continue;
            }
            if (!name.startsWith(RESTORE_PREFIX) || name.length() == RESTORE_PREFIX.length()) {
                continue;
            }
            java.io.File target = new java.io.File(dir, name.substring(RESTORE_PREFIX.length()));
            java.io.File result = null;
            if (!target.exists()) {
                result = moveRestoreCopy(file, target) ? target : null;
            } else if (!ContentHelper.isReadable(target)) {
                continue;
            } else if (ContentHelper.sameContent(file, target)) {
                file.delete();
                continue;
            } else {
                result = moveRestoreCopyToFreeName(file, target);
            }
            Log.d(TAG, "recoverRestoreCopies " + file + " -> " + result);
            if (result != null) {
                if (recovered.length() > 0) {
                    recovered.append('\n');
                }
                recovered.append(result.getAbsolutePath());
            }
        }
    }

    // Renames copy to target; when the rename fails, copies the bytes and keeps copy unless
    // target holds the same bytes. false: copy is still in place and target does not exist.
    private boolean moveRestoreCopy(java.io.File copy, java.io.File target) {
        if (copy.renameTo(target)) {
            return true;
        }
        // target is removed below only when this call has just written it
        if (!copy.isFile()) {
            return false;
        }
        if (ContentHelper.copyContentToFile(this, Uri.fromFile(copy), target.getAbsolutePath())
            && ContentHelper.sameContent(copy, target)) {
            copy.delete();
            return true;
        }
        target.delete();
        return false;
    }

    // Puts copy under the first free <name>_N next to target, as a picked file with a taken name
    // gets; null when no name could be used (copy kept).
    private java.io.File moveRestoreCopyToFreeName(java.io.File copy, java.io.File target) {
        String fileName = target.getName();
        int dot = fileName.lastIndexOf('.');
        String base = dot > 0 ? fileName.substring(0, dot) : fileName;
        String extension = dot > 0 ? fileName.substring(dot) : "";
        for (int attempt = 1; attempt < 100; attempt++) {
            java.io.File candidate = new java.io.File(target.getParentFile(), base + "_" + attempt + extension);
            if (!candidate.exists()) {
                return moveRestoreCopy(copy, candidate) ? candidate : null;
            }
        }
        return null;
    }

    private void releaseFolderGrant(Uri treeUri) {
        try {
            getContentResolver().releasePersistableUriPermission(treeUri,
                Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        } catch (Exception e) {
            try {
                getContentResolver().releasePersistableUriPermission(treeUri, Intent.FLAG_GRANT_READ_URI_PERMISSION);
            } catch (Exception e2) {
                Log.d(TAG, "releasePersistableUriPermission failed " + e2);
            }
        }
    }

    // The history database of a previous install cannot be opened, so the app runs on a spare
    // one; once its workouts are copied, the old file (and its journal) is removed through the
    // grant, and the next start goes back to ddb.sqlite. Spares of earlier installs are locked
    // the same way and go too, all but keepDatabase, the one this install is running on.
    private void deleteOldHistoryDatabase(Uri treeUri, String rootId, String keepDatabase) {
        Uri childrenUri = android.provider.DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, rootId);
        android.database.Cursor cursor = null;
        String databaseId = null;
        String journalId = null;
        java.util.ArrayList<String> spareIds = new java.util.ArrayList<String>();
        java.util.ArrayList<String> spareJournalIds = new java.util.ArrayList<String>();
        try {
            cursor = getContentResolver().query(childrenUri, new String[] {
                android.provider.DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                android.provider.DocumentsContract.Document.COLUMN_DISPLAY_NAME}, null, null, null);
            while (cursor != null && cursor.moveToNext()) {
                String name = cursor.getString(1);
                if ("ddb.sqlite".equals(name)) {
                    databaseId = cursor.getString(0);
                } else if ("ddb.sqlite-journal".equals(name)) {
                    journalId = cursor.getString(0);
                } else if (name != null && name.matches("ddb-\\d+\\.sqlite")
                           && !name.equals(keepDatabase)) {
                    spareIds.add(cursor.getString(0));
                } else if (name != null && name.matches("ddb-\\d+\\.sqlite-journal")
                           && !name.equals(keepDatabase + "-journal")) {
                    spareJournalIds.add(cursor.getString(0));
                }
            }
        } catch (Exception e) {
            Log.d(TAG, "deleteOldHistoryDatabase query failed " + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        // the journal goes first: a foreign journal left next to a new ddb.sqlite would break it
        if (journalId != null && !deleteTreeDocument(treeUri, journalId)) {
            return;
        }
        if (databaseId != null) {
            deleteTreeDocument(treeUri, databaseId);
        }
        for (String id : spareJournalIds) {
            deleteTreeDocument(treeUri, id);
        }
        for (String id : spareIds) {
            deleteTreeDocument(treeUri, id);
        }
    }

    private boolean deleteTreeDocument(Uri treeUri, String documentId) {
        try {
            boolean deleted = android.provider.DocumentsContract.deleteDocument(getContentResolver(),
                android.provider.DocumentsContract.buildDocumentUriUsingTree(treeUri, documentId));
            Log.d(TAG, "deleteTreeDocument " + documentId + " deleted=" + deleted);
            return deleted;
        } catch (Exception e) {
            Log.d(TAG, "deleteTreeDocument " + documentId + " failed " + e);
            return false;
        }
    }

    // the workout files importFitFilesFromTree() copies: the same walk and the same filter, no copy
    private int countFitFilesInTree(Uri treeUri, String parentId, boolean descendIntoFit) {
        Uri childrenUri = android.provider.DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, parentId);
        String[] projection = new String[] {
            android.provider.DocumentsContract.Document.COLUMN_DOCUMENT_ID,
            android.provider.DocumentsContract.Document.COLUMN_DISPLAY_NAME,
            android.provider.DocumentsContract.Document.COLUMN_MIME_TYPE};
        int count = 0;
        android.database.Cursor cursor = null;
        try {
            cursor = getContentResolver().query(childrenUri, projection, null, null, null);
            while (cursor != null && cursor.moveToNext()) {
                String documentId = cursor.getString(0);
                String name = cursor.getString(1);
                if (name == null || documentId == null) {
                    continue;
                }
                if (android.provider.DocumentsContract.Document.MIME_TYPE_DIR.equals(cursor.getString(2))) {
                    if (descendIntoFit && name.equalsIgnoreCase("fit")) {
                        count += countFitFilesInTree(treeUri, documentId, false);
                    }
                    continue;
                }
                String lowerName = name.toLowerCase(java.util.Locale.ROOT);
                if (lowerName.endsWith(".fit") && !lowerName.contains("backup")) {
                    count++;
                }
            }
        } catch (Exception e) {
            Log.d(TAG, "countFitFilesInTree failed " + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return count;
    }

    // progress: the file under way and the files to copy (countFitFilesInTree()); each workout file
    // moves it on by one and is passed to the native side
    private void importFitFilesFromTree(Uri treeUri, String parentId, String destinationDir,
                                        StringBuilder localPaths, boolean descendIntoFit, int[] progress) {
        Uri childrenUri = android.provider.DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, parentId);
        String[] projection = new String[] {
            android.provider.DocumentsContract.Document.COLUMN_DOCUMENT_ID,
            android.provider.DocumentsContract.Document.COLUMN_DISPLAY_NAME,
            android.provider.DocumentsContract.Document.COLUMN_MIME_TYPE,
            android.provider.DocumentsContract.Document.COLUMN_SIZE};
        android.database.Cursor cursor = null;
        try {
            cursor = getContentResolver().query(childrenUri, projection, null, null, null);
            while (cursor != null && cursor.moveToNext()) {
                String documentId = cursor.getString(0);
                String name = cursor.getString(1);
                String mimeType = cursor.getString(2);
                long size = cursor.isNull(3) ? -1 : cursor.getLong(3);
                if (name == null || documentId == null) {
                    continue;
                }
                if (android.provider.DocumentsContract.Document.MIME_TYPE_DIR.equals(mimeType)) {
                    if (descendIntoFit && name.equalsIgnoreCase("fit")) {
                        importFitFilesFromTree(treeUri, documentId, destinationDir, localPaths, false, progress);
                    }
                    continue;
                }
                String lowerName = name.toLowerCase(java.util.Locale.ROOT);
                // QZ-backup-*.fit are partial copies written during a workout, skipped at start
                // as well; importing them would show the workout twice
                if (!lowerName.endsWith(".fit") || lowerName.contains("backup")) {
                    continue;
                }
                // never past the count: a file may have come into the folder since it was counted
                if (progress[1] > 0) {
                    progress[0] = Math.min(progress[0] + 1, progress[1]);
                    nativeOnFitImportProgress(progress[0], progress[1]);
                }
                Uri documentUri = android.provider.DocumentsContract.buildDocumentUriUsingTree(treeUri, documentId);
                String localPath = ContentHelper.importContentToAppDirKeepExisting(this, documentUri, name, size,
                                                                                   destinationDir);
                if (!localPath.isEmpty()) {
                    if (localPaths.length() > 0) {
                        localPaths.append('\n');
                    }
                    localPaths.append(localPath);
                }
            }
        } catch (Exception e) {
            Log.d(TAG, "importFitFilesFromTree failed " + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    private void handleDocumentPickerResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == DOCUMENT_PICKER_FIT_FOLDER_REQUEST_CODE) {
            handleFitFolderResult(requestCode, resultCode, data, false);
            return;
        }
        if (requestCode == DOCUMENT_PICKER_RESTORE_FOLDER_REQUEST_CODE) {
            handleRestoreFolderResult(requestCode, resultCode, data);
            return;
        }
        String destinationDir = pendingImportDirectories.get(requestCode, "");
        pendingImportDirectories.remove(requestCode);

        String action = "";
        int flags = 0;
        int clipItemCount = 0;
        String uriString = "";
        String localPath = "";

        if (data != null) {
            action = data.getAction() == null ? "" : data.getAction();
            flags = data.getFlags();
            if (data.getClipData() != null) {
                clipItemCount = data.getClipData().getItemCount();
            }
            if (data.getData() != null) {
                Uri uri = data.getData();
                uriString = uri.toString();
                if (resultCode == RESULT_OK && requestCode == DOCUMENT_PICKER_FIT_REQUEST_CODE) {
                    nativeOnFitImportStarted();
                    // a workout must not replace another one that has the same file name
                    localPath = ContentHelper.importContentToAppDirKeepExisting(this, uri, "",
                                                                                ContentHelper.getFileSize(this, uri),
                                                                                destinationDir);
                } else if (resultCode == RESULT_OK) {
                    localPath = ContentHelper.importContentToAppDir(this, uri, destinationDir);
                }
            }
        }

        Log.d(TAG, "handleDocumentPickerResult requestCode=" + requestCode
            + " resultCode=" + resultCode
            + " action=" + action
            + " flags=0x" + Integer.toHexString(flags)
            + " clipItems=" + clipItemCount
            + " uri=" + uriString
            + " localPath=" + localPath);
        nativeOnDocumentPicked(requestCode, resultCode, localPath);
    }
}
