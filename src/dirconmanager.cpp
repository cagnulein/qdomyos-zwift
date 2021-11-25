#include "dirconmanager.h"
#include <QNetworkInterface>
#include <QSettings>
#include <chrono>

QString DirconManager::getMacAddress() {
    QString addr;
    foreach (QNetworkInterface netInterface, QNetworkInterface::allInterfaces()) {
        // Return only the first non-loopback MAC Address
        addr = netInterface.hardwareAddress();
        if (!(netInterface.flags() & QNetworkInterface::IsLoopBack) && !addr.isEmpty()) {
            const auto entries = netInterface.addressEntries();
            for (const QNetworkAddressEntry &newEntry : entries) {
                QHostAddress address = newEntry.ip();
                if ((address.protocol() == QAbstractSocket::IPv4Protocol)) {
                    return addr;
                }
            }
        }
    }
    return QString();
}

DirconManager::DirconManager(bluetoothdevice *Bike, uint8_t bikeResistanceOffset, double bikeResistanceGain,
                             QObject *parent)
    : QObject(parent) {
    QSettings settings;
    DirconProcessor *processor;
    DirconProcessorService *service;
    QList<DirconProcessorService *> services;
    qDebug() << "Building Dircom Manager";
    QString server_base_name =
        settings.value(QStringLiteral("dircon_server_base_name"), QStringLiteral("DIRCON")).toString();
    uint16_t server_base_port = settings.value(QStringLiteral("dircon_server_base_port"), 4810).toUInt();
    notif2AD2 = new CharacteristicNotifier2AD2(Bike, this);
    notif2A63 = new CharacteristicNotifier2A63(Bike, this);
    notif2A37 = new CharacteristicNotifier2A37(Bike, this);
    writeP2AD9 = new CharacteristicWriteProcessor2AD9(bikeResistanceGain, bikeResistanceOffset, Bike, this);
    DM_CHAR_OP(DM_CHAR_INIT_OP, services, service, 0)
    connect(writeP2AD9, SIGNAL(changeInclination(double, double)), this, SIGNAL(changeInclination(double, double)));
    connect(writeP2AD9, SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
            SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    QObject::connect(&bikeTimer, &QTimer::timeout, this, &DirconManager::bikeProvider);
    QString mac = getMacAddress();
    foreach (service, services) {
        qDebug() << "Initializing dircon for" << service->uuid;
        processors.append(processor = new DirconProcessor(service, mac, this));
        if (!processor->init()) {
            qDebug() << "Error initializing" << service->name;
        }
    }
    bikeTimer.start(1000);
}

void DirconManager::bikeProvider() {
    QByteArray all2AD2;
    QByteArray all2A37;
    QByteArray all2A63;
    int rv2AD2 = notif2AD2->notify(all2AD2);
    int rv2A37 = notif2A37->notify(all2A37);
    int rv2A63 = notif2A63->notify(all2A63);
    foreach (DirconProcessor *processor, processors) {
        if (rv2AD2 == CN_OK)
            processor->sendCharacteristicNotification(0x2ad2, all2AD2);
        if (rv2A37 == CN_OK)
            processor->sendCharacteristicNotification(0x2a37, all2A37);
        if (rv2A63 == CN_OK)
            processor->sendCharacteristicNotification(0x2a63, all2A63);
    }
}
