#ifndef IOSELITEARIAFAN_H
#define IOSELITEARIAFAN_H

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

@interface iOS_EliteAriaFan : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>

@property (strong, nonatomic) CBCentralManager *centralManager;
@property (strong, nonatomic) CBPeripheral *connectedPeripheral;
@property (strong, nonatomic) NSString *targetDeviceName;
@property (strong, nonatomic) CBCharacteristic *characteristicUUID1;
@property (strong, nonatomic) CBCharacteristic *characteristicUUID2;

- (instancetype)init;
- (void)fanSpeedRequest:(uint8_t)speed;

@end

#endif // IOSELITEARIAFAN_H
