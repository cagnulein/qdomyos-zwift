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

static bool liveActivitiesSupportedOnCurrentRuntime() {
    if (!@available(iOS 16.1, *)) {
        qDebug() << "Live Activities require iOS 16.1 or later";
        return false;
    }

    if (@available(iOS 14.0, *)) {
        NSProcessInfo *processInfo = [NSProcessInfo processInfo];
        if ([processInfo respondsToSelector:@selector(isiOSAppOnMac)] && processInfo.isiOSAppOnMac) {
            qDebug() << "Live Activities disabled for iOS apps running on macOS";
            return false;
        }
    }

    return true;
}

void ios_liveactivity::startLiveActivity(const char* deviceName, bool useMiles, const char* compactLeadingMetric,
                                         const char* compactTrailingMetric) {
    if (!liveActivitiesSupportedOnCurrentRuntime()) {
        return;
    }

    if (_liveActivityManager == nil) {
        _liveActivityManager = [[LiveActivityBridge alloc] init];
    }
    NSString *name = [NSString stringWithCString:deviceName encoding:NSUTF8StringEncoding];
    NSString *leadingMetric = [NSString stringWithCString:compactLeadingMetric encoding:NSUTF8StringEncoding];
    NSString *trailingMetric = [NSString stringWithCString:compactTrailingMetric encoding:NSUTF8StringEncoding];
    [_liveActivityManager startActivityWithDeviceName:name
                                             useMiles:useMiles
                                 compactLeadingMetric:leadingMetric
                                compactTrailingMetric:trailingMetric];
    qDebug() << "Live Activity started for device:" << deviceName << "useMiles:" << useMiles;
}

void ios_liveactivity::updateLiveActivity(double speed, double cadence, double power, int heartRate, double distance,
                                          double kcal, bool useMiles, const char* compactLeadingMetric,
                                          int compactLeadingValue, const char* compactTrailingMetric,
                                          int compactTrailingValue) {
    if (!liveActivitiesSupportedOnCurrentRuntime()) {
        return;
    }

    if (_liveActivityManager != nil) {
        NSString *leadingMetric = [NSString stringWithCString:compactLeadingMetric encoding:NSUTF8StringEncoding];
        NSString *trailingMetric = [NSString stringWithCString:compactTrailingMetric encoding:NSUTF8StringEncoding];
        [_liveActivityManager updateActivityWithSpeed:speed
                                              cadence:cadence
                                                power:power
                                            heartRate:heartRate
                                             distance:distance
                                                 kcal:kcal
                                   compactLeadingMetric:leadingMetric
                                    compactLeadingValue:compactLeadingValue
                                  compactTrailingMetric:trailingMetric
                                   compactTrailingValue:compactTrailingValue
                                             useMiles:useMiles];
    }
}

void ios_liveactivity::endLiveActivity() {
    if (!liveActivitiesSupportedOnCurrentRuntime()) {
        return;
    }

    if (_liveActivityManager != nil) {
        [_liveActivityManager endActivity];
        qDebug() << "Live Activity ended";
    }
}

bool ios_liveactivity::isLiveActivityRunning() {
    if (!liveActivitiesSupportedOnCurrentRuntime()) {
        return false;
    }

    if (_liveActivityManager != nil) {
        return [_liveActivityManager isActivityRunning];
    }
    return false;
}

#endif
