#include "smartspin2k.h"
#include "devices/ftmsbike/ftmsbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>

#include <QNetworkDatagram>
#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QLowEnergyConnectionParameters>
#endif

#include <chrono>

using namespace std::chrono_literals;

smartspin2k::smartspin2k(bool noWriteResistance, bool noHeartService, resistance_t max_resistance, bike *parentDevice) {
    QSettings settings;
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->max_resistance = max_resistance;
    this->parentDevice = parentDevice;
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;

    calibrateShiftStep();

    initDone = false;
    bool r = udpSocket->bind(QHostAddress::Any, 10000);
    if (!r)
        qDebug() << "SS2K UDP Socket Failed!";
    connect(udpSocket, &QUdpSocket::readyRead, this, &smartspin2k::readPendingDatagrams);
    connect(refresh, &QTimer::timeout, this, &smartspin2k::update);
    refresh->start(200ms);
}

void smartspin2k::readPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        qDebug() << QStringLiteral("SS2K UDP LOG << ") << datagram.data();
    }
}

void smartspin2k::autoResistanceChanged(bool value) {
    if (value) {
        qDebug() << "ehi mark, i'm doing your stuff :) resetting everything for SS2K";
        lowInit(Resistance.value());
    }
}

void smartspin2k::setShiftStep(uint16_t steps) {
    uint8_t shiftStep[] = {0x02, 0x08, 0x00, 0x00};
    shiftStep[2] = (uint8_t)(steps & 0xFF);
    shiftStep[3] = (uint8_t)(steps >> 8);
    writeCharacteristic(shiftStep, sizeof(shiftStep), "BLE_setShiftStep", false, true);
}

void smartspin2k::lowInit(resistance_t resistance) {
    uint8_t enable_syncmode[] = {0x02, 0x1B, 0x01};
    uint8_t disable_syncmode[] = {0x02, 0x1B, 0x00};
    writeCharacteristic(enable_syncmode, sizeof(enable_syncmode), "BLE_syncMode enabling", false, true);
    forceResistance(resistance);
    QThread::sleep(2);
    writeCharacteristic(disable_syncmode, sizeof(disable_syncmode), "BLE_syncMode disabling", false, true);

    uint8_t simulate_watt[] = {0x02, 0x0E, 0x01};
    uint8_t simulate_cad[] = {0x02, 0x0F, 0x01};
    uint8_t simulate_hr[] = {0x02, 0x0D, 0x01};

    writeCharacteristic(simulate_watt, sizeof(simulate_watt), "simulate_watt", false, true);
    writeCharacteristic(simulate_cad, sizeof(simulate_cad), "simulate_cad", false, true);
    writeCharacteristic(simulate_hr, sizeof(simulate_hr), "simulate_hr", false, true);
}

void smartspin2k::resistanceReadFromTheBike(resistance_t resistance) {

    qDebug() << "resistanceReadFromTheBike startupResistance:" << startupResistance << "initRequest:" << initRequest;
    if (startupResistance == -1) {
        startupResistance = resistance;
    }
    if (initRequest && first) {

        first = false;
        lowInit(startupResistance);
        initRequest = false;
    }
    Resistance = resistance;
}

void smartspin2k::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                      bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (gattCommunicationChannelService == nullptr || gattCommunicationChannelService == nullptr) {
        qDebug() << "pointer no valid" << gattCommunicationChannelService << gattCommunicationChannelService;
        return;
    }

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
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void smartspin2k::writeCharacteristicFTMS(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                          bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (!gattWriteCharControlPointId.isValid()) {
        qDebug() << QStringLiteral("gattWriteCharControlPointId is not valid");
        return;
    }

    if (wait_for_response) {
        connect(gattCommunicationChannelServiceFTMS,
                SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    } else {
        connect(gattCommunicationChannelServiceFTMS,
                SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelServiceFTMS->writeCharacteristic(gattWriteCharControlPointId, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void smartspin2k::calibrateShiftStep() {
    QSettings settings;
    double x[max_calibration_samples], y[max_calibration_samples];
    uint8_t nSamples = 0;

    for (int i = 0; i < max_calibration_samples; ++i) {
        x[nSamples] = settings.value(QStringLiteral("ss2k_resistance_sample_") + QString::number(i + 1)).toDouble();
        y[nSamples] = settings.value(QStringLiteral("ss2k_shiftstep_sample_") + QString::number(i + 1)).toDouble();
        if (x[nSamples] > 0 && y[nSamples] > 0) {
            ++nSamples;
        }
    }

    if (nSamples == 0) {
        slope = 0;
        intercept = settings.value(QZSettings::ss2k_shift_step, QZSettings::default_ss2k_shift_step).toUInt();
        return;
    } else if (nSamples == 1) {
        slope = 0;
        intercept = y[0];
    } else {
        // calculate slope and intercept using least squares regression
        double xsum = 0, ysum = 0, x2sum = 0, xysum = 0;

        for (int8_t i = 0; i < nSamples; i++) {
            xsum += x[i];
            ysum += y[i];
            x2sum += x[i] * x[i];
            xysum += x[i] * y[i];
        }

        slope = (nSamples * xysum - xsum * ysum) / (nSamples * x2sum - xsum * xsum);
        intercept = (x2sum * ysum - xsum * xysum) / (x2sum * nSamples - xsum * xsum);
    }
    emit debug(QStringLiteral("Calibrating SS2K:  slope=") + QString::number(slope) + QStringLiteral(" intercept=") +
               QString::number(intercept));
}

void smartspin2k::forceResistance(resistance_t requestResistance) {

    QSettings settings;

    double ss2k_min_resistance =
        settings.value(QZSettings::ss2k_min_resistance, QZSettings::default_ss2k_min_resistance).toDouble();
    double ss2k_max_resistance =
        settings.value(QZSettings::ss2k_max_resistance, QZSettings::default_ss2k_max_resistance).toDouble();

    if (requestResistance > ss2k_max_resistance)
        requestResistance = ss2k_max_resistance;
    if (requestResistance < ss2k_min_resistance)
        requestResistance = ss2k_min_resistance;

    // if not calibrated, slope=0 and intercept is the configured shift step
    uint16_t steps = slope * requestResistance + intercept;

    setShiftStep(steps);

    lastRequestResistance = requestResistance;

    uint8_t write[] = {0x02, 0x17, 0x00, 0x00};
    write[2] = (uint8_t)(requestResistance & 0xFF);
    write[3] = (uint8_t)(requestResistance >> 8);

    writeCharacteristic(write, sizeof(write), QStringLiteral("forceResistance ") + QString::number(requestResistance),
                        false, true);
}

void smartspin2k::update() {

    if (!m_control)
        return;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest) {

        // if the first event from the bike occurs before connecting with SS2k, we have to force it
        if (startupResistance != -1)
            resistanceReadFromTheBike(startupResistance);
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

            if (parentDevice) {
                // watt sync
                uint8_t watt[] = {0x02, 0x03, 0x00, 0x00};
                watt[2] = (uint8_t)((uint16_t)(parentDevice->wattsMetric().value()) & 0xFF);
                watt[3] = (uint8_t)((uint16_t)(parentDevice->wattsMetric().value()) >> 8);

                writeCharacteristic(watt, sizeof(watt),
                                    QStringLiteral("watt sync ") + QString::number(parentDevice->wattsMetric().value()),
                                    false, true);

                // cadence sync
                uint8_t cadence[] = {0x02, 0x05, 0x00, 0x00};
                cadence[2] = (uint8_t)((uint16_t)(parentDevice->currentCadence().value()) & 0xFF);
                cadence[3] = (uint8_t)((uint16_t)(parentDevice->currentCadence().value()) >> 8);

                writeCharacteristic(cadence, sizeof(cadence),
                                    QStringLiteral("cadence sync ") +
                                        QString::number(parentDevice->currentCadence().value()),
                                    false, true);

                // hr sync
                uint8_t heart[] = {0x02, 0x04, 0x00, 0x00};
                heart[2] = (uint8_t)((uint16_t)(parentDevice->currentHeart().value()) & 0xFF);
                heart[3] = (uint8_t)((uint16_t)(parentDevice->currentHeart().value()) >> 8);

                writeCharacteristic(
                    heart, sizeof(heart),
                    QStringLiteral("heart sync ") + QString::number(parentDevice->currentHeart().value()), false, true);
            }
        }

        if (parentDevice && parentDevice->ergManagedBySS2K() && parentDevice->lastRequestedPower().value() != 0) {
            uint8_t write[] = {FTMS_SET_TARGET_POWER, 0x00, 0x00};

            write[1] = ((uint16_t)parentDevice->lastRequestedPower().value()) & 0xFF;
            write[2] = ((uint16_t)parentDevice->lastRequestedPower().value()) >> 8;

            writeCharacteristicFTMS(write, sizeof(write),
                                    QStringLiteral("forcePower to SS2K ") +
                                        QString::number(parentDevice->lastRequestedPower().value()),
                                    false, true);

            requestResistance = -1;
        }

        if (requestResistance != -1) {
            if (requestResistance > max_resistance) {
                requestResistance = max_resistance;
            } // TODO, use the bluetooth value
            else if (requestResistance == 0) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));

                forceResistance(requestResistance);
            }
            requestResistance = -1;
        } else {
            uint8_t read[] = {0x01, 0x17};
            writeCharacteristic(read, sizeof(read), QStringLiteral("polling"), false, true);
        }
    }
}

void smartspin2k::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void smartspin2k::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {

    Q_UNUSED(characteristic);

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    if (newValue.length() >= 4 && newValue.at(1) == 0x17) {
        Resistance = (int16_t)(((uint16_t)newValue.at(2)) + ((((uint16_t)newValue.at(3)) << 8) & 0xFF00));
        if (parentDevice && (!parentDevice->ergManagedBySS2K() || parentDevice->lastRequestedPower().value() == 0) &&
            lastRequestResistance == -1) {
            if (Resistance.value() > (double)lastResistance) {
                qDebug() << QStringLiteral("SS2K Gear Up!");
                emit gearUp();
            } else if (Resistance.value() < (double)lastResistance) {
                qDebug() << QStringLiteral("SS2K Gear Down!");
                emit gearDown();
            }
        }
        lastResistance = Resistance.value();

        // target point matched
        if (lastRequestResistance == Resistance.value())
            lastRequestResistance = -1;
        emit resistanceRead(Resistance.value());
        qDebug() << "Resistance received from SS2k:" << Resistance.value();
    }
}

void smartspin2k::stateChangedFTMS(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharControlPointId = gattCommunicationChannelServiceFTMS->characteristic(_gattWriteCharControlPointId);
        Q_ASSERT(gattWriteCharControlPointId.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelServiceFTMS,
                SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
                SLOT(characteristicChanged(QLowEnergyCharacteristic, QByteArray)));
        connect(gattCommunicationChannelServiceFTMS,
                SIGNAL(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)), this,
                SLOT(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)));
        connect(gattCommunicationChannelServiceFTMS, SIGNAL(error(QLowEnergyService::ServiceError)), this,
                SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelServiceFTMS,
                SIGNAL(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)), this,
                SLOT(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)));
    }
}

void smartspin2k::stateChanged(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattWriteCharacteristicId((QString) "77776277-7877-7774-4466-896665500001");
    // QBluetoothUuid _gattNotify1CharacteristicId((QString)"77776277-7877-7774-4466-896665500001");

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        // gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        // Q_ASSERT(gattNotify1Characteristic.isValid());

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
        descriptor.append((char)0x02);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattWriteCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void smartspin2k::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void smartspin2k::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void smartspin2k::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {

    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void smartspin2k::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void smartspin2k::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId((QString) "77776277-7877-7774-4466-896665500000");

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this,
            SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();

    QBluetoothUuid _gattCommunicationChannelServiceFTMSId((quint16)0x1826);

    gattCommunicationChannelServiceFTMS = m_control->createServiceObject(_gattCommunicationChannelServiceFTMSId);
    connect(gattCommunicationChannelServiceFTMS, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this,
            SLOT(stateChangedFTMS(QLowEnergyService::ServiceState)));
    gattCommunicationChannelServiceFTMS->discoverDetails();
}

void smartspin2k::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("smartspin2k::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());

    m_control->disconnectFromDevice();
}

void smartspin2k::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("smartspin2k::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());

    m_control->disconnectFromDevice();
}

void smartspin2k::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &smartspin2k::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &smartspin2k::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &smartspin2k::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &smartspin2k::controllerStateChanged);

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

bool smartspin2k::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t smartspin2k::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void smartspin2k::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        first = true;
        m_control->connectToDevice();
    }
}
