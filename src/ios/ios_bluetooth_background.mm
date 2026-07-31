#ifndef IO_UNDER_QT
#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>
#import <objc/runtime.h>

// Qt creates its CBCentralManager with -initWithDelegate:queue:, so there is no
// way to ask for CoreBluetooth state preservation/restoration through the Qt
// API.  Without a restore identifier iOS drops every central (and every pending
// connection) as soon as the app is terminated in background, and it never
// relaunches us to hand the peripherals back.
//
// Swizzling the designated initializer lets us add the restore identifier
// without rebuilding QtBluetooth.  We only do it for the Qt owned central
// manager: any other CBCentralManager in the process (or inside a third party
// framework) keeps the standard behaviour.

static NSString * const QZCentralRestoreIdentifier = @"org.cagnulein.qdomyoszwift.qtcentral";

static BOOL QZShouldAddRestoreIdentifier(id delegate)
{
    if (!delegate) {
        return NO;
    }

    NSString *className = NSStringFromClass([delegate class]);
    return [className containsString:@"OSXBTCentralManager"];
}

// CoreBluetooth expects the delegate to implement centralManager:willRestoreState:
// when a restore identifier is used.  Qt's central manager does not, so install a
// logging-only implementation at runtime: Qt rediscovers and reconnects the
// peripherals by itself, we just must not leave the selector missing.
static void QZInstallWillRestoreState(id delegate)
{
    Class delegateClass = [delegate class];
    if ([delegateClass instancesRespondToSelector:@selector(centralManager:willRestoreState:)]) {
        return;
    }

    IMP implementation = imp_implementationWithBlock(^(id _self, CBCentralManager *central, NSDictionary *state) {
        NSArray *peripherals = state[CBCentralManagerRestoredStatePeripheralsKey];
        NSLog(@"QZ iOS BLE: CoreBluetooth restored %lu peripheral(s) on %@",
              (unsigned long)peripherals.count, central);
    });

    class_addMethod(delegateClass, @selector(centralManager:willRestoreState:), implementation, "v@:@@");
}

@interface CBCentralManager (QZBackgroundRestore)
- (instancetype)qz_initWithDelegate:(id<CBCentralManagerDelegate>)delegate queue:(dispatch_queue_t)queue;
@end

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
        // after the swizzle this selector points to the original implementation
        return [self qz_initWithDelegate:delegate queue:queue];
    }

    QZInstallWillRestoreState(delegate);

    NSDictionary *options = @{ CBCentralManagerOptionRestoreIdentifierKey: QZCentralRestoreIdentifier };
    NSLog(@"QZ iOS BLE: enabling CoreBluetooth restore identifier %@ for %@",
          QZCentralRestoreIdentifier, NSStringFromClass([delegate class]));
    return [self initWithDelegate:delegate queue:queue options:options];
}

@end
#endif
