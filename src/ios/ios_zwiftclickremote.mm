#import <CoreBluetooth/CoreBluetooth.h>
#import "ios_zwiftclickremote.h"

@implementation ios_zwiftclickremote

- (instancetype)initWithNameAndUUID:(NSString *)deviceName uuid:(NSString *)uuid qtDevice:(void*)qtDevice {
    self = [super init];
    if (self) {
        _targetDeviceName = [deviceName copy];
        _targetDeviceUUID = [uuid copy];
        qDebug() << _targetDeviceName << "UUID:" << _targetDeviceUUID;
        _qtDevice = (zwiftclickremote*)qtDevice;
        _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
        _discoveredServices = [[NSMutableArray alloc] init];
        _discoveredCharacteristics = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (instancetype)init:(NSString *)deviceName qtDevice:(void*)qtDevice {
    self = [super init];
    if (self) {
        _targetDeviceName = [deviceName copy];
        qDebug() << _targetDeviceName;
        _qtDevice = (zwiftclickremote*)qtDevice;
        _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
        _discoveredServices = [[NSMutableArray alloc] init];
        _discoveredCharacteristics = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    if (central.state == CBManagerStatePoweredOn) {
        qDebug() << "centralManagerDidUpdateState" << central.state;
        [self.centralManager scanForPeripheralsWithServices:nil options:nil];
    }
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary<NSString *,id> *)advertisementData RSSI:(NSNumber *)RSSI {
    if (peripheral) {
        // If we have a UUID, try to match against it
        if (_targetDeviceUUID && peripheral.identifier) {
            NSString *peripheralUUID = peripheral.identifier.UUIDString;
            if ([peripheralUUID isEqualToString:_targetDeviceUUID]) {
                self.connectedPeripheral = peripheral;
                [self.centralManager stopScan];
                [self.centralManager connectPeripheral:peripheral options:nil];
                qDebug() << "Connected to device by UUID";
                return;
            }
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
    [_discoveredServices removeAllObjects];
    for (CBService *service in peripheral.services) {
        [peripheral discoverCharacteristics:nil forService:service];
        [_discoveredServices addObject:service];
    }
    
    // Chiamiamo serviceScanDone() per continuare con la logica C++
    _qtDevice->serviceScanDone();
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error {
    if (characteristic.value) {
        qDebug() << "didUpdateValueForCharacteristic" << characteristic;
        // Extract the data from characteristic.value and process it as needed
        NSData *receivedData = characteristic.value;
        NSLog(@"UUID: %@ Received data: %@", characteristic.UUID, receivedData);
        
        // Converti l'UUID CoreBluetooth in QBluetoothUuid
        QString uuidString = QString::fromNSString([characteristic.UUID UUIDString]);
        QBluetoothUuid uuid(uuidString);
        
        // Converti i dati da NSData a QByteArray
        QByteArray b;
        const uint8_t* d = (const uint8_t*)[receivedData bytes];
        for(int i=0; i<receivedData.length; i++) {
            b.append(d[i]);
        }
        
        // Passa UUID e dati al wrapper
        self.qtDevice->handleCharacteristicValueChanged(uuid, b);
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error {
    CBUUID *syncRxUUID = [CBUUID UUIDWithString:@"00000003-19CA-4651-86E5-FA29DCDD09D1"];
    
    int notifyCharacteristicsFound = 0;
    int totalCharacteristics = 0;
    
    for (CBCharacteristic *characteristic in service.characteristics) {
        totalCharacteristics++;
        [_discoveredCharacteristics setObject:characteristic forKey:characteristic.UUID];
        
        if ([characteristic.UUID isEqual:syncRxUUID]) {
            self.gattWrite1Characteristic = characteristic;
            qDebug() << "Found Zwift Play write characteristic";
        }
        
        // Set notifications for characteristics that support it
        if (characteristic.properties & CBCharacteristicPropertyNotify) {
            [peripheral setNotifyValue:YES forCharacteristic:characteristic];
            notifyCharacteristicsFound++;
            qDebug() << "Subscribing to notification for characteristic:" << characteristic.UUID;
        } else if (characteristic.properties & CBCharacteristicPropertyIndicate) {
            [peripheral setNotifyValue:YES forCharacteristic:characteristic];
            notifyCharacteristicsFound++;
            qDebug() << "Subscribing to indication for characteristic:" << characteristic.UUID;
        }
    }
    
    qDebug() << "Found" << notifyCharacteristicsFound << "notification/indication characteristics out of" << totalCharacteristics;
    
    // Notifica alla classe C++ che le caratteristiche sono state scoperte
    self.qtDevice->stateChanged(QLowEnergyService::ServiceDiscovered);
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error {
    static int subscribedCharacteristics = 0;
    
    if (error) {
        qDebug() << "Error changing notification state:" << error;
    } else {
        subscribedCharacteristics++;
        qDebug() << "Notification state changed for characteristic:" << characteristic.UUID
                 << "(subscribed to" << subscribedCharacteristics << "characteristics)";
        
        // Dopo aver sottoscritto a tutte le caratteristiche, notifica alla classe C++
        self.qtDevice->descriptorWritten(QLowEnergyDescriptor(), QByteArray());
    }
}

- (void)writeCharacteristic:(uint8_t *)data length:(NSUInteger)length {
    if (self.connectedPeripheral.state != CBPeripheralStateConnected) {
        qDebug() << "Cannot send. Peripheral is not connected.";
        return;
    }
        
    NSData *dataToSend = [NSData dataWithBytes:data length:length];
    
    qDebug() << "writeCharacteristic" << dataToSend;
    
    if (self.gattWrite1Characteristic) {
        [self.connectedPeripheral writeValue:dataToSend forCharacteristic:self.gattWrite1Characteristic type:CBCharacteristicWriteWithoutResponse];
    } else {
        qDebug() << "Cannot write - gattWrite1Characteristic is not set";
    }
}

@end
