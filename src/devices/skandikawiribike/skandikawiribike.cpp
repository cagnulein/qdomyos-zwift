#include "skandikawiribike.h"
#include "qzsettings.h"
#include "homeform.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

skandikawiribike::skandikawiribike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                   double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);

    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    initDone = false;
    connect(refresh, &QTimer::timeout, this, &skandikawiribike::update);
    refresh->start(300ms);
}

skandikawiribike::~skandikawiribike() { qDebug() << QStringLiteral("~skandikawiribike()"); }

void skandikawiribike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                           bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
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
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

/*
void skandikawiribike::forceResistance(resistance_t requestResistance)
{

}
*/
void skandikawiribike::update() {
    uint8_t noOpData[] = {0x20, 0x00, 0x00, 0x00, 0x00};

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && initDone) {
        update_metrics(true, watts());

        // updating the treadmill console every second
        if (sec1Update++ == (1000 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed.value());
        } else {
            noOpData[1] = 0x01;
            noOpData[2] = lastRequestedResistance().value(); // + ((lastRequestedResistance().value() / 10) * 6);
            if (noOpData[2] == 0)
                noOpData[2] = 1;

            for (uint8_t i = 0; i < sizeof(noOpData) - 1; i++) {
                noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
            }

            writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), true, true);
        }

        if (requestResistance != -1) {
            if (requestResistance > 32) {
                requestResistance = 32;
            } else if (requestResistance < 1) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                // forceResistance(requestResistance);
            }
            requestResistance = -1;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            requestStart = -1;
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            requestStop = -1;
        }
    }
}

void skandikawiribike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

uint8_t skandikawiribike::convertHexToDec(uint8_t a) {
    char buffer[33];
    itoa(a, buffer, 16);
    return QString::fromLatin1(buffer).toUShort();
}

char *skandikawiribike::itoa(int num, char *buffer, int base) {
    int curr = 0;

    if (num == 0) {
        // Base case
        buffer[curr++] = '0';
        buffer[curr] = 0;
        return buffer;
    }

    int num_digits = 0;

    if (num < 0) {
        if (base == 10) {
            num_digits++;
            buffer[curr] = '-';
            curr++;
            // Make it positive and finally add the minus sign
            num *= -1;
        } else {
            // Unsupported base. Return NULL
            return nullptr;
        }
    }

    num_digits += (int)floor(log(num) / log(base)) + 1;

    // Go through the digits one by one
    // from left to right
    while (curr < num_digits) {
        // Get the base value. For example, 10^2 = 1000, for the third digit
        int base_val = (int)pow(base, num_digits - 1 - curr);

        // Get the numerical value
        int num_val = num / base_val;

        char value = num_val + '0';
        buffer[curr] = value;

        curr++;
        num -= base_val * num_val;
    }
    buffer[curr] = 0;
    return buffer;
}

void skandikawiribike::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;
    if (newValue.length() == 5 && X2000 == false) {
        if (newValue.at(2) < 33) {
            Resistance = newValue.at(2);
            emit debug(QStringLiteral("Current resistance: ") + QString::number(Resistance.value()));
        }
        return;
    } else if (newValue.length() == 6 && X2000 == true) {
        if (newValue.at(2) < 33) {
            Resistance = newValue.at(2);
            emit debug(QStringLiteral("Current resistance: ") + QString::number(Resistance.value()));
        }
        return;
    } else if (newValue.length() != 12) {
        return;
    }

    if ((newValue.at(1) == 0x00 && X2000 == false) || (newValue.at(1) == 0x20 && X2000 == true)) {
        double speed = GetSpeedFromPacket(newValue);
        emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
        if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
            Speed = speed;
        } else {
            Speed = metric::calculateSpeedFromPower(
                watts(), Inclination.value(), Speed.value(),
                fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
        }
    } else if ((newValue.at(1) == 0x10 && X2000 == false) || (newValue.at(1) == 0x30 && X2000 == true)) {
        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            Cadence = GetCadenceFromPacket(newValue);
        }
    }

    double kcal = GetKcalFromPacket(newValue);

    if (X2000) {
        m_watts = wattFromHR(true);
    } else {
        m_watts = GetWattFromPacket(newValue);
    }
    if (Resistance.value() < 1) {
        emit debug(QStringLiteral("invalid resistance value ") + QString::number(Resistance.value()) +
                   QStringLiteral(" putting to default"));
        Resistance = 1;
    }
    emit resistanceRead(Resistance.value());

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            if (X2000 || delightechBike) {
                Heart = newValue.at(8); // X-2000 or delightech app/protocol compatible bike (e.g. Skandika Morpheus)
            } else {
                Heart = 0;
            }
        }
    }

    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    emit debug(QStringLiteral("Current cadence: ") + QString::number(Cadence.value()));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    KCal = kcal;
}

double skandikawiribike::GetSpeedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (convertHexToDec(packet.at(2)) * 100) + convertHexToDec(packet.at(3));
    double data = (double)convertedData / 10.0;
    return data;
}

double skandikawiribike::GetWattFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (convertHexToDec(packet.at(9)) * 100) + convertHexToDec(packet.at(10));
    double data = (double)convertedData;
    return data;
}

double skandikawiribike::GetCadenceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (convertHexToDec(packet.at(2)) * 100) + convertHexToDec(packet.at(3));
    double data = (double)convertedData;
    return data;
}

double skandikawiribike::GetKcalFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (convertHexToDec(packet.at(6)) * 100) + convertHexToDec(packet.at(7));
    return (double)convertedData;
}

void skandikawiribike::btinit() {
    if (X2000) {
        uint8_t initData1[] = {0x40, 0x00, 0x9a, 0x38, 0x12};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    } else {
        uint8_t initData1[] = {0x40, 0x00, 0x9a, 0x24, 0xfe};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    }

    initDone = true;
}

void skandikawiribike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
    QBluetoothUuid _gattNotifyCharacteristicId((quint16)0xfff1);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &skandikawiribike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &skandikawiribike::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &skandikawiribike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &skandikawiribike::descriptorWritten);

        // ******************************************* virtual bike init *************************************
        if (!firstStateChanged && !this->hasVirtualDevice()
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            && !h
#endif
#endif
        ) {
            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence =
                settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
            bool ios_peloton_workaround =
                settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
            if (ios_peloton_workaround && cadence) {
                qDebug() << "ios_peloton_workaround activated!";
                h = new lockscreen();
                h->virtualbike_ios();
            } else
#endif
#endif
                if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual bike interface..."));
                auto virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // connect(virtualBike,&virtualbike::debug ,this,&skandikawiribike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &skandikawiribike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void skandikawiribike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void skandikawiribike::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void skandikawiribike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    if (!gattCommunicationChannelService) {
        homeform::singleton()->setToastRequested(
            "no service found, contact me to roberto.viola83@gmail.com!");
        return;
    }
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &skandikawiribike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void skandikawiribike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("skandikawiribike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void skandikawiribike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("skandikawiribike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void skandikawiribike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
    {
        bluetoothDevice = device;

        if (device.name().toUpper().startsWith(QLatin1String("HT"))) {
            QSettings settings;
            bool x2000_protocol_enabled = settings.value(QZSettings::skandika_wiri_x2000_protocol,
                                                         QZSettings::default_skandika_wiri_x2000_protocol).toBool();

            if (device.name().length() == 11) { // Bikes like the Skandika X-2000 Foldaway Bike or HT211212095
                X2000 = x2000_protocol_enabled;
                if (X2000) {
                    qDebug() << "X-2000 PROTOCOL ENABLED!";
                } else {
                    qDebug() << "STANDARD PROTOCOL (X-2000 protocol disabled in settings)";
                }
            } else if (device.name().length() == 12) // Bikes compatible with delightech app/protocol, for example Skandika Morpheus
            {
                qDebug() << "deligthechbike WORKAROUND!";
                delightechBike = true;
            }
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &skandikawiribike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &skandikawiribike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &skandikawiribike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &skandikawiribike::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
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

bool skandikawiribike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t skandikawiribike::watts() {
    QSettings settings;
    // double v = 0; // NOTE: unused variable v
    // const resistance_t max_resistance = 15;
    // ref
    // https://translate.google.com/translate?hl=it&sl=en&u=https://support.wattbike.com/hc/en-us/articles/115001881825-Power-Resistance-and-Cadence-Tables&prev=search&pto=aue

    if (currentCadence().value() == 0) { // only update watts if pedaling
        return 0;
    }

    return m_watts;
}

void skandikawiribike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
