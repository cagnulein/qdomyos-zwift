//
//  IQApp.h
//  ConnectIQ
//
//  Copyright (c) 2014 Garmin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <ConnectIQ/IQDevice.h>
#import <ConnectIQ/IQAppStatus.h>

/// @brief  Represents an instance of a ConnectIQ app that is installed on a
///         Garmin device.
@interface IQApp : NSObject <NSSecureCoding>

/// @brief  The unique identifier for this app.
@property (nonatomic, readonly) NSUUID *uuid;

/// @brief  The unique identifier for this app in the store.
@property (nonatomic, readonly) NSUUID *storeUuid;

/// @brief  The device that this app is installed on.
@property (nonatomic, readonly) IQDevice *device;

/// @brief  Creates a new app instance.
///
/// @param  uuid        The UUID of the app to create.
/// @param  storeUuid   The store UUID of the app to create.
/// @param  device      The device the app to create is installed on.
///
/// @return A new IQApp instance with the appropriate values set.
+ (IQApp *)appWithUUID:(NSUUID *)uuid storeUuid:(NSUUID *)storeUuid device:(IQDevice *)device;

@end
