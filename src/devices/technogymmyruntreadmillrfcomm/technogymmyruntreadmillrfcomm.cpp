#include "technogymmyruntreadmillrfcomm.h"
#include "virtualdevices/virtualtreadmill.h"
#include "homeform.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

technogymmyruntreadmillrfcomm::technogymmyruntreadmillrfcomm() {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &technogymmyruntreadmillrfcomm::update);
    refresh->start(1s);

    retryTimer = new QTimer(this);
    retryTimer->setSingleShot(true);
    connect(retryTimer, &QTimer::timeout, this, &technogymmyruntreadmillrfcomm::retryConnection);

    connectionTimeout = new QTimer(this);
    connectionTimeout->setSingleShot(true);
    connect(connectionTimeout, &QTimer::timeout, this, [this]() {
        if (socket && socket->state() != QBluetoothSocket::ConnectedState) {
            emit debug(QStringLiteral("MyRun RFCOMM connection timed out"));
            scheduleReconnect();
        }
    });
}

bool technogymmyruntreadmillrfcomm::connected() {
    // The base treadmill implementation always reports false. For this
    // Bluetooth Classic driver the RFCOMM socket is the connection source
    // of truth, but report ready only after the MyRun init sequence.
    return initDone && myRunStatusReceived && socket &&
           socket->state() == QBluetoothSocket::ConnectedState;
}

void technogymmyruntreadmillrfcomm::showConnectedToastIfReady() {
    if (connectedToastShown || !connected())
        return;

    connectedToastShown = true;
    reconnectAttempts = 0;
    emit debug(QStringLiteral("MyRun RFCOMM ready: handshake and status received"));
    if (homeform::singleton())
        homeform::singleton()->setToastRequested(QObject::tr("Technogym MyRun connected!"));
}

void technogymmyruntreadmillrfcomm::scheduleReconnect() {
    initDone = false;
    myRunStatusReceived = false;
    connectedToastShown = false;
    connectionTimeout->stop();
    if (retryTimer->isActive() || reconnectAttempts >= maxReconnectAttempts)
        return;

    const int delayMs = (reconnectAttempts + 1) * 5000;
    emit debug(QStringLiteral("MyRun RFCOMM retry %1/%2 in %3 seconds")
                   .arg(reconnectAttempts + 1)
                   .arg(maxReconnectAttempts)
                   .arg(delayMs / 1000));
    retryTimer->start(delayMs);
}

void technogymmyruntreadmillrfcomm::retryConnection() {
    ++reconnectAttempts;
    if (socket) {
        QBluetoothSocket *oldSocket = socket;
        socket = nullptr;
        oldSocket->abort();
        oldSocket->deleteLater();
    }

    // Refresh SDP on every retry: the previous SerialPort service may no
    // longer be valid after the treadmill is power-cycled.
    found = false;
    serialPortService = QBluetoothServiceInfo();
    if (!discoveryAgent)
        return;
    if (discoveryAgent->isActive())
        discoveryAgent->stop();
    emit debug(QStringLiteral("MyRun RFCOMM retry: restarting service discovery"));
    discoveryAgent->start(QBluetoothServiceDiscoveryAgent::MinimalDiscovery);
}

void technogymmyruntreadmillrfcomm::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    // if (device.name().startsWith(QStringLiteral("TRX ROUTE KEY")))
    {
        bluetoothDevice = device;

        // Create a discovery agent and connect to its signals
        discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered, this,
                &technogymmyruntreadmillrfcomm::serviceDiscovered);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled, this,
                &technogymmyruntreadmillrfcomm::serviceCanceled);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this,
                &technogymmyruntreadmillrfcomm::serviceFinished);

        // Start a discovery
        qDebug() << QStringLiteral("technogymmyruntreadmillrfcomm::deviceDiscovered");
        // the full discovery works but it requires 2 or more minutes to complete.
        // if you have issue finding your device, open the Technogym app, connect to your
        // treadmill, and then you should be safe to use QZ for that moment on
        //discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
        discoveryAgent->start(QBluetoothServiceDiscoveryAgent::MinimalDiscovery);
        return;
    }
}

void technogymmyruntreadmillrfcomm::serviceFinished(void) {
    qDebug() << QStringLiteral("technogymmyruntreadmillrfcomm::serviceFinished") << socket;
    if (!socket) {
        socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

        connect(socket, &QBluetoothSocket::readyRead, this, &technogymmyruntreadmillrfcomm::readSocket);
        connect(socket, &QBluetoothSocket::connected, this,
                QOverload<>::of(&technogymmyruntreadmillrfcomm::rfCommConnected));
        connect(socket, &QBluetoothSocket::disconnected, this, &technogymmyruntreadmillrfcomm::onSocketDisconnected);
        connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error), this,
                &technogymmyruntreadmillrfcomm::onSocketErrorOccurred);

#ifdef Q_OS_ANDROID
        socket->setPreferredSecurityFlags(QBluetooth::NoSecurity);
#endif

        emit debug(QStringLiteral("Create socket"));
        if(!found) {
            qDebug() << QStringLiteral("technogymmyruntreadmillrfcomm::serviceFinished, no service found, trying workaround");
            socket->connectToService(bluetoothDevice.address(), QBluetoothUuid(QBluetoothUuid::SerialPort));
        } else {
            socket->connectToService(serialPortService);
        }
        emit debug(QStringLiteral("ConnectToService done"));
        if (socket && socket->state() != QBluetoothSocket::ConnectedState &&
            !retryTimer->isActive())
            connectionTimeout->start(30000);
    }
}

void technogymmyruntreadmillrfcomm::serviceCanceled(void) {
    qDebug() << QStringLiteral("technogymmyruntreadmillrfcomm::serviceCanceled");
}

// In your local slot, read information about the found devices
void technogymmyruntreadmillrfcomm::serviceDiscovered(const QBluetoothServiceInfo &service) {
    qDebug() << QStringLiteral("Found new service: ") << service.serviceName() << '(' << service.serviceUuid().toString() << ') ' << service.device().address() << bluetoothDevice.address();

    if (found == true) {
        qDebug() << QStringLiteral("technogymmyruntreadmillrfcomm::serviceDiscovered socket already initialized");
        //return;
    }

    qDebug() << QStringLiteral("technogymmyruntreadmillrfcomm::serviceDiscovered") << service;
    if (service.device().address() == bluetoothDevice.address()) {        
        if (service.serviceName().startsWith(QStringLiteral("SerialPort")) ||
            service.serviceName().startsWith(QStringLiteral("Serial Port"))) {
            serialPortService = service;
            found = true;
            emit debug(QStringLiteral("Serial port service found"));
            //discoveryAgent->stop();
        }
    }
}

void technogymmyruntreadmillrfcomm::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void technogymmyruntreadmillrfcomm::update() {
    QSettings settings;

    if (connected()) {
        // ******************************************* virtual treadmill init *************************************
        if (!this->hasVirtualDevice()) {
            QSettings settings;
            bool virtual_device_enabled = settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual treadmill interface..."));
                auto virtualTreadMill = new virtualtreadmill(this, true);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &technogymmyruntreadmillrfcomm::debug);
                connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                        &technogymmyruntreadmillrfcomm::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        // ********************************************************************************************************

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 20) {
                QString force =
                    QStringLiteral("!DEV,024,") + QString::number(requestSpeed) + QStringLiteral(",1.8#").toLocal8Bit();
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed) + " " + force);
                socket->write(force.toLocal8Bit());
            }
            requestSpeed = -1;
        } else if (requestInclination != -100) {
            if(requestInclination < 0)
                requestInclination = 0;
            if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                requestInclination <= 12) {
                QString force = QStringLiteral("!DEV,025,") + QString::number(requestInclination) +
                                QStringLiteral("#").toLocal8Bit();
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination) + " " + force);
                socket->write(force.toLocal8Bit());
            }
            requestInclination = -100;
        } else if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            /*
            const uint8_t start[] = {0x55, 0x17, 0x01, 0x01, 0x55, 0xb5, 0x01, 0xff};
            socket->write((char *)start, sizeof(start));
            */
            requestStart = -1;
            emit tapeStarted();
        } else {
            /*
            const char poll[] = {0x55, 0x17, 0x01, 0x01};
            socket->write(poll, sizeof(poll));
            emit debug(QStringLiteral("write poll"));
            */
        }

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));
    }
}

void technogymmyruntreadmillrfcomm::rfCommConnected() {
    connectionTimeout->stop();
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState)
        return;
    retryTimer->stop();
    emit debug(QStringLiteral("RFCOMM socket connected: ") + socket->peerName());

    // Preserve the existing MyRun handshake and its timings. If the socket
    // drops during a waitForAPacket() nested event loop, stop immediately.
    QBluetoothSocket *initialSocket = socket;
    auto writeInitCommand = [this, initialSocket](const QString &command, int step) {
        if (socket != initialSocket || !socket ||
            socket->state() != QBluetoothSocket::ConnectedState)
            return false;
        socket->write(command.toLocal8Bit());
        qDebug() << QStringLiteral(" init%1 write").arg(step);
        waitForAPacket();
        return socket == initialSocket && socket &&
               socket->state() == QBluetoothSocket::ConnectedState &&
               !retryTimer->isActive();
    };

    if (!writeInitCommand(QStringLiteral("@FWVER#"), 1))
        return;
    if (!writeInitCommand(QStringLiteral("!DEV,027#"), 2))
        return;
    if (!writeInitCommand(QStringLiteral("!DEV,060#"), 3))
        return;
    if (!writeInitCommand(QStringLiteral("!DEV,101,1,21,4#@rtg_id#"), 4))
        return;
    if (!writeInitCommand(QStringLiteral("!DEV,073#"), 5))
        return;
    if (!writeInitCommand(QStringLiteral("!DEV,102#"), 6))
        return;
    if (!writeInitCommand(QStringLiteral("!DEV,104#"), 7))
        return;
    if (!writeInitCommand(QStringLiteral("!DEV,073#"), 8))
        return;
    if (!writeInitCommand(QStringLiteral("!DEV,053,0.0,0#@SSIUNITS#@DISABLE_PACE#"), 9))
        return;
    if (!writeInitCommand(QStringLiteral("!DEV,032,0#"), 10))
        return;

    initDone = true;
    emit connectedAndDiscovered();
    showConnectedToastIfReady();
}

void technogymmyruntreadmillrfcomm::waitForAPacket() {
    QEventLoop loop;
    QTimer timeout;
    connect(this, &technogymmyruntreadmillrfcomm::packetReceived, &loop, &QEventLoop::quit);
    timeout.singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}

void technogymmyruntreadmillrfcomm::readSocket() {
    if (!socket)
        return;

    emit packetReceived();

    while (socket->bytesAvailable()) {
        QByteArray line = socket->readAll();
        qDebug() << QStringLiteral(" << ") << line.size() << line;

        if (line.contains("!MYR,031,") && line.contains('#')) {
            myRunStatusReceived = true;
            showConnectedToastIfReady();
        }

        if (line.length() > 60) {
            QStringList values = QString(line).split(QStringLiteral(","));
            Speed = values.at(2).toDouble();
            Inclination = values.at(3).toDouble();

            qDebug() << values;
            emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
            emit debug(QStringLiteral("Current incline: ") + QString::number(Inclination.value()));
        }
    }
}

void technogymmyruntreadmillrfcomm::onSocketDisconnected() {
    if (sender() != socket)
        return;
    emit debug(QStringLiteral("MyRun RFCOMM disconnected"));
    emit disconnected();
    scheduleReconnect();
}

void technogymmyruntreadmillrfcomm::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    if (sender() != socket)
        return;
    emit debug(QStringLiteral("MyRun RFCOMM socket error %1: %2")
                   .arg(static_cast<int>(error))
                   .arg(socket ? socket->errorString() : QString()));
    scheduleReconnect();
}
