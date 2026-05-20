#import "ios_accessibility_helper.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <WebKit/WebKit.h>

static NSString *qzBool(BOOL value) {
    return value ? @"YES" : @"NO";
}

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

static void qzDumpViewTree(UIView *view, NSInteger depth, NSInteger maxDepth) {
    if (!view || depth > maxDepth) {
        return;
    }

    NSMutableString *indent = [NSMutableString string];
    for (NSInteger i = 0; i < depth; i++) {
        [indent appendString:@"  "];
    }

    NSString *label = view.accessibilityLabel ?: @"";
    NSString *identifier = view.accessibilityIdentifier ?: @"";
    NSLog(@"[QZ VO] %@%@ frame=%@ hidden=%@ alpha=%.2f user=%@ axElement=%@ axHidden=%@ modal=%@ label='%@' id='%@' subviews=%lu",
          indent,
          NSStringFromClass(view.class),
          NSStringFromCGRect(view.frame),
          qzBool(view.hidden),
          view.alpha,
          qzBool(view.userInteractionEnabled),
          qzBool(view.isAccessibilityElement),
          qzBool(view.accessibilityElementsHidden),
          qzBool(view.accessibilityViewIsModal),
          label,
          identifier,
          (unsigned long)view.subviews.count);

    for (UIView *subview in view.subviews) {
        qzDumpViewTree(subview, depth + 1, maxDepth);
    }
}

static NSMutableArray<WKWebView *> *qzFindVisibleWebViews(UIView *view, NSMutableArray<WKWebView *> *webViews) {
    if (!view || view.hidden || view.alpha < 0.01) {
        return webViews;
    }

    if ([view isKindOfClass:[WKWebView class]] && !CGRectIsEmpty(view.bounds)) {
        [webViews addObject:(WKWebView *)view];
    }

    for (UIView *subview in view.subviews) {
        qzFindVisibleWebViews(subview, webViews);
    }
    return webViews;
}

static WKWebView *qzFindTopmostVisibleWebView(UIView *view) {
    NSMutableArray<WKWebView *> *webViews = qzFindVisibleWebViews(view, [NSMutableArray array]);
    return webViews.lastObject;
}

static void qzLogWebViewDom(WKWebView *webView) {
    if (!webView) {
        NSLog(@"[QZ VO] DOM dump skipped: no WKWebView");
        return;
    }

    NSString *script = @"JSON.stringify({"
                       @"title: document.title,"
                       @"readyState: document.readyState,"
                       @"url: location.href,"
                       @"activeTag: document.activeElement ? document.activeElement.tagName : null,"
                       @"buttons: document.querySelectorAll('button').length,"
                       @"inputs: document.querySelectorAll('input').length,"
                       @"selects: document.querySelectorAll('select').length,"
                       @"links: document.querySelectorAll('a').length,"
                       @"interactive: document.querySelectorAll('button,input,select,textarea,a,[role=button],[tabindex]').length,"
                       @"bodyText: document.body ? document.body.innerText.slice(0, 120) : ''"
                       @"})";
    [webView evaluateJavaScript:script completionHandler:^(id result, NSError *error) {
        if (error) {
            NSLog(@"[QZ VO] DOM dump error: %@", error.localizedDescription);
        } else {
            NSLog(@"[QZ VO] DOM dump: %@", result);
        }
    }];
}

static void qzLogFocusedElement() {
    id focused = UIAccessibilityFocusedElement(UIAccessibilityNotificationVoiceOverIdentifier);
    NSLog(@"[QZ VO] focused accessibility element: %@", focused ? focused : @"(nil)");
}

static void qzPrepareWebViewAccessibility(WKWebView *webView) {
    if (!webView) {
        NSLog(@"[QZ VO] prepare skipped: WKWebView not found");
        return;
    }

    NSLog(@"[QZ VO] preparing WKWebView=%@ frame=%@ url=%@", webView, NSStringFromCGRect(webView.frame), webView.URL.absoluteString);

    webView.userInteractionEnabled = YES;
    webView.accessibilityElementsHidden = NO;
    webView.isAccessibilityElement = NO;

    webView.scrollView.userInteractionEnabled = YES;
    webView.scrollView.accessibilityElementsHidden = NO;
    webView.scrollView.isAccessibilityElement = NO;

    UIView *parent = webView.superview;
    NSInteger parentDepth = 0;
    while (parent && ![parent isKindOfClass:[UIWindow class]]) {
        NSLog(@"[QZ VO] parent[%ld] %@ before: axElement=%@ axHidden=%@ user=%@ frame=%@",
              (long)parentDepth,
              NSStringFromClass(parent.class),
              qzBool(parent.isAccessibilityElement),
              qzBool(parent.accessibilityElementsHidden),
              qzBool(parent.userInteractionEnabled),
              NSStringFromCGRect(parent.frame));
        parent.accessibilityElementsHidden = NO;
        parent.isAccessibilityElement = NO;
        parent.userInteractionEnabled = YES;
        parent = parent.superview;
        parentDepth++;
    }
}

void ios_accessibility_helper::prepareEmbeddedWebViewForVoiceOver() {
    dispatch_async(dispatch_get_main_queue(), ^{
        UIWindow *window = qzKeyWindow();
        NSLog(@"[QZ VO] prepareEmbeddedWebViewForVoiceOver begin; window=%@ voiceOverRunning=%@", window, qzBool(UIAccessibilityIsVoiceOverRunning()));
        qzLogFocusedElement();
        qzDumpViewTree(window, 0, 7);

        NSMutableArray<WKWebView *> *webViews = qzFindVisibleWebViews(window, [NSMutableArray array]);
        NSLog(@"[QZ VO] visible WKWebView count=%lu", (unsigned long)webViews.count);
        for (NSUInteger i = 0; i < webViews.count; i++) {
            WKWebView *webView = webViews[i];
            NSLog(@"[QZ VO] WKWebView[%lu]=%@ frame=%@ hidden=%@ alpha=%.2f user=%@ axElement=%@ axHidden=%@ url=%@",
                  (unsigned long)i,
                  webView,
                  NSStringFromCGRect(webView.frame),
                  qzBool(webView.hidden),
                  webView.alpha,
                  qzBool(webView.userInteractionEnabled),
                  qzBool(webView.isAccessibilityElement),
                  qzBool(webView.accessibilityElementsHidden),
                  webView.URL.absoluteString);
        }

        WKWebView *webView = qzFindTopmostVisibleWebView(window);
        qzPrepareWebViewAccessibility(webView);
        qzLogWebViewDom(webView);
        if (webView) {
            UIAccessibilityPostNotification(UIAccessibilityScreenChangedNotification, webView);
            NSLog(@"[QZ VO] posted UIAccessibilityScreenChangedNotification with WKWebView target");
        }

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.35 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            NSLog(@"[QZ VO] delayed accessibility check");
            WKWebView *delayedWebView = qzFindTopmostVisibleWebView(qzKeyWindow());
            qzPrepareWebViewAccessibility(delayedWebView);
            qzLogWebViewDom(delayedWebView);
            qzLogFocusedElement();
            if (delayedWebView) {
                UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, delayedWebView);
                NSLog(@"[QZ VO] posted UIAccessibilityLayoutChangedNotification with WKWebView target");
            }
        });
    });
}
