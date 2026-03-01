#include "sportstechbike.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>

using namespace std::chrono_literals;

namespace {
// Local clamp helper for environments without std::clamp (pre-C++17)
template <typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

constexpr int kEsx500MinCadence = 30;
constexpr int kEsx500MaxCadence = 100;
constexpr int kEsx500CadenceCount = (kEsx500MaxCadence - kEsx500MinCadence) + 1;
constexpr int kEsx500ResistanceCount = 11; // 0..10

// Sportstech ESX500 table from "velo sportstech esx500.xlsx".
// Rows are resistance levels 0..10, columns are cadence 30..100 RPM.
constexpr std::array<std::array<int, kEsx500CadenceCount>, kEsx500ResistanceCount> kEsx500PowerTable = {{
    {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 11, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 25, 27, 30, 32, 35,
      37, 40, 42, 43, 45, 46, 47, 47, 48, 49, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
      66, 67, 68, 69, 70, 71, 73, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92}},
    {{-1, -1, -1, -1, -1, -1, 9, 9, 10, 10, 11, 12, 13, 14, 14, 15, 16, 17, 18, 19, 20, 22, 25, 27, 30, 32, 35,
      37, 40, 42, 43, 45, 46, 47, 47, 48, 49, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
      66, 67, 68, 69, 70, 71, 73, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92}},
    {{11, 11, 12, 13, 13, 14, 15, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 30, 33, 35, 38, 41, 43,
      46, 48, 51, 53, 54, 55, 55, 56, 57, 57, 58, 58, 59, 60, 61, 62, 63, 65, 66, 67, 69, 70, 71, 73, 74, 76, 78,
      80, 82, 84, 86, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 108, 110, 112}},
    {{13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 29, 30, 31, 32, 33, 35, 37, 40, 43, 46, 49, 51,
      54, 57, 60, 63, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 74, 77, 79, 82, 85, 87, 90, 92, 95, 98, 100, 102, 104,
      106, 109, 111, 113, 115, 117, 120, 123, 126, 129, 133, 136, 139, 143, 146, 149, 153}},
    {{-1, -1, -1, -1, 24, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 36, 37, 38, 39, 40, 43, 46, 49, 52, 56, 59,
      62, 65, 68, 72, 73, 75, 77, 79, 81, 82, 84, 86, 88, 90, 93, 97, 100, 104, 107, 111, 114, 118, 121, 125, 128,
      131, 134, 137, 140, 143, 146, 149, 152, 156, 159, 163, 167, 171, 175, 178, 182, 186, 190, 194}},
    {{22, 24, 26, 28, 29, 31, 33, 34, 36, 38, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 53, 56, 59, 62, 65, 68,
      71, 74, 77, 80, 83, 86, 89, 92, 95, 97, 100, 103, 106, 109, 112, 116, 120, 124, 128, 131, 135, 139, 143, 147,
      150, 154, 158, 161, 165, 169, 172, 176, 180, 184, 187, 190, 193, 196, 199, 202, 205, 208, 211, 215}},
    {{28, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 48, 50, 52, 54, 56, 57, 59, 61, 63, 65, 67, 70, 72, 75, 77, 80,
      82, 85, 87, 90, 94, 98, 102, 106, 110, 114, 118, 122, 126, 131, 135, 139, 143, 147, 151, 155, 159, 163, 167,
      172, 176, 180, 184, 188, 192, 196, 200, 204, 208, 212, 215, 218, 221, 225, 229, 232, 235, 239, 242, 246}},
    {{33, 35, 37, 39, 41, 44, 46, 49, 51, 53, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 77, 79, 81, 83, 85, 88, 90,
      92, 94, 96, 99, 104, 110, 115, 121, 127, 132, 138, 143, 149, 155, 160, 165, 170, 175, 180, 185, 190, 195, 200,
      205, 209, 213, 218, 222, 227, 231, 235, 240, 244, 249, 252, 256, 260, 264, 268, 271, 275, 279, 283, 287}},
    {{38, 40, 43, 46, 48, 51, 54, 56, 59, 62, 65, 67, 69, 71, 74, 76, 78, 81, 83, 85, 88, 90, 92, 94, 96, 98, 100,
      102, 104, 106, 108, 115, 122, 129, 137, 144, 151, 159, 166, 173, 181, 185, 189, 193, 197, 201, 205, 209, 213,
      217, 221, 225, 230, 235, 239, 244, 248, 252, 257, 262, 267, 272, 277, 282, 287, 292, 297, 302, 307, 312, 317}},
    {{42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 72, 73, 75, 77, 79, 81, 82, 84, 86, 88, 90, 92, 95, 98, 100, 103, 106,
      108, 111, 114, 117, 125, 134, 142, 151, 159, 168, 176, 185, 193, 202, 207, 212, 217, 222, 228, 233, 238, 243,
      248, 254, 259, 265, 271, 277, 283, 289, 295, 301, 307, 313, 317, 322, 326, 331, 336, 340, 345, 349, 354, 359}},
    {{46, 49, 52, 55, 59, 62, 65, 69, 72, 75, 79, 80, 82, 84, 85, 87, 89, 90, 92, 94, 96, 99, 102, 105, 108, 111,
      114, 117, 120, 123, 126, 135, 144, 153, 163, 172, 181, 191, 200, 209, 219, 224, 230, 236, 242, 248, 254, 260,
      266, 272, 278, 284, 290, 296, 302, 309, 315, 322, 328, 334, 339, 345, 350, 355, 360, 365, 370, 375, 380, 385,
      390}}
}};

double interpolateCadence(const std::array<int, kEsx500CadenceCount> &row, double cadence) {
    const double clampedCadence = clamp(cadence, (double)kEsx500MinCadence, (double)kEsx500MaxCadence);
    int left = (int)std::floor(clampedCadence) - kEsx500MinCadence;
    int right = (int)std::ceil(clampedCadence) - kEsx500MinCadence;
    left = clamp(left, 0, kEsx500CadenceCount - 1);
    right = clamp(right, 0, kEsx500CadenceCount - 1);

    while (left > 0 && row[left] < 0) {
        --left;
    }
    while (right < kEsx500CadenceCount - 1 && row[right] < 0) {
        ++right;
    }

    if (row[left] < 0 && row[right] < 0) {
        return 0.0;
    }
    if (left == right || row[left] < 0) {
        return row[right];
    }
    if (row[right] < 0) {
        return row[left];
    }

    const double c0 = kEsx500MinCadence + left;
    const double c1 = kEsx500MinCadence + right;
    if (c1 <= c0) {
        return row[left];
    }

    const double t = (clampedCadence - c0) / (c1 - c0);
    return row[left] + ((row[right] - row[left]) * t);
}
} // namespace

sportstechbike::sportstechbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                               double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &sportstechbike::update);
    refresh->start(200ms);
}

void sportstechbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                         bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &sportstechbike::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + " // " + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void sportstechbike::forceResistance(resistance_t requestResistance) {
    Q_UNUSED(requestResistance)
    /*
    uint8_t resistance[] = { 0xf0, 0xa6, 0x01, 0x01, 0x00, 0x00 };
    resistance[4] = requestResistance + 1;
    for(uint8_t i=0; i<sizeof(resistance)-1; i++)
    {
       resistance[5] += resistance[i]; // the last byte is a sort of a checksum
    }
    writeCharacteristic((uint8_t*)resistance, sizeof(resistance), "resistance " + QString::number(requestResistance),
    false, true);
    */
}

void sportstechbike::update() {
    // qDebug() << bike.isValid() << m_control->state() << gattCommunicationChannelService <<
    // gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone;

    if (!m_control) {
        return;
    }

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit(false);
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && initDone) {
        update_metrics(false, 0);

        // updating the bike console every second
        if (sec1update++ == (1000 / refresh->interval())) {
            sec1update = 0;
            // updateDisplay(elapsed);
        }

        QSettings settings;
        uint8_t noOpData[] = {0xf2, 0xc3, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xbe};
        if (requestResistance < 0) {
            requestResistance = 0;
        }
        if (requestResistance > 23) {
            requestResistance = 23;
        }
        noOpData[4] = requestResistance;
        noOpData[10] += requestResistance;
        writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
    }
}

void sportstechbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void sportstechbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    emit packetReceived();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;
    if (newValue.length() != 20) {
        return;
    }

    double speed = GetSpeedFromPacket(newValue);
    double cadence = GetCadenceFromPacket(newValue);
    double resistance = GetResistanceFromPacket(newValue);
    double kcal = GetKcalFromPacket(newValue);
    double watt = GetWattFromPacket(newValue);
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {

            uint8_t heart = ((uint8_t)newValue.at(11));
            if (heart == 0 || disable_hr_frommachinery) {
                update_hr_from_external();
            } else {
                Heart = heart;
            }
        }
    }

    if (!firstCharChanged) {
        Distance += ((speed / 3600.0) / (1000.0 / (lastTimeCharChanged.msecsTo(QTime::currentTime()))));
    }

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current cadence: ") + QString::number(cadence));
    emit debug(QStringLiteral("Current resistance: ") + QString::number(resistance));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
    emit debug(QStringLiteral("Current watt: ") + QString::number(watt));
    emit debug(QStringLiteral("Current Elapsed from the bike (not used): ") +
               QString::number(GetElapsedFromPacket(newValue)));
    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = speed;
    } else {
        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(),
            fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }
    Resistance = requestResistance;
    emit resistanceRead(Resistance.value());
    KCal = kcal;
    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        Cadence = cadence;
    }
    if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled")))
        m_watt = watt;

    lastTimeCharChanged = QTime::currentTime();
    firstCharChanged = false;
}

uint16_t sportstechbike::GetElapsedFromPacket(const QByteArray &packet) {
    uint16_t convertedDataSec = (packet.at(4));
    uint16_t convertedDataMin = (packet.at(3));
    uint16_t convertedData = convertedDataMin * 60.f + convertedDataSec;
    return convertedData;
}

double sportstechbike::GetSpeedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(12) << 8) | ((uint8_t)packet.at(13));
    double data = (double)(convertedData) / 10.0f;
    return data;
}

double sportstechbike::GetKcalFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(7) << 8) | ((uint8_t)packet.at(8));
    return (double)(convertedData);
}

double sportstechbike::GetWattFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(9) << 8) | ((uint8_t)packet.at(10));
    double data = ((double)(convertedData));
    return data;
}

double sportstechbike::GetCadenceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = packet.at(17);
    double data = (convertedData);
    if (data < 0) {
        return 0;
    }
    return data;
}

double sportstechbike::GetResistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = packet.at(15);
    double data = (convertedData);
    if (data < 0) {
        return 0;
    }
    return data;
}

void sportstechbike::btinit(bool startTape) {
    Q_UNUSED(startTape);
    QSettings settings;

    const uint8_t initData1[] = {0xf2, 0xc0, 0x00, 0xb2};
    const uint8_t initData2[] = {0xf2, 0xc1, 0x05, 0x01, 0xff, 0xff, 0xff, 0xff, 0xb5};
    const uint8_t initData3[] = {0xf2, 0xc4, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xc0};
    const uint8_t initData4[] = {0xf2, 0xc3, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xbb};

    writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);

    initDone = true;
}

void sportstechbike::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            emit debug(QStringLiteral("characteristic ") + c.uuid().toString());
        }

        //        QString uuidWrite = "0000fff2-0000-1000-8000-00805f9b34fb";
        //        QString uuidNotify1 = "0000fff1-0000-1000-8000-00805f9b34fb";

        QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("0000fff2-0000-1000-8000-00805f9b34fb"));
        QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("0000fff1-0000-1000-8000-00805f9b34fb"));

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &sportstechbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &sportstechbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &sportstechbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &sportstechbike::descriptorWritten);

        // ******************************************* virtual bike init *************************************
        if (!firstVirtualBike && !this->hasVirtualDevice()) {
            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual bike interface..."));
                auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // connect(virtualBike,&virtualbike::debug ,this,&sportstechbike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &sportstechbike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        firstVirtualBike = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void sportstechbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void sportstechbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void sportstechbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    // QString uuid = "0000fff0-0000-1000-8000-00805f9b34fb";

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb"));
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if (gattCommunicationChannelService == nullptr) {
        qDebug() << QStringLiteral("invalid service") << _gattCommunicationChannelServiceId.toString();
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &sportstechbike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void sportstechbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("sportstechbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void sportstechbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("sportstechbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void sportstechbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &sportstechbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &sportstechbike::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &sportstechbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &sportstechbike::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    emit debug(QStringLiteral("Cannot connect to remote device."));
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("Controller connected. Search services..."));
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("LowEnergy controller disconnected"));
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

uint16_t sportstechbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

bool sportstechbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

resistance_t sportstechbike::maxResistance() {
    QSettings settings;
    const bool sportstechEsx500 =
        settings.value(QZSettings::sportstech_esx500, QZSettings::default_sportstech_esx500).toBool();
    return sportstechEsx500 ? 10 : 24;
}

void sportstechbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

uint16_t sportstechbike::wattsFromResistance(double resistance) {
    QSettings settings;
    const bool sportstechEsx500 =
        settings.value(QZSettings::sportstech_esx500, QZSettings::default_sportstech_esx500).toBool();
    if (sportstechEsx500) {
        const double cadence = Cadence.value();
        // ESX500 table already uses device resistance levels (0..10), no 24->10 remapping.
        const double mappedResistance = clamp(resistance, 0.0, (double)(kEsx500ResistanceCount - 1));

        int lowerResistance = (int)std::floor(mappedResistance);
        int upperResistance = (int)std::ceil(mappedResistance);
        lowerResistance = clamp(lowerResistance, 0, kEsx500ResistanceCount - 1);
        upperResistance = clamp(upperResistance, 0, kEsx500ResistanceCount - 1);

        const double lowerWatt = interpolateCadence(kEsx500PowerTable[lowerResistance], cadence);
        const double upperWatt = interpolateCadence(kEsx500PowerTable[upperResistance], cadence);

        double interpolatedWatt = lowerWatt;
        if (upperResistance != lowerResistance) {
            const double t = mappedResistance - lowerResistance;
            interpolatedWatt = lowerWatt + ((upperWatt - lowerWatt) * t);
        }

        return (uint16_t)std::lround(std::max(0.0, interpolatedWatt));
    }

    // Coefficients from the polynomial regression
    double intercept = 14.4968;
    double b1 = -4.1878;
    double b2 = -0.5051;
    double b3 = 0.00387;
    double b4 = 0.2392;
    double b5 = 0.01108;
    double cadence = Cadence.value();

    // Calculate power using the polynomial equation
    double power = intercept +
                   (b1 * resistance) +
                   (b2 * cadence) +
                   (b3 * resistance * resistance) +
                   (b4 * resistance * cadence) +
                   (b5 * cadence * cadence);

    return power;
}

resistance_t sportstechbike::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << Cadence.value();
    QSettings settings;
    const bool sportstechEsx500 =
        settings.value(QZSettings::sportstech_esx500, QZSettings::default_sportstech_esx500).toBool();

    if (Cadence.value() == 0) {
        esx500UnderTargetSinceMs = -1;
        esx500LastTargetPower = 0;
        return 1;
    }

    resistance_t selectedResistance = 1;
    bool foundBracket = false;

    for (resistance_t i = 1; i < maxResistance(); i++) {
        const uint16_t lowerWatt = wattsFromResistance(i);
        const uint16_t upperWatt = wattsFromResistance(i + 1);
        if (lowerWatt <= power && upperWatt >= power) {
            foundBracket = true;
            if (sportstechEsx500) {
                // ESX500 has coarse resistance levels: choose nearest level, with upward bias.
                const double midpoint = lowerWatt + ((upperWatt - lowerWatt) / 2.0);
                selectedResistance = (power >= midpoint) ? (i + 1) : i;
            } else {
                selectedResistance = i;
            }
            qDebug() << QStringLiteral("resistanceFromPowerRequest") << lowerWatt << upperWatt << power
                     << QStringLiteral("selected") << selectedResistance;
            break;
        }
    }

    if (!foundBracket) {
        if (power < wattsFromResistance(1))
            selectedResistance = 1;
        else
            selectedResistance = maxResistance();
    }

    if (sportstechEsx500) {
        // If we stay below target for a while, force one level up to avoid sticking.
        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        const double currentWatt = wattsMetric().value();
        constexpr double kUnderTargetMarginWatt = 8.0;
        constexpr qint64 kUnderTargetHoldMs = 2500;
        if (currentWatt > 0.0 && (currentWatt + kUnderTargetMarginWatt) < power) {
            if (esx500UnderTargetSinceMs < 0 || esx500LastTargetPower != power) {
                esx500UnderTargetSinceMs = nowMs;
            } else if ((nowMs - esx500UnderTargetSinceMs) >= kUnderTargetHoldMs &&
                       selectedResistance < maxResistance()) {
                selectedResistance++;
                esx500UnderTargetSinceMs = nowMs;
                qDebug() << QStringLiteral("resistanceFromPowerRequest ESX500 upstep") << selectedResistance
                         << QStringLiteral("currentWatt") << currentWatt << QStringLiteral("target") << power;
            }
        } else {
            esx500UnderTargetSinceMs = -1;
        }
        esx500LastTargetPower = power;
    }

    return selectedResistance;
}
