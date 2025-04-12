#import <CoreBluetooth/CoreBluetooth.h>
#import "ios_wahookickrsnapbike.h"

@implementation ios_wahookickrsnapbike

- (instancetype)init:(NSString *)deviceName qtDevice:(void*)qtDevice {
    self = [super init];
    if (self) {
        _targetDeviceName = [deviceName copy];
        qDebug() << _targetDeviceName;
        _qtDevice = (wahookickrsnapbike*)qtDevice;
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
    qDebug() << _targetDeviceName << peripheral.name;
    if (peripheral && _targetDeviceName && peripheral.name) {
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
    bool zwift_found = false;
    bool wahoo_found = false;
    
    [_discoveredServices removeAllObjects];
    for (CBService *service in peripheral.services) {
        [peripheral discoverCharacteristics:nil forService:service];
        [_discoveredServices addObject:service];
        
        // Verifica i servizi e imposta le variabili per serviceScanDone
        if ([service.UUID isEqual:[CBUUID UUIDWithString:@"00000001-19CA-4651-86E5-FA29DCDD09D1"]]) {
            zwift_found = true;
            qDebug() << "Zwift Hub service found";
        } else if ([service.UUID isEqual:[CBUUID UUIDWithString:@"A026EE01-0A7D-4AB3-97FA-F1500F9FEB8B"]]) {
            wahoo_found = true;
            qDebug() << "Wahoo service found";
        }
    }
    
    // Imposta le variabili nel codice C++ prima di chiamare serviceScanDone
    _qtDevice->zwift_found = zwift_found;
    _qtDevice->wahoo_found = wahoo_found;
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
    CBUUID *writeUUID = [CBUUID UUIDWithString:@"A026E005-0A7D-4AB3-97FA-F1500F9FEB8B"]; // Wahoo write characteristic
    
    int notifyCharacteristicsFound = 0;
    int totalCharacteristics = 0;
    
    for (CBCharacteristic *characteristic in service.characteristics) {
        totalCharacteristics++;
        [_discoveredCharacteristics setObject:characteristic forKey:characteristic.UUID];
        
        if ([characteristic.UUID isEqual:writeUUID]) {
            self.gattWriteCharacteristic = characteristic;
            qDebug() << "Found Wahoo write characteristic";
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
    
    if (self.gattWriteCharacteristic) {
        _qtDevice->stateChanged(QLowEnergyService::ServiceDiscovered);
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error {
    static int subscribedCharacteristics = 0;
    
    if (error) {
        qDebug() << "Error changing notification state:" << error;
    } else {
        subscribedCharacteristics++;
        qDebug() << "Notification state changed for characteristic:" << characteristic.UUID
                 << "(subscribed to" << subscribedCharacteristics << "characteristics)";
        
        _qtDevice->descriptorWritten(QLowEnergyDescriptor(), QByteArray());
    }
}

- (void)writeCharacteristic:(uint8_t *)data length:(NSUInteger)length {
    if (self.connectedPeripheral.state != CBPeripheralStateConnected) {
        qDebug() << "Cannot send. Peripheral is not connected.";
        return;
    }
        
    NSData *dataToSend = [NSData dataWithBytes:data length:length];
    
    qDebug() << "writeCharacteristic" << dataToSend;
    
    if (self.gattWriteCharacteristic) {
        [self.connectedPeripheral writeValue:dataToSend forCharacteristic:self.gattWriteCharacteristic type:CBCharacteristicWriteWithResponse];
    } else {
        qDebug() << "Cannot write - gattWriteCharacteristic is not set";
    }
}

@end
