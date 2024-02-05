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
