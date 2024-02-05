import Foundation

enum ZapConstants {
    static let zwiftManufacturerId = 2378 // Zwift, Inc
    static let rc1LeftSide: UInt8 = 3
    static let rc1RightSide: UInt8 = 2
    static let zwiftClick: UInt8 = 9

    static let rideOn = Data([82, 105, 100, 101, 79, 110])

    // these don't actually seem to matter, its just the header has to be 7 bytes RIDEON + 2
    static let requestStart = Data([0, 9]) //Data([1, 2])
    static let responseStart = Data([1, 3]) // from device

    // Message types received from device
    static let controllerNotificationMessageType: UInt8 = 7
    static let emptyMessageType: UInt8 = 21
    static let batteryLevelType: UInt8 = 25
    static let clickType: UInt8 = 55
}
