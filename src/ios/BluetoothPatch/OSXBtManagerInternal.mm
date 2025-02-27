#define QT_IOS_BLUETOOTH
#import "osxbtcentralmanager_p.h"
#import <objc/runtime.h>

QT_USE_NAMESPACE

// Definizione della struttura dei membri privati
@interface QT_MANGLE_NAMESPACE(OSXBTCentralManager) () {
@package  // Rende i membri accessibili alla categoria
    OSXBluetooth::CharHash charMap;
    OSXBluetooth::DescHash descMap;
    OSXBluetooth::ValueHash valuesToWrite;
}
@end

@implementation QT_MANGLE_NAMESPACE(OSXBTCentralManager) (SafeCache)

- (bool)cacheWriteValue:(const QByteArray &)value for:(NSObject *)obj
{
    // Initial check for null object
    if (!obj) {
        qDebug() << "Error: Invalid object (nil)";
        return false;
    }
    
    @autoreleasepool {
        // Safely verify that the object is valid without calling methods on it
        Class objClass = [obj class];
        if (!objClass) {
            qDebug() << "Error: Cannot get class of the object, probably deallocated";
            return false;
        }
        
        // Safe access to private variables
        @try {
            // Safely access private variables through self
            QT_MANGLE_NAMESPACE(OSXBTCentralManager) *selfPtr = (QT_MANGLE_NAMESPACE(OSXBTCentralManager) *)self;
            if (!selfPtr) {
                qDebug() << "Error: Invalid self pointer";
                return false;
            }
            
            OSXBluetooth::CharHash &localCharMap = selfPtr->charMap;
            OSXBluetooth::DescHash &localDescMap = selfPtr->descMap;
            OSXBluetooth::ValueHash &localValuesToWrite = selfPtr->valuesToWrite;
            
            // Safely verify object type
            BOOL isCharacteristic = NO;
            BOOL isDescriptor = NO;
            
            // Test with standard CoreBluetooth classes
            const char *className = class_getName(objClass);
            if (className) {
                isCharacteristic = strcmp(className, "CBCharacteristic") == 0;
                isDescriptor = strcmp(className, "CBDescriptor") == 0;
                
                // Also check for subclasses
                if (!isCharacteristic && !isDescriptor) {
                    Class charClass = NSClassFromString(@"CBCharacteristic");
                    Class descClass = NSClassFromString(@"CBDescriptor");
                    
                    if (charClass && class_conformsToProtocol(objClass, @protocol(NSObject))) {
                        isCharacteristic = class_getSuperclass(objClass) == charClass;
                    }
                    
                    if (descClass && class_conformsToProtocol(objClass, @protocol(NSObject))) {
                        isDescriptor = class_getSuperclass(objClass) == descClass;
                    }
                }
            }
            
            // Verify based on identified type
            if (isCharacteristic) {
                CBCharacteristic *ch = (CBCharacteristic *)obj;
                // Verify using only C functions to avoid Objective-C messages if possible
                if (!localCharMap.key(ch)) {
                    qDebug() << "Error: Unexpected characteristic, no handle found";
                    return false;
                }
            } else if (isDescriptor) {
                CBDescriptor *d = (CBDescriptor *)obj;
                if (!localDescMap.key(d)) {
                    qDebug() << "Error: Unexpected descriptor, no handle found";
                    return false;
                }
            } else {
                qDebug() << "Error: Object is neither a characteristic nor a descriptor";
                return false;
            }
            
            // Existing cache management with protection
            if (localValuesToWrite.contains(obj)) {
                qDebug() << "Warning: Already has a cached value for this object, the value will be replaced";
            }
            
            // Safe assignment
            try {
                localValuesToWrite[obj] = value;
            } catch (const std::exception& e) {
                qDebug() << "C++ exception during cache operation:" << e.what();
                return false;
            } catch (...) {
                qDebug() << "Unknown C++ exception during cache operation";
                return false;
            }
            
            return true;
        } @catch (NSException *e) {
            qDebug() << "Objective-C exception in cacheWriteValue:" << e.reason;
            return false;
        }
    }
    
    // This point should never be reached due to previous returns
    return false;
}

@end
