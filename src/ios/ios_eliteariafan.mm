#import <CoreBluetooth/CoreBluetooth.h>
#import "ios_eliteariafan.h"

@implementation ios_eliteariafan

- (instancetype)init {
    self = [super init];
    if (self) {
        _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
        _targetDeviceName = @"ARIA";  // Nome del dispositivo Bluetooth
    }
    return self;
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    if (central.state == CBManagerStatePoweredOn) {
        [self.centralManager scanForPeripheralsWithServices:nil options:nil];
    }
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary<NSString *,id> *)advertisementData RSSI:(NSNumber *)RSSI {
    if ([peripheral.name isEqualToString:self.targetDeviceName]) {
        self.connectedPeripheral = peripheral;
        [self.centralManager stopScan];
        [self.centralManager connectPeripheral:peripheral options:nil];
    }
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
    peripheral.delegate = self;
    [peripheral discoverServices:nil];
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error {
    for (CBService *service in peripheral.services) {
        [peripheral discoverCharacteristics:nil forService:service];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error {
    CBUUID *uuid1 = [CBUUID UUIDWithString:@"347b0012-7635-408b-8918-8ff3949ce592"];
    CBUUID *uuid2 = [CBUUID UUIDWithString:@"347b0040-7635-408b-8918-8ff3949ce592"];

    for (CBCharacteristic *characteristic in service.characteristics) {
        if ([characteristic.UUID isEqual:uuid1]) {
            self.characteristicUUID1 = characteristic;
        } else if ([characteristic.UUID isEqual:uuid2]) {
            self.characteristicUUID2 = characteristic;
        }
    }

    // Verifica se entrambe le caratteristiche sono state trovate
    if (self.characteristicUUID1 && self.characteristicUUID2) {
        // Invia init1
        uint8_t init1[] = {0x02, 0x00, 0x00, 0x3d, 0x00};
        NSData *dataToSend1 = [NSData dataWithBytes:init1 length:sizeof(init1)];
        [peripheral writeValue:dataToSend1 forCharacteristic:self.characteristicUUID1 type:CBCharacteristicWriteWithResponse];

        // Invia init2
        uint8_t init2[] = {0x05, 0x00};
        NSData *dataToSend2 = [NSData dataWithBytes:init2 length:sizeof(init2)];
        [peripheral writeValue:dataToSend2 forCharacteristic:self.characteristicUUID2 type:CBCharacteristicWriteWithResponse];
        
        [self fanSpeedRequest:0];
    }
}

- (void)fanSpeedRequest:(uint8_t)speed {
    uint8_t init10[] = {0x03, 0x01, 0x0e};
    init10[2] = speed;
    NSData *dataToSend = [NSData dataWithBytes:init10 length:sizeof(init10)];
    
    if (self.characteristicUUID2) {
        [self.connectedPeripheral writeValue:dataToSend forCharacteristic:self.characteristicUUID2 type:CBCharacteristicWriteWithResponse];
    }
}

@end
