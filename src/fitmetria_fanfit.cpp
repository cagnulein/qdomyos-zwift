#include "fitmetria_fanfit.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

fitmetria_fanfit::fitmetria_fanfit(bluetoothdevice *parentDevice) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    this->parentDevice = parentDevice;
}

void fitmetria_fanfit::update() {}

void fitmetria_fanfit::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void fitmetria_fanfit::disconnectBluetooth() {
    qDebug() << QStringLiteral("fitmetria_fanfit::disconnect") << m_control;

    if (m_control) {
        m_control->disconnectFromDevice();
    }
}

void fitmetria_fanfit::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    emit packetReceived();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));
}

void fitmetria_fanfit::fanSpeedRequest(uint8_t speed) {
    QSettings settings;
    if (speed > 102)
        speed = 102;
    double max = settings.value(QZSettings::fitmetria_fanfit_max, QZSettings::default_fitmetria_fanfit_max).toDouble();
    double min = settings.value(QZSettings::fitmetria_fanfit_min, QZSettings::default_fitmetria_fanfit_min).toDouble();

    uint16_t speed16 = (uint16_t)((double)speed * ((max * 10.0) - (min * 10.0)) / 100.0 + (min * 10.0));
    const uint8_t brightness = 5;
    const uint8_t leds_max = 15;

    uint8_t leds[leds_max] = {30, 30, 30, 15, 15, 15, 13, 24, 24, 8, 27, 27, 27, 26, 26};
    // 0~5~30~30~30~15~15~15~13~24~24~8~27~27~27~26~26
    // 0 - fans peed
    // 1 - brightness
    // all the rest - colors
    /*
        myCollorDict = {}
        myCollorDict['Black'] = 0
        myCollorDict['Blue'] = 1
        myCollorDict['Brown'] = 2
        myCollorDict['Cyan'] = 3
        myCollorDict['DarkBlue'] = 4
        myCollorDict['DarkCyan'] = 5
        myCollorDict['DarkGrey'] = 6
        myCollorDict['DarkGreen'] = 7
        myCollorDict['DarkOrange'] = 8
        myCollorDict['DarkRed'] = 9
        myCollorDict['DarkViolet'] = 10
        myCollorDict['DeepPink'] = 11
        myCollorDict['DimGrey'] = 12
        myCollorDict['Gold'] = 13
        myCollorDict['Grey'] = 14
        myCollorDict['Green'] = 15
        myCollorDict['LightBlue'] = 16
        myCollorDict['LightCyan'] = 17
        myCollorDict['LightGreen'] = 18
        myCollorDict['LightGrey'] = 19
        myCollorDict['LightPink'] = 20
        myCollorDict['LightYellow'] = 21
        myCollorDict['MediumBlue'] = 22
        myCollorDict['MediumPurple'] = 23
        myCollorDict['Orange'] = 24
        myCollorDict['Pink'] = 25
        myCollorDict['Purple'] = 26
        myCollorDict['Red'] = 27
        myCollorDict['Silver'] = 28
        myCollorDict['Violet'] = 29
        myCollorDict['White'] = 30
        myCollorDict['Yellow'] = 31
    */

    QString s = QString::number((speed16)) + "~" + QString::number(brightness);

    if (!settings.value(QZSettings::fitmetria_fanfit_mode, QZSettings::default_fitmetria_fanfit_mode)
             .toString()
             .compare(QStringLiteral("Power")) &&
        parentDevice) {
        double ftp = parentDevice->currentPowerZone().value();
        if (ftp < 1.3)
            memset(&leds[1], 0, leds_max - 1);
        else if (ftp < 1.6)
            memset(&leds[2], 0, leds_max - 2);
        else if (ftp < 2)
            memset(&leds[3], 0, leds_max - 3);
        else if (ftp < 2.4)
            memset(&leds[4], 0, leds_max - 4);
        else if (ftp < 3)
            memset(&leds[5], 0, leds_max - 5);
        else if (ftp < 3.3)
            memset(&leds[6], 0, leds_max - 6);
        else if (ftp < 3.6)
            memset(&leds[7], 0, leds_max - 7);
        else if (ftp < 4)
            memset(&leds[8], 0, leds_max - 8);
        else if (ftp < 5)
            memset(&leds[9], 0, leds_max - 9);
        else if (ftp < 5.6)
            memset(&leds[10], 0, leds_max - 10);
        else if (ftp < 6)
            memset(&leds[11], 0, leds_max - 11);
        else if (ftp < 6.6)
            memset(&leds[12], 0, leds_max - 12);
        else if (ftp < 7)
            memset(&leds[13], 0, leds_max - 13);
        else if (ftp < 7.1)
            memset(&leds[14], 0, leds_max - 14);
    } else if (!settings.value(QZSettings::fitmetria_fanfit_mode, QZSettings::default_fitmetria_fanfit_mode)
                    .toString()
                    .compare(QStringLiteral("Heart")) &&
               parentDevice) {
        double ftp = parentDevice->currentHeartZone().value();
        if (ftp < 1.3)
            memset(&leds[1], 0, leds_max - 1);
        else if (ftp < 1.6)
            memset(&leds[2], 0, leds_max - 2);
        else if (ftp < 1.9)
            memset(&leds[3], 0, leds_max - 3);
        else if (ftp < 2)
            memset(&leds[4], 0, leds_max - 4);
        else if (ftp < 2.3)
            memset(&leds[5], 0, leds_max - 5);
        else if (ftp < 2.6)
            memset(&leds[6], 0, leds_max - 6);
        else if (ftp < 2.9)
            memset(&leds[7], 0, leds_max - 7);
        else if (ftp < 3.3)
            memset(&leds[8], 0, leds_max - 8);
        else if (ftp < 3.6)
            memset(&leds[9], 0, leds_max - 9);
        else if (ftp < 4)
            memset(&leds[10], 0, leds_max - 10);
        else if (ftp < 4.3)
            memset(&leds[11], 0, leds_max - 11);
        else if (ftp < 4.9)
            memset(&leds[12], 0, leds_max - 12);
        else if (ftp < 5)
            memset(&leds[13], 0, leds_max - 13);
        else if (ftp < 5.1)
            memset(&leds[14], 0, leds_max - 14);
    } else {
        memset(&leds, 0, leds_max);
    }
    for (int i = 0; i < leds_max; i++)
        s += "~" + QString::number(leds[i]);

    // only if the string differs i update the value over bluetooth
    if (lastValueSent != s)
        writeCharacteristic((uint8_t *)s.toLocal8Bit().data(), s.length(), QStringLiteral("forcing fan ") + s, false,
                            true);
    lastValueSent = s;
}

void fitmetria_fanfit::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                           bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (gattCommunicationChannelService == nullptr || gattWriteCharacteristic.isValid() == false) {
        qDebug() << QStringLiteral(
            "fitmetria_fanfit trying to change the fan speed before the connection is estabilished");
        return;
    }

    // if there are some crash here, maybe it's better to use 2 separate event for the characteristicChanged.
    // one for the resistance changed event (spontaneous), and one for the other ones.
    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("writeCharacteristic error because the connection is closed");
        return;
    }

    if (!gattWriteCharacteristic.isValid()) {
        qDebug() << QStringLiteral("gattWriteCharacteristic is invalid");
        return;
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                        QStringLiteral(" // ") + info;
    }

    loop.exec();
}

void fitmetria_fanfit::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("19c95d4e-f9ec-4528-8313-f8f92c147cd8"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            emit debug(QStringLiteral("characteristic ") + c.uuid().toString());
        }

        /*
        gattNotifyCharacteristic =
            gattCommunicationChannelService->characteristic(QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement));
        Q_ASSERT(gattNotifyCharacteristic.isValid());*/

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &fitmetria_fanfit::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &fitmetria_fanfit::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &fitmetria_fanfit::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &fitmetria_fanfit::descriptorWritten);

        /*
        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);*/
    }
}

void fitmetria_fanfit::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));
}

void fitmetria_fanfit::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void fitmetria_fanfit::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("5b3c6a8f-4d54-400e-82db-b7b083d3c5c3"));
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &fitmetria_fanfit::stateChanged);
    gattCommunicationChannelService->discoverDetails();

    lastValueSent = "";
}

void fitmetria_fanfit::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("fitmetria_fanfit::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void fitmetria_fanfit::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("fitmetria_fanfit::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void fitmetria_fanfit::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &fitmetria_fanfit::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &fitmetria_fanfit::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &fitmetria_fanfit::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &fitmetria_fanfit::controllerStateChanged);

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

bool fitmetria_fanfit::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void fitmetria_fanfit::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        m_control->connectToDevice();
    }
}
