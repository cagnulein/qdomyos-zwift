//
//  IQConstants.h
//  ConnectIQ
//
//  Copyright (c) 2014 Garmin. All rights reserved.
//

#import <Foundation/Foundation.h>

/// @brief  The current version of the ConnectIQ SDK.
extern int const IQSDKVersion;

/// @brief  The bundle identifier for the Garmin Connect Mobile app.
extern NSString * const IQGCMBundle;

/// @brief  The bundle identifier for the Garmin Connect Mobile Beta app.
extern NSString * const IQGCMInternalBetaBundle;

/// @brief  The result of a SendMessage operation
typedef NS_ENUM(NSInteger, IQSendMessageResult){
    ///! @brief  The message was sent successfully.
    IQSendMessageResult_Success,

    /// @brief  The message failed to send due to an unknown error.
    IQSendMessageResult_Failure_Unknown,

    /// @brief  The message failed to send. There was an error within the SDK or
    ///         on the device.
    IQSendMessageResult_Failure_InternalError,

    /// @brief  The message failed to send. The device is not available right now.
    IQSendMessageResult_Failure_DeviceNotAvailable,

    /// @brief  The message failed to send. The app is not installed on the
    ///         device.
    IQSendMessageResult_Failure_AppNotFound,

    /// @brief  The message failed to send. The device is busy and cannot receive
    ///         the message right now.
    IQSendMessageResult_Failure_DeviceIsBusy,

    /// @brief  The message failed to send. The message contained an unsupported
    ///         type.
    IQSendMessageResult_Failure_UnsupportedType,

    /// @brief  The message failed to send. The device does not have enough memory
    ///         to receive the message.
    IQSendMessageResult_Failure_InsufficientMemory,

    /// @brief  The message failed to send. The connection timed out while sending
    ///         the message.
    IQSendMessageResult_Failure_Timeout,

    /// @brief  The message failed to send and was retried, but could not complete
    ///         after a number of tries.
    IQSendMessageResult_Failure_MaxRetries,

    /// @brief  The message was received by the device but it chose not to display
    ///         a message prompt, ignoring the message.
    IQSendMessageResult_Failure_PromptNotDisplayed,

    /// @brief  The message was received by the device but the app to open
    ///         was already running on the device.
    IQSendMessageResult_Failure_AppAlreadyRunning,
};
NSString *NSStringFromSendMessageResult(IQSendMessageResult value);
