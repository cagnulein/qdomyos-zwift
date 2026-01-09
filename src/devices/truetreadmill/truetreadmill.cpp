#include "truetreadmill.h"
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

truetreadmill::truetreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                             double forceInitInclination) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    if (forceInitSpeed > 0) {
        lastSpeed = forceInitSpeed;
    }

    if (forceInitInclination > 0) {
        lastInclination = forceInitInclination;
    }

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &truetreadmill::update);
    refresh->start(pollDeviceTime);
}

void truetreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    if (grade < 0)
        grade = 0;
    changeInclination(grade, percentage);
}

void truetreadmill::update() {

    if (!m_control)
        return;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initDone = true;
        initRequest = false;
    } else if (/*bluetoothDevice.isValid() &&*/
               m_control->state() == QLowEnergyController::DiscoveredState && gattCommunicationChannelService &&
               gattNotifyCharacteristic.isValid() && initDone) {

        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstInit && !this->hasVirtualDevice()) {
            bool virtual_device_enabled = settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike = settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike).toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadMill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &truetreadmill::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &truetreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &truetreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        // debug("Domyos Treadmill RSSI " + QString::number(bluetoothDevice.rssi()));

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // updating the treadmill console every second
        if (sec1Update++ >= (1000 / refresh->interval())) {
        }

        {
            if (requestSpeed != -1) {
                if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                    emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                }
                requestSpeed = -1;
            }
            if (requestInclination != -100) {
                if (requestInclination < 0)
                    requestInclination = 0;
                // only 0.5 steps ara available
                requestInclination = qRound(requestInclination * 2.0) / 2.0;
                if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                    requestInclination <= 15) {
                    emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                }
                requestInclination = -100;
            }
            if (requestStart != -1) {
                emit debug(QStringLiteral("starting..."));
                if (lastSpeed == 0.0) {

                    lastSpeed = 0.5;
                }
                requestStart = -1;
                emit tapeStarted();
            }
            if (requestStop != -1) {
                emit debug(QStringLiteral("stopping..."));
                requestStop = -1;
            }
            if (requestFanSpeed != -1) {
                emit debug(QStringLiteral("changing fan speed..."));
                requestFanSpeed = -1;
            }
            if (requestIncreaseFan != -1) {
                emit debug(QStringLiteral("increasing fan speed..."));
                requestIncreaseFan = -1;
            } else if (requestDecreaseFan != -1) {
                emit debug(QStringLiteral("decreasing fan speed..."));
                requestDecreaseFan = -1;
            }
        }
    }
}

void truetreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void truetreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    Q_UNUSED(characteristic);
    QByteArray avalue = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(newValue.length()) + QStringLiteral(" ") + newValue.toHex(' '));

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {

            uint8_t heart = 0;
            if (heart == 0 || disable_hr_frommachinery) {
                update_hr_from_external();
            } else {
                Heart = heart;
            }
        }
    }

    cadenceFromAppleWatch();

    double speed = Speed.value();

    if (wdway_treadmill) {
        if (avalue.length() == 18) {
            uint16_t convertedData = (avalue.at(7) << 8) | ((uint8_t)avalue.at(6));
            speed = ((double)convertedData) / 100.0;

            double incline = ((double)(avalue.at(16))) / 10.0;

            if (Inclination.value() != incline) {

                emit inclinationChanged(0, incline);
            }
            Inclination = incline;
            emit debug(QStringLiteral("Current incline: ") + QString::number(incline));            
        } else {
            return;
        }
    } else if (assault_treadmill) {
        if (avalue.length() == 16) {
            uint16_t convertedData = (avalue.at(6) << 8) | ((uint8_t)avalue.at(5));
            speed = ((double)convertedData) / 100.0;
        } else {
            return;
        }
    } else {
        if (avalue.length() == 16) {
            uint16_t convertedData = (avalue.at(7) << 8) | ((uint8_t)avalue.at(6));
            speed = ((double)convertedData) / 100.0;
            double incline = ((double)(avalue.at(14))) / 10.0;

            if (Inclination.value() != incline) {

                emit inclinationChanged(0, incline);
            }
            Inclination = incline;
            emit debug(QStringLiteral("Current incline: ") + QString::number(incline));

        } else if (avalue.length() == 19) {
            uint16_t convertedData = (avalue.at(8) << 8) | ((uint8_t)avalue.at(7));
            speed = ((double)convertedData) / 100.0;
        } else if (avalue.length() == 4) {
            double incline = ((double)(avalue.at(2))) / 10.0;
            if (Inclination.value() != incline) {

                emit inclinationChanged(0, incline);
            }
            Inclination = incline;
            emit debug(QStringLiteral("Current incline: ") + QString::number(incline));
            return;
        }
    }

    if (!firstCharacteristicChanged) {
        if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                    1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
        Distance += ((speed / (double)3600.0) /
                     ((double)1000.0 / (double)(lastTimeCharacteristicChanged.msecsTo(now))));
        lastTimeCharacteristicChanged = now;
    }

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    if (Speed.value() != speed) {

        emit speedChanged(speed);
    }
    Speed = speed;

    if (speed > 0) {

        lastSpeed = speed;
        lastInclination = Inclination.value();
    }

    firstCharacteristicChanged = false;
}

void truetreadmill::stateChanged(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattNotifyCharacteristicId(QStringLiteral("a026e01d-0a7d-4ab3-97fa-f1500f9feb8b"));
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::RemoteServiceDiscovered) {

        // qDebug() << gattCommunicationChannelService->characteristics();

        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &truetreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &truetreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &truetreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &truetreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void truetreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void truetreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void truetreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));
    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("a026ee07-0a7d-4ab3-97fa-f1500f9feb8b"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &truetreadmill::stateChanged);
    if(gattCommunicationChannelService) {
        gattCommunicationChannelService->discoverDetails();
    } else {
        qDebug() << QStringLiteral("ERROR! gattCommunicationChannelService is NULL");
    }
}

void truetreadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("truetreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void truetreadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("truetreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void truetreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    {

        bluetoothDevice = device;
        if (device.name().toUpper().startsWith(QStringLiteral("ASSAULT TREADMILL "))) {
            assault_treadmill = true;
            qDebug() << QStringLiteral("ASSAULT TREADMILL enabled!");
        } else if (device.name().toUpper().startsWith(QStringLiteral("WDWAY")) && device.name().length() == 8) {
            wdway_treadmill = true;
            qDebug() << QStringLiteral("WDWAY TREADMILL workaround ON!");
        }
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &truetreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &truetreadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &truetreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &truetreadmill::controllerStateChanged);

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

void truetreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

bool truetreadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

