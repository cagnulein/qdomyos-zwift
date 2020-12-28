#ifndef IO_UNDER_QT
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <WatchConnectivity/WatchConnectivity.h>
#import "qdomyoszwift-Swift.h"
#include "ios/lockscreen.h"

static healthkit* h = 0;

void lockscreen::setTimerDisabled() {
     [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
}

@class healthkit;

void lockscreen::request()
{
    h = [[healthkit alloc] init];
    [h request];
}

long lockscreen::heartRate()
{
    return [h heartRate];
}
#endif
