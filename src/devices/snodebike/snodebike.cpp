#include "snodebike.h"

#include "devices/ftmsbike/ftmsbike.h"

#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QLowEnergyConnectionParameters>
#endif

#include <chrono>

using namespace std::chrono_literals;

snodebike::snodebike(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &snodebike::update);
    refresh->start(200ms);
}
/*
void snodebike::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool
wait_for_response)
{
    QEventLoop loop;
    QTimer timeout;
    if(wait_for_response)
    {
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }
    else
    {
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray((const char*)data,
data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();
}*/

void snodebike::update() {

    if (!m_control)
        return;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
    } else if (bluetoothDevice.isValid() //&&
                                         // m_control->state() == QLowEnergyController::DiscoveredState //&&
                                         // gattCommunicationChannelService &&
                                         // gattWriteCharacteristic.isValid() &&
                                         // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {
        update_metrics(true, watts());

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (requestResistance != -1) {
            if (requestResistance > 15) {
                requestResistance = 15;
            } else if (requestResistance == 0) {
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

            // btinit();

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void snodebike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void snodebike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    double heart = 0.0;
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    if (characteristic.uuid() != QBluetoothUuid((quint16)0x2AD2)) {
        return;
    }

    lastPacket = newValue;

    // 54 09 default flags for this bike

    union flags {
        struct {
            uint16_t moreData : 1;
            uint16_t avgSpeed : 1;
            uint16_t instantCadence : 1;
            uint16_t avgCadence : 1;
            uint16_t totDistance : 1;
            uint16_t resistanceLvl : 1;
            uint16_t instantPower : 1;
            uint16_t avgPower : 1;
            uint16_t expEnergy : 1;
            uint16_t heartRate : 1;
            uint16_t metabolic : 1;
            uint16_t elapsedTime : 1;
            uint16_t remainingTime : 1;
            uint16_t spare : 3;
        };

        uint16_t word_flags;
    };

    flags Flags;
    int index = 0;
    Flags.word_flags = (newValue.at(1) << 8) | newValue.at(0);
    index += 2;

    if (!Flags.moreData) {
        if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
            Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                              (uint16_t)((uint8_t)newValue.at(index)))) /
                    100.0;
        } else {
            Speed = metric::calculateSpeedFromPower(
                watts(), Inclination.value(), Speed.value(),
                fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
        }
        index += 2;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    }

    if (Flags.avgSpeed) {
        double avgSpeed;
        avgSpeed =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) /
            100.0;
        index += 2;
        emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
    }

    if (Flags.instantCadence) {
        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled")))
            Cadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                (uint16_t)((uint8_t)newValue.at(index)))) /
                      2.0;
        index += 2;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    }

    if (Flags.avgCadence) {
        double avgCadence;
        avgCadence =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) /
            2.0;
        index += 2;
        emit debug(QStringLiteral("Current Average Cadence: ") + QString::number(avgCadence));
    }

    // ignore the distance value because it's a total odometer
    if (Flags.totDistance) {
        // Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
        // (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)))) / 1000.0;
        index += 3;
    }
    // else
    {
        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
    }

    emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

    if (Flags.resistanceLvl) {
        Resistance =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
        index += 2;
        emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    }

    if (Flags.instantPower) {
        if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled")))
            m_watt = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                               (uint16_t)((uint8_t)newValue.at(index))));
        index += 2;
        emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
    }

    if (Flags.avgPower) {
        double avgPower;
        avgPower =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
        index += 2;
        emit debug(QStringLiteral("Current Average Watt: ") + QString::number(avgPower));
    }

    // the snode bike KCal calculation is very bad, a user said, so i will skip it
    if (Flags.expEnergy) {
        // KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
        // (uint16_t)((uint8_t)newValue.at(index))));
        index += 2;

        // energy per hour
        index += 2;

        // energy per minute
        index += 1;
    } /*else*/
    {
        if (watts())
            KCal +=
                ((((0.048 * ((double)watts()) + 1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
    }

    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (Flags.heartRate) {
            heart = ((double)(((uint8_t)newValue.at(index))));
            // index += 1; // NOTE: clang-analyzer-deadcode.DeadStores
            emit debug(QStringLiteral("Current Heart: ") + QString::number(heart));
        }
    }

    if (Flags.metabolic) {
        // todo
    }

    if (Flags.elapsedTime) {
        // todo
    }

    if (Flags.remainingTime) {
        // todo
    }

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    double ac = 0.01243107769;
    double bc = 1.145964912;
    double cc = -23.50977444;

    double ar = 0.1469553975;
    double br = -5.841344538;
    double cr = 97.62165482;

    m_pelotonResistance =
        (sqrt(pow(br, 2.0) -
              4.0 * ar *
                  (cr - (m_watt.value() * 132.0 / (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
         br) /
        (2.0 * ar);
    Resistance = m_pelotonResistance;
    emit resistanceRead(Resistance.value());

    lastRefreshCharacteristicChanged = now;

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        if (heart == 0.0) {
            update_hr_from_external();
        } else {
            Heart = heart;
        }
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && cadence && h && firstStateChanged) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    emit debug(QStringLiteral("Current Calculated Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void snodebike::stateChanged(QLowEnergyService::ServiceState state) {
    if (state != QLowEnergyService::RemoteServiceDiscovered) {
        return;
    }

    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0x2AD2);
    gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);

    QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);
    gattWriteCharControlPointId = gattCommunicationChannelService->characteristic(_gattWriteCharControlPointId);

    qDebug() << state;

    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);

    // i need to remove read request from QT framework in order to get Schwinn compatibility
    // QSharedPointer<QLowEnergyServicePrivate> qzService = gattCommunicationChannelService->d_ptr;
    // m_control->d_ptr->writeDescriptor(qzService, 0x30, 0x31, descriptor);

    gattCommunicationChannelService->writeDescriptor(
        gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);

    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
            &snodebike::characteristicChanged);

    initRequest = false;
    emit connectedAndDiscovered();

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
            auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
            // connect(virtualBike,&virtualbike::debug ,this,&snodebike::debug);
            connect(virtualBike, &virtualbike::changeInclination, this, &snodebike::changeInclination);
            connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this, &snodebike::ftmsCharacteristicChanged);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void snodebike::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QByteArray b = newValue;
    if (gattWriteCharControlPointId.isValid()) {
        qDebug() << "routing FTMS packet to the bike from virtualbike" << characteristic.uuid() << newValue.toHex(' ');

        // this bike doesn't handle negative values, so i have to filter it
        if (newValue.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS) {
            int16_t iresistance = (((uint8_t)b.at(3)) + (b.at(4) << 8));
            if (iresistance < 0) {
                b[3] = 0;
                b[4] = 0;
            }
        }

        if (writeBuffer) {
            delete writeBuffer;
        }
        writeBuffer = new QByteArray(b);

        gattCommunicationChannelService->writeCharacteristic(gattWriteCharControlPointId, *writeBuffer);
    }
}

void snodebike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void snodebike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void snodebike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void snodebike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void snodebike::serviceScanDone() {
    emit debug(QStringLiteral("serviceScanDone"));

    gattCommunicationChannelService = m_control->createServiceObject(QBluetoothUuid((quint16)0x1826));
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &snodebike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void snodebike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("snodebike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void snodebike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("snodebike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void snodebike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &snodebike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &snodebike::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &snodebike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &snodebike::controllerStateChanged);

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

bool snodebike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t snodebike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void snodebike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
