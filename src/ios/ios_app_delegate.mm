#ifndef IO_UNDER_QT
#import <ConnectIQ/ConnectIQ.h>
#import "UIKit/UIKit.h"
#import "UserNotifications/UserNotifications.h"

@interface QIOSApplicationDelegate <IQAppMessageDelegate, IQUIOverrideDelegate, IQDeviceEventDelegate>
@end

@interface QIOSApplicationDelegate (QZApplicationDelegate) <IQAppMessageDelegate, IQUIOverrideDelegate, IQDeviceEventDelegate>
@end

@implementation QIOSApplicationDelegate (QZApplicationDelegate)

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    NSLog(@"launch!");
    UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
    [center requestAuthorizationWithOptions:UNAuthorizationOptionBadge
      completionHandler:^(BOOL granted, NSError *error){
          if(granted == YES) {
              [[UIApplication sharedApplication] setMinimumBackgroundFetchInterval:UIApplicationBackgroundFetchIntervalMinimum];
          };
      }];

    [[ConnectIQ sharedInstance] initializeWithUrlScheme:@"org.cagnulein.connectiqcomms-ciq"
                                 uiOverrideDelegate:nil];
    
    [[ConnectIQ sharedInstance] showConnectIQDeviceSelection];

    return YES;
}

- (void)application:(UIApplication *)application
performFetchWithCompletionHandler:(void (^)(UIBackgroundFetchResult result))completionHandler
{
}
@end
#endif
