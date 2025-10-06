//
//  swiftDebug.m
//  qdomyoszwift
//
//  Created by Roberto Viola on 14/12/23.
//
#import "swiftDebug.h"
#import "swiftDebugBridge.h"
#import "homeform.h"
#import "bike.h"
#include <QDebug>
#include <stdarg.h>
#include <dispatch/dispatch.h>

@implementation swiftDebug

- (void)qtDebug:(NSString *)inputString {
    qDebug() << inputString;
}

- (void)gearUp {
    ((bike*)(homeform::singleton()->bluetoothManager->device()))->gearUp();
}

- (void)gearDown {
    ((bike*)(homeform::singleton()->bluetoothManager->device()))->gearDown();
}

@end

static swiftDebug *SwiftDebugSharedLogger()
{
    static swiftDebug *logger = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        logger = [[swiftDebug alloc] init];
    });
    return logger;
}

static NSString *SwiftDebugBridgeStringFromCString(const char *cString)
{
    if (!cString)
    {
        return nil;
    }
    NSString *message = [[NSString alloc] initWithCString:cString encoding:NSUTF8StringEncoding];
    if (!message)
    {
        message = [NSString stringWithFormat:@"(invalid UTF8) %s", cString];
    }
    return message;
}

void SwiftDebugLogCString(const char *message)
{
    NSString *string = SwiftDebugBridgeStringFromCString(message);
    if (!string)
    {
        return;
    }
    SwiftDebugLogNSString(string);
}

void SwiftDebugLogNSString(NSString *message)
{
    if (!message || [message length] == 0)
    {
        return;
    }
    [SwiftDebugSharedLogger() qtDebug:message];
}

void SwiftDebugLogFormatV(const char *format, va_list args)
{
    if (!format)
    {
        return;
    }

    va_list argsCopy;
    va_copy(argsCopy, args);
    NSString *formatString = SwiftDebugBridgeStringFromCString(format);
    if (!formatString)
    {
        formatString = @"(null format)";
    }
    NSString *message = [[NSString alloc] initWithFormat:formatString arguments:argsCopy];
    va_end(argsCopy);

    SwiftDebugLogNSString(message);
}

void SwiftDebugLogFormat(const char *format, ...)
{
    if (!format)
    {
        return;
    }

    va_list args;
    va_start(args, format);
    SwiftDebugLogFormatV(format, args);
    va_end(args);
}
