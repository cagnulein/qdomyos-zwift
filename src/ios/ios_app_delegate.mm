#ifndef IO_UNDER_QT
#import <ConnectIQ/ConnectIQ.h>
#import "UIKit/UIKit.h"
#import "UserNotifications/UserNotifications.h"
#include "lockscreen.h"

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
		if (@available(iOS 13.0, *)) {
	    [self setupDynamicQuickActions];
		}

    return YES;
}

- (void)setupDynamicQuickActions {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths firstObject];
    NSString *profilesDirectory = [documentsDirectory stringByAppendingPathComponent:@"profiles"];
    
    NSError *error = nil;
    NSArray *files = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:profilesDirectory error:&error];
    if (error) {
        NSLog(@"Error reading files: %@", error.localizedDescription);
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
        NSLog(@"performActionForShortcutItem");
        NSString *fileName = [shortcutItem.type stringByReplacingOccurrencesOfString:@"org.cagnulein.qdomyoszwift." withString:@""];
        NSString *fileNameWithExtension = [fileName stringByAppendingString:@".qzs"];
        //self.selectedShortcutItem = fileNameWithExtension;
        lockscreen::set_action_profile([fileNameWithExtension UTF8String]);
        NSLog(@"performActionForShortcutItem %@", [[NSString alloc] initWithUTF8String:lockscreen::get_action_profile()]);
    }
	}

    return YES;
}

- (void)application:(UIApplication *)application
performFetchWithCompletionHandler:(void (^)(UIBackgroundFetchResult result))completionHandler
{
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    lockscreen ls;
    ls.startBackgroundAudio();
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    lockscreen ls;
    ls.stopBackgroundAudio();
}
@end
#endif
