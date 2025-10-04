#ifndef IO_UNDER_QT
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <WatchConnectivity/WatchConnectivity.h>
#import <ActivityKit/ActivityKit.h>
#import <ConnectIQ/ConnectIQ.h>
#import "qdomyoszwift-Swift2.h"
#include "ios/ios_liveactivity.h"
#include <QDebug>

@class LiveActivityManager;

static LiveActivityManager* _liveActivityManager = nil;

void ios_liveactivity::startLiveActivity(const char* deviceName) {
    if (@available(iOS 16.1, *)) {
        if (_liveActivityManager == nil) {
            _liveActivityManager = [[LiveActivityManager alloc] init];
        }
        NSString *name = [NSString stringWithCString:deviceName encoding:NSASCIIStringEncoding];
        [_liveActivityManager startActivityWithDeviceName:name];
        qDebug() << "Live Activity started for device:" << deviceName;
    } else {
        qDebug() << "Live Activities require iOS 16.1 or later";
    }
}

void ios_liveactivity::updateLiveActivity(double speed, double cadence, double power, int heartRate, double distance, double kcal) {
    if (@available(iOS 16.1, *)) {
        if (_liveActivityManager != nil) {
            [_liveActivityManager updateActivityWithSpeed:speed
                                                  cadence:cadence
                                                    power:power
                                                heartRate:heartRate
                                                 distance:distance
                                                     kcal:kcal];
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
