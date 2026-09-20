// Qt's generated iOS project defines IO_UNDER_QT for the application sources.
// Keep the UIScene bridge outside that guard: it is an overlay for the Qt
// platform plugin and must be present in the final application binary.
#import "UIKit/UIKit.h"
#import <objc/runtime.h>
#include <QGuiApplication>
#include <QScreen>
#include <QWindow>
#include <QtMath>

static BOOL qz_desktopManagerOverlayInstalled = NO;
static UIWindow *qz_sceneWindow = nil;

static void qz_installDesktopManagerOverlay(void);

static BOOL qz_isIPhoneDuoWindow(UIWindow *window)
{
    if (!window || UI_USER_INTERFACE_IDIOM() != UIUserInterfaceIdiomPhone)
        return NO;

    const CGFloat longSide = MAX(window.bounds.size.width, window.bounds.size.height);
    const CGFloat shortSide = MIN(window.bounds.size.width, window.bounds.size.height);
    return (longSide >= 1400 && longSide <= 1450 && shortSide >= 980 && shortSide <= 1020) ||
           (longSide >= 650 && longSide <= 700 && shortSide >= 450 && shortSide <= 500) ||
           (longSide >= 1000 && longSide <= 1030 && shortSide >= 680 && shortSide <= 710) ||
           (longSide >= 930 && longSide <= 970 && shortSide >= 650 && shortSide <= 690);
}

static BOOL qz_isStandardIPhoneWindow(UIWindow *window)
{
    return window && UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone &&
           !qz_isIPhoneDuoWindow(window);
}

static void qz_layoutQtViews(UIView *container)
{
    if (!container)
        return;

    for (UIView *subview in container.subviews) {
        if ([NSStringFromClass(subview.class) isEqualToString:@"QUIView"]) {
            UIWindow *window = container.window;
            if (qz_isStandardIPhoneWindow(window))
                subview.frame = UIEdgeInsetsInsetRect(container.bounds, container.safeAreaInsets);
            else if (qz_isIPhoneDuoWindow(window))
                subview.frame = container.bounds;
            else if (CGRectIsEmpty(subview.frame))
                subview.frame = container.bounds;
            subview.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
            [subview setNeedsLayout];
        }
        qz_layoutQtViews(subview);
    }
}

static void qz_sendUpdatedExposeEvent(UIView *view)
{
    if (!view)
        return;

    if ([NSStringFromClass(view.class) isEqualToString:@"QUIView"]) {
        // QUIView asserts when exposure is refreshed synchronously from
        // viewDidLayoutSubviews. Rotation on iPhone Duo enters that callback
        // while UIKit is still committing the new geometry, so defer the
        // refresh until the layout transaction has completed.
        UIView *qtView = view;
        dispatch_async(dispatch_get_main_queue(), ^{
            if (qtView && qtView.window && [qtView respondsToSelector:@selector(sendUpdatedExposeEvent)])
                [qtView performSelector:@selector(sendUpdatedExposeEvent)];
        });
    }

    for (UIView *subview in view.subviews)
        qz_sendUpdatedExposeEvent(subview);
}

static void qz_desktopManagerDidAddSubview(id managerView, SEL selector, UIView *subview)
{
    Q_UNUSED(subview)

    UIWindow *window = [managerView window];
    if (window && window.windowScene) {
        // Qt 5.15 uses UIWindow.screen here. With UIScene lifecycle enabled,
        // iOS 27 rejects that path; the scene delegate already attached the
        // same Qt window to its UIWindowScene.
        window.hidden = NO;
        qz_layoutQtViews(window.rootViewController.view);
        if (!qz_isStandardIPhoneWindow(window))
            qz_sendUpdatedExposeEvent(subview);
        return;
    }

    // During the first subview insertion UIKit may not have populated
    // managerView.window yet. If the scene delegate already connected Qt's
    // UIWindow, keep using that scene-owned window and never fall through to
    // Qt 5.15's deprecated UIWindow.screen assignment.
    if (qz_sceneWindow && qz_sceneWindow.windowScene) {
        qz_sceneWindow.hidden = NO;
        qz_layoutQtViews(qz_sceneWindow.rootViewController.view);
        if (!qz_isStandardIPhoneWindow(qz_sceneWindow))
            qz_sendUpdatedExposeEvent(subview);
        return;
    }

    // The subview has already been inserted. Do not call Qt 5.15's original
    // implementation here: its only remaining action is UIWindow.screen =,
    // which iOS 27 rejects for scene-based applications. The scene delegate
    // attaches and shows the same UIWindow once the scene is ready.
    Q_UNUSED(selector)
}

static void qz_installDesktopManagerOverlay(void)
{
    if (qz_desktopManagerOverlayInstalled)
        return;

    Class desktopManagerViewClass = NSClassFromString(@"QIOSDesktopManagerView");
    if (!desktopManagerViewClass) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 50 * NSEC_PER_MSEC),
                       dispatch_get_main_queue(), ^{
            qz_installDesktopManagerOverlay();
        });
        return;
    }

    Method desktopManagerMethod = class_getInstanceMethod(desktopManagerViewClass,
                                                           @selector(didAddSubview:));
    if (!desktopManagerMethod)
        return;

    method_setImplementation(desktopManagerMethod, (IMP)qz_desktopManagerDidAddSubview);
    qz_desktopManagerOverlayInstalled = YES;
}

@interface QIOSApplicationDelegate : UIResponder <UIApplicationDelegate>
@end

@interface QIOSViewController : UIViewController
@end

@interface QZWindowSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property(nonatomic, retain) UIWindow *window;
@end

static void *qz_screenForWindowScene(UIWindowScene *windowScene)
{
    if (!qGuiApp || !windowScene)
        return nullptr;

    const CGSize sceneBounds = windowScene.coordinateSpace.bounds.size;
    void *matchingScreen = nullptr;
    for (QScreen *screen : qGuiApp->screens()) {
        if (!screen || !screen->handle())
            continue;

        const QSize screenSize = screen->geometry().size();
        const BOOL sameSize = qRound(sceneBounds.width) == screenSize.width() &&
                              qRound(sceneBounds.height) == screenSize.height();
        const BOOL rotatedSize = qRound(sceneBounds.width) == screenSize.height() &&
                                 qRound(sceneBounds.height) == screenSize.width();
        if (sameSize || rotatedSize) {
            matchingScreen = screen->handle();
            break;
        }
    }

    // Avoid calling private QIOSScreen methods here: the device and simulator
    // archives have different private implementations. Matching the scene
    // coordinate size keeps the correct QScreen stable on dual-display iPhone.
    if (matchingScreen)
        return matchingScreen;

    return qGuiApp->screens().size() == 1 && qGuiApp->screens().first()
        ? qGuiApp->screens().first()->handle()
        : nullptr;
}

static UIWindow *qz_existingQtWindowForScreen(void *platformScreen)
{
    if (!qGuiApp || !platformScreen)
        return nil;

    UIWindow *fallbackWindow = nil;
    for (QWindow *window : qGuiApp->topLevelWindows()) {
        if (!window || !window->handle())
            continue;

        UIView *view = reinterpret_cast<UIView *>(window->winId());
        if (!view.window)
            continue;

        if (window->screen() && window->screen()->handle() == platformScreen)
            return view.window;

        if (!fallbackWindow)
            fallbackWindow = view.window;
    }

    return fallbackWindow;
}

@implementation QZWindowSceneDelegate

- (void)scene:(UIScene *)scene
    willConnectToSession:(UISceneSession *)session
             options:(UISceneConnectionOptions *)connectionOptions
{
    Q_UNUSED(session)
    Q_UNUSED(connectionOptions)

    if (![scene isKindOfClass:[UIWindowScene class]])
        return;

    UIWindowScene *windowScene = static_cast<UIWindowScene *>(scene);
    void *platformScreen = qz_screenForWindowScene(windowScene);
    if (!platformScreen) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 50 * NSEC_PER_MSEC),
                       dispatch_get_main_queue(), ^{
            [self scene:scene willConnectToSession:session options:connectionOptions];
        });
        return;
    }

    // Qt 5.15 has already created the UIWindow and attached the QQuick view
    // to it. Reuse that exact window; creating a second UIWindow leaves the
    // scene black because Qt continues rendering into its original one.
    UIWindow *qtWindow = qz_existingQtWindowForScreen(platformScreen);
    if (!qtWindow) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 50 * NSEC_PER_MSEC),
                       dispatch_get_main_queue(), ^{
            [self scene:scene willConnectToSession:session options:connectionOptions];
        });
        return;
    }

    // Qt owns this UIWindow. Keep the scene delegate's property empty: on
    // iOS 27 retaining the pre-scene Qt window here can race its native
    // teardown and crash in objc_retain before QML is shown.
    qz_sceneWindow = qtWindow;
    qz_sceneWindow.windowScene = windowScene;
    qz_sceneWindow.hidden = NO;
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone &&
        !qz_isIPhoneDuoWindow(qz_sceneWindow)) {
        qz_sceneWindow.rootViewController.view.backgroundColor = UIColor.blackColor;
    }

    // The Qt window may have been created before UIKit connected the scene.
    // Detach its platform view from the pre-scene desktop manager and apply
    // the already requested state again, matching the upstream Qt scene fix.
    for (QWindow *window : qGuiApp->topLevelWindows()) {
        if (!window || !window->handle() || !window->screen()
            || window->screen()->handle() != platformScreen)
            continue;

        window->setParent(nullptr);
        window->setWindowStates(window->windowStates());
    }

    qz_installDesktopManagerOverlay();
    [qz_sceneWindow makeKeyAndVisible];
    [qz_sceneWindow.rootViewController.view setNeedsLayout];
    [qz_sceneWindow.rootViewController.view layoutIfNeeded];
    qz_layoutQtViews(qz_sceneWindow.rootViewController.view);
    if (!qz_isStandardIPhoneWindow(qz_sceneWindow))
        qz_sendUpdatedExposeEvent(qz_sceneWindow.rootViewController.view);
}

@end

@interface QIOSApplicationDelegate (QZSceneLifecycle)
@end

@implementation QIOSApplicationDelegate (QZSceneLifecycle)

- (UISceneConfiguration *)application:(UIApplication *)application
    configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession
                                  options:(UISceneConnectionOptions *)options
{
    Q_UNUSED(application)
    Q_UNUSED(options)

    UISceneConfiguration *configuration = connectingSceneSession.configuration;
    configuration.delegateClass = [QZWindowSceneDelegate class];
    return configuration;
}

@end

@interface QIOSViewController (QZSceneLayout)
- (void)qz_viewDidLayoutSubviews;
@end

@implementation QIOSViewController (QZSceneLayout)

+ (void)load
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        dispatch_async(dispatch_get_main_queue(), ^{
            qz_installDesktopManagerOverlay();
        });

        Method originalMethod = class_getInstanceMethod(self, @selector(viewDidLayoutSubviews));
        Method overlayMethod = class_getInstanceMethod(self, @selector(qz_viewDidLayoutSubviews));
        if (!originalMethod || !overlayMethod)
            return;

        // QIOSViewController inherits viewDidLayoutSubviews from UIKit. A
        // direct exchange would therefore modify UIViewController globally and
        // make unrelated UIKit windows call this overlay selector. Install a
        // private override on QIOSViewController instead.
        const BOOL added = class_addMethod(self,
                                           @selector(viewDidLayoutSubviews),
                                           method_getImplementation(overlayMethod),
                                           method_getTypeEncoding(overlayMethod));
        if (added) {
            class_addMethod(self,
                            @selector(qz_viewDidLayoutSubviews),
                            method_getImplementation(originalMethod),
                            method_getTypeEncoding(originalMethod));
        } else {
            method_exchangeImplementations(originalMethod, overlayMethod);
        }
    });
}

- (void)qz_viewDidLayoutSubviews
{
    static BOOL updatingGeometry = NO;
    if (updatingGeometry)
        return;

    updatingGeometry = YES;
    [self qz_viewDidLayoutSubviews];

    if (!qGuiApp || !self.view.window) {
        updatingGeometry = NO;
        return;
    }

    const CGSize bounds = self.view.bounds.size;
    const QSize sceneSize(qRound(bounds.width), qRound(bounds.height));
    if (sceneSize.isEmpty()) {
        updatingGeometry = NO;
        return;
    }

    const BOOL standardIPhone = qz_isStandardIPhoneWindow(self.view.window);
    for (QWindow *window : qGuiApp->topLevelWindows()) {
            if (!window || !window->isVisible())
                continue;
            if (!(window->windowStates() & (Qt::WindowFullScreen | Qt::WindowMaximized)))
                continue;

            const UIEdgeInsets safeInsets = standardIPhone ? self.view.window.safeAreaInsets : UIEdgeInsetsZero;
            const QRect sceneGeometry = standardIPhone
                ? QRect(qRound(safeInsets.left), qRound(safeInsets.top),
                        qMax(0, sceneSize.width() - qRound(safeInsets.left + safeInsets.right)),
                        qMax(0, sceneSize.height() - qRound(safeInsets.top + safeInsets.bottom)))
                : QRect(0, 0, sceneSize.width(), sceneSize.height());
            const QRect currentGeometry = window->geometry();
            if (currentGeometry != sceneGeometry)
                window->setGeometry(sceneGeometry);
    }
    [self.view layoutIfNeeded];
    qz_layoutQtViews(self.view);
    if (!standardIPhone)
        qz_sendUpdatedExposeEvent(self.view);
    updatingGeometry = NO;
}

@end

#ifndef IO_UNDER_QT
#import <ConnectIQ/ConnectIQ.h>
#import "UIKit/UIKit.h"
#import "UserNotifications/UserNotifications.h"
#import <objc/runtime.h>
#include <QDebug>
#include <QGuiApplication>
#include <QMetaObject>
#include <QScreen>
#include <QWindow>
#include "homeform.h"
#include "lockscreen.h"
#include "authutils.h"

// Qt defines QIOSApplicationDelegate internally as a UIResponder-backed
// UIApplicationDelegate.  Keep the local declaration aligned with that shape
// so category methods can legally forward unhandled events to super.
@interface QIOSApplicationDelegate : UIResponder <UIApplicationDelegate, IQAppMessageDelegate, IQUIOverrideDelegate, IQDeviceEventDelegate>
@end

@interface QIOSApplicationDelegate (QZApplicationDelegate) <IQAppMessageDelegate, IQUIOverrideDelegate, IQDeviceEventDelegate>
@end

@interface UIApplication (QZKeyboardShortcuts)
- (void)qz_sendEvent:(UIEvent *)event;
@end

@implementation UIApplication (QZKeyboardShortcuts)

+ (void)load {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        Method originalMethod = class_getInstanceMethod(self, @selector(sendEvent:));
        Method swizzledMethod = class_getInstanceMethod(self, @selector(qz_sendEvent:));
        method_exchangeImplementations(originalMethod, swizzledMethod);
    });
}

- (void)qz_sendEvent:(UIEvent *)event {
    bool didHandleShortcut = false;

    if (@available(iOS 13.4, *)) {
        if ([event isKindOfClass:[UIPressesEvent class]]) {
            UIPressesEvent *pressesEvent = (UIPressesEvent *)event;
            for (UIPress *press in pressesEvent.allPresses) {
                if (press.phase != UIPressPhaseBegan) {
                    continue;
                }

                UIKey *key = press.key;
                if (key == nil || key.charactersIgnoringModifiers.length == 0) {
                    continue;
                }

                const QString sequence =
                    QString::fromUtf8(key.charactersIgnoringModifiers.UTF8String).trimmed().toUpper();
                if (homeform::singleton() && homeform::singleton()->handleKeyboardShortcut(sequence)) {
                    didHandleShortcut = true;
                }
            }
        }
    }

    if (!didHandleShortcut) {
        [self qz_sendEvent:event];
    }
}

@end

@implementation QIOSApplicationDelegate (QZApplicationDelegate)

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    qDebug() << "QZ iOS launch";
    UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
    [center requestAuthorizationWithOptions:UNAuthorizationOptionBadge
      completionHandler:^(BOOL granted, NSError *error){
          if(granted == YES) {
              [[UIApplication sharedApplication] setMinimumBackgroundFetchInterval:UIApplicationBackgroundFetchIntervalMinimum];
          };
      }];
		if (@available(iOS 13.0, *)) {
	    [self setupDynamicQuickActions];
		}

    return YES;
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    Q_UNUSED(application)
    qDebug() << "QZ iOS lifecycle: applicationWillEnterForeground";
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    Q_UNUSED(application)
    qDebug() << "QZ iOS lifecycle: applicationDidBecomeActive";
}

- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
    bool didHandleShortcut = false;

    if (@available(iOS 13.4, *)) {
        for (UIPress *press in presses) {
            UIKey *key = press.key;
            if (key == nil || key.charactersIgnoringModifiers.length == 0) {
                continue;
            }

            const QString sequence =
                QString::fromUtf8(key.charactersIgnoringModifiers.UTF8String).trimmed().toUpper();
            if (homeform::singleton() && homeform::singleton()->handleKeyboardShortcut(sequence)) {
                didHandleShortcut = true;
            }
        }
    }

    if (!didHandleShortcut) {
        [super pressesBegan:presses withEvent:event];
    }
}

- (void)setupDynamicQuickActions {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths firstObject];
    NSString *profilesDirectory = [documentsDirectory stringByAppendingPathComponent:@"profiles"];
    
    NSError *error = nil;
    NSArray *files = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:profilesDirectory error:&error];
    if (error) {
        qWarning() << "Error reading files:" << QString::fromUtf8(error.localizedDescription.UTF8String);
        return;
    }
    
    NSMutableArray *quickActions = [NSMutableArray array];

    for (NSString *fileName in files) {
        if (quickActions.count >= 4) {
            break;
        }
        NSString *fileNameWithoutExtension = [fileName stringByDeletingPathExtension];
        UIApplicationShortcutIcon *icon = [UIApplicationShortcutIcon iconWithType:UIApplicationShortcutIconTypeCompose];
        UIApplicationShortcutItem *shortcutItem = [[UIApplicationShortcutItem alloc] initWithType:[@"org.cagnulein.qdomyoszwift." stringByAppendingString:fileNameWithoutExtension]
                                                                                localizedTitle:fileNameWithoutExtension
                                                                                localizedSubtitle:nil
                                                                                            icon:icon
                                                                                        userInfo:nil];
        [quickActions addObject:shortcutItem];
    }

    [UIApplication sharedApplication].shortcutItems = quickActions;
}

- (BOOL)application:(UIApplication *)application willFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	if (@available(iOS 13.0, *)) {
    UIApplicationShortcutItem *shortcutItem = [launchOptions objectForKey:UIApplicationLaunchOptionsShortcutItemKey];
    if ([shortcutItem.type hasPrefix:@"org.cagnulein.qdomyoszwift."]) {
        qDebug() << "performActionForShortcutItem";
        NSString *fileName = [shortcutItem.type stringByReplacingOccurrencesOfString:@"org.cagnulein.qdomyoszwift." withString:@""];
        NSString *fileNameWithExtension = [fileName stringByAppendingString:@".qzs"];
        //self.selectedShortcutItem = fileNameWithExtension;
        lockscreen::set_action_profile([fileNameWithExtension UTF8String]);
        qDebug() << "performActionForShortcutItem" << QString::fromUtf8(lockscreen::get_action_profile());
    }
	}

    return YES;
}

- (void)application:(UIApplication *)application
performFetchWithCompletionHandler:(void (^)(UIBackgroundFetchResult result))completionHandler
{
}

- (BOOL)application:(UIApplication *)application
 continueUserActivity:(NSUserActivity *)userActivity
   restorationHandler:(void (^)(NSArray * _Nullable))restorationHandler
{
    Q_UNUSED(application)
    Q_UNUSED(restorationHandler)

    qDebug() << "QZ iOS continueUserActivity called: activityType="
             << QString::fromUtf8(userActivity.activityType.UTF8String);

    if ([userActivity.activityType isEqualToString:NSUserActivityTypeBrowsingWeb]) {
        NSURL *url = userActivity.webpageURL;
        qDebug() << "QZ iOS continueUserActivity webpageURL="
                 << (url ? sanitizedOAuthCallbackUrl(QString::fromUtf8(url.absoluteString.UTF8String))
                         : QStringLiteral("(null)"));
        if (url != nil && homeform::singleton()) {
            const QString callbackUrl = QString::fromUtf8(url.absoluteString.UTF8String);
            const QUrl qUrl(callbackUrl);
            if (qUrl.isValid() && qUrl.host() == QStringLiteral("www.qzfitness.com") &&
                qUrl.path().startsWith(QStringLiteral("/peloton/callback"))) {
                qDebug() << "QZ iOS continueUserActivity matched Peloton callback";
                QMetaObject::invokeMethod(homeform::singleton(), "handleOAuthCallbackUrl", Qt::QueuedConnection,
                                          Q_ARG(QString, callbackUrl));
                return YES;
            }
            qDebug() << "QZ iOS continueUserActivity ignored URL";
        }
    }

    qDebug() << "QZ iOS continueUserActivity returning NO";
    return NO;
}
@end
#endif
