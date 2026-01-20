#include "sportsplusrower.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualrower.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

sportsplusrower::sportsplusrower(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &sportsplusrower::update);
    refresh->start(200ms);
}

void sportsplusrower::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                         bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &sportsplusrower::packetReceived, &loop, &QEventLoop::quit);
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

void sportsplusrower::forceResistance(resistance_t requestResistance) {
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

void sportsplusrower::update() {
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
        uint8_t noOpData[] = {0x20, 0x01, 0x09, 0x00, 0x2a};
        if (requestResistance < 0) {
            requestResistance = 0;
        }
        if (requestResistance > max_resistance) {
            requestResistance = max_resistance;
        }
        noOpData[2] = requestResistance;
        noOpData[4] = (0x21 + requestResistance);
        writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
    }
}

void sportsplusrower::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void sportsplusrower::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    emit packetReceived();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;
    if (newValue.length() != 12) {
        return;
    }

    double cadence = 0;
    double kcal = 0;
    bool cadence_eval = false;

    m_watt = ((newValue.at(10) >> 4) * 10) + (newValue.at(10) & 0x0F);
    emit debug(QStringLiteral("Current watt: ") + QString::number(m_watt.value()));

    if(newValue.at(1) == 0x10) {
        Cadence = ((newValue.at(3) >> 4) * 10) + (newValue.at(3) & 0x0F);
        Speed = 0.37497622 * ((double)Cadence.value());
        emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
    }

    if (!firstCharChanged) {
        Distance +=
            ((Speed.value() / 3600.0) / (1000.0 / (lastTimeCharChanged.msecsTo(now))));
    }

    lastTimeCharChanged = now;
    kcal +=
        ((((0.048 * ((double)watts()) + 1.19) *
           settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
          200.0) /
         (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                        now)))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                          //* 3.5) / 200 ) / 60

    lastRefreshCharacteristicChanged = now;

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            Heart = ((uint8_t)newValue.at(8));
        }
    }
    FanSpeed = 0;

    emit debug(QStringLiteral("Current cadence: ") + QString::number(Cadence.value()));
    // emit debug(QStringLiteral("Current resistance: ") + QString::number(resistance));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    Resistance = requestResistance;
    KCal = kcal;
    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        if(cadence_eval)
            Cadence = cadence;
    }
    firstCharChanged = false;
}

uint16_t sportsplusrower::GetElapsedFromPacket(const QByteArray &packet) {
    uint16_t convertedDataSec = (packet.at(4));
    uint16_t convertedDataMin = (packet.at(3));
    uint16_t convertedData = convertedDataMin * 60.f + convertedDataSec;
    return convertedData;
}

double sportsplusrower::GetKcalFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(6) << 8) | ((uint8_t)packet.at(7));
    return (double)(convertedData);
}

double sportsplusrower::GetWattFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(2) << 8) | ((uint8_t)packet.at(3));
    double data = ((double)(convertedData));
    return data;
}

void sportsplusrower::btinit(bool startTape) {
    Q_UNUSED(startTape);

    if(XM_FITNESS) {
        const uint8_t initData1[] = {0x40, 0x00, 0xd1, 0x5f, 0x70};    
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    } else {
        const uint8_t initData1[] = {0x40, 0x00, 0x9a, 0x56, 0x30};    
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    }

    initDone = true;
}

void sportsplusrower::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            emit debug(QStringLiteral("characteristic ") + c.uuid().toString());
        }

        //        QString uuidWrite = "0000fff2-0000-1000-8000-00805f9b34fb";
        //        QString uuidNotify1 = "0000fff1-0000-1000-8000-00805f9b34fb";

        // QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("0000fff2-0000-1000-8000-00805f9b34fb"));
        QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("0000fff1-0000-1000-8000-00805f9b34fb"));
        QBluetoothUuid _gattNotify2CharacteristicId(QStringLiteral("0000fff2-0000-1000-8000-00805f9b34fb"));
        QBluetoothUuid _gattNotify2AltCharacteristicId(QStringLiteral("0000ffe2-0000-1000-8000-00805f9b34fb"));
        QBluetoothUuid _gattNotify3CharacteristicId(QStringLiteral("0000fff3-0000-1000-8000-00805f9b34fb"));

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);
        // Some devices use ffe2 instead of fff2
        if (!gattWriteCharacteristic.isValid()) {
            XM_FITNESS = true;
            gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattNotify2AltCharacteristicId);
        }
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);
        if (!gattNotify2Characteristic.isValid()) {
            gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2AltCharacteristicId);
        }
        gattNotify3Characteristic = gattCommunicationChannelService->characteristic(_gattNotify3CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &sportsplusrower::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &sportsplusrower::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &sportsplusrower::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &sportsplusrower::descriptorWritten);

        // ******************************************* virtual bike/rower init *************************************
        if (!firstVirtualBike && !this->hasVirtualDevice()) {
            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_rower =
                settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_rower) {
                    emit debug(QStringLiteral("creating virtual bike interface..."));
                    auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                    // connect(virtualBike,&virtualbike::debug ,this,&sportsplusrower::debug);
                    connect(virtualBike, &virtualbike::changeInclination, this, &sportsplusrower::changeInclination);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    emit debug(QStringLiteral("creating virtual rower interface..."));
                    auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                    // connect(virtualRower,&virtualrower::debug ,this,&sportsplusrower::debug);
                    this->setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::PRIMARY);
                }
            }
        }
        firstVirtualBike = 1;
        // ********************************************************************************************************

        qDebug() << "gattNotify2Characteristic" << gattNotify2Characteristic.isValid()
                 << gattNotify2Characteristic.properties();
        qDebug() << "gattNotify3Characteristic" << gattNotify3Characteristic.isValid()
                 << gattNotify3Characteristic.properties();

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
        if (gattNotify2Characteristic.isValid() &&
            (gattNotify2Characteristic.properties() & QLowEnergyCharacteristic::Notify) ==
                QLowEnergyCharacteristic::Notify) {
            gattCommunicationChannelService->writeDescriptor(
                gattNotify2Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
        }
        if (gattNotify3Characteristic.isValid() &&
            (gattNotify3Characteristic.properties() & QLowEnergyCharacteristic::Notify) ==
                QLowEnergyCharacteristic::Notify)
            gattCommunicationChannelService->writeDescriptor(
                gattNotify3Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void sportsplusrower::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void sportsplusrower::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void sportsplusrower::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    // QString uuid = "0000fff0-0000-1000-8000-00805f9b34fb";

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb"));
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if (gattCommunicationChannelService == nullptr) {
        qDebug() << QStringLiteral("invalid service") << _gattCommunicationChannelServiceId.toString();
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &sportsplusrower::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void sportsplusrower::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("sportsplusrower::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void sportsplusrower::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("sportsplusrower::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void sportsplusrower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;
        requestResistance = 1;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &sportsplusrower::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &sportsplusrower::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &sportsplusrower::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &sportsplusrower::controllerStateChanged);

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

uint16_t sportsplusrower::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

bool sportsplusrower::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void sportsplusrower::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
