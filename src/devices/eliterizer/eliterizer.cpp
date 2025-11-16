#include "eliterizer.h"
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

#include <chrono>

using namespace std::chrono_literals;

eliterizer::eliterizer(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &eliterizer::update);
    refresh->start(200ms);
}

void eliterizer::autoResistanceChanged(bool value) { Q_UNUSED(value); }

void eliterizer::changeInclinationRequested(double grade, double percentage) {
    Q_UNUSED(grade);
    uint8_t incline[] = {0x0a, 0x00, 0x00};
    QSettings settings;
    double gain = settings.value(QZSettings::elite_rizer_gain, QZSettings::default_elite_rizer_gain).toDouble();
    percentage = percentage * gain;

    incline[1] = ((int16_t)(percentage * 10.0)) & 0xff;
    incline[2] = (((int16_t)(percentage * 10.0)) >> 8) & 0xff;
    writeCharacteristic(incline, sizeof(incline),
                        QStringLiteral("changeInclinationRequested ") + QString::number(percentage), false, true);
}

void eliterizer::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                     bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    if (wait_for_response) {
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    } else {
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
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
}

void eliterizer::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest) {

    } else if (bluetoothDevice.isValid() &&
               m_control->state() == QLowEnergyController::DiscoveredState //&&
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
    }
}

void eliterizer::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void eliterizer::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {

    emit debug(QStringLiteral(" << ") + characteristic.uuid().toString() + QStringLiteral(" ") + newValue.toHex(' '));

    lastPacket = newValue;

    if (characteristic == gattNotifyCharacteristic) {
        if (newValue.length() >= 3) {
            uint8_t response = newValue.at(2);
            switch (response) {
            case 0:
                qDebug() << "Wrong parameters command";
                break;
            case 1:
                qDebug() << "Out of range";
                break;
            case 2:
                qDebug() << "Success";
                break;
            case 3:
                qDebug() << "Wrong";
                break;
            case 4:
                qDebug() << "Fail";
                break;
            case 5:
                qDebug() << "Command not accessible";
                break;
            }
        }
    } else if (characteristic == gattNotify3Characteristic && newValue.length() >= 4) {
        Inclination = (((double)newValue.at(1)) - 100.0) / 2;
        qDebug() << QStringLiteral("Rizer Current Inclination: ") << Inclination.value();
    } else {
        if (newValue.length() >= 4) {
            const float *ptrFloat = reinterpret_cast<const float *>(newValue.constData());
            emit debug(QStringLiteral("Steering Angle: ") + QString::number(*ptrFloat) + "°");
            m_steeringAngle = *ptrFloat;
            emit steeringAngleChanged(m_steeringAngle.value());
        }
    }
}

void eliterizer::stateChanged(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("347b0020-7635-408b-8918-8ff3949ce592"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("347b0021-7635-408b-8918-8ff3949ce592"));
    QBluetoothUuid _gattNotify2CharacteristicId(QStringLiteral("347b0030-7635-408b-8918-8ff3949ce592"));
    QBluetoothUuid _gattNotify3CharacteristicId(QStringLiteral("347b0022-7635-408b-8918-8ff3949ce592"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            qDebug() << QStringLiteral("char uuid") << c.uuid();
            auto descriptors_list = c.descriptors();
            for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                qDebug() << QStringLiteral("descriptor uuid") << d.uuid();
            }
        }

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);
        gattNotify3Characteristic = gattCommunicationChannelService->characteristic(_gattNotify3CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());
        Q_ASSERT(gattNotify2Characteristic.isValid());
        Q_ASSERT(gattNotify3Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)),
                this, SLOT(characteristicChanged(QLowEnergyCharacteristic, QByteArray)));
        connect(gattCommunicationChannelService,
                SIGNAL(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)), this,
                SLOT(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(error(QLowEnergyService::ServiceError)), this,
                SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelService,
                SIGNAL(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)), this,
                SLOT(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)));

        firstStateChanged = 1;

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify2Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify3Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void eliterizer::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void eliterizer::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void eliterizer::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {

    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void eliterizer::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void eliterizer::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("347b0001-7635-408b-8918-8ff3949ce592"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this,
            SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void eliterizer::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("eliterizer::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void eliterizer::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("eliterizer::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void eliterizer::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &eliterizer::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &eliterizer::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &eliterizer::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &eliterizer::controllerStateChanged);

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

bool eliterizer::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t eliterizer::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void eliterizer::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}
