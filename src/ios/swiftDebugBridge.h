#ifndef SWIFT_DEBUG_BRIDGE_H
#define SWIFT_DEBUG_BRIDGE_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void SwiftDebugLogCString(const char *message);
void SwiftDebugLogFormat(const char *format, ...);
void SwiftDebugLogFormatV(const char *format, va_list args);

#ifdef __OBJC__
@class NSString;
void SwiftDebugLogNSString(NSString *message);
#endif

#ifdef __cplusplus
}
#endif

#endif /* SWIFT_DEBUG_BRIDGE_H */
