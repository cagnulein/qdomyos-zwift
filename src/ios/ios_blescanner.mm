#ifndef IO_UNDER_QT
#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import "qdomyoszwift-Swift2.h"
#include "ios/ios_blescanner.h"
#include <QDebug>

static BleScanner* _bleScanner = nil;

void ios_blescanner::startScan() {
    if (_bleScanner == nil) {
        _bleScanner = [BleScanner shared];
    }
    [_bleScanner startScan];
    qDebug() << "iOS BLE scanner started for generic device detection";
}

void ios_blescanner::stopScan() {
    if (_bleScanner != nil) {
        [_bleScanner stopScan];
        qDebug() << "iOS BLE scanner stopped";
    }
}

QString ios_blescanner::getDeviceServices(const QString& deviceId) {
    if (_bleScanner == nil) {
        return QString();
    }

    NSString *nsDeviceId = deviceId.toNSString();
    NSString *services = [_bleScanner getDeviceServicesWithMacAddress:nsDeviceId];

    if (services != nil && services.length > 0) {
        return QString::fromNSString(services);
    }

    return QString();
}

#endif
