#ifndef IO_UNDER_QT
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <WatchConnectivity/WatchConnectivity.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import "qdomyoszwift-Swift2.h"
#include "ios/lockscreen.h"

@class virtualbike_ios_swift;
@class virtualbike_zwift;
@class healthkit;

static healthkit* h = 0;
static virtualbike_ios_swift* _virtualbike = nil;
static virtualbike_zwift* _virtualbike_zwift = nil;

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

void lockscreen::setKcal(double kcal)
{
    [h setKcalWithKcal:kcal];
}

void lockscreen::setDistance(double distance)
{
    [h setDistanceWithDistance:distance];
}

void lockscreen::virtualbike_ios()
{
    _virtualbike = [[virtualbike_ios_swift alloc] init];
}

void lockscreen::virtualbike_setHeartRate(unsigned char heartRate)
{
    if(_virtualbike != nil)
        [_virtualbike updateHeartRateWithHeartRate:heartRate];
    if(_virtualbike_zwift != nil)
        [_virtualbike_zwift updateHeartRateWithHeartRate:heartRate];
}

void lockscreen::virtualbike_setCadence(unsigned short crankRevolutions, unsigned short lastCrankEventTime)
{
    if(_virtualbike != nil)
        [_virtualbike updateCadenceWithCrankRevolutions:crankRevolutions LastCrankEventTime:lastCrankEventTime];
}

void lockscreen::virtualbike_zwift_ios()
{
    _virtualbike_zwift = [[virtualbike_zwift alloc] init];
}

double lockscreen::virtualbike_getCurrentSlope()
{
    if(_virtualbike_zwift != nil)
    {
        return [_virtualbike_zwift readCurrentSlope];
    }
    return 0;
}

double lockscreen::virtualbike_getPowerRequested()
{
    if(_virtualbike_zwift != nil)
    {
        return [_virtualbike_zwift readPowerRequested];
    }
    return 0;
}

void lockscreen::virtualbike_updateFTMS(UInt16 normalizeSpeed, UInt8 currentResistance, UInt16 currentCadence, UInt16 currentWatt)
{
    if(_virtualbike_zwift != nil)
        [_virtualbike_zwift updateFTMSWithNormalizeSpeed:normalizeSpeed currentCadence:currentCadence currentResistance:currentResistance currentWatt:currentWatt];
}
#endif
