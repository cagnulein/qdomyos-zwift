#import "osxbtcentralmanager_p.h"  // header del modulo originale
#import <objc/runtime.h>

@implementation QT_MANGLE_NAMESPACE(OSXBTCentralManager) (SafeCache)

- (bool)cacheWriteValue:(const QByteArray &)value for:(NSObject *)obj
{
    @try {
        // Verifica validità oggetto
        if (!obj) {
            qCWarning(QT_BT_OSX) << "Error: Invalid object (nil)";
            return false;
        }
        
        // Verifica tipo oggetto
        if ([obj isKindOfClass:[CBCharacteristic class]]) {
            @try {
                CBCharacteristic *const ch = (CBCharacteristic *)obj;
                if (!charMap.key(ch)) {
                    qCWarning(QT_BT_OSX) << "Error: Unexpected characteristic, no handle found";
                    return false;
                }
            } @catch (NSException *e) {
                qCWarning(QT_BT_OSX) << "Exception handling characteristic:" << e.reason;
                return false;
            }
        } else if ([obj isKindOfClass:[CBDescriptor class]]) {
            @try {
                CBDescriptor *const d = (CBDescriptor *)obj;
                if (!descMap.key(d)) {
                    qCWarning(QT_BT_OSX) << "Error: Unexpected descriptor, no handle found";
                    return false;
                }
            } @catch (NSException *e) {
                qCWarning(QT_BT_OSX) << "Exception handling descriptor:" << e.reason;
                return false;
            }
        } else {
            qCWarning(QT_BT_OSX) << "Error: Invalid object type, characteristic or descriptor required";
            return false;
        }

        // Gestione cache esistente
        @try {
            if (valuesToWrite.contains(obj)) {
                qCWarning(QT_BT_OSX) << "Warning: Already has a cached value for this object, the value will be replaced";
            }
            
            valuesToWrite[obj] = value;
        } @catch (NSException *e) {
            qCWarning(QT_BT_OSX) << "Exception during cache operation:" << e.reason;
            return false;
        }
        
        return true;
        
    } @catch (NSException *e) {
        qCWarning(QT_BT_OSX) << "Unexpected exception in cacheWriteValue:" << e.reason;
        return false;
    }
}

@end