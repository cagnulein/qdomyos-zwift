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

// minimum delay between two Live Activity updates while the app is in background,
// must stay well below the inactivity timeout of LiveActivityBridge.swift
static const NSTimeInterval QZLiveActivityBackgroundInterval = 5.0;

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

    // ActivityKit gives the app a limited budget of Live Activity updates while
    // it is not in foreground: pushing one update per second burns it in a few
    // minutes, after which iOS silently stops applying the new content and the
    // Live Activity freezes (and is then rendered as "stale").  Slow down to one
    // update every 5 seconds while we are in background / screen off.
    static NSTimeInterval lastBackgroundUpdate = 0;
    if ([UIApplication sharedApplication].applicationState != UIApplicationStateActive) {
        const NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
        if (lastBackgroundUpdate != 0 && (now - lastBackgroundUpdate) < QZLiveActivityBackgroundInterval) {
            return;
        }
        lastBackgroundUpdate = now;
    } else {
        // back in foreground: let the next background update go through at once
        lastBackgroundUpdate = 0;
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
