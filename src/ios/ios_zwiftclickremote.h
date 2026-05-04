#ifndef IOSZWIFTCLICKREMOTE_H
#define IOSZWIFTCLICKREMOTE_H

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import "zwiftclickremote.h"

@interface ios_zwiftclickremote : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>

@property (strong, nonatomic) CBCentralManager *centralManager;
@property (strong, nonatomic) CBPeripheral *connectedPeripheral;
@property (strong, nonatomic) NSString *targetDeviceName;
@property (strong, nonatomic) NSString *targetDeviceUUID;
@property zwiftclickremote *qtDevice;
@property (strong, nonatomic) CBCharacteristic *gattWrite1Characteristic;
@property (strong, nonatomic) NSMutableArray<CBService *> *discoveredServices;
@property (strong, nonatomic) NSMutableDictionary<CBUUID *, CBCharacteristic *> *discoveredCharacteristics;

- (instancetype)init:(NSString *)deviceName qtDevice:(void*)qtDevice;
- (instancetype)initWithNameAndUUID:(NSString *)deviceName uuid:(NSString *)uuid qtDevice:(void*)qtDevice;
- (void)writeCharacteristic:(uint8_t *)data length:(NSUInteger)length;

@end
#endif // IOSZWIFTCLICKREMOTE_H
