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
    [self setupDynamicQuickActions];

    return YES;
}

- (void)setupDynamicQuickActions {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths firstObject];
    NSString *profilesDirectory = [documentsDirectory stringByAppendingPathComponent:@"profiles"];
    
    NSError *error;
    NSArray *files = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:profilesDirectory error:&error];
    if (error) {
        NSLog(@"Error reading files: %@", error.localizedDescription);
        return;
    }
    
    NSMutableArray *quickActions = [NSMutableArray array];
    
    for (NSString *fileName in files) {
        UIApplicationShortcutIcon *icon = [UIApplicationShortcutIcon iconWithType:UIApplicationShortcutIconTypeCompose];
        UIApplicationShortcutItem *shortcutItem = [[UIApplicationShortcutItem alloc] initWithType:[@"org.cagnulein.qdomyoszwift." stringByAppendingString:fileName]
                                                                                   localizedTitle:fileName
                                                                                localizedSubtitle:nil
                                                                                             icon:icon
                                                                                         userInfo:nil];
        [quickActions addObject:shortcutItem];
    }
    
    [UIApplication sharedApplication].shortcutItems = quickActions;
}

- (void)application:(UIApplication *)application performActionForShortcutItem:(UIApplicationShortcutItem *)shortcutItem completionHandler:(void(^)(BOOL succeeded))completionHandler {
    if ([shortcutItem.type hasPrefix:@"com.yourapp."]) {
        NSString *fileName = [shortcutItem.type stringByReplacingOccurrencesOfString:@"org.cagnulein.qdomyoszwift." withString:@""];
        //self.selectedShortcutItem = fileName;
    }
}

- (void)application:(UIApplication *)application
performFetchWithCompletionHandler:(void (^)(UIBackgroundFetchResult result))completionHandler
{
}
@end
#endif
