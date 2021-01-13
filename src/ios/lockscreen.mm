#ifndef IO_UNDER_QT
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <WatchConnectivity/WatchConnectivity.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import "qdomyoszwift-Swift2.h"
#include "ios/lockscreen.h"

@class virtualbike_ios_swift;
@class healthkit;

static healthkit* h = 0;
static virtualbike_ios_swift* _virtualbike = nil;

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
    _virtualbike = [[virtualbike_ios_swift alloc] init];
}

void lockscreen::virtualbike_setHeartRate(unsigned char heartRate)
{
    if(_virtualbike != nil)
        [_virtualbike updateHeartRateWithHeartRate:heartRate];
}

void lockscreen::virtualbike_setCadence(unsigned short crankRevolutions, unsigned short lastCrankEventTime)
{
    if(_virtualbike != nil)
        [_virtualbike updateCadenceWithCrankRevolutions:crankRevolutions LastCrankEventTime:lastCrankEventTime];
}
#endif
