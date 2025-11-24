#include "schwinnic4bike.h"


#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>

#include <QThread>
#include <math.h>
#include <limits>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QLowEnergyConnectionParameters>
#endif

#include <chrono>

using namespace std::chrono_literals;

schwinnic4bike::schwinnic4bike(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &schwinnic4bike::update);
    refresh->start(200ms);
}
/*
void schwinnic4bike::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool
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

void schwinnic4bike::update() {
    qDebug() << gattNotify1Characteristic.isValid();

    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest) {

        initRequest = false;
    } else if (gattNotify1Characteristic.isValid()) {

        update_metrics(false, watts());

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {

            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (requestResistance != -1) {
            if (requestResistance > max_resistance)
                requestResistance = max_resistance;
            else if (requestResistance == 0) {
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

void schwinnic4bike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void schwinnic4bike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    double heart = 0.0;

    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    if (characteristic.uuid() != QBluetoothUuid((quint16)0x2AD2))
        return;

    lastPacket = newValue;

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
        Cadence =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) /
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

    if (Flags.totDistance) {
        Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
                              (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint32_t)((uint8_t)newValue.at(index)))) /
                   1000.0;
        index += 3;
    } else {
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

    if (Flags.expEnergy) {

        KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
        index += 2;

        // energy per hour
        index += 2;

        // energy per minute
        index += 1;
    } else {
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
            // index += 1; //NOTE: clang-analyzer-deadcode..DeadStores            
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

    // if we change this, also change the wattsFromResistance function. We can create a standard function in order to
    // have all the costants in one place (I WANT MORE TIME!!!)
    double ac = 0.01243107769;
    double bc = 1.145964912;
    double cc = -23.50977444;

    double ar = 0.1469553975;
    double br = -5.841344538;
    double cr = 97.62165482;

    double res =
        (((sqrt(pow(br, 2.0) -
                4.0 * ar *
                    (cr - (m_watt.value() * 132.0 / (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
           br) /
          (2.0 * ar)) *
         settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
        settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();

    double resistance;
    if (isnan(res)) {
        res = 0;
    }

    bool schwinn_bike_resistance_v2 =
        settings.value(QZSettings::schwinn_bike_resistance_v2, QZSettings::default_schwinn_bike_resistance_v2).toBool();
    bool schwinn_bike_resistance_v3 =
        settings.value(QZSettings::schwinn_bike_resistance_v3, QZSettings::default_schwinn_bike_resistance_v3).toBool();

    if (settings.value(QZSettings::schwinn_bike_resistance, QZSettings::default_schwinn_bike_resistance).toBool() || schwinn_bike_resistance_v2 ||
        schwinn_bike_resistance_v3) {
        resistance = pelotonToBikeResistance(res);
    } else {
        resistance = res;
    }

    if(ResistanceFromFTMSAccessory.value() == 0) {
        if (qFabs(resistance - Resistance.value()) >=
            (double)settings.value(QZSettings::schwinn_resistance_smooth, QZSettings::default_schwinn_resistance_smooth)
                .toInt()) {
            Resistance = resistance;
            m_pelotonResistance = res;
        } else {
            // to calculate correctly the averages
            Resistance = Resistance.value();
            m_pelotonResistance = m_pelotonResistance.value();

            qDebug() << QStringLiteral("resistance not updated cause to schwinn_resistance_smooth setting");
        }
        emit resistanceRead(Resistance.value());
    } else {
        Resistance = ResistanceFromFTMSAccessory.value();
        m_pelotonResistance = bikeResistanceToPeloton(ResistanceFromFTMSAccessory.value());
    }

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

    emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current Peloton Resistance: ") + QString::number(m_pelotonResistance.value()));
    emit debug(QStringLiteral("Current Calculated Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void schwinnic4bike::stateChanged(QLowEnergyService::ServiceState state) {
    if (state != QLowEnergyService::ServiceDiscovered) {
        return;
    }

    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0x2AD2);
    gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);

    qDebug() << state;

    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);

    // i need to remove read request from QT framework in order to get Schwinn compatibility
    // QSharedPointer<QLowEnergyServicePrivate> qzService = gattCommunicationChannelService->d_ptr;
    // m_control->d_ptr->writeDescriptor(qzService, 0x30, 0x31, descriptor);

    gattCommunicationChannelService->writeDescriptor(
        gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);

    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
            &schwinnic4bike::characteristicChanged);

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

            int8_t bikeResistanceOffset =
                settings.value(QZSettings::bike_resistance_offset, QZSettings::default_bike_resistance_offset).toInt();
            double bikeResistanceGain =
                settings.value(QZSettings::bike_resistance_gain_f, QZSettings::default_bike_resistance_gain_f)
                    .toDouble();
            auto virtualBike =
                new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
            // connect(virtualBike,&virtualbike::debug ,this,&schwinnic4bike::debug);
            connect(virtualBike, &virtualbike::changeInclination, this, &schwinnic4bike::changeInclination);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void schwinnic4bike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void schwinnic4bike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void schwinnic4bike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {

    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void schwinnic4bike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void schwinnic4bike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    gattCommunicationChannelService = m_control->createServiceObject(QBluetoothUuid((quint16)0x1826));
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &schwinnic4bike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void schwinnic4bike::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("schwinnic4bike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void schwinnic4bike::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("schwinnic4bike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void schwinnic4bike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &schwinnic4bike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &schwinnic4bike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &schwinnic4bike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &schwinnic4bike::controllerStateChanged);

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

bool schwinnic4bike::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t schwinnic4bike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void schwinnic4bike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

resistance_t schwinnic4bike::pelotonToBikeResistance(int pelotonResistance) {
    QSettings settings;
    bool schwinn_bike_resistance_v2 =
        settings.value(QZSettings::schwinn_bike_resistance_v2, QZSettings::default_schwinn_bike_resistance_v2).toBool();
    bool schwinn_bike_resistance_v3 =
        settings.value(QZSettings::schwinn_bike_resistance_v3, QZSettings::default_schwinn_bike_resistance_v3).toBool();
    if (schwinn_bike_resistance_v3) {
        // y = -35,3 + 1,91x + -0,0358x^2 + 4,3E-04x^3
        if (pelotonResistance < 30)
           return 0;

        return -35.3 + 1.91 * pelotonResistance - 0.0358 * pow(pelotonResistance, 2) + 4.3E-04 * pow(pelotonResistance, 3);
    } else if (!schwinn_bike_resistance_v2) {
        if (pelotonResistance > 54)
            return pelotonResistance;
        if (pelotonResistance < 26)
            return pelotonResistance / 5;

        // y = 0,04x2 - 1,32x + 11,8
        return ((0.04 * pow(pelotonResistance, 2)) - (1.32 * pelotonResistance) + 11.8);
    } else {
        if (pelotonResistance > 20)
            return (((double)pelotonResistance - 20.0) * 1.25);
        else
            return 1;
    }
}

uint16_t schwinnic4bike::wattsFromResistance(double resistance) {
    QSettings settings;

    double ac = 0.01243107769;
    double bc = 1.145964912;
    double cc = -23.50977444;

    double ar = 0.1469553975;
    double br = -5.841344538;
    double cr = 97.62165482;

    for (uint16_t i = 1; i < 2000; i += 5) {
        double res =
            (((sqrt(pow(br, 2.0) -
                    4.0 * ar *
                        (cr - ((double)i * 132.0 / (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
               br) /
              (2.0 * ar)) *
             settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
            settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();

        if (!isnan(res) && res >= resistance) {
            return i;
        }
    }

    return 0;
}

void schwinnic4bike::resistanceFromFTMSAccessory(resistance_t res) {
    ResistanceFromFTMSAccessory = res;
    qDebug() << QStringLiteral("resistanceFromFTMSAccessory") << res;
}

double schwinnic4bike::bikeResistanceToPeloton(double bikeResistance) {
    // brute-force inverse of pelotonToBikeResistance with current settings
    double bestPeloton = 0;
    double bestDiff = std::numeric_limits<double>::max();
    for (int peloton = 0; peloton <= 100; peloton++) {
        resistance_t converted = pelotonToBikeResistance(peloton);
        double diff = qFabs((double)converted - bikeResistance);
        if (diff < bestDiff) {
            bestDiff = diff;
            bestPeloton = peloton;
        }
    }
    return bestPeloton;
}

/*
uint8_t schwinnic4bike::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << Cadence.value() << power;

    if (Cadence.value() == 0)
        return 1;

    for (int i = 1; i < max_resistance; i++) {
        if (wattsFromResistance(i) <= power && wattsFromResistance(i + 1) >= power) {
            resistance_t res = pelotonToBikeResistance(i);
            qDebug() << QStringLiteral("resistanceFromPowerRequest") << wattsFromResistance(i)
                     << wattsFromResistance(i + 1) << QStringLiteral("power=") << power << QStringLiteral("res=")
                     << res;
            // if the SS2K didn't send resistance at all or
            // only if the resistance requested is higher and the current wattage is lower than the target
            // only if the resistance requested is lower and the current wattage is higher than the target
            // the main issue about schwinn is that the formula to get the wattage from the resistance is not so good
            // so we need to put some constraint in the ERG mode
            if (ResistanceFromFTMSAccessory.value() == 0 ||
                ((power > m_watt.value() && res > (resistance_t)ResistanceFromFTMSAccessory.value()) ||
                 ((power < m_watt.value() && res < (resistance_t)ResistanceFromFTMSAccessory.value())))) {
                return res;
            } else {
                if (power > m_watt.value())
                    return ResistanceFromFTMSAccessory.value() + 1;
                else
                    return ResistanceFromFTMSAccessory.value() - 1;
            }
        }
    }
    if (power < wattsFromResistance(1))
        return 1;
    else
        return 1;
}
*/
