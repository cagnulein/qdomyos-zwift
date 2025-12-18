#include "devices/dircon/dirconmanager.h"
#include "devices/bike.h"
#include <QNetworkInterface>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

#define DM_MACHINE_TYPE_BIKE 1
#define DM_MACHINE_TYPE_TREADMILL 2

#define DM_SERV_OP(OP, P1, P2, P3, ZWIFT_ENABLED)                                                                     \
    OP(FITNESS_MACHINE_CYCLE, 0x1826, WAHOO_KICKR, P1, P2, P3)                                                         \
    OP(FITNESS_MACHINE_TREADMILL, 0x1826, WAHOO_TREADMILL, P1, P2, P3)                                                 \
    OP(CYCLING_POWER, 0x1818, WAHOO_KICKR, P1, P2, P3)                                                                 \
    OP(CYCLING_SPEED_AND_CADENCE, 0x1816, WAHOO_KICKR, P1, P2, P3)                                                     \
    OP(RUNNING_SPEED_AND_CADENCE, 0x1814, WAHOO_TREADMILL, P1, P2, P3)                                                 \
    OP(HEART_RATE, 0x180D, WAHOO_BLUEHR, P1, P2, P3)                                                                   \
    ZWIFT_ENABLED(OP, ZWIFT_PLAY_EMULATOR, ZWIFT_PLAY_ENUM_VALUE, WAHOO_KICKR, P1, P2, P3)

#define ZWIFT_ENABLED_OP(OP, DESC, UUID, MACHINE, P1, P2, P3) OP(DESC, UUID, MACHINE, P1, P2, P3)
#define ZWIFT_DISABLED_OP(OP, DESC, UUID, MACHINE, P1, P2, P3)

#define DM_MACHINE_OP(OP, P1, P2, P3)                                                                                  \
    OP(WAHOO_KICKR, "Wahoo KICKR $uuid_hex$", DM_MACHINE_TYPE_TREADMILL | DM_MACHINE_TYPE_BIKE, P1, P2, P3)            \
    OP(WAHOO_BLUEHR, "Wahoo HRM", DM_MACHINE_TYPE_BIKE | DM_MACHINE_TYPE_TREADMILL, P1, P2, P3)                        \
    OP(WAHOO_RPM_SPEED, "Wahoo SPEED $uuid_hex$", DM_MACHINE_TYPE_BIKE, P1, P2, P3)                                    \
    OP(WAHOO_TREADMILL, "Wahoo TREAD $uuid_hex$", DM_MACHINE_TYPE_TREADMILL, P1, P2, P3)

#define DM_MACHINE_OP_ROUVY(OP, P1, P2, P3)                                                                            \
    OP(WAHOO_KICKR, "ELITE AVANTI $uuid_hex$ W", DM_MACHINE_TYPE_TREADMILL | DM_MACHINE_TYPE_BIKE, P1, P2, P3)            \
    OP(WAHOO_BLUEHR, "Wahoo HRM", DM_MACHINE_TYPE_BIKE | DM_MACHINE_TYPE_TREADMILL, P1, P2, P3)                        \
    OP(WAHOO_RPM_SPEED, "Wahoo SPEED $uuid_hex$", DM_MACHINE_TYPE_BIKE, P1, P2, P3)                                    \
    OP(WAHOO_TREADMILL, "Wahoo TREAD $uuid_hex$", DM_MACHINE_TYPE_TREADMILL, P1, P2, P3)

#define DP_PROCESS_WRITE_0003() (zwift_play_emulator ? writeP0003 : 0)
#define DP_PROCESS_WRITE_2AD9() writeP2AD9
#define DP_PROCESS_WRITE_2AD9T() writeP2AD9
#define DP_PROCESS_WRITE_E005() writePE005
#define DP_PROCESS_WRITE_E005T() writePE005
#define DP_PROCESS_WRITE_2A55() 0
#define DP_PROCESS_WRITE_2A55T() 0
#define DP_PROCESS_WRITE_NULL() 0

#define DM_BT(A) QByteArrayLiteral(A)

#define DM_CHAR_OP(OP, P1, P2, P3, ZWIFT_ENABLED)                                                                     \
    OP(FITNESS_MACHINE_CYCLE, 0x2ACC, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x83\x14\x00\x00\x0C\xE0\x00\x00"),             \
       DP_PROCESS_WRITE_NULL, P1, P2, P3)                                                                              \
    OP(FITNESS_MACHINE_CYCLE, 0x2AD6, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x0A\x00\x96\x00\x0A\x00"),                     \
       DP_PROCESS_WRITE_NULL, P1, P2, P3)                                                                              \
    OP(FITNESS_MACHINE_CYCLE, 0x2AD9, DPKT_CHAR_PROP_FLAG_WRITE | DPKT_CHAR_PROP_FLAG_INDICATE, DM_BT("\x00"), DP_PROCESS_WRITE_2AD9, P1, P2, P3)     \
    OP(FITNESS_MACHINE_CYCLE, 0xE005, DPKT_CHAR_PROP_FLAG_WRITE, DM_BT("\x00"), DP_PROCESS_WRITE_E005, P1, P2, P3)     \
    OP(FITNESS_MACHINE_CYCLE, 0x2AD2, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2, P3)    \
    OP(FITNESS_MACHINE_CYCLE, 0x2AD3, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x00\x01"), DP_PROCESS_WRITE_NULL, P1, P2, P3)  \
    OP(FITNESS_MACHINE_TREADMILL, 0x2ACC, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x08\x14\x00\x00\x00\x00\x00\x00"),         \
       DP_PROCESS_WRITE_NULL, P1, P2, P3)                                                                              \
    OP(FITNESS_MACHINE_TREADMILL, 0x2AD6, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x0A\x00\x96\x00\x0A\x00"),                 \
       DP_PROCESS_WRITE_NULL, P1, P2, P3)                                                                              \
    OP(FITNESS_MACHINE_TREADMILL, 0x2AD9, DPKT_CHAR_PROP_FLAG_WRITE | DPKT_CHAR_PROP_FLAG_INDICATE, DM_BT("\x00"), DP_PROCESS_WRITE_2AD9T, P1, P2,    \
       P3)                                                                                                             \
    OP(FITNESS_MACHINE_TREADMILL, 0x2ACD, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2,    \
       P3)                                                                                                             \
    OP(FITNESS_MACHINE_TREADMILL, 0x2ADA, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2,    \
       P3)                                                                                                             \
    OP(FITNESS_MACHINE_TREADMILL, 0x2AD3, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x00\x01"), DP_PROCESS_WRITE_NULL, P1, P2,  \
       P3)                                                                                                             \
    OP(FITNESS_MACHINE_TREADMILL, 0x2AD2, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2,    \
       P3)                                                                                                             \
    OP(CYCLING_POWER, 0x2A65, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x08\x00\x00\x00"), DP_PROCESS_WRITE_NULL, P1, P2, P3)  \
    OP(CYCLING_POWER, 0x2A5D, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x0d"), DP_PROCESS_WRITE_NULL, P1, P2, P3)              \
    OP(CYCLING_POWER, 0x2A63, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2, P3)            \
    OP(CYCLING_SPEED_AND_CADENCE, 0x2A5C, DPKT_CHAR_PROP_FLAG_READ,                                                    \
       (bike_wheel_revs ? DM_BT("\x03\x00") : DM_BT("\x02\x00")), DP_PROCESS_WRITE_NULL, P1, P2, P3)                   \
    OP(CYCLING_SPEED_AND_CADENCE, 0x2A5D, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x0d"), DP_PROCESS_WRITE_NULL, P1, P2, P3)  \
    OP(CYCLING_SPEED_AND_CADENCE, 0x2A5B, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2,    \
       P3)                                                                                                             \
    OP(CYCLING_SPEED_AND_CADENCE, 0x2A55, DPKT_CHAR_PROP_FLAG_WRITE, DM_BT("\x00"), DP_PROCESS_WRITE_2A55, P1, P2, P3) \
    OP(RUNNING_SPEED_AND_CADENCE, 0x2A54, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x02\x00"), DP_PROCESS_WRITE_NULL, P1, P2,  \
       P3)                                                                                                             \
    OP(RUNNING_SPEED_AND_CADENCE, 0x2A5D, DPKT_CHAR_PROP_FLAG_READ, DM_BT("\x01"), DP_PROCESS_WRITE_NULL, P1, P2, P3)  \
    OP(RUNNING_SPEED_AND_CADENCE, 0x2A55, DPKT_CHAR_PROP_FLAG_WRITE, DM_BT("\x00"), DP_PROCESS_WRITE_2A55T, P1, P2,    \
       P3)                                                                                                             \
    OP(RUNNING_SPEED_AND_CADENCE, 0x2A53, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2,    \
       P3)                                                                                                             \
    OP(HEART_RATE, 0x2A37, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2, P3)               \
    ZWIFT_ENABLED(OP, ZWIFT_PLAY_EMULATOR, 0x0003, DPKT_CHAR_PROP_FLAG_WRITE, DM_BT("\x00"), DP_PROCESS_WRITE_0003, P1, P2, P3) \
    ZWIFT_ENABLED(OP, ZWIFT_PLAY_EMULATOR, 0x0002, DPKT_CHAR_PROP_FLAG_NOTIFY, DM_BT("\x00"), DP_PROCESS_WRITE_NULL, P1, P2, P3) \
    ZWIFT_ENABLED(OP, ZWIFT_PLAY_EMULATOR, 0x0004, DPKT_CHAR_PROP_FLAG_INDICATE, DM_BT("\x02\x00"), DP_PROCESS_WRITE_NULL, P1, P2, P3)

#define ZWIFT_CHAR_ENABLED_OP(OP, DESC, UUID, TYPE, READV, WRITEP, P1, P2, P3) \
    OP(DESC, UUID, TYPE, READV, WRITEP, P1, P2, P3)

#define ZWIFT_CHAR_DISABLED_OP(OP, DESC, UUID, TYPE, READV, WRITEP, P1, P2, P3)

#define DM_MACHINE_ENUM_OP(DESC, NAME, TYPE, P1, P2, P3) DM_MACHINE_##DESC,

enum { DM_MACHINE_OP(DM_MACHINE_ENUM_OP, 0, 0, 0) };

#define DM_SERV_ENUMU_OP(DESC, UUID, MACHINE, P1, P2, P3) DM_SERV_U_##DESC = UUID,

enum {
    DM_SERV_OP(DM_SERV_ENUMU_OP, 0, 0, 0, ZWIFT_ENABLED_OP)
};

#define DM_SERV_ENUMM_OP(DESC, UUID, MACHINE, P1, P2, P3) DM_SERV_M_##DESC = DM_MACHINE_##MACHINE,

enum {
    DM_SERV_OP(DM_SERV_ENUMM_OP, 0, 0, 0, ZWIFT_ENABLED_OP)
};

#define DM_SERV_ENUMI_OP(DESC, UUID, MACHINE, P1, P2, P3) DM_SERV_I_##DESC,

enum {
    DM_SERV_OP(DM_SERV_ENUMI_OP, 0, 0, 0, ZWIFT_ENABLED_OP)
    DM_SERV_I_NUM
};

#define DM_CHAR_INIT_OP(SDESC, UUID, TYPE, READV, WRITEP, P1, P2, P3)                                                  \
    if (P1.size() <= DM_SERV_I_##SDESC) {                                                                              \
        P2 = new DirconProcessorService(QStringLiteral(#SDESC), DM_SERV_U_##SDESC, DM_SERV_M_##SDESC, this);           \
        P1.append(P2);                                                                                                 \
    } else                                                                                                             \
        P2 = P1.at(DM_SERV_I_##SDESC);                                                                                 \
    P2->chars.append(new DirconProcessorCharacteristic(UUID, TYPE, READV, WRITEP(), P2));

#define DM_MACHINE_INIT_OP(DESC, NAME, TYPE, P1, P2, P3)                                                               \
    if (P3 & (TYPE)) {                                                                                                 \
        P2.clear();                                                                                                    \
        foreach (DirconProcessorService *s, P1) {                                                                      \
            if (s->machine_id == DM_MACHINE_##DESC) {                                                                  \
                P2.append(s);                                                                                          \
            }                                                                                                          \
        }                                                                                                              \
        if (P2.size()) {                                                                                               \
            QString dircon_id = QString("%1").arg(settings.value(QZSettings::dircon_id,                                \
            QZSettings::default_dircon_id).toInt(), rouvy_compatibility ? 5 : 4, 10, QChar('0'));                     \
            DirconProcessor *processor = new DirconProcessor(                                                          \
                P2,                                                                                                    \
                QString(QStringLiteral(NAME))                                                                          \
                    .replace(QStringLiteral("$uuid_hex$"), dircon_id),                                                 \
                server_base_port + DM_MACHINE_##DESC, rouvy_compatibility ? dircon_id : QString(QStringLiteral("%1")).arg(DM_MACHINE_##DESC), mac,       \
                this);                                                                                                 \
            QString servdesc;                                                                                          \
            foreach (DirconProcessorService *s, P2) { servdesc += *s + QStringLiteral(","); }                          \
            qDebug() << "Initializing dircon for" << QString(QStringLiteral(NAME)) << "with serv" << servdesc;         \
            processors.append(processor);                                                                              \
            if (!processor->init()) {                                                                                  \
                qDebug() << "Error initializing" << QString(QStringLiteral(NAME));                                     \
            }                                                                                                          \
        }                                                                                                              \
    }

QString DirconManager::getMacAddress(bool rouvy_compatibility, int dircon_id) {
    // When Rouvy compatibility is enabled, use a specific MAC address with the last byte set to dircon_id
    if (rouvy_compatibility) {
        // Use base MAC address "24:DC:C3:E3:B5:XX" where XX is the dircon_id
        // Ensure dircon_id is in the valid range 0-255
        int last_byte = dircon_id & 0xFF;
        return QString("24:DC:C3:E3:B5:%1").arg(last_byte, 2, 16, QChar('0')).toUpper();
    }

    // Default behavior: get MAC address from network interfaces
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

#define DM_CHAR_NOTIF_BUILD_OP(UUID, P1, P2, P3) notif##UUID = new CharacteristicNotifier##UUID(P1, this);

DirconManager::DirconManager(bluetoothdevice *Bike, int8_t bikeResistanceOffset, double bikeResistanceGain,
                             QObject *parent)
    : QObject(parent) {
    QSettings settings;
    DirconProcessorService *service;
    QList<DirconProcessorService *> services, proc_services;
    BLUETOOTH_TYPE dt = Bike->deviceType();
    bt = Bike;
    uint8_t type = dt == TREADMILL || dt == ELLIPTICAL ? DM_MACHINE_TYPE_TREADMILL
                                                                                         : DM_MACHINE_TYPE_BIKE;
    qDebug() << "Building Dircom Manager";
    uint16_t server_base_port =
        settings.value(QZSettings::dircon_server_base_port, QZSettings::default_dircon_server_base_port).toUInt();
    bool bike_wheel_revs = settings.value(QZSettings::bike_wheel_revs, QZSettings::default_bike_wheel_revs).toBool();
    bool zwift_play_emulator = settings.value(QZSettings::zwift_play_emulator, QZSettings::default_zwift_play_emulator).toBool();
    bool rouvy_compatibility = settings.value(QZSettings::rouvy_compatibility, QZSettings::default_rouvy_compatibility).toBool();
    
    DM_CHAR_NOTIF_OP(DM_CHAR_NOTIF_BUILD_OP, Bike, 0, 0)
    
    writeP2AD9 = new CharacteristicWriteProcessor2AD9(bikeResistanceGain, bikeResistanceOffset, Bike, notif2AD9, this);
    writePE005 = new CharacteristicWriteProcessorE005(bikeResistanceGain, bikeResistanceOffset, Bike, this);
    
    writeP0003 = new CharacteristicWriteProcessor0003(bikeResistanceGain, bikeResistanceOffset, Bike, notif0002, notif0004, this);
    
    if (zwift_play_emulator) {
        DM_CHAR_OP(DM_CHAR_INIT_OP, services, service, 0, ZWIFT_CHAR_ENABLED_OP);
    } else {
        DM_CHAR_OP(DM_CHAR_INIT_OP, services, service, 0, ZWIFT_CHAR_DISABLED_OP);
    }
    
    connect(writeP2AD9, SIGNAL(changeInclination(double, double)), this, SIGNAL(changeInclination(double, double)));
    connect(writeP2AD9, SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
            SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    connect(writePE005, SIGNAL(changeInclination(double, double)), this, SIGNAL(changeInclination(double, double)));
    connect(writePE005, SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
            SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    
    if (zwift_play_emulator && writeP0003) {
        connect(writeP0003, SIGNAL(changeInclination(double, double)), this, SIGNAL(changeInclination(double, double)));
        connect(writeP0003, SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
                SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    }
    
    QObject::connect(&bikeTimer, &QTimer::timeout, this, &DirconManager::bikeProvider);
    int dircon_id_value = settings.value(QZSettings::dircon_id, QZSettings::default_dircon_id).toInt();
    QString mac = getMacAddress(rouvy_compatibility, dircon_id_value);
    if (rouvy_compatibility) {
        DM_MACHINE_OP_ROUVY(DM_MACHINE_INIT_OP, services, proc_services, type)
    } else {
        DM_MACHINE_OP(DM_MACHINE_INIT_OP, services, proc_services, type)
    }
    
    if (zwift_play_emulator || settings.value(QZSettings::race_mode, QZSettings::default_race_mode).toBool())
        bikeTimer.start(50ms);
    else
        bikeTimer.start(1s);
}

#define DM_CHAR_NOTIF_NOTIF1_OP(UUID, P1, P2, P3)                                                                      \
    QByteArray all##UUID;                                                                                              \
    int rv##UUID = notif##UUID ? notif##UUID->notify(all##UUID) : 0;

#define DM_CHAR_NOTIF_NOTIF2_OP(UUID, P1, P2, P3)                                                                      \
    if (rv##UUID == CN_OK)                                                                                             \
        P1->sendCharacteristicNotification(0x##UUID, all##UUID);

void DirconManager::bikeProvider() {
    DM_CHAR_NOTIF_OP(DM_CHAR_NOTIF_NOTIF1_OP, 0, 0, 0)
    foreach (DirconProcessor *processor, processors) {
        DM_CHAR_NOTIF_OP(DM_CHAR_NOTIF_NOTIF2_OP, processor, 0, 0)
    }
}

double DirconManager::currentGear() {
    QSettings settings;
    if(settings.value(QZSettings::zwift_play_emulator, QZSettings::default_zwift_play_emulator).toBool() && writeP0003)
        return writeP0003->currentGear();
    else if(bt && bt->deviceType() == BIKE)
        return ((bike*)bt)->gears();
    return 0;
}
