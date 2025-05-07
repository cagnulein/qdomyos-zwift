#import <CoreBluetooth/CoreBluetooth.h>
#import "ios_echelonconnectsport.h"

@implementation ios_echelonconnectsport

- (instancetype)init:(NSString *)deviceName qtDevice:(void*)qtDevice {
    self = [super init];
    if (self) {
        _targetDeviceName = [deviceName copy];
        qDebug() << _targetDeviceName;
        _qtDevice = (echelonconnectsport*)qtDevice;
        _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];

        // Register for application lifecycle notifications
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(applicationWillTerminate:)
                                                     name:UIApplicationWillTerminateNotification
                                                   object:nil];
    }
    return self;
}

// Add dealloc method to clean up resources
- (void)dealloc {
    // Remove notification observers
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    // Disconnect if still connected
    [self disconnectPeripheral];
}

// Handle app termination
- (void)applicationWillTerminate:(NSNotification *)notification {
    qDebug() << "Application will terminate - disconnecting peripheral";
    [self disconnectPeripheral];
}


- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    if (central.state == CBManagerStatePoweredOn) {
        qDebug() << "centralManagerDidUpdateState" << central.state;
        [self.centralManager scanForPeripheralsWithServices:nil options:nil];
    }
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary<NSString *,id> *)advertisementData RSSI:(NSNumber *)RSSI {
    if (peripheral && _targetDeviceName && peripheral.name) {
        qDebug() << _targetDeviceName;
        if ([peripheral.name isEqualToString:_targetDeviceName]) {
            self.connectedPeripheral = peripheral;
            [self.centralManager stopScan];
            [self.centralManager connectPeripheral:peripheral options:nil];
            qDebug() << "didDiscoverPeripheral";
        }
    }
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
    peripheral.delegate = self;
    [peripheral discoverServices:nil];
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error {
    qDebug() << "Peripheral disconnected:" << peripheral << error;
    if ([peripheral.name isEqualToString:self.targetDeviceName]) {
        _qtDevice->controllerStateChanged(QLowEnergyController::UnconnectedState);
        [self.centralManager connectPeripheral:peripheral options:nil];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error {
    for (CBService *service in peripheral.services) {
        [peripheral discoverCharacteristics:nil forService:service];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error {
    if (characteristic.value) {
        qDebug() << "didUpdateValueForCharacteristic" << characteristic;
        // Extract the data from characteristic.value and process it as needed
        NSData *receivedData = characteristic.value;
        NSLog(@"UUID: %@ Received data: %@", characteristic.UUID, receivedData);
        // Your processing logic here
        //[self.qtDevice characteristicChanged:characteristic.UUID data:receivedData];
        QLowEnergyCharacteristic c;
        QByteArray b;
        const uint8_t* d = (const uint8_t*)[receivedData bytes];
        for(int i=0; i<receivedData.length; i++) {
            b.append(d[i]);
        }
        self.qtDevice->characteristicChanged(c, b);
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error {
    CBUUID *write = [CBUUID UUIDWithString:@"0bf669f2-45f2-11e7-9598-0800200c9a66"];
    CBUUID *uuid1 = [CBUUID UUIDWithString:@"0bf669f3-45f2-11e7-9598-0800200c9a66"];
    CBUUID *uuid2 = [CBUUID UUIDWithString:@"0bf669f4-45f2-11e7-9598-0800200c9a66"];

    for (CBCharacteristic *characteristic in service.characteristics) {
        if ([characteristic.UUID isEqual:uuid1]) {
            self.gattNotify1Characteristic = characteristic;
            [peripheral setNotifyValue:YES forCharacteristic:self.gattNotify1Characteristic];
        } else if ([characteristic.UUID isEqual:uuid2]) {
            self.gattNotify2Characteristic = characteristic;
            [peripheral setNotifyValue:YES forCharacteristic:self.gattNotify2Characteristic];
        } else if ([characteristic.UUID isEqual:write]) {
            self.gattWriteCharacteristic = characteristic;
        }
    }

    // Verifica se entrambe le caratteristiche sono state trovate
    if (self.gattNotify1Characteristic && self.gattNotify2Characteristic && self.gattWriteCharacteristic) {
        self.qtDevice->stateChanged(QLowEnergyService::ServiceDiscovered);
        self.qtDevice->descriptorWritten(QLowEnergyDescriptor(), QByteArray());
    }
}

- (void)writeCharacteristc:(uint8_t *)data length:(NSUInteger)length {
    if (self.connectedPeripheral.state != CBPeripheralStateConnected) {
        qDebug() << "Cannot send. Peripheral is not connected.";
        return;
    }
        
    NSData *dataToSend = [NSData dataWithBytes:data length:length];
    
    qDebug() << "writeCharacteristc" << dataToSend;
    
    if (self.gattWriteCharacteristic) {
        [self.connectedPeripheral writeValue:dataToSend forCharacteristic:self.gattWriteCharacteristic type:CBCharacteristicWriteWithResponse];
    }
}

@end
