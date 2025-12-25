#include "inspirebike.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif

using namespace std::chrono_literals;
//#include <QtBluetooth/private/qlowenergycontrollerbase_p.h>
//#include <QtBluetooth/private/qlowenergyserviceprivate_p.h>

inspirebike::inspirebike(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    t_timeout = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    // initDone = false;
    connect(refresh, &QTimer::timeout, this, &inspirebike::update);
    connect(t_timeout, &QTimer::timeout, this, &inspirebike::connection_timeout);
    refresh->start(200ms);
}

/*void inspirebike::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool
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

void inspirebike::connection_timeout() {
    qDebug() << QStringLiteral("connection timeout triggered!");
    m_control->disconnectFromDevice();
}

void inspirebike::update() {
    qDebug() << m_control->state() << bluetoothDevice.isValid() << gattCommunicationChannelService <<
        // gattWriteCharacteristic.isValid() <<
        gattNotify1Characteristic.isValid() /*<<
        initDone*/
        ;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    /*if(initRequest)
    {
        initRequest = false;
        btinit();
    }
    else*/ if(bluetoothDevice.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState &&
       gattCommunicationChannelService &&
       //gattWriteCharacteristic.isValid() &&
       gattNotify1Characteristic.isValid()/* &&
       initDone*/)
    {
        update_metrics(true, watts());

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (requestResistance != -1) {
            if (requestResistance > max_resistance) {
                requestResistance = max_resistance;
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

void inspirebike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void inspirebike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    if (newValue.length() != 8) {
        return;
    }

    Resistance = newValue.at(6);
    emit resistanceRead(Resistance.value());
    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        Cadence = ((uint8_t)newValue.at(3));
    }
    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = 0.37497622 * ((double)Cadence.value());
    } else {
        Speed = metric::calculateSpeedFromPower(watts(),  Inclination.value(), Speed.value(),fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }
    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) * settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            now)))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                              //* 3.5) / 200 ) / 60
    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(now)));

    if (settings.value(QZSettings::inspire_peloton_formula2, QZSettings::default_inspire_peloton_formula2).toBool()) {
        // y = 0,0002x^3 - 0.1478x^2 + 4.2412x + 1.8102
        m_pelotonResistance = (((pow(Resistance.value(), 3) * 0.002) - (pow(Resistance.value(), 2) * 0.1478) +
                                (4.2412 * Resistance.value()) + 1.8102) *
                               settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                              settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
    } else if (settings.value(QZSettings::inspire_peloton_formula, QZSettings::default_inspire_peloton_formula).toBool()) {
        // y = 0.0014x^3 - 0.0796x^2 + 2.575x + 0.0444
        m_pelotonResistance = (((pow(Resistance.value(), 3) * 0.0014) - (pow(Resistance.value(), 2) * 0.0796) +
                                (2.575 * Resistance.value()) + 0.0444) *
                               settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                              settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
    } else {
        m_pelotonResistance = Resistance.value() * 2.5;
    }

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    lastRefreshCharacteristicChanged = now;

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            update_hr_from_external();
        }
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround = settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && cadence && h && firstStateChanged) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

    t_timeout->start(3s);

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void inspirebike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("7ee72ad1-6fd8-4cbd-b648-a2cdcfa4e7f4"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // qDebug() << gattNotify1Characteristic.descriptors();

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &inspirebike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &inspirebike::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &inspirebike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &inspirebike::descriptorWritten);

        // ******************************************* virtual bike init *************************************
        if (!firstStateChanged && !this->hasVirtualDevice()
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            && !h
#endif
#endif
        ) {
            QSettings settings;
            bool virtual_device_enabled = settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
            bool ios_peloton_workaround = settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
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
                // connect(virtualBike,&virtualbike::debug ,this,&inspirebike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &inspirebike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);

        // it doesn't work neither on android, always description write error
        // QSharedPointer<QLowEnergyServicePrivate> qzService = gattCommunicationChannelService->d_ptr;
        // m_control->d_ptr->writeDescriptor(qzService, 0x18, 0x1E, descriptor);

        emit connectedAndDiscovered();
    }
}

void inspirebike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));
}

void inspirebike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void inspirebike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("7ee7e1fa-6fd8-4cbd-b648-a2cdcfa4e7f4"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &inspirebike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void inspirebike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("inspirebike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void inspirebike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("inspirebike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void inspirebike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &inspirebike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &inspirebike::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &inspirebike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &inspirebike::controllerStateChanged);

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

bool inspirebike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t inspirebike::watts() {
    QSettings settings;
    if (currentCadence().value() == 0) {
        return 0;
    }

    if (settings.value(QZSettings::inspire_peloton_formula2, QZSettings::default_inspire_peloton_formula2).toBool()) {
        const double m[] = {0.6,  0.7, 0.75, 0.7, 0.9,  0.85, 0.85, 1,    1.1,  1.25, 1.5,  1.4,  1.4, 1.5,
                            1.5,  1.8, 1.75, 2,   2.15, 2.55, 2.65, 2.65, 2.85, 2.9,  3.25, 3.65, 3.4, 4.05,
                            4.05, 4.2, 4.25, 4.8, 5.35, 5.5,  6.1,  5.35, 5.95, 6.65, 7.25, 7.05};
        const double q[] = {
            -7.666666667, -13.33333333, -13.5,        -6,           -20,          -12.83333333, -7.5,   -16.66666667,
            -21.33333333, -29.16666667, -44.33333333, -31.66666667, -23.66666667, -27,          -20,    -40.33333333,
            -25.83333333, -38.66666667, -43.16666667, -72.5,        -71.5,        -65.5,        -68.5,  -62.66666667,
            -80.5,        -103.1666667, -66,          -109.8333333, -95.16666667, -95,          -79.5,  -111.6666667,
            -136.8333333, -132.3333333, -160,         -66.16666667, -93.5,        -131.5,       -149.5, -92.16666667};

        uint8_t res = qRound(currentResistance().value());
        if (res - 1 < max_resistance && res > 0) {
            double w = ((m[res - 1] * (double)(currentCadence().value())) + q[res - 1]);
            if (w < 0)
                w = 0;
            return (uint16_t)w;
        } else
            return 0;

    } else if (settings.value(QZSettings::inspire_peloton_formula, QZSettings::default_inspire_peloton_formula).toBool()) {
        return (uint16_t)(((3.59 * exp(0.0217 * (double)(currentCadence().value()))) *
                           exp(0.088 * (double)(currentResistance().value()))) /
                          2.2);
    } else {
        // https://github.com/cagnulein/qdomyos-zwift/issues/62#issuecomment-736913564
        if (currentCadence().value() < 90) {
            return (uint16_t)((3.59 * exp(0.0217 * (double)(currentCadence().value()))) *
                              exp(0.095 * (double)(currentResistance().value())));
        } else {
            return (uint16_t)((3.59 * exp(0.0217 * (double)(currentCadence().value()))) *
                              exp(0.088 * (double)(currentResistance().value())));
        }
    }
}

void inspirebike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        // initDone = false;
        m_control->connectToDevice();
    }
}
