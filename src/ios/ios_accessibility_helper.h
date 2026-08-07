#ifndef IOS_ACCESSIBILITY_HELPER_H
#define IOS_ACCESSIBILITY_HELPER_H

class ios_accessibility_helper {
  public:
    static void prepareEmbeddedWebViewForVoiceOver();

    // When modal=true: marks the topmost Qt overlay as accessibilityViewIsModal so
    // VoiceOver cannot escape to elements behind the drawer.
    // When modal=false: restores normal accessibility traversal.
    static void setDrawerAccessibilityModal(bool modal);

    // Returns true when VoiceOver is currently active.
    static bool isVoiceOverRunning();

    // --- Native modal Workout Editor -------------------------------------------
    //
    // Qt 5.15's iOS accessibility bridge cannot expose an embedded WKWebView's web
    // content tree to VoiceOver as a navigable container (it treats it as a single
    // opaque element). The robust fix is to present the workout editor in a native
    // full-screen WKWebView inside a UIViewController presented modally by UIKit,
    // OUTSIDE Qt's view hierarchy. There VoiceOver navigates the HTML natively.
    //
    // The web page communicates with the app over the QZWS websocket on localhost,
    // so a native WKWebView loading the same URL behaves identically.

    typedef void (*WorkoutEditorClosedCallback)();

    // Register a callback invoked (on the main thread) when the user dismisses the
    // native editor via its Close button.
    static void setWorkoutEditorClosedCallback(WorkoutEditorClosedCallback cb);

    // Present a native full-screen modal WKWebView loading the given URL.
    static void presentNativeWorkoutEditor(const char *url);

    // Dismiss the native editor if currently presented (safe no-op otherwise).
    // Used when the workout is started via "Save & Start" from the web form.
    static void dismissNativeWorkoutEditor();
};

#endif // IOS_ACCESSIBILITY_HELPER_H
