#ifndef IOS_ACCESSIBILITY_HELPER_H
#define IOS_ACCESSIBILITY_HELPER_H

class ios_accessibility_helper {
  public:
    static void prepareEmbeddedWebViewForVoiceOver();

    // When modal=true: marks the topmost Qt overlay as accessibilityViewIsModal so
    // VoiceOver cannot escape to elements behind the drawer.
    // When modal=false: restores normal accessibility traversal.
    static void setDrawerAccessibilityModal(bool modal);
};

#endif // IOS_ACCESSIBILITY_HELPER_H
