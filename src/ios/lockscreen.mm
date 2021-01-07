#ifndef IO_UNDER_QT
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <WatchConnectivity/WatchConnectivity.h>
#import "qdomyoszwift-Swift1.h"
#include "ios/lockscreen.h"

@class virtualdevice_ios;
@class healthkit;

static healthkit* h = 0;
static virtualdevice_ios* _virtualbike;

void lockscreen::setTimerDisabled() {
     [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
}

void lockscreen::request()
{
    h = [[healthkit alloc] init];
    [h request];
}

long lockscreen::heartRate()
{
    return [h heartRate];
}

void lockscreen::virtualbike_ios()
{
    _virtualbike = [[virtualdevice_ios alloc] init];
}
#endif
