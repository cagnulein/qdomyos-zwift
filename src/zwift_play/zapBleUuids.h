#ifndef ZAPBLEUUIDS_H
#define ZAPBLEUUIDS_H

#include <QUuid>
#include <QString>

class ZapBleUuids {
public:
    static const QUuid ZWIFT_CUSTOM_SERVICE_UUID;
    static const QUuid ZWIFT_ASYNC_CHARACTERISTIC_UUID;
    static const QUuid ZWIFT_SYNC_RX_CHARACTERISTIC_UUID;
    static const QUuid ZWIFT_SYNC_TX_CHARACTERISTIC_UUID;
    static const QUuid ZWIFT_UNKNOWN_6_CHARACTERISTIC_UUID;
};

const QUuid ZapBleUuids::ZWIFT_CUSTOM_SERVICE_UUID = QUuid("00000001-19CA-4651-86E5-FA29DCDD09D1");
const QUuid ZapBleUuids::ZWIFT_ASYNC_CHARACTERISTIC_UUID = QUuid("00000002-19CA-4651-86E5-FA29DCDD09D1");
const QUuid ZapBleUuids::ZWIFT_SYNC_RX_CHARACTERISTIC_UUID = QUuid("00000003-19CA-4651-86E5-FA29DCDD09D1");
const QUuid ZapBleUuids::ZWIFT_SYNC_TX_CHARACTERISTIC_UUID = QUuid("00000004-19CA-4651-86E5-FA29DCDD09D1");
const QUuid ZapBleUuids::ZWIFT_UNKNOWN_6_CHARACTERISTIC_UUID = QUuid("00000006-19CA-4651-86E5-FA29DCDD09D1");

class GenericBleUuids {
public:
    static QUuid uuidFromShortString(const QString& shortString) {
        QString uuidBase("0000xxxx-0000-1000-8000-00805F9B34FB");
        return QUuid(uuidBase.replace("xxxx", shortString));
    }

    static const QUuid GENERIC_ACCESS_SERVICE_UUID;
    static const QUuid DEVICE_NAME_CHARACTERISTIC_UUID;
    static const QUuid APPEARANCE_CHARACTERISTIC_UUID;
    static const QUuid PREFERRED_CONNECTION_PARAMS_CHARACTERISTIC_UUID;
    static const QUuid CENTRAL_ADDRESS_RESOLUTION_CHARACTERISTIC_UUID;
    static const QUuid GENERIC_ATTRIBUTE_SERVICE_UUID;
    static const QUuid SERVICE_CHANGED_CHARACTERISTIC_UUID;
    static const QUuid DEVICE_INFORMATION_SERVICE_UUID;
    static const QUuid MANUFACTURER_NAME_STRING_CHARACTERISTIC_UUID;
    static const QUuid SERIAL_NUMBER_STRING_CHARACTERISTIC_UUID;
    static const QUuid HARDWARE_REVISION_STRING_CHARACTERISTIC_UUID;
    static const QUuid FIRMWARE_REVISION_STRING_CHARACTERISTIC_UUID;
    static const QUuid BATTERY_SERVICE_UUID;
    static const QUuid BATTERY_LEVEL_CHARACTERISTIC_UUID;
    static const QUuid DEFAULT_DESCRIPTOR_UUID;
};

const QUuid GenericBleUuids::GENERIC_ACCESS_SERVICE_UUID = GenericBleUuids::uuidFromShortString("1800");
const QUuid GenericBleUuids::DEVICE_NAME_CHARACTERISTIC_UUID = GenericBleUuids::uuidFromShortString("2A00");
const QUuid GenericBleUuids::APPEARANCE_CHARACTERISTIC_UUID = GenericBleUuids::uuidFromShortString("2A01");
const QUuid GenericBleUuids::PREFERRED_CONNECTION_PARAMS_CHARACTERISTIC_UUID = GenericBleUuids::uuidFromShortString("2A04");
const QUuid GenericBleUuids::CENTRAL_ADDRESS_RESOLUTION_CHARACTERISTIC_UUID = GenericBleUuids::uuidFromShortString("2AA6");
const QUuid GenericBleUuids::GENERIC_ATTRIBUTE_SERVICE_UUID = GenericBleUuids::uuidFromShortString("1801");
const QUuid GenericBleUuids::SERVICE_CHANGED_CHARACTERISTIC_UUID = GenericBleUuids::uuidFromShortString("2A05");
const QUuid GenericBleUuids::DEVICE_INFORMATION_SERVICE_UUID = GenericBleUuids::uuidFromShortString("180A");
const QUuid GenericBleUuids::MANUFACTURER_NAME_STRING_CHARACTERISTIC_UUID = GenericBleUuids::uuidFromShortString("2A29");
const QUuid GenericBleUuids::SERIAL_NUMBER_STRING_CHARACTERISTIC_UUID = GenericBleUuids::uuidFromShortString("2A25");
const QUuid GenericBleUuids::HARDWARE_REVISION_STRING_CHARACTERISTIC_UUID = GenericBleUuids::uuidFromShortString("2A27");
const QUuid GenericBleUuids::FIRMWARE_REVISION_STRING_CHARACTERISTIC_UUID = GenericBleUuids::uuidFromShortString("2A26");
const QUuid GenericBleUuids::BATTERY_SERVICE_UUID = GenericBleUuids::uuidFromShortString("180F");
const QUuid GenericBleUuids::BATTERY_LEVEL_CHARACTERISTIC_UUID = GenericBleUuids::uuidFromShortString("2A19");
const QUuid GenericBleUuids::DEFAULT_DESCRIPTOR_UUID = GenericBleUuids::uuidFromShortString("2902");


#endif // ZAPBLEUUIDS_H
