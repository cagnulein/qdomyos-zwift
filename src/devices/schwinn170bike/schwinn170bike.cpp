#include "schwinn170bike.h"

#include "ios/lockscreen.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>

#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include <QLowEnergyConnectionParameters>
#endif
#include "keepawakehelper.h"
#include <chrono>

using namespace std::chrono_literals;

schwinn170bike::schwinn170bike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                               double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &schwinn170bike::update);
    refresh->start(200ms);
}

void schwinn170bike::writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic,
                                         uint8_t *data, uint8_t data_len, QString info, bool disable_log,
                                         bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    if (wait_for_response) {
        connect(service, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    } else {
        connect(service, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    service->writeCharacteristic(characteristic, *writeBuffer);

    if (!disable_log)
        debug(" >> " + writeBuffer->toHex(' ') + " // " + info);

    loop.exec();
}

void schwinn170bike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest) {

        initRequest = false;
    } else {

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

void schwinn170bike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void schwinn170bike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    double heart = 0.0;

    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << QStringLiteral(" << ") << newValue.toHex(' ') << characteristic.uuid();

    if (newValue.length() == 20) {
        Resistance = newValue.at(18);

        emit resistanceRead(Resistance.value());
        return;
    }

    if (newValue.length() == 17) {
        double cadence_gain = settings.value(QZSettings::cadence_gain, QZSettings::default_cadence_gain).toDouble();
        double cadence_offset = settings.value(QZSettings::cadence_offset, QZSettings::default_cadence_offset).toDouble();

        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            double current = ((double)(((uint16_t)((uint8_t)newValue.at(5)) << 8) | (uint16_t)((uint8_t)newValue.at(4))));
            if (current != lastCadenceValue) {
                QDateTime now = QDateTime::currentDateTime();
                double c = ((current - lastCadenceValue) / fabs(now.msecsTo(lastCadenceChanged))) * 60000.0;
                lastCadenceValue = current;
                lastCadenceChanged = now;
                if (c < 255) {
                    if (c > 0)
                        Cadence = (c * cadence_gain) + cadence_offset;
                    else
                        Cadence = 0;
                } else {
                    qDebug() << "Cadence too high" << c << "discarded!";
                }
            }
        }

        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

        return;
    }
    
    if (newValue.length() != 14)
        return;

    lastPacket = newValue;

    m_watt = ((double)(((uint16_t)((uint8_t)newValue.at(7)) << 8) | (uint16_t)((uint8_t)newValue.at(6)))) / 100.0;
    emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));

    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = ((double)(((uint16_t)((uint8_t)newValue.at(4)) << 8) | (uint16_t)((uint8_t)newValue.at(3)))) / 100.0;
    } else {
        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(),
            fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }
    emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(now)));

    emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

    if (watts())
        KCal += ((((0.048 * ((double)watts()) + 1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
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
    if (settings.value(QZSettings::schwinn_bike_resistance, QZSettings::default_schwinn_bike_resistance).toBool())
        resistance = pelotonToBikeResistance(res);
    else
        resistance = res;
    if (qFabs(resistance - Resistance.value()) >=
        (double)settings.value(QZSettings::schwinn_resistance_smooth, QZSettings::default_schwinn_resistance_smooth)
            .toInt()) {
        m_pelotonResistance = res;
    } else {
        // to calculate correctly the averages
        m_pelotonResistance = m_pelotonResistance.value();

        qDebug() << QStringLiteral("resistance not updated cause to schwinn_resistance_smooth setting");
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

    emit debug(QStringLiteral("Current Calculated Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void schwinn170bike::stateChanged(QLowEnergyService::ServiceState state) {
    QSettings settings;
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    if (state != QLowEnergyService::ServiceState::ServiceDiscovered) {
        qDebug() << QStringLiteral("ignoring this state");
        return;
    }

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &schwinn170bike::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &schwinn170bike::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &schwinn170bike::characteristicRead);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &schwinn170bike::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &schwinn170bike::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &schwinn170bike::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle();
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid() << QStringLiteral("handle") << d.handle();
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Indicate) ==
                           QLowEnergyCharacteristic::Indicate) {
                    QByteArray descriptor;
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("indication subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Read) == QLowEnergyCharacteristic::Read) {
                    // s->readCharacteristic(c);
                    // qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                } else if ((c.properties() & QLowEnergyCharacteristic::Write) == QLowEnergyCharacteristic::Write &&
                           (c.uuid() == QBluetoothUuid(QStringLiteral("5ec4e520-9804-11e3-b4b9-0002a5d5c51b")) ||
                            c.uuid() == QBluetoothUuid(QStringLiteral("1717b3c0-9803-11e3-90e1-0002a5d5c51b")))) {
                    qDebug() << s->serviceUuid() << c.uuid() << "writing!";
                    uint8_t init[] = {0x07, 0x01, 0xd3, 0x00, 0x1f, 0x05, 0x01};
                    writeCharacteristic(s, c, init, sizeof(init), "init");
                } else if ((c.properties() & QLowEnergyCharacteristic::WriteNoResponse) ==
                           QLowEnergyCharacteristic::WriteNoResponse) {
                    qDebug() << s->serviceUuid() << c.uuid() << "writing!";
                    uint8_t init[] = {0x07, 0x01, 0xd3, 0x00, 0x1f, 0x05, 0x01};
                    writeCharacteristic(s, c, init, sizeof(init), "init");
                }
            }
        }
    }

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
            // connect(virtualBike,&virtualbike::debug ,this,&ftmsbike::debug);
            connect(virtualBike, &virtualbike::changeInclination, this, &schwinn170bike::changeInclination);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void schwinn170bike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void schwinn170bike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void schwinn170bike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {

    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void schwinn170bike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void schwinn170bike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    initRequest = false;
    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &schwinn170bike::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
    }
}

void schwinn170bike::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("schwinn170bike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void schwinn170bike::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("schwinn170bike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void schwinn170bike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &schwinn170bike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &schwinn170bike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &schwinn170bike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &schwinn170bike::controllerStateChanged);

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

bool schwinn170bike::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t schwinn170bike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void schwinn170bike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

resistance_t schwinn170bike::pelotonToBikeResistance(int pelotonResistance) {
    QSettings settings;
    bool schwinn_bike_resistance_v2 =
        settings.value(QZSettings::schwinn_bike_resistance_v2, QZSettings::default_schwinn_bike_resistance_v2).toBool();
    if (!schwinn_bike_resistance_v2) {
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

uint16_t schwinn170bike::wattsFromResistance(double resistance) {
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

void schwinn170bike::resistanceFromFTMSAccessory(resistance_t res) {
    ResistanceFromFTMSAccessory = res;
    qDebug() << QStringLiteral("resistanceFromFTMSAccessory") << res;
}

/*
uint8_t schwinn170bike::resistanceFromPowerRequest(uint16_t power) {
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
