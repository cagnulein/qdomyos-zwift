#ifndef IO_UNDER_QT
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "qdomyoszwift-Swift2.h"
#include "ios/ios_bluetoothsettings.h"
#include <QDebug>
#include <QString>
#include <QStringList>

// ---------------------------------------------------------------------------
// All public methods guard on @available(iOS 18, *) so the binary can still
// be deployed on older iOS versions without issues.
// ---------------------------------------------------------------------------

void ios_bluetoothsettings::activateSession() {
    if (@available(iOS 18.0, *)) {
        [[AccessorySetupManager shared] activateSession];
        qDebug() << "ios_bluetoothsettings: AccessorySetupKit session activated";
    } else {
        qDebug() << "ios_bluetoothsettings: AccessorySetupKit requires iOS 18+";
    }
}

void ios_bluetoothsettings::showAccessoryPicker() {
    if (@available(iOS 18.0, *)) {
        [[AccessorySetupManager shared] showPickerWithCompletion:^(BOOL success) {
            if (success) {
                qDebug() << "ios_bluetoothsettings: Accessory picker completed – device(s) authorized";
            } else {
                qDebug() << "ios_bluetoothsettings: Accessory picker cancelled or failed";
            }
        }];
    } else {
        qDebug() << "ios_bluetoothsettings: showAccessoryPicker – requires iOS 18+";
    }
}

QStringList ios_bluetoothsettings::getAuthorizedBluetoothUUIDs() {
    QStringList result;
    if (@available(iOS 18.0, *)) {
        NSArray<NSString *> *uuids = [[AccessorySetupManager shared] getAuthorizedBluetoothUUIDs];
        for (NSString *uuid in uuids) {
            result << QString::fromNSString(uuid);
        }
    }
    return result;
}

QStringList ios_bluetoothsettings::getAuthorizedAccessoryNames() {
    QStringList result;
    if (@available(iOS 18.0, *)) {
        NSArray<NSString *> *names = [[AccessorySetupManager shared] getAuthorizedAccessoryNames];
        for (NSString *name in names) {
            result << QString::fromNSString(name);
        }
    }
    return result;
}

bool ios_bluetoothsettings::hasAuthorizedAccessories() {
    if (@available(iOS 18.0, *)) {
        return [[AccessorySetupManager shared] hasAuthorizedAccessories];
    }
    return false;
}

int ios_bluetoothsettings::authorizedAccessoryCount() {
    if (@available(iOS 18.0, *)) {
        return (int)[[AccessorySetupManager shared] authorizedAccessoryCount];
    }
    return 0;
}

void ios_bluetoothsettings::removeAccessory(const QString &bluetoothUUID) {
    if (@available(iOS 18.0, *)) {
        NSString *uuidStr = bluetoothUUID.toNSString();
        [[AccessorySetupManager shared] removeAccessoryWithBluetoothUUID:uuidStr
                                                              completion:^(BOOL success) {
            if (success) {
                qDebug() << "ios_bluetoothsettings: Removed accessory" << bluetoothUUID;
            } else {
                qDebug() << "ios_bluetoothsettings: Failed to remove accessory" << bluetoothUUID;
            }
        }];
    }
}

void ios_bluetoothsettings::removeAllAccessories() {
    if (@available(iOS 18.0, *)) {
        [[AccessorySetupManager shared] removeAllAccessories];
        qDebug() << "ios_bluetoothsettings: All accessories removed";
    }
}

#endif // IO_UNDER_QT
