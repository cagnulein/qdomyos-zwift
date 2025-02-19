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
    @try {
        // Accesso alle variabili private tramite self
        OSXBluetooth::CharHash &localCharMap = ((QT_MANGLE_NAMESPACE(OSXBTCentralManager) *)self)->charMap;
        OSXBluetooth::DescHash &localDescMap = ((QT_MANGLE_NAMESPACE(OSXBTCentralManager) *)self)->descMap;
        OSXBluetooth::ValueHash &localValuesToWrite = ((QT_MANGLE_NAMESPACE(OSXBTCentralManager) *)self)->valuesToWrite;

        // Verifica validità oggetto
        if (!obj) {
            qDebug() << "Error: Invalid object (nil)";
            return false;
        }
        
        // Verifica tipo oggetto
        if ([obj isKindOfClass:[CBCharacteristic class]]) {
            @try {
                CBCharacteristic *const ch = (CBCharacteristic *)obj;
                if (!localCharMap.key(ch)) {
                    qDebug() << "Error: Unexpected characteristic, no handle found";
                    return false;
                }
            } @catch (NSException *e) {
                qDebug() << "Exception handling characteristic:" << e.reason;
                return false;
            }
        } else if ([obj isKindOfClass:[CBDescriptor class]]) {
            @try {
                CBDescriptor *const d = (CBDescriptor *)obj;
                if (!localDescMap.key(d)) {
                    qDebug() << "Error: Unexpected descriptor, no handle found";
                    return false;
                }
            } @catch (NSException *e) {
                qDebug() << "Exception handling descriptor:" << e.reason;
                return false;
            }
        } else {
            qDebug() << "Error: Invalid object type, characteristic or descriptor required";
            return false;
        }

        // Gestione cache esistente
        @try {
            if (localValuesToWrite.contains(obj)) {
                qDebug() << "Warning: Already has a cached value for this object, the value will be replaced";
            }
            
            localValuesToWrite[obj] = value;
        } @catch (NSException *e) {
            qDebug() << "Exception during cache operation:" << e.reason;
            return false;
        }
        
        return true;
        
    } @catch (NSException *e) {
        qDebug() << "Unexpected exception in cacheWriteValue:" << e.reason;
        return false;
    }
}

@end
