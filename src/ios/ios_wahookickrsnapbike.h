#ifndef IOSWAHOOKICKRSNAPBIKE_H
#define IOSWAHOOKICKRSNAPBIKE_H

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import "wahookickrsnapbike.h"

@interface ios_wahookickrsnapbike : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>

@property (strong, nonatomic) CBCentralManager *centralManager;
@property (strong, nonatomic) CBPeripheral *connectedPeripheral;
@property (strong, nonatomic) NSString *targetDeviceName;
@property wahookickrsnapbike *qtDevice;
@property (strong, nonatomic) CBCharacteristic *gattWriteCharacteristic;
@property (strong, nonatomic) NSMutableArray<CBService *> *discoveredServices;
@property (strong, nonatomic) NSMutableDictionary<CBUUID *, CBCharacteristic *> *discoveredCharacteristics;

- (instancetype)init:(NSString *)deviceName qtDevice:(void*)qtDevice;
- (void)writeCharacteristic:(uint8_t *)data length:(NSUInteger)length;

@end

#endif // IOSWAHOOKICKRSNAPBIKE_H
