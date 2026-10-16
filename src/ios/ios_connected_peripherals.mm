#import <CoreBluetooth/CoreBluetooth.h>

#include "ios_connected_peripherals.h"

#include <QBluetoothUuid>
#include <QDebug>

@interface QZConnectedFtmsCentral : NSObject <CBCentralManagerDelegate>
@property (nonatomic, strong) CBCentralManager *centralManager;
@end

@implementation QZConnectedFtmsCentral

- (instancetype)init {
    self = [super init];
    if (self) {
        _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
    }
    return self;
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    Q_UNUSED(central);
}

@end

static CBCentralManager *qzConnectedFtmsCentralManager() {
    static QZConnectedFtmsCentral *owner = [[QZConnectedFtmsCentral alloc] init];
    return owner.centralManager;
}

QList<QBluetoothDeviceInfo> iosConnectedFtmsDevices() {
    QList<QBluetoothDeviceInfo> result;
    CBCentralManager *centralManager = qzConnectedFtmsCentralManager();
    if (centralManager.state != CBManagerStatePoweredOn) {
        return result;
    }

    CBUUID *ftmsService = [CBUUID UUIDWithString:@"1826"];
    NSArray<CBPeripheral *> *peripherals =
        [centralManager retrieveConnectedPeripheralsWithServices:@[ftmsService]];

    const QBluetoothUuid ftmsUuid(quint16(0x1826));
    for (CBPeripheral *peripheral in peripherals) {
        if (!peripheral.identifier || !peripheral.name) {
            continue;
        }

        const QBluetoothUuid peripheralUuid(QString::fromNSString(peripheral.identifier.UUIDString));
        QBluetoothDeviceInfo device(peripheralUuid, QString::fromNSString(peripheral.name), 0);
        QVector<QBluetoothUuid> services;
        services.append(ftmsUuid);
        device.setServiceUuids(services);
        result.append(device);
    }

    if (!result.isEmpty()) {
        qDebug() << "iOS connected FTMS peripherals:" << result.size();
    }
    return result;
}
