#import "ios_accessibility_helper.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <WebKit/WebKit.h>

static NSString *qzBool(BOOL value) {
    return value ? @"YES" : @"NO";
}

// Returns current time as "HH:MM:SS.mmm" for inline log timestamps.
static NSString *qzNow() {
    NSDate *now = [NSDate date];
    NSCalendar *cal = [NSCalendar calendarWithIdentifier:NSCalendarIdentifierGregorian];
    NSDateComponents *c = [cal components:(NSCalendarUnitHour | NSCalendarUnitMinute |
                                           NSCalendarUnitSecond)
                                 fromDate:now];
    NSTimeInterval ti = now.timeIntervalSinceReferenceDate;
    NSInteger ms = (NSInteger)(fmod(ti, 1.0) * 1000);
    return [NSString stringWithFormat:@"%02ld:%02ld:%02ld.%03ld",
            (long)c.hour, (long)c.minute, (long)c.second, (long)ms];
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
    if (!focused) return;

    // Log the container that owns this element — this tells us exactly which UIView
    // is responsible for the zero-frame ghost inside Qt's accessibility tree.
    if ([focused respondsToSelector:@selector(accessibilityContainer)]) {
        id container = ((UIAccessibilityElement *)focused).accessibilityContainer;
        NSLog(@"[QZ VO] focused element container: class=%@ ptr=%p",
              NSStringFromClass([container class]), container);
        if ([container isKindOfClass:[UIView class]]) {
            UIView *cv = (UIView *)container;
            NSArray *cvElements = cv.accessibilityElements;
            NSLog(@"[QZ VO]   container.accessibilityElements.count=%lu frame=%@",
                  (unsigned long)cvElements.count, NSStringFromCGRect(cv.frame));
        }
    }
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

void ios_accessibility_helper::setDrawerAccessibilityModal(bool modal) {
    // Qt 5.15 iOS uses virtual UIAccessibilityElement objects (not UIViews) for its
    // accessibility tree, so UIView-level accessibilityElementsHidden is ineffective.
    // The real work is done by the QML Accessible.ignored bindings on each element.
    // This function simply posts UIAccessibilityScreenChangedNotification so VoiceOver
    // re-reads the tree immediately after those bindings update (150 ms delay in QML).
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"[QZ VO][%@] setDrawerAccessibilityModal=%d → posting ScreenChangedNotification", qzNow(), (int)modal);
        UIAccessibilityPostNotification(UIAccessibilityScreenChangedNotification, nil);
    });
}

// Replace the QMacAccessibilityElement with zero frame (Qt's ghost element for the
// WebView area) with the real WKWebView inside Qt's accessibility elements array.
//
// Why this is needed:
//   Qt creates a QMacAccessibilityElement for every QML item that has Accessible.*
//   properties (or a default role).  For the WebView item, this element ends up with
//   frame {{0,0},{0,0}} — a zero-size ghost.  VoiceOver CAN focus on it (reads the
//   page title "Workout Editor") but swipe-navigation fails because iOS cannot compute
//   "what's spatially next" after a zero-size element.
//   By replacing the ghost with the actual WKWebView (frame = {0,48,440,812}),
//   VoiceOver treats it as a proper container, enters its web-area, and lets the user
//   swipe through all HTML elements (inputs, buttons, etc.).
//
// Lifetime:
//   accessibilityElements is reset to nil after 4 s so Qt's dynamic protocol
//   (accessibilityElementCount / accessibilityElementAtIndex) takes over again once
//   VoiceOver has entered the web area.
static void qzInjectWKWebViewIntoQtAccessibilityTree(WKWebView *webView) {
    if (!webView) return;

    UIWindow *window = qzKeyWindow();

    // Find QIOSDesktopManagerView — Qt's top-level accessibility host.
    // It exposes QMacAccessibilityElement objects via accessibilityElementAtIndex:
    // (NOT via the accessibilityElements property, which returns nil on that view).
    // accessibilityElementCount also returns NSNotFound (= NSIntegerMax) so we must
    // NOT use it as a loop bound — probe with small indices only.
    UIView *qtDesktop = nil;
    UIView *cursor = webView.superview;
    while (cursor && cursor != window) {
        NSString *cls = NSStringFromClass(cursor.class);
        if ([cls containsString:@"DesktopManager"] || [cls hasPrefix:@"QIOS"]) {
            qtDesktop = cursor;   // keep updating to find the outermost QIOS view
        }
        cursor = cursor.superview;
    }

    // Also remember the immediate Qt rendering view (QUIView) as fallback.
    UIView *quiView = webView.superview; // QUIView is always the direct parent

    if (!qtDesktop) {
        NSLog(@"[QZ VO] QIOSDesktopManagerView not found");
    }

    // --- Pass 1: QIOSDesktopManagerView via accessibilityElementAtIndex: ---
    // Qt's accessibilityElementCount returns NSNotFound but the index method may work.
    UIView *hostView = nil;
    NSMutableArray *probed = [NSMutableArray array];

    if (qtDesktop) {
        NSLog(@"[QZ VO] probing %@ via accessibilityElementAtIndex: (cap 50)",
              NSStringFromClass(qtDesktop.class));
        for (NSInteger i = 0; i < 50; i++) {
            id el = [qtDesktop accessibilityElementAtIndex:i];
            if (!el) break;
            CGRect frame = CGRectZero;
            if ([el respondsToSelector:@selector(accessibilityFrame)])
                frame = ((UIAccessibilityElement *)el).accessibilityFrame;
            NSString *label = @"";
            if ([el respondsToSelector:@selector(accessibilityLabel)])
                label = ((UIAccessibilityElement *)el).accessibilityLabel ?: @"";
            NSLog(@"[QZ VO]   QIOSDesktop[%ld] class=%@ label='%@' frame=%@",
                  (long)i, NSStringFromClass([el class]), label, NSStringFromCGRect(frame));
            [probed addObject:el];
        }
        NSLog(@"[QZ VO] probed %lu elements from QIOSDesktopManagerView", (unsigned long)probed.count);
        if (probed.count > 0) hostView = qtDesktop;
    }

    // --- Pass 2: fallback to QUIView.accessibilityElements property ---
    if (!hostView && quiView) {
        NSArray *quiElements = quiView.accessibilityElements;
        NSLog(@"[QZ VO] fallback: QUIView.accessibilityElements.count=%lu",
              (unsigned long)quiElements.count);
        if (quiElements.count > 0) {
            [probed addObjectsFromArray:quiElements];
            hostView = quiView;
            for (id el in quiElements) {
                CGRect frame = CGRectZero;
                if ([el respondsToSelector:@selector(accessibilityFrame)])
                    frame = ((UIAccessibilityElement *)el).accessibilityFrame;
                NSString *label = @"";
                if ([el respondsToSelector:@selector(accessibilityLabel)])
                    label = ((UIAccessibilityElement *)el).accessibilityLabel ?: @"";
                NSLog(@"[QZ VO]   QUIView element class=%@ label='%@' frame=%@",
                      NSStringFromClass([el class]), label, NSStringFromCGRect(frame));
            }
        }
    }

    if (!hostView) {
        NSLog(@"[QZ VO] no host found — injection not possible");
        return;
    }
    NSLog(@"[QZ VO] host: %@ (%lu elements)", NSStringFromClass(hostView.class), (unsigned long)probed.count);

    // Replace any zero-frame ghost element with the actual WKWebView.
    NSMutableArray *newElements = [NSMutableArray arrayWithCapacity:probed.count + 1];
    BOOL injected = NO;
    for (id el in probed) {
        CGRect frame = CGRectZero;
        if ([el respondsToSelector:@selector(accessibilityFrame)])
            frame = ((UIAccessibilityElement *)el).accessibilityFrame;
        NSString *label = @"";
        if ([el respondsToSelector:@selector(accessibilityLabel)])
            label = ((UIAccessibilityElement *)el).accessibilityLabel ?: @"";

        if (!injected && frame.size.width < 1.0 && frame.size.height < 1.0) {
            NSLog(@"[QZ VO]   → replacing zero-frame ghost '%@' with WKWebView", label);
            [newElements addObject:webView];
            injected = YES;
        } else {
            [newElements addObject:el];
        }
    }
    if (!injected) {
        NSLog(@"[QZ VO] no zero-frame ghost — appending WKWebView");
        [newElements addObject:webView];
    }

    // Setting accessibilityElements on the view makes UIKit use this static array
    // exclusively, bypassing Qt's dynamic protocol methods.
    hostView.accessibilityElements = newElements;
    NSLog(@"[QZ VO][%@] patched %@ (%lu elements)", qzNow(), NSStringFromClass(hostView.class), (unsigned long)newElements.count);

    // Keep WKWebView in the accessibility array for 120 s so the user has enough
    // time to navigate the form (42 interactive elements). After 120 s Qt's
    // dynamic protocol takes over again.
    UIView * __unsafe_unretained unsafeHostView = hostView;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(120.0 * NSEC_PER_SEC)),
                   dispatch_get_main_queue(), ^{
        if (unsafeHostView.accessibilityElements != nil) {
            unsafeHostView.accessibilityElements = nil;
            NSLog(@"[QZ VO][%@] %@ restored to dynamic mode", qzNow(), NSStringFromClass(unsafeHostView.class));
        }
    });
}

void ios_accessibility_helper::prepareEmbeddedWebViewForVoiceOver() {
    dispatch_async(dispatch_get_main_queue(), ^{
        UIWindow *window = qzKeyWindow();
        NSLog(@"[QZ VO][%@] prepareEmbeddedWebViewForVoiceOver begin; voiceOverRunning=%@",
              qzNow(), qzBool(UIAccessibilityIsVoiceOverRunning()));
        qzLogFocusedElement();

        NSMutableArray<WKWebView *> *webViews = qzFindVisibleWebViews(window, [NSMutableArray array]);
        NSLog(@"[QZ VO] visible WKWebView count=%lu", (unsigned long)webViews.count);
        for (NSUInteger i = 0; i < webViews.count; i++) {
            WKWebView *wv = webViews[i];
            NSLog(@"[QZ VO] WKWebView[%lu] frame=%@ axElement=%@ axHidden=%@ url=%@",
                  (unsigned long)i, NSStringFromCGRect(wv.frame),
                  qzBool(wv.isAccessibilityElement),
                  qzBool(wv.accessibilityElementsHidden),
                  wv.URL.absoluteString);
        }

        WKWebView *webView = qzFindTopmostVisibleWebView(window);
        if (!webView) {
            NSLog(@"[QZ VO] no visible WKWebView found – giving up");
            return;
        }

        qzPrepareWebViewAccessibility(webView);
        qzLogWebViewDom(webView);

        // Inject WKWebView into Qt's accessibility elements array so that
        // VoiceOver can reach it via swipe navigation after the notification.
        qzInjectWKWebViewIntoQtAccessibilityTree(webView);

        // Focus a heading element (non-interactive) so VoiceOver enters the web area
        // in BROWSE mode — not edit mode. Focusing an <input> earlier caused VoiceOver
        // to enter edit mode where swipe-right moves the text cursor instead of
        // navigating to the next form field.
        //
        // We then post UIAccessibilityScreenChangedNotification three times with
        // increasing delays. The first post may be ignored while VoiceOver is still
        // processing the stale ghost reference; subsequent posts act as fallback.
        // Focus the first <label> so VoiceOver announces something clearly different
        // from the Qt toolbar "Workout Editor" label — both the Qt element and the
        // HTML <h1> say "Workout Editor", making it impossible for the user to tell
        // whether they are inside the form or still in Qt's accessibility tree.
        // A <label> element (e.g. "Name") is unambiguous.
        NSString *focusScript =
            @"(function(){"
            @"  var el = document.querySelector('label, [role=label]');"
            @"  if (!el) el = document.querySelector('h1,h2,h3,[role=heading],legend');"
            @"  if (!el) el = document.body;"
            @"  el.setAttribute('tabindex','-1'); el.focus();"
            @"  return el.tagName+':'+(el.textContent||'').trim().slice(0,40);"
            @"})()";

        // SINGLE notification only. Repeated ScreenChangedNotification posts each
        // RESET VoiceOver focus back to the hinted element — if the user swipes to
        // the next field, a delayed notification yanks them back, making navigation
        // appear "stuck". One post is enough to enter the web area.
        WKWebView * __unsafe_unretained unsafeWebView = webView;
        [webView evaluateJavaScript:focusScript completionHandler:^(id result, NSError *error) {
            NSLog(@"[QZ VO][%@] JS heading focus: %@ (error: %@)", qzNow(), result, error.localizedDescription);
            UIAccessibilityPostNotification(UIAccessibilityScreenChangedNotification, unsafeWebView);
            NSLog(@"[QZ VO][%@] ScreenChangedNotification (single)", qzNow());
        }];
    });
}

// ============================================================================
// Native modal Workout Editor (VoiceOver-friendly path)
// ============================================================================

static ios_accessibility_helper::WorkoutEditorClosedCallback s_workoutEditorClosedCallback = nullptr;

// Find the top-most presented view controller to present from.
static UIViewController *qzTopViewController() {
    UIWindow *window = qzKeyWindow();
    UIViewController *vc = window.rootViewController;
    while (vc.presentedViewController) {
        vc = vc.presentedViewController;
    }
    return vc;
}

// ---------------------------------------------------------------------------
// QZWorkoutEditorViewController — a plain native UIViewController hosting a
// full-screen WKWebView. Presented inside a UINavigationController so we get a
// nav bar with an accessible "Done" button. Because UIKit presents this modally
// (outside Qt's QUIView), VoiceOver navigates the WKWebView's HTML tree natively.
// ---------------------------------------------------------------------------
@interface QZWorkoutEditorViewController : UIViewController <WKNavigationDelegate>
@property (nonatomic, retain) NSString *urlToLoad;
@property (nonatomic, assign) WKWebView *webView; // owned by self.view subview tree
@end

// Weak-ish static reference to the currently presented navigation controller so
// dismissNativeWorkoutEditor() can dismiss it. Valid only while presented.
static UINavigationController * __unsafe_unretained s_presentedNav = nil;

@implementation QZWorkoutEditorViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"Workout Editor";
    self.view.backgroundColor = [UIColor systemBackgroundColor];

    UIBarButtonItem *done =
        [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone
                                                      target:self
                                                      action:@selector(closeTapped)];
    self.navigationItem.leftBarButtonItem = done;
    [done release];

    WKWebViewConfiguration *config = [[WKWebViewConfiguration alloc] init];
    WKWebView *wv = [[WKWebView alloc] initWithFrame:self.view.bounds configuration:config];
    [config release];
    wv.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    wv.navigationDelegate = self;
    [self.view addSubview:wv];
    self.webView = wv;
    [wv release]; // retained by the subview tree

    if (self.urlToLoad.length > 0) {
        NSURL *url = [NSURL URLWithString:self.urlToLoad];
        NSLog(@"[QZ VO][native] loading %@", url.absoluteString);
        [wv loadRequest:[NSURLRequest requestWithURL:url]];
    }
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation {
    NSLog(@"[QZ VO][native] page loaded — VoiceOver can navigate the form natively");
    UIAccessibilityPostNotification(UIAccessibilityScreenChangedNotification, webView);
}

- (void)closeTapped {
    NSLog(@"[QZ VO][native] Done tapped — dismissing native editor");
    ios_accessibility_helper::dismissNativeWorkoutEditor();
    if (s_workoutEditorClosedCallback) {
        s_workoutEditorClosedCallback();
    }
}

- (void)dealloc {
    [_urlToLoad release];
    [super dealloc];
}

@end

void ios_accessibility_helper::setWorkoutEditorClosedCallback(WorkoutEditorClosedCallback cb) {
    s_workoutEditorClosedCallback = cb;
}

bool ios_accessibility_helper::isVoiceOverRunning() {
    return UIAccessibilityIsVoiceOverRunning();
}

void ios_accessibility_helper::presentNativeWorkoutEditor(const char *url) {
    if (!url) return;
    NSString *urlString = [NSString stringWithUTF8String:url];
    dispatch_async(dispatch_get_main_queue(), ^{
        if (s_presentedNav) {
            NSLog(@"[QZ VO][native] editor already presented — skipping");
            return;
        }
        QZWorkoutEditorViewController *editorVC = [[QZWorkoutEditorViewController alloc] init];
        editorVC.urlToLoad = urlString;

        UINavigationController *nav =
            [[UINavigationController alloc] initWithRootViewController:editorVC];
        [editorVC release];
        nav.modalPresentationStyle = UIModalPresentationFullScreen;

        UIViewController *top = qzTopViewController();
        NSLog(@"[QZ VO][native] presenting native editor from %@", NSStringFromClass(top.class));
        s_presentedNav = nav; // UIKit retains while presented
        [top presentViewController:nav animated:YES completion:^{
            NSLog(@"[QZ VO][native] native editor presented");
        }];
        [nav release]; // presentViewController retains it
    });
}

void ios_accessibility_helper::dismissNativeWorkoutEditor() {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!s_presentedNav) return;
        UINavigationController *nav = s_presentedNav;
        s_presentedNav = nil;
        NSLog(@"[QZ VO][native] dismissing native editor");
        [nav dismissViewControllerAnimated:YES completion:^{
            NSLog(@"[QZ VO][native] native editor dismissed");
        }];
    });
}
