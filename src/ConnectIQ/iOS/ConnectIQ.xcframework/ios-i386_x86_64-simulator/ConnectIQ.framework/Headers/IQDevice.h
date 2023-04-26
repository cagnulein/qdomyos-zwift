//
//  IQDevice.h
//  ConnectIQ
//
//  Copyright (c) 2014 Garmin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

/// @brief  The current status of an IQDevice.
typedef NS_ENUM(NSInteger, IQDeviceStatus){
    /// @brief  No device with this UUID has been registered for status events
    ///         the SDK.
    IQDeviceStatus_InvalidDevice,

    /// @brief  Bluetooth is either powered off or resetting.
    IQDeviceStatus_BluetoothNotReady,

    /// @brief  This device could not be found by iOS. Perhaps the user removed
    ///         the device?
    IQDeviceStatus_NotFound,

    /// @brief  The device is recognized by iOS, but it is not currently
    ///         connected.
    IQDeviceStatus_NotConnected,

    /// @brief  The device is connected and ready to communicate.
    IQDeviceStatus_Connected,
};

/// @brief  Represents a ConnectIQ-compatible Garmin device.
@interface IQDevice : NSObject <NSSecureCoding>

/// @brief  The unique identifier for this device.
@property (nonatomic, readonly) NSUUID *uuid;

/// @brief  The model name of the device provided by Garmin Connect Mobile.
@property (nonatomic, readonly) NSString *modelName;

/// @brief  The friendly name of the device, set by the user and provided by
///         Garmin Connect Mobile.
@property (nonatomic, readonly) NSString *friendlyName;

/// @brief  Creates a new device instance.
///
/// @param  uuid         The UUID of the device to create.
/// @param  modelName    The model name of the device to create.
/// @param  friendlyName The friendly name of the device to create.
///
/// @return A new IQDevice instance with the appropriate values set.
+ (IQDevice *)deviceWithId:(NSUUID *)uuid modelName:(NSString *)modelName friendlyName:(NSString *)friendlyName;

/// @brief  Creates a new device instance by copying another device's values.
///
/// @param  device The device to copy values from.
///
/// @return A new IQDevice instance with all values copied.
- (instancetype)initWithDevice:(IQDevice *)device;

@end
