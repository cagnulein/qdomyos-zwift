package com.che.zap.device

import com.che.zap.utils.BleUuids
import com.che.zap.utils.BleUuids.uuidFromShortString
import java.util.UUID

object ZapBleUuids {

    // The Zwift custom characteristic details were found from decompiling the Zwift Companion app and searching for the service UUID

    // ZAP Service - Zwift Accessory Protocol
    val ZWIFT_CUSTOM_SERVICE_UUID: UUID = UUID.fromString("00000001-19CA-4651-86E5-FA29DCDD09D1")
    val ZWIFT_ASYNC_CHARACTERISTIC_UUID: UUID = UUID.fromString("00000002-19CA-4651-86E5-FA29DCDD09D1")
    val ZWIFT_SYNC_RX_CHARACTERISTIC_UUID: UUID = UUID.fromString("00000003-19CA-4651-86E5-FA29DCDD09D1")
    val ZWIFT_SYNC_TX_CHARACTERISTIC_UUID: UUID = UUID.fromString("00000004-19CA-4651-86E5-FA29DCDD09D1")
    // This doesn't appear in the real hardware but is found in the companion app code.
    // val ZWIFT_DEBUG_CHARACTERISTIC_UUID: UUID = UUID.fromString("00000005-19CA-4651-86E5-FA29DCDD09D1")
    // I have not seen this characteristic used. Guess it could be for Device Firmware Update (DFU)? it is a chip from Nordic.
    val ZWIFT_UNKNOWN_6_CHARACTERISTIC_UUID: UUID = UUID.fromString("00000006-19CA-4651-86E5-FA29DCDD09D1")
}

object GenericBleUuids {

    val GENERIC_ACCESS_SERVICE_UUID = uuidFromShortString("1800")
    val DEVICE_NAME_CHARACTERISTIC_UUID =  uuidFromShortString("2A00") // Zwift Play
    val APPEARANCE_CHARACTERISTIC_UUID =  uuidFromShortString("2A01") // [964] Gamepad (HID Subtype)
    val PREFERRED_CONNECTION_PARAMS_CHARACTERISTIC_UUID =  uuidFromShortString("2A04")
    val CENTRAL_ADDRESS_RESOLUTION_CHARACTERISTIC_UUID =  uuidFromShortString("2AA6")

    val GENERIC_ATTRIBUTE_SERVICE_UUID = uuidFromShortString("1801")
    val SERVICE_CHANGED_CHARACTERISTIC_UUID =  uuidFromShortString("2A05")

    val DEVICE_INFORMATION_SERVICE_UUID =  uuidFromShortString("180A")
    val MANUFACTURER_NAME_STRING_CHARACTERISTIC_UUID =  uuidFromShortString("2A29")  // Zwift Inc.
    val SERIAL_NUMBER_STRING_CHARACTERISTIC_UUID =  uuidFromShortString("2A25") // 02-1[MAC]
    val HARDWARE_REVISION_STRING_CHARACTERISTIC_UUID =  uuidFromShortString("2A27") // B.0
    val FIRMWARE_REVISION_STRING_CHARACTERISTIC_UUID =  uuidFromShortString("2A26") // 1.1.0

    val BATTERY_SERVICE_UUID = uuidFromShortString("180F")
    val BATTERY_LEVEL_CHARACTERISTIC_UUID = uuidFromShortString("2A19") // 89

    val DEFAULT_DESCRIPTOR_UUID = uuidFromShortString("2902")
}

