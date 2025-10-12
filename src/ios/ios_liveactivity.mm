#ifndef IO_UNDER_QT
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <WatchConnectivity/WatchConnectivity.h>
#import <ConnectIQ/ConnectIQ.h>
#import <ActivityKit/ActivityKit.h>
#import "qdomyoszwift-Swift2.h"
#include "ios/ios_liveactivity.h"
#include <QDebug>

@class LiveActivityBridge;

static LiveActivityBridge* _liveActivityManager = nil;

void ios_liveactivity::startLiveActivity(const char* deviceName, bool useMiles) {
    if (@available(iOS 16.1, *)) {
        if (_liveActivityManager == nil) {
            _liveActivityManager = [[LiveActivityBridge alloc] init];
        }
        NSString *name = [NSString stringWithCString:deviceName encoding:NSUTF8StringEncoding];
        [_liveActivityManager startActivityWithDeviceName:name useMiles:useMiles];
        qDebug() << "Live Activity started for device:" << deviceName << "useMiles:" << useMiles;
    } else {
        qDebug() << "Live Activities require iOS 16.1 or later";
    }
}

void ios_liveactivity::updateLiveActivity(double speed, double cadence, double power, int heartRate, double distance, double kcal, bool useMiles) {
    if (@available(iOS 16.1, *)) {
        if (_liveActivityManager != nil) {
            [_liveActivityManager updateActivityWithSpeed:speed
                                                  cadence:cadence
                                                    power:power
                                                heartRate:heartRate
                                                 distance:distance
                                                     kcal:kcal
                                                 useMiles:useMiles];
        }
    }
}

void ios_liveactivity::endLiveActivity() {
    if (@available(iOS 16.1, *)) {
        if (_liveActivityManager != nil) {
            [_liveActivityManager endActivity];
            qDebug() << "Live Activity ended";
        }
    }
}

bool ios_liveactivity::isLiveActivityRunning() {
    if (@available(iOS 16.1, *)) {
        if (_liveActivityManager != nil) {
            return [_liveActivityManager isActivityRunning];
        }
    }
    return false;
}

#endif
