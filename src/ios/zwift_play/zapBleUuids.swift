import Foundation
import CoreBluetooth

enum ZapBleUuids {
    // ZAP Service - Zwift Accessory Protocol
    static let zwiftCustomServiceUUID = CBUUID(string: "00000001-19CA-4651-86E5-FA29DCDD09D1")
    static let zwiftAsyncCharacteristicUUID = CBUUID(string: "00000002-19CA-4651-86E5-FA29DCDD09D1")
    static let zwiftSyncRxCharacteristicUUID = CBUUID(string: "00000003-19CA-4651-86E5-FA29DCDD09D1")
    static let zwiftSyncTxCharacteristicUUID = CBUUID(string: "00000004-19CA-4651-86E5-FA29DCDD09D1")
    // This doesn't appear in the real hardware but is found in the companion app code.
    // static let zwiftDebugCharacteristicUUID = CBUUID(string: "00000005-19CA-4651-86E5-FA29DCDD09D1")
    // I have not seen this characteristic used. Guess it could be for Device Firmware Update (DFU)? it is a chip from Nordic.
    static let zwiftUnknown6CharacteristicUUID = CBUUID(string: "00000006-19CA-4651-86E5-FA29DCDD09D1")
}

enum GenericBleUuids {
    static let genericAccessServiceUUID = CBUUID(string: BleUuids.uuidFromShortString("1800"))
    static let deviceNameCharacteristicUUID = CBUUID(string: BleUuids.uuidFromShortString("2A00")) // Zwift Play
    static let appearanceCharacteristicUUID = CBUUID(string: BleUuids.uuidFromShortString("2A01")) // [964] Gamepad (HID Subtype)
    static let preferredConnectionParamsCharacteristicUUID = CBUUID(string: BleUuids.uuidFromShortString("2A04"))
    static let centralAddressResolutionCharacteristicUUID = CBUUID(string: BleUuids.uuidFromShortString("2AA6"))

    static let genericAttributeServiceUUID = CBUUID(string: BleUuids.uuidFromShortString("1801"))
    static let serviceChangedCharacteristicUUID = CBUUID(string: BleUuids.uuidFromShortString("2A05"))

    static let deviceInformationServiceUUID = CBUUID(string: BleUuids.uuidFromShortString("180A"))
    static let manufacturerNameStringCharacteristicUUID = CBUUID(string: BleUuids.uuidFromShortString("2A29")) // Zwift Inc.
    static let serialNumberStringCharacteristicUUID = CBUUID(string: BleUuids.uuidFromShortString("2A25")) // 02-1[MAC]
    static let hardwareRevisionStringCharacteristicUUID = CBUUID(string: BleUuids.uuidFromShortString("2A27")) // B.0
    static let firmwareRevisionStringCharacteristicUUID = CBUUID(string: BleUuids.uuidFromShortString("2A26")) // 1.1.0

    static let batteryServiceUUID = CBUUID(string: BleUuids.uuidFromShortString("180F"))
    static let batteryLevelCharacteristicUUID = CBUUID(string: BleUuids.uuidFromShortString("2A19")) // 89

    static let defaultDescriptorUUID = CBUUID(string: BleUuids.uuidFromShortString("2902"))
}
