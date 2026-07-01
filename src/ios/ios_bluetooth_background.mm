#ifndef IO_UNDER_QT
#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>
#import <objc/runtime.h>

static NSString * const QZCentralRestoreIdentifier = @"org.cagnulein.qdomyoszwift.qtcentral";

static BOOL QZShouldAddRestoreIdentifier(id delegate)
{
    if (!delegate) {
        return NO;
    }

    NSString *className = NSStringFromClass([delegate class]);
    return [className containsString:@"OSXBTCentralManager"];
}

@implementation CBCentralManager (QZBackgroundRestore)

+ (void)load
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        Method originalMethod = class_getInstanceMethod(self, @selector(initWithDelegate:queue:));
        Method swizzledMethod = class_getInstanceMethod(self, @selector(qz_initWithDelegate:queue:));
        if (originalMethod && swizzledMethod) {
            method_exchangeImplementations(originalMethod, swizzledMethod);
        }
    });
}

- (instancetype)qz_initWithDelegate:(id<CBCentralManagerDelegate>)delegate queue:(dispatch_queue_t)queue
{
    if (!QZShouldAddRestoreIdentifier(delegate)) {
        return [self qz_initWithDelegate:delegate queue:queue];
    }

    NSDictionary *options = @{ CBCentralManagerOptionRestoreIdentifierKey: QZCentralRestoreIdentifier };
    NSLog(@"QZ iOS BLE: enabling CoreBluetooth restore identifier %@ for %@",
          QZCentralRestoreIdentifier, NSStringFromClass([delegate class]));
    return [self initWithDelegate:delegate queue:queue options:options];
}

@end
#endif