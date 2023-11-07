//
//  IQAppStatus.h
//  ConnectIQ
//
//  Copyright (c) 2014 Garmin. All rights reserved.
//

#import <Foundation/Foundation.h>

/// @brief  Represents the current status of an app on a Garmin device.
@interface IQAppStatus : NSObject

/// @brief  YES if the app is installed on the device, NO if it isn't.
@property (nonatomic, readonly) BOOL isInstalled;

/// @brief  The version of the app that is currently installed on the device. If
///         the app is not installed, this value is unused.
@property (nonatomic, readonly) uint16_t version;

@end
