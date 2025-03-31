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

// Safe method to check if an object is valid without risking a crash
- (bool)isObjectValid:(NSObject *)obj
{
    // First check for nil
    if (!obj) {
        qDebug() << "Error: Object is nil";
        return false;
    }
    
    @try {
        // Check if pointer is valid - this is safer than directly messaging the object
        void *rawPtr = (__bridge void *)obj;
        if (!rawPtr) {
            qDebug() << "Error: Invalid raw pointer";
            return false;
        }
        
        // Indirectly verify by checking class with runtime functions
        Class possibleClass = object_getClass(obj);
        if (!possibleClass) {
            qDebug() << "Error: Cannot get class of the object, probably deallocated";
            return false;
        }
        
        // Additional validation - verify that the class looks reasonable
        const char *className = class_getName(possibleClass);
        if (!className || strlen(className) == 0) {
            qDebug() << "Error: Invalid class name";
            return false;
        }
        
        // The object appears to be valid
        return true;
    }
    @catch (NSException *exception) {
        // If any exception occurs, the object is invalid
        qDebug() << "Exception when validating object:" << exception.reason;
        return false;
    }
}

- (bool)cacheWriteValue:(const QByteArray &)value for:(NSObject *)obj
{
    // Initial validity check using the safe method
    if (![self isObjectValid:obj]) {
        qDebug() << "Error: Invalid object passed to cacheWriteValue";
        return false;
    }
    
    @autoreleasepool {
        @try {
            // Safely access private variables through self
            QT_MANGLE_NAMESPACE(OSXBTCentralManager) *selfPtr = (QT_MANGLE_NAMESPACE(OSXBTCentralManager) *)self;
            if (![self isObjectValid:selfPtr]) {
                qDebug() << "Error: Invalid self pointer";
                return false;
            }
            
            OSXBluetooth::CharHash &localCharMap = selfPtr->charMap;
            OSXBluetooth::DescHash &localDescMap = selfPtr->descMap;
            OSXBluetooth::ValueHash &localValuesToWrite = selfPtr->valuesToWrite;
            
            // Safely verify object type
            BOOL isCharacteristic = NO;
            BOOL isDescriptor = NO;
            
            // Get class using the safe runtime method
            Class objClass = object_getClass(obj);
            
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
            
            // Verify based on identified type using the hash tables
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
