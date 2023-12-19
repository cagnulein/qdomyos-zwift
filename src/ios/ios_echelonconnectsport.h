#ifndef IOSELITEARIAFAN_H
#define IOSELITEARIAFAN_H

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

@interface ios_echelonconnectsport : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>

@property (strong, nonatomic) CBCentralManager *centralManager;
@property (strong, nonatomic) CBPeripheral *connectedPeripheral;
@property (strong, nonatomic) NSString *targetDeviceName;
@property (strong, nonatomic) CBCharacteristic *gattWriteCharacteristic;
@property (strong, nonatomic) CBCharacteristic *gattNotify1Characteristic;
@property (strong, nonatomic) CBCharacteristic *gattNotify2Characteristic;

- (instancetype)init:(NSString *)deviceName;
- (void)writeCharacteristc:(uint8_t *)data length:(NSUInteger)length;

@end

#endif // IOSELITEARIAFAN_H
