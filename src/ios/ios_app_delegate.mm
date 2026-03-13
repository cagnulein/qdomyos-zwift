#ifndef IO_UNDER_QT
#import <ConnectIQ/ConnectIQ.h>
#import "UIKit/UIKit.h"
#import "UserNotifications/UserNotifications.h"
#include <QDebug>
#include <QMetaObject>
#include "homeform.h"
#include "lockscreen.h"

@interface QIOSApplicationDelegate <IQAppMessageDelegate, IQUIOverrideDelegate, IQDeviceEventDelegate>
@end

@interface QIOSApplicationDelegate (QZApplicationDelegate) <IQAppMessageDelegate, IQUIOverrideDelegate, IQDeviceEventDelegate>
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
                 << (url ? QString::fromUtf8(url.absoluteString.UTF8String) : QStringLiteral("(null)"));
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
