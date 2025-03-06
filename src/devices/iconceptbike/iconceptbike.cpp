#include "iconceptbike.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

iconceptbike::iconceptbike() {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &iconceptbike::update);
    refresh->start(1s);
}

void iconceptbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    //if (device.name().toUpper().startsWith(QStringLiteral("BH DUALKIT")))
    {
        if (device.name().toUpper().startsWith(QStringLiteral("BH-"))) {
            i_Nexor = true;
            qDebug() << "BH i-Nexor workaround enabled";
        }

        bluetoothDevice = device;

        // Create a discovery agent and connect to its signals
        discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered, this,
                &iconceptbike::serviceDiscovered);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this, &iconceptbike::serviceFinished);

        // Start a discovery
        qDebug() << QStringLiteral("iconceptbike::deviceDiscovered");
        discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
        return;
    }
}

void iconceptbike::serviceFinished() {
    qDebug() << QStringLiteral("iconceptbike::serviceFinished") << socket;
    if (socket) {
#ifdef Q_OS_ANDROID
        socket->setPreferredSecurityFlags(QBluetooth::NoSecurity);
#endif

        emit debug(QStringLiteral("Create socket"));
        socket->connectToService(serialPortService);
        emit debug(QStringLiteral("ConnectToService done"));
    }
}

// In your local slot, read information about the found devices
void iconceptbike::serviceDiscovered(const QBluetoothServiceInfo &service) {
    // this treadmill has more serial port, just the first one is the right one.
    if (socket != nullptr) {
        qDebug() << QStringLiteral("iconceptbike::serviceDiscovered socket already initialized");
        return;
    }

    qDebug() << QStringLiteral("iconceptbike::serviceDiscovered") << service;
    if (service.device().address() == bluetoothDevice.address()) {
        emit debug(QStringLiteral("Found new service: ") + service.serviceName() + '(' +
                   service.serviceUuid().toString() + ')');

        if ((service.serviceName().startsWith(QStringLiteral("SerialPort")) ||
             service.serviceName().startsWith(QStringLiteral("Serial Port"))) &&
            // android 13 workaround
            service.serviceUuid() == QBluetoothUuid(QStringLiteral("00001101-0000-1000-8000-00805f9b34fb"))) {
            emit debug(QStringLiteral("Serial port service found"));
            // discoveryAgent->stop(); // could lead to a crash?

            serialPortService = service;
            socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

            connect(socket, &QBluetoothSocket::readyRead, this, &iconceptbike::readSocket);
            connect(socket, &QBluetoothSocket::connected, this, QOverload<>::of(&iconceptbike::rfCommConnected));
            connect(socket, &QBluetoothSocket::disconnected, this, &iconceptbike::disconnected);
            connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error), this,
                    &iconceptbike::onSocketErrorOccurred);
        } else {
            qDebug () << QStringLiteral("service ignored!");
        }
    }
}

void iconceptbike::update() {
    QSettings settings;

    if (initDone) {
        // ******************************************* virtual treadmill init *************************************
        if (!firstStateChanged && !hasVirtualDevice()) {
            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual bike interface..."));
                auto virtualBike = new virtualbike(this, true);
                connect(virtualBike, &virtualbike::changeInclination, this, &iconceptbike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        firstStateChanged = 1;

        // ********************************************************************************************************

        if (requestResistance != -1) {
            if (requestResistance > 12) {
                requestResistance = 12;
            } else if (requestResistance < 1) {
                requestResistance = 1;
            }
            char resValues[] = {0x08, 0x0a, 0x0b, 0x0d, 0x0e, 0x10, 0x11, 0x13, 0x14, 0x16, 0x17, 0x18};
            char res[] = {0x55, 0x11, 0x01, 0x12};
            res[3] = resValues[requestResistance - 1];
            qDebug() << QStringLiteral(">>") << QByteArray(res, sizeof(res)).toHex(' ');
            socket->write(res, sizeof(res));
            Resistance = requestResistance;
            requestResistance = -1;
        } else {
            const char poll[] = {0x55, 0x17, 0x01, 0x01};
            qDebug() << QStringLiteral(">>") << QByteArray(poll, sizeof(poll)).toHex(' ');
            socket->write(poll, sizeof(poll));
            emit debug(QStringLiteral("write poll"));
        }

        update_metrics(false, watts());
    }
}

void iconceptbike::rfCommConnected() {
    emit debug(QStringLiteral("connected ") + socket->peerName());

    if(i_Nexor) {
        const uint8_t init01[] = {0x55, 0x0c, 0x01, 0xff, 0x55, 0xbb, 0x01, 0xff, 0x55, 0x24, 0x01, 0xff, 0x55, 0x25, 0x01,
                                 0xff, 0x55, 0x26, 0x01, 0xff, 0x55, 0x27, 0x01, 0xff, 0x55, 0x02, 0x01, 0xff, 0x55, 0x03,
                                 0x01, 0xff, 0x55, 0x04, 0x01, 0xff, 0x55, 0x06, 0x01, 0xff, 0x55, 0x1f, 0x01, 0xff, 0x55,
                                 0xa0, 0x01, 0xff, 0x55, 0xb0, 0x01, 0xff, 0x55, 0xb2, 0x01, 0xff, 0x55, 0xb3, 0x01, 0xff,
                                 0x55, 0xb4, 0x01, 0xff, 0x55, 0xb5, 0x01, 0xff, 0x55, 0xb6, 0x01, 0xff, 0x55, 0xb7, 0x01,
                                 0xff, 0x55, 0xb8, 0x01, 0xff, 0x55, 0xb9, 0x01, 0xff, 0x55, 0xba, 0x01, 0xff};
        const uint8_t init02[] = {0x55, 0x0b, 0x01, 0xff, 0x55, 0x18, 0x01, 0xff, 0x55, 0x19,
                                 0x01, 0xff, 0x55, 0x1a, 0x01, 0xff, 0x55, 0x1b, 0x01, 0xff};
        socket->write((char *)init01, sizeof(init01));
        qDebug() << QStringLiteral(" init01 write");
        socket->write((char *)init02, sizeof(init02));
        qDebug() << QStringLiteral(" init02 write");
        QThread::msleep(2000);

        const uint8_t init1[] = {0x55, 0x0a, 0x01, 0x02, 0x53};
        socket->write((char *)init1, sizeof(init1));
        qDebug() << QStringLiteral(" init1 write");

        const uint8_t init2[] = {0x55, 0x17, 0x01, 0x01, 0x4f};
        socket->write((char *)init2, sizeof(init2));
        qDebug() << QStringLiteral(" init2 write");
        QThread::msleep(600);

        const uint8_t init3[] = {0x55, 0x01, 0x06, 0x21, 0x01, 0x50, 0x00, 0xb4, 0x00};
        socket->write((char *)init3, sizeof(init3));
        qDebug() << QStringLiteral(" init3 write");
        QThread::msleep(400);

        const uint8_t init4[] = {0x55, 0x17, 0x01, 0x01};
        socket->write((char *)init4, sizeof(init4));
        qDebug() << QStringLiteral(" init4 write");
        QThread::msleep(200);

        const uint8_t init5[] = {0x55, 0x15, 0x01, 0x00};
        socket->write((char *)init5, sizeof(init5));
        qDebug() << QStringLiteral(" init5 write");
        QThread::msleep(600);

        const uint8_t init6[] = {0x55, 0x11, 0x01, 0x01};
        socket->write((char *)init6, sizeof(init6));
        qDebug() << QStringLiteral(" init6 write");
        QThread::msleep(200);

        socket->write((char *)init4, sizeof(init4));
        qDebug() << QStringLiteral(" init4 write");
        QThread::msleep(400);

        const uint8_t init7[] = {0x55, 0x0a, 0x01, 0x01};
        socket->write((char *)init7, sizeof(init7));
        qDebug() << QStringLiteral(" init7 write");
        QThread::msleep(600);

        const uint8_t init8[] = {0x55, 0x07, 0x01, 0xff};
        socket->write((char *)init8, sizeof(init8));
        qDebug() << QStringLiteral(" init8 write");
        QThread::msleep(600);
    } else {
        const uint8_t init1[] = {0x55, 0x0c, 0x01, 0xff, 0x55, 0xbb, 0x01, 0xff, 0x55, 0x24, 0x01, 0xff, 0x55, 0x25, 0x01,
                                 0xff, 0x55, 0x26, 0x01, 0xff, 0x55, 0x27, 0x01, 0xff, 0x55, 0x02, 0x01, 0xff, 0x55, 0x03,
                                 0x01, 0xff, 0x55, 0x04, 0x01, 0xff, 0x55, 0x06, 0x01, 0xff, 0x55, 0x1f, 0x01, 0xff, 0x55,
                                 0xa0, 0x01, 0xff, 0x55, 0xb0, 0x01, 0xff, 0x55, 0xb2, 0x01, 0xff, 0x55, 0xb3, 0x01, 0xff,
                                 0x55, 0xb4, 0x01, 0xff, 0x55, 0xb5, 0x01, 0xff, 0x55, 0xb6, 0x01, 0xff, 0x55, 0xb7, 0x01,
                                 0xff, 0x55, 0xb8, 0x01, 0xff, 0x55, 0xb9, 0x01, 0xff, 0x55, 0xba, 0x01, 0xff};
        const uint8_t init2[] = {0x55, 0x0b, 0x01, 0xff, 0x55, 0x18, 0x01, 0xff, 0x55, 0x19,
                                 0x01, 0xff, 0x55, 0x1a, 0x01, 0xff, 0x55, 0x1b, 0x01, 0xff};
        const uint8_t init3[] = {0x55, 0x17, 0x01, 0x01, 0x55, 0xb5, 0x01, 0xff};
        const uint8_t init4[] = {0x55, 0x01, 0x06, 0x1e, 0x00, 0x3c, 0x00, 0xaa, 0x00};
        const uint8_t init5[] = {0x55, 0x15, 0x01, 0x00};
        const uint8_t init6[] = {0x55, 0x11, 0x01, 0x01};
        const uint8_t init7[] = {0x55, 0x0a, 0x01, 0x01};
        const uint8_t init8[] = {0x55, 0x07, 0x01, 0xff};
        const uint8_t init9[] = {0x55, 0x11, 0x01, 0x08};

        socket->write((char *)init1, sizeof(init1));
        qDebug() << QStringLiteral(" init1 write");
        socket->write((char *)init2, sizeof(init2));
        qDebug() << QStringLiteral(" init2 write");
        QThread::msleep(2000);
        socket->write((char *)init3, sizeof(init3));
        qDebug() << QStringLiteral(" init3 write");
        QThread::msleep(2000);
        socket->write((char *)init3, sizeof(init3));
        qDebug() << QStringLiteral(" init3 write");
        QThread::msleep(500);
        socket->write((char *)init4, sizeof(init4));
        qDebug() << QStringLiteral(" init4 write");
        QThread::msleep(600);
        socket->write((char *)init5, sizeof(init5));
        qDebug() << QStringLiteral(" init5 write");
        QThread::msleep(600);
        socket->write((char *)init6, sizeof(init6));
        qDebug() << QStringLiteral(" init6 write");
        QThread::msleep(600);
        socket->write((char *)init7, sizeof(init7));
        qDebug() << QStringLiteral(" init7 write");
        QThread::msleep(600);
        socket->write((char *)init8, sizeof(init8));
        qDebug() << QStringLiteral(" init8 write");
        QThread::msleep(600);
        socket->write((char *)init9, sizeof(init9));
        qDebug() << QStringLiteral(" init9 write");
    }

    initDone = true;
    // requestStart = 1;
    emit connectedAndDiscovered();
}

void iconceptbike::readSocket() {
    if (!socket)
        return;

    while (socket->bytesAvailable()) {
        QByteArray line = socket->readAll();
        qDebug() << QStringLiteral(" << ") + line.toHex(' ');

        if (line.length() == 16) {
            QSettings settings;
            QString heartRateBeltName =
                settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
            bool bh_spada_2_watt =
                settings.value(QZSettings::bh_spada_2_watt, QZSettings::default_bh_spada_2_watt).toBool();
            elapsed = GetElapsedTimeFromPacket(line);
            //Distance = GetDistanceFromPacket(line);
            QDateTime now = QDateTime::currentDateTime();
            Distance += ((Speed.value() / 3600000.0) * ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
            KCal = GetCaloriesFromPacket(line);
            if (bh_spada_2_watt) {
                m_watt = GetWattFromPacket(line);
                if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                    Speed = GetSpeedFromPacket(line) / 2.0;
                } else {
                    Speed = metric::calculateSpeedFromPower(
                        watts(), Inclination.value(), Speed.value(),
                        fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
                }
                if (watts())
                    KCal +=
                        ((((0.048 * ((double)watts()) + 1.19) *
                           settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                          200.0) /
                         (60000.0 /
                          ((double)lastRefreshCharacteristicChanged.msecsTo(
                              now)))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                                //* 3.5) / 200 ) / 60
            } else {
                Speed = GetSpeedFromPacket(line);
            }
            Cadence = (uint8_t)line.at(13);
            // Heart = GetHeartRateFromPacket(line);

            if (Cadence.value() > 0) {
                CrankRevs++;
                LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
            }

            lastRefreshCharacteristicChanged = now;

#ifdef Q_OS_ANDROID
            if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
                Heart = (uint8_t)KeepAwakeHelper::heart();
            } else
#endif
            {
                if (heartRateBeltName.startsWith(QLatin1String("Disabled"))) {
                    update_hr_from_external();
                }
            }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence =
                settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
            bool ios_peloton_workaround =
                settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
            if (ios_peloton_workaround && cadence && h && firstStateChanged) {
                        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
                h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
            }
#endif
#endif

            // these useless lines are needed to calculate the AVG resistance and AVG peloton resistance since
            // echelon just send the resistance values when it changes
            Resistance = Resistance.value();
            m_pelotonResistance = m_pelotonResistance.value();

            emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
            emit debug(QStringLiteral("Current cadence: ") + QString::number(Cadence.value()));
            // emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
            emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
            emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
            qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());
        }
    }
}

double iconceptbike::GetSpeedFromPacket(const QByteArray &packet) {
    double convertedData = ((double)(((double)((uint8_t)packet.at(9))) * 256) + ((double)packet.at(10))) / 100.0;
    return convertedData;
}

double iconceptbike::GetWattFromPacket(const QByteArray &packet) {
    double convertedData = ((double)(((double)((uint8_t)packet.at(14))) * 256) + ((double)packet.at(15)));
    return convertedData;
}

uint16_t iconceptbike::GetCaloriesFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    return convertedData;
}

uint16_t iconceptbike::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(5) << 8) | packet.at(6);
    return convertedData;
}

uint16_t iconceptbike::GetElapsedTimeFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(3) << 8) | packet.at(4);
    return convertedData;
}

void iconceptbike::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    emit debug(QStringLiteral("onSocketErrorOccurred ") + QString::number(error));
}

uint16_t iconceptbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }
    return m_watt.value();
}
