#include "dirconmanager.h"
#include <QNetworkInterface>
#include <QSettings>

#define DM_SERV_OP(OP, P1, P2, P3)                                                                                     \
    OP(FITNESS_MACHINE, 0x1826, WAHOO_KICKR, P1, P2, P3)                                                               \
    OP(CYCLING_POWER, 0x1818, WAHOO_KICKR, P1, P2, P3)                                                                 \
    OP(CYCLING_SPEED_AND_CADENCE, 0x1816, WAHOO_KICKR, P1, P2, P3)                                                     \
    OP(HEART_RATE, 0x180D, WAHOO_BLUEHR, P1, P2, P3)

#define DM_MACHINE_OP(OP, P1, P2, P3)                                                                                  \
    OP(WAHOO_KICKR, "Wahoo KICKR $uuid_hex$", P1, P2, P3)                                                              \
    OP(WAHOO_BLUEHR, "Wahoo HRM", P1, P2, P3)                                                                          \
    OP(WAHOO_RPM_SPEED, "Wahoo SPEED $uuid_hex$", P1, P2, P3)

#define DP_PROCESS_WRITE_2AD9() writeP2AD9
#define DP_PROCESS_WRITE_2A55() 0
#define DP_PROCESS_WRITE_NULL() 0

#define DM_BT(A) QByteArrayLiteral(A)

#define DM_CHAR_OP(OP, P1, P2, P3)                                                                                     \
    OP(FITNESS_MACHINE, 0x2ACC, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x83\x14\x00\x00\x0C\xE0\x00\x00"),                   \
       DP_PROCESS_WRITE_NULL, P1, P2, P3)                                                                              \
    OP(FITNESS_MACHINE, 0x2AD6, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x0A\x00\x96\x00\x0A\x00"), DP_PROCESS_WRITE_NULL,    \
       P1, P2, P3)                                                                                                     \
    OP(FITNESS_MACHINE, 0x2AD9, DPKT_CHAR_PROP_FLAG_WRITE, DM_BT("\x00"), DP_PROCESS_WRITE_2AD9, P1, P2, P3)           \
    OP(FITNESS_MACHINE, 0x2AD2, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2, P3)          \
    OP(FITNESS_MACHINE, 0x2AD3, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x00\x01"), DP_PROCESS_WRITE_NULL, P1, P2, P3)        \
    OP(CYCLING_POWER, 0x2A65, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x08\x00\x00\x00"), DP_PROCESS_WRITE_NULL, P1, P2, P3)  \
    OP(CYCLING_POWER, 0x2A5D, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x0d"), DP_PROCESS_WRITE_NULL, P1, P2, P3)              \
    OP(CYCLING_POWER, 0x2A63, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2, P3)            \
    OP(CYCLING_SPEED_AND_CADENCE, 0x2A5C, DPKT_CHAR_PROP_FLAG_READ,                                                    \
       (bike_wheel_revs ? DM_BT("\x03\x00") : DM_BT("\x02\x00")), DP_PROCESS_WRITE_NULL, P1, P2, P3)                   \
    OP(CYCLING_SPEED_AND_CADENCE, 0x2A5D, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x0d"), DP_PROCESS_WRITE_NULL, P1, P2, P3)  \
    OP(CYCLING_SPEED_AND_CADENCE, 0x2A5B, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2,    \
       P3)                                                                                                             \
    OP(CYCLING_SPEED_AND_CADENCE, 0x2A55, DPKT_CHAR_PROP_FLAG_WRITE, DM_BT("\x00"), DP_PROCESS_WRITE_2A55, P1, P2, P3) \
    OP(HEART_RATE, 0x2A37, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2, P3)

#define DM_MACHINE_ENUM_OP(DESC, NAME, P1, P2, P3) DM_MACHINE_##DESC,

enum { DM_MACHINE_OP(DM_MACHINE_ENUM_OP, 0, 0, 0) };

#define DM_SERV_ENUMU_OP(DESC, UUID, MACHINE, P1, P2, P3) DM_SERV_U_##DESC = UUID,

enum { DM_SERV_OP(DM_SERV_ENUMU_OP, 0, 0, 0) };

#define DM_SERV_ENUMM_OP(DESC, UUID, MACHINE, P1, P2, P3) DM_SERV_M_##DESC = DM_MACHINE_##MACHINE,

enum { DM_SERV_OP(DM_SERV_ENUMM_OP, 0, 0, 0) };

#define DM_SERV_ENUMI_OP(DESC, UUID, MACHINE, P1, P2, P3) DM_SERV_I_##DESC,

enum { DM_SERV_OP(DM_SERV_ENUMI_OP, 0, 0, 0) DM_SERV_I_NUM };

#define DM_CHAR_INIT_OP(SDESC, UUID, TYPE, READV, WRITEP, P1, P2, P3)                                                  \
    if (P1.size() <= DM_SERV_I_##SDESC) {                                                                              \
        P2 = new DirconProcessorService(QStringLiteral(#SDESC), DM_SERV_U_##SDESC, DM_SERV_M_##SDESC, this);           \
        P1.append(P2);                                                                                                 \
    } else                                                                                                             \
        P2 = P1.at(DM_SERV_I_##SDESC);                                                                                 \
    P2->chars.append(new DirconProcessorCharacteristic(UUID, TYPE, READV, WRITEP(), P2));

#define DM_MACHINE_INIT_OP(DESC, NAME, P1, P2, P3)                                                                     \
    P2.clear();                                                                                                        \
    foreach (DirconProcessorService *s, P1) {                                                                          \
        if (s->machine_id == DM_MACHINE_##DESC) {                                                                      \
            P2.append(s);                                                                                              \
        }                                                                                                              \
    }                                                                                                                  \
    if (P2.size()) {                                                                                                   \
        P3 = new DirconProcessor(                                                                                      \
            P2, QString(QStringLiteral(NAME))                                                                          \
                    .replace(QStringLiteral("$uuid_hex$"),                                                             \
                             QString(QStringLiteral("%1")).arg(DM_MACHINE_##DESC, 4, 10, QLatin1Char('0'))),           \
            server_base_port + DM_MACHINE_##DESC, QString(QStringLiteral("%1")).arg(DM_MACHINE_##DESC), mac, this);    \
        qDebug() << "Initializing dircon for" << QString(QStringLiteral(NAME));                                        \
        processors.append(P3);                                                                                         \
        if (!P3->init()) {                                                                                             \
            qDebug() << "Error initializing" << QString(QStringLiteral(NAME));                                         \
        }                                                                                                              \
    }

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
    return QString(QStringLiteral("00:11:22:33:44"));
}

DirconManager::DirconManager(bluetoothdevice *Bike, uint8_t bikeResistanceOffset, double bikeResistanceGain,
                             QObject *parent)
    : QObject(parent) {
    QSettings settings;
    DirconProcessor *processor;
    DirconProcessorService *service;
    QList<DirconProcessorService *> services, proc_services;
    qDebug() << "Building Dircom Manager";
    uint16_t server_base_port = settings.value(QStringLiteral("dircon_server_base_port"), 4810).toUInt();
    bool bike_wheel_revs = settings.value(QStringLiteral("bike_wheel_revs"), false).toBool();
    notif2AD2 = new CharacteristicNotifier2AD2(Bike, this);
    notif2A63 = new CharacteristicNotifier2A63(Bike, this);
    notif2A37 = new CharacteristicNotifier2A37(Bike, this);
    notif2A5B = new CharacteristicNotifier2A5B(Bike, this);
    writeP2AD9 = new CharacteristicWriteProcessor2AD9(bikeResistanceGain, bikeResistanceOffset, Bike, this);
    DM_CHAR_OP(DM_CHAR_INIT_OP, services, service, 0)
    connect(writeP2AD9, SIGNAL(changeInclination(double, double)), this, SIGNAL(changeInclination(double, double)));
    connect(writeP2AD9, SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
            SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    QObject::connect(&bikeTimer, &QTimer::timeout, this, &DirconManager::bikeProvider);
    QString mac = getMacAddress();
    DM_MACHINE_OP(DM_MACHINE_INIT_OP, services, proc_services, processor)
    bikeTimer.start(1000);
}

void DirconManager::bikeProvider() {
    QByteArray all2AD2;
    QByteArray all2A37;
    QByteArray all2A63;
    QByteArray all2A5B;
    int rv2AD2 = notif2AD2->notify(all2AD2);
    int rv2A37 = notif2A37->notify(all2A37);
    int rv2A63 = notif2A63->notify(all2A63);
    int rv2A5B = notif2A5B->notify(all2A5B);
    foreach (DirconProcessor *processor, processors) {
        if (rv2AD2 == CN_OK)
            processor->sendCharacteristicNotification(0x2ad2, all2AD2);
        if (rv2A37 == CN_OK)
            processor->sendCharacteristicNotification(0x2a37, all2A37);
        if (rv2A63 == CN_OK)
            processor->sendCharacteristicNotification(0x2a63, all2A63);
        if (rv2A5B == CN_OK)
            processor->sendCharacteristicNotification(0x2a5b, all2A5B);
    }
}
