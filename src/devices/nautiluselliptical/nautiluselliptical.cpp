#include "nautiluselliptical.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>

#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

nautiluselliptical::nautiluselliptical(bool noWriteResistance, bool noHeartService, bool testResistance,
                                       int8_t bikeResistanceOffset, double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);

    this->testResistance = testResistance;
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    initDone = false;
    connect(refresh, &QTimer::timeout, this, &nautiluselliptical::update);
    refresh->start(300ms);
}

nautiluselliptical::~nautiluselliptical() { qDebug() << QStringLiteral("~nautiluselliptical()"); }

void nautiluselliptical::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
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
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

/*
void nautiluselliptical::forceResistanceAndInclination(int8_t requestResistance, uint8_t inclination) {

    // TODO do the level down command
    uint8_t write[] = {0x5b, 0x04, 0x00, 0xf1, 0x4f, 0x4b, 0x5d};
    uint8_t writeUp[] = {0x5b, 0x02, 0xf1, 0x02, 0x5d};

    uint8_t writeDown[] = {0x5b, 0x02, 0xf1, 0x03, 0x5d};

    if (currentResistance().value() < requestResistance) {
        writeCharacteristic(write, sizeof(write),
                            QStringLiteral("forceResistance ") + QString::number(requestResistance) +
                                QStringLiteral(" Inclination ") + inclination,
                            false, true);
        writeCharacteristic(writeUp, sizeof(writeUp),
                            QStringLiteral("forceResistance ") + QString::number(requestResistance) +
                                QStringLiteral(" Inclination ") + inclination,
                            false, true);
    } else if (currentResistance().value() > requestResistance) {
        writeCharacteristic(writeDown, sizeof(writeDown),
                            QStringLiteral("forceResistance ") + QString::number(requestResistance) +
                                QStringLiteral(" Inclination ") + inclination,
                            false, true);
        writeCharacteristic(write, sizeof(write),
                            QStringLiteral("forceResistance ") + QString::number(requestResistance) +
                                QStringLiteral(" Inclination ") + inclination,
                            false, true);
    }
}
*/

void nautiluselliptical::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void nautiluselliptical::update() {

    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest) {

        initRequest = false;
        btinit(false);
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && gattNotify2Characteristic.isValid() && initDone) {

        update_metrics(true, watts());

        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstVirtual && searchStopped && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadmill, &virtualtreadmill::debug, this, &nautiluselliptical::debug);
                    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                            &nautiluselliptical::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &nautiluselliptical::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstVirtual = 1;
            }
        }
        // ********************************************************************************************************

        // updating the treadmill console every second
        if (sec1Update++ == (1000 / refresh->interval())) {

            sec1Update = 0;
        }

        if (testResistance) {
            if ((((int)elapsed.value()) % 5) == 0) {

                uint8_t new_res = currentResistance().value() + 1;
                if (new_res > 15) {
                    new_res = 1;
                }
                // forceResistanceAndInclination(new_res, currentInclination().value());
            }
        }

        // Resistance as incline on Sole E95s Elliptical #419
        if (requestInclination != -100)
            requestResistance = requestInclination;

        if (requestResistance != -1) {
            if (requestResistance > 20) {
                requestResistance = 20;
            } else if (requestResistance == 0) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));

                // forceResistanceAndInclination(requestResistance, currentInclination().value());
            }
            requestResistance = -1;
        } else if (requestInclination != -100) {
            if (requestInclination > 15) {
                requestInclination = 15;
            } else if (requestInclination == 0) {
                requestInclination = 1;
            }

            if (requestInclination != currentInclination().value()) {
                emit debug(QStringLiteral("writing inclination ") + QString::number(requestInclination));

                // forceResistanceAndInclination(currentResistance().value(), requestInclination);
            }
            requestInclination = -100;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));

            btinit(true);

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

void nautiluselliptical::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void nautiluselliptical::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                               const QByteArray &newValue) {

    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    if (newValue.length() == 20) {

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            uint8_t heart = ((uint8_t)newValue.at(16));
            if (heartRateBeltName.startsWith(QStringLiteral("Disabled")) && heart != 0) {
                Heart = heart;
            }
        }

        Resistance = newValue.at(18);
        emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        return;
    }

    if ((newValue.length() != 14 && (bt_variant == 0 || bt_variant == 2)) || (newValue.length() != 12 && bt_variant == 1)) {
        return;
    }

    double speed =
        GetSpeedFromPacket(newValue) *
        settings.value(QZSettings::domyos_elliptical_speed_ratio, QZSettings::default_domyos_elliptical_speed_ratio)
            .toDouble();
    if (watts())
        KCal += ((((0.048 * ((double)watts()) + 1.19) * weight * 3.5) / 200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
    // double kcal = GetKcalFromPacket(newValue);
    // double distance = GetDistanceFromPacket(newValue) *
    // settings.value(QZSettings::domyos_elliptical_speed_ratio,
    // QZSettings::default_domyos_elliptical_speed_ratio).toDouble(); uint16_t watt = (newValue.at(13) << 8) |
    // newValue.at(14);

    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
                if(bt_variant == 2) {
                    Cadence = ((uint8_t)newValue.at(1));
                } else {
                    Cadence = ((uint8_t)newValue.at(5));
                }
    }
    // m_watt = watt;
    Speed = speed;

    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

    CrankRevs++;
    LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current cadence: ") + QString::number(Cadence.value()));
    emit debug(QStringLiteral("Current inclination: ") + QString::number(Inclination.value()));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
    emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

double nautiluselliptical::GetSpeedFromPacket(const QByteArray &packet) {

    uint16_t convertedData = 0;
    double data = 0;
    if (bt_variant == 0 || bt_variant == 2) {
        convertedData = (packet.at(4) << 8) | packet.at(3);
        data = (double)convertedData / 100.0f;
    } else {
        convertedData = (packet.at(8) << 8) | packet.at(7);
        data = (double)convertedData / 10.0f;
    }

    return data;
}

double nautiluselliptical::GetKcalFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(7) << 8) | ((uint8_t)packet.at(8));
    return (double)convertedData / 10.0;
}

double nautiluselliptical::GetDistanceFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

void nautiluselliptical::btinit(bool startTape) {

    QSettings settings;
    Q_UNUSED(startTape)

    uint8_t initData1[] = {0x07, 0x01, 0xd3, 0x00, 0x1f, 0x05, 0x01};

    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"));

    initDone = true;
}

void nautiluselliptical::stateChanged(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("1717b3c0-9803-11e3-90e1-0002a5d5c51b"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("a46a4a80-9803-11e3-8f3c-0002a5d5c51b"));
    QBluetoothUuid _gattNotify2CharacteristicId(QStringLiteral("6be8f580-9803-11e3-ab03-0002a5d5c51b"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {

        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle();
            auto descriptors_list = c.descriptors();
            for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                qDebug() << QStringLiteral("descriptor uuid") << d.uuid() << QStringLiteral("handle") << d.handle();
            }
        }

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);

        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());
        Q_ASSERT(gattNotify2Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &nautiluselliptical::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &nautiluselliptical::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &nautiluselliptical::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &nautiluselliptical::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);

        gattCommunicationChannelService->writeDescriptor(
            gattNotify2Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void nautiluselliptical::searchingStop() { searchStopped = true; }

void nautiluselliptical::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void nautiluselliptical::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                               const QByteArray &newValue) {

    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void nautiluselliptical::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("3a1a1d3f-3d83-4c43-aa7b-81664ed75ec8"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if (gattCommunicationChannelService == nullptr) {
        qDebug() << QStringLiteral("main UUID not found, trying the fallback...");
        bt_variant = 1;
        QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("ac8f2400-9804-11e3-b25b-0002a5d5c51b"));

        gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
        if (gattCommunicationChannelService == nullptr) {
            qDebug() << QStringLiteral("backup UUID not found, trying the 2nd fallback...");
            bt_variant = 1;
            QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("b6492080-7f04-11e4-a8b1-0002a5d5c51b"));

            gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
            if (gattCommunicationChannelService == nullptr) {
                qDebug() << QStringLiteral("backup UUID not found, trying the 3rd fallback...");
                bt_variant = 1;
                QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("219fbae0-9df6-11e5-a87d-0002a5d5c51b"));

                gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
                if (gattCommunicationChannelService == nullptr) {
                    if (gattCommunicationChannelService == nullptr) {
                        qDebug() << QStringLiteral("backup UUID not found, trying the 4th fallback...");
                        bt_variant = 2;
                        QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("7DF7A3F7-F013-492F-A58E-68A8F078AB96"));

                        gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
                        if (gattCommunicationChannelService == nullptr) {
                            qDebug() << QStringLiteral("neither the fallback worked, exiting...");
                            return;
                        }
                    }
                }
            }
        }
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &nautiluselliptical::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void nautiluselliptical::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("nautiluselliptical::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void nautiluselliptical::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("nautiluselliptical::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void nautiluselliptical::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');

    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &nautiluselliptical::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &nautiluselliptical::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &nautiluselliptical::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &nautiluselliptical::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    emit debug(QStringLiteral("Cannot connect to remote device."));
                    searchStopped = false;
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
            searchStopped = false;
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool nautiluselliptical::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void nautiluselliptical::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}
