//
//  ConnectIQ.h
//  ConnectIQ
//
//  Copyright (c) 2014 Garmin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <ConnectIQ/IQConstants.h>
#import <ConnectIQ/IQDevice.h>
#import <ConnectIQ/IQApp.h>

// --------------------------------------------------------------------------------
#pragma mark - PUBLIC TYPES
// --------------------------------------------------------------------------------

/// @brief  SendMessage progress callback block
///
/// @param  sentBytes  The number of bytes that have been successfully transferred
///                    to the device so far for this connection.
/// @param  totalBytes The total number of bytes to transfer for this connection.
typedef void (^IQSendMessageProgress)(uint32_t sentBytes, uint32_t totalBytes);

/// @brief  SendMessage completion callback block
///
/// @param  result The result of the SendMessage operation.
typedef void (^IQSendMessageCompletion)(IQSendMessageResult result);

/// @brief  Conforming to the IQUIOverrideDelegate protocol indicates that an
///         object handles one or more events triggered by the ConnectIQ SDK that
///         require user input.
@protocol IQUIOverrideDelegate <NSObject>
@optional
/// @brief  Called by the ConnectIQ SDK when an action has been requested that
///         requires Garmin Connect Mobile to be installed.
///
///         The receiver should choose whether or not to launch the Apple App
///         Store page for GCM, ideally by presenting the user with a choice.
///
///         If the receiver of this message decides to install GCM, it must call
///         showAppStoreForConnectMobile.
- (void)needsToInstallConnectMobile;
@end

/// @brief  Conforming to the IQDeviceEventDelegate protocol indicates that an
///         object handles ConnectIQ device status events.
@protocol IQDeviceEventDelegate <NSObject>
@optional
/// @brief  Called by the ConnectIQ SDK when an IQDevice's connection status has
///         changed.
///
/// @param  device The IQDevice whose status changed.
/// @param  status The new status of the device.
- (void)deviceStatusChanged:(IQDevice *)device status:(IQDeviceStatus)status;
@end

/// @brief  Conforming to the IQAppMessageDelegate protocol indicates that an
///         object handles messages from ConnectIQ apps on compatible devices.
@protocol IQAppMessageDelegate <NSObject>
@optional
/// @brief  Called by the ConnectIQ SDK when a message is received from a device.
///
/// @param  message The message that was received.
/// @param  app     The device app that sent the message.
- (void)receivedMessage:(id)message fromApp:(IQApp *)app;
@end

// --------------------------------------------------------------------------------
#pragma mark - CLASS DEFINITION
// --------------------------------------------------------------------------------

/// @brief  The root of the ConnectIQ SDK API.
@interface ConnectIQ : NSObject

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

// --------------------------------------------------------------------------------
#pragma mark - SINGLETON ACCESS
// --------------------------------------------------------------------------------

/// @brief  Exposes the single static instance of the ConnectIQ class.
///
/// @return The single status instance of the ConnectIQ class.
+ (ConnectIQ *)sharedInstance;

// --------------------------------------------------------------------------------
#pragma mark - INITIALIZATION
// --------------------------------------------------------------------------------

/// @brief  Initializes the ConnectIQ SDK with startup parameters necessary for
///         its operation.
///
/// @param  urlScheme The URL scheme for this companion app. When Garmin Connect
///                   Mobile is launched, it will return to the companion app by
///                   launching a URL with this scheme.
/// @param  delegate  The delegate that the SDK will use for notifying the
///                   companion app about events that require user input. If this
///                   is nil, the SDK's default UI will be used.
- (void)initializeWithUrlScheme:(NSString *)urlScheme uiOverrideDelegate:(id<IQUIOverrideDelegate>)delegate;

// --------------------------------------------------------------------------------
#pragma mark - EXTERNAL LAUNCHING
// --------------------------------------------------------------------------------

/// @brief  Launches the Apple App Store page for the Garmin Connect Mobile app.
///         The companion app should only call this in response to a
///         needsToInstallConnectMobile event that gets triggered on the
///         IQUIOverrideDelegate.
- (void)showAppStoreForConnectMobile;

/// @brief  Launches Garmin Connect Mobile for the purpose of retrieving a list of
///         ConnectIQ-compatible devices.
///
///         Once the user has chosen which ConnectIQ devices to share with the
///         companion app, GCM will return those devices to the companion app by
///         opening a URL with the scheme registered in
///         initializeWithUrlScheme:uiOverrideDelegate:.
///
///         The companion app should handle this URL by passing it in to the
///         parseDeviceSelectionResponseFromURL: method to get the list of devices
///         that the user permitted the companion app to communicate with.
- (void)showConnectIQDeviceSelection;

/// @brief  Parses a URL opened from Garmin Connect Mobile into a list of devices.
///
/// @param  url The URL to parse.
///
/// @return An array of IQDevice objects representing the ConnectIQ-compatible
///         devices that the user allowed GCM to share with the companion app.
///
/// @seealso showConnectIQDeviceSelection
- (NSArray *)parseDeviceSelectionResponseFromURL:(NSURL *)url;

/// @brief  Launches Garmin Connect Mobile and shows the ConnectIQ app store page
///         for the given app.
///
///         The companion app should call this if the user would like to manage
///         the app on the device, such as to install, upgrade, uninstall, or
///         modify settings.
///
/// @param  app The app to show the ConnectIQ app store page for.
- (void)showConnectIQStoreForApp:(IQApp *)app;

// --------------------------------------------------------------------------------
#pragma mark - DEVICE MANAGEMENT
// --------------------------------------------------------------------------------

/// @brief  Registers an object as a listener for ConnectIQ device status events.
///
///         A device may have multiple device event listeners if this method is
///         called more than once.
///
/// @param  device   A device to listen for status events from.
/// @param  delegate The listener which will receive status events for this device.
- (void)registerForDeviceEvents:(IQDevice *)device delegate:(id<IQDeviceEventDelegate>)delegate;

/// @brief  Unregisters a listener for a specific device.
///
/// @param  device The device to unregister the listener for.
/// @param  delegate The listener to remove from the device.
- (void)unregisterForDeviceEvents:(IQDevice *)device delegate:(id<IQDeviceEventDelegate>)delegate;

/// @brief  Unregisters the specified listener from all devices for which it had
///         previously been registered.
///
/// @param  delegate The listener to unregister.
- (void)unregisterForAllDeviceEvents:(id<IQDeviceEventDelegate>)delegate;

/// @brief  Gets the current connection status of a device.
///
///         The device must have been registered for event notifications by
///         calling registerForDeviceEvents:delegate: or this method will return
///         IQDeviceStatus_InvalidDevice.
///
/// @param  device The device to get the status for.
///
/// @return The device's current connection status.
- (IQDeviceStatus)getDeviceStatus:(IQDevice *)device;

// --------------------------------------------------------------------------------
#pragma mark - APP MANAGEMENT
// --------------------------------------------------------------------------------

/// @brief  Begins getting the status of an app on a device. This method returns
///         immediately.
///
/// @param  app        The IQApp to get the status for.
/// @param  completion The completion block that will be triggered when the device
///                    status operation is complete.
- (void)getAppStatus:(IQApp *)app completion:(void(^)(IQAppStatus *appStatus))completion;

/// @brief  Registers an object as a listener for ConnectIQ messages from an app
///         on a device.
///
///         An app may have multiple message listeners if this method is called
///         more than once.
///
/// @param  app      The app to listen for messages from.
/// @param  delegate The listener which will receive messages for this app.
- (void)registerForAppMessages:(IQApp *)app delegate:(id<IQAppMessageDelegate>)delegate;

/// @brief  Unregisters a listener for a specific app.
///
/// @param  app      The app to unregister a listener for.
/// @param  delegate The listener to remove from the app.
- (void)unregisterForAppMessages:(IQApp *)app delegate:(id<IQAppMessageDelegate>)delegate;

/// @brief  Unregisters all previously registered apps for a specific listener.
///
/// @param  delegate The listener to unregister.
- (void)unregisterForAllAppMessages:(id<IQAppMessageDelegate>)delegate;

/// @brief  Begins sending a message to an app. This method returns immediately.
///
/// @param  message    The message to send to the app. This message must be one of
///                    the following types: NSString, NSNumber, NSNull, NSArray,
///                    or NSDictionary. Arrays and dictionaries may be nested.
/// @param  app        The app to send the message to.
/// @param  progress   A progress block that will be triggered periodically
///                    throughout the transfer. This is guaranteed to be triggered
///                    at least once.
/// @param  completion A completion block that will be triggered when the send
///                    message operation is complete.
- (void)sendMessage:(id)message toApp:(IQApp *)app progress:(IQSendMessageProgress)progress completion:(IQSendMessageCompletion)completion;

/// @brief  Sends an open app request message request to the device. This method returns immediately.
///
/// @param  app        The app to open.
/// @param  completion A completion block that will be triggered when the send
///                    message operation is complete.
- (void)openAppRequest:(IQApp *)app completion:(IQSendMessageCompletion)completion;

// TODO *** Holding off on documenting this until this method actually works.
- (void)sendImage:(NSData *)bitmap toApp:(IQApp *)app progress:(IQSendMessageProgress)progress completion:(IQSendMessageCompletion)completion;

@end
