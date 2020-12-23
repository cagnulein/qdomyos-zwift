#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <WatchConnectivity/WatchConnectivity.h>
#import "qdomyoszwift-Swift.h"
#include "ios/lockscreen.h"

void lockscreen::setTimerDisabled() {
     [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
}

@class healthkit;

void lockscreen::request()
{
    healthkit* h = [[healthkit alloc] init];
    [h request];
}
