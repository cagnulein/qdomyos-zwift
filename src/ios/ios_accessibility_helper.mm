#import "ios_accessibility_helper.h"

#import <UIKit/UIKit.h>
#import <WebKit/WebKit.h>

static UIWindow *qzKeyWindow() {
    if (@available(iOS 13.0, *)) {
        for (UIScene *scene in UIApplication.sharedApplication.connectedScenes) {
            if (scene.activationState == UISceneActivationStateForegroundActive &&
                [scene isKindOfClass:[UIWindowScene class]]) {
                UIWindowScene *windowScene = (UIWindowScene *)scene;
                for (UIWindow *window in windowScene.windows) {
                    if (window.isKeyWindow) {
                        return window;
                    }
                }
            }
        }
    }
    return UIApplication.sharedApplication.keyWindow;
}

static WKWebView *qzFindVisibleWebView(UIView *view) {
    if (!view || view.hidden || view.alpha < 0.01) {
        return nil;
    }

    if ([view isKindOfClass:[WKWebView class]] && !CGRectIsEmpty(view.bounds)) {
        return (WKWebView *)view;
    }

    for (UIView *subview in view.subviews.reverseObjectEnumerator) {
        WKWebView *webView = qzFindVisibleWebView(subview);
        if (webView) {
            return webView;
        }
    }
    return nil;
}

static void qzPrepareWebViewAccessibility(WKWebView *webView) {
    if (!webView) {
        return;
    }

    webView.userInteractionEnabled = YES;
    webView.accessibilityElementsHidden = NO;
    webView.isAccessibilityElement = NO;

    webView.scrollView.userInteractionEnabled = YES;
    webView.scrollView.accessibilityElementsHidden = NO;
    webView.scrollView.isAccessibilityElement = NO;

    UIView *parent = webView.superview;
    while (parent && ![parent isKindOfClass:[UIWindow class]]) {
        parent.accessibilityElementsHidden = NO;
        parent.isAccessibilityElement = NO;
        parent = parent.superview;
    }
}

void ios_accessibility_helper::prepareEmbeddedWebViewForVoiceOver() {
    dispatch_async(dispatch_get_main_queue(), ^{
        UIWindow *window = qzKeyWindow();
        WKWebView *webView = qzFindVisibleWebView(window);
        qzPrepareWebViewAccessibility(webView);
        if (webView) {
            UIAccessibilityPostNotification(UIAccessibilityScreenChangedNotification, webView);
        }

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.35 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            WKWebView *delayedWebView = qzFindVisibleWebView(qzKeyWindow());
            qzPrepareWebViewAccessibility(delayedWebView);
            if (delayedWebView) {
                UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, delayedWebView);
            }
        });
    });
}
