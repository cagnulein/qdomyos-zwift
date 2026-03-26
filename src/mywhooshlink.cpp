#include "mywhooshlink.h"
#include "devices/bluetoothdevice.h"
#include "bluetooth.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDebug>

MyWhooshLink *MyWhooshLink::s_instance = nullptr;

MyWhooshLink *MyWhooshLink::instance() {
    return s_instance;
}

MyWhooshLink::MyWhooshLink(bluetooth *manager, QObject *parent)
    : QObject(parent)
    , tcpServer(nullptr)
    , statusTimer(nullptr)
    , device(nullptr)
    , bluetoothManager(manager)
    , enabled(false)
    , overrideGears(false)
    , currentCameraAngle(1)
    , currentEmote(1)
    , leftPaddlePressed(false)
    , rightPaddlePressed(false)
{
    s_instance = this;
    qDebug() << "MyWhooshLink: Constructor called";
    loadSettings();

    if (enabled) {
        qDebug() << "MyWhooshLink: Enabled=true, starting server...";
        start();

        // Start status check timer
        statusTimer = new QTimer(this);
        connect(statusTimer, &QTimer::timeout, this, &MyWhooshLink::checkServerStatus);
        statusTimer->start(10000); // Check every 10 seconds
        qDebug() << "MyWhooshLink: Status timer started";
    } else {
        qDebug() << "MyWhooshLink: Enabled=false, not starting server";
    }
}

MyWhooshLink::~MyWhooshLink() {
    stop();
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

void MyWhooshLink::loadSettings() {
    enabled = settings.value(QZSettings::mywhoosh_link_enabled,
                             QZSettings::default_mywhoosh_link_enabled).toBool();
    overrideGears = settings.value(QZSettings::mywhoosh_link_override_gears,
                                   QZSettings::default_mywhoosh_link_override_gears).toBool();

    // Load button mappings
    leftUpAction = static_cast<Action>(settings.value(QZSettings::mywhoosh_link_left_up,
                                                       QZSettings::default_mywhoosh_link_left_up).toInt());
    leftDownAction = static_cast<Action>(settings.value(QZSettings::mywhoosh_link_left_down,
                                                         QZSettings::default_mywhoosh_link_left_down).toInt());
    leftLeftAction = static_cast<Action>(settings.value(QZSettings::mywhoosh_link_left_left,
                                                         QZSettings::default_mywhoosh_link_left_left).toInt());
    leftRightAction = static_cast<Action>(settings.value(QZSettings::mywhoosh_link_left_right,
                                                          QZSettings::default_mywhoosh_link_left_right).toInt());
    leftShoulderAction = static_cast<Action>(settings.value(QZSettings::mywhoosh_link_left_shoulder,
                                                             QZSettings::default_mywhoosh_link_left_shoulder).toInt());
    leftPowerAction = static_cast<Action>(settings.value(QZSettings::mywhoosh_link_left_power,
                                                          QZSettings::default_mywhoosh_link_left_power).toInt());

    rightYAction = static_cast<Action>(settings.value(QZSettings::mywhoosh_link_right_y,
                                                       QZSettings::default_mywhoosh_link_right_y).toInt());
    rightAAction = static_cast<Action>(settings.value(QZSettings::mywhoosh_link_right_a,
                                                       QZSettings::default_mywhoosh_link_right_a).toInt());
    rightBAction = static_cast<Action>(settings.value(QZSettings::mywhoosh_link_right_b,
                                                       QZSettings::default_mywhoosh_link_right_b).toInt());
    rightZAction = static_cast<Action>(settings.value(QZSettings::mywhoosh_link_right_z,
                                                       QZSettings::default_mywhoosh_link_right_z).toInt());
    rightShoulderAction = static_cast<Action>(settings.value(QZSettings::mywhoosh_link_right_shoulder,
                                                              QZSettings::default_mywhoosh_link_right_shoulder).toInt());
    rightPowerAction = static_cast<Action>(settings.value(QZSettings::mywhoosh_link_right_power,
                                                           QZSettings::default_mywhoosh_link_right_power).toInt());

    currentCameraAngle = settings.value(QZSettings::mywhoosh_link_camera_value,
                                        QZSettings::default_mywhoosh_link_camera_value).toInt();
    currentEmote = settings.value(QZSettings::mywhoosh_link_emote_value,
                                  QZSettings::default_mywhoosh_link_emote_value).toInt();
}

bool MyWhooshLink::isEnabled() const {
    return enabled;
}

bool MyWhooshLink::isRunning() const {
    return tcpServer && tcpServer->isListening();
}

bool MyWhooshLink::overrideLocalGears() const {
    return overrideGears;
}

void MyWhooshLink::start() {
    if (isRunning()) {
        qDebug() << "MyWhooshLink: Already running";
        return;
    }

    // Print all available network addresses
    qDebug() << "MyWhooshLink: Available network interfaces:";
    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            foreach (const QNetworkAddressEntry &entry, interface.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    qDebug() << "  " << interface.name() << ":" << entry.ip().toString();
                }
            }
        }
    }

    tcpServer = new QTcpServer(this);
    connect(tcpServer, &QTcpServer::newConnection, this, &MyWhooshLink::onNewConnection);

    // Try binding to all IPv4 addresses first
    bool success = false;
    if (tcpServer->listen(QHostAddress::Any, PORT)) {
        success = true;
        qDebug() << "MyWhooshLink: Server started on 0.0.0.0:" << PORT;
    } else {
        qDebug() << "MyWhooshLink: Failed to bind to IPv4:" << tcpServer->errorString();
        // Try IPv6 dual-stack
        if (tcpServer->listen(QHostAddress::AnyIPv6, PORT)) {
            success = true;
            qDebug() << "MyWhooshLink: Server started on [::]:" << PORT << "(dual-stack)";
        }
    }

    if (success) {
        tcpServer->setMaxPendingConnections(10);
        qDebug() << "MyWhooshLink: Server listening on port" << PORT;
        qDebug() << "MyWhooshLink: MyWhoosh should connect to one of the IPs above";
    } else {
        qDebug() << "MyWhooshLink: Failed to start server:" << tcpServer->errorString();
        delete tcpServer;
        tcpServer = nullptr;
    }
}

void MyWhooshLink::stop() {
    if (tcpServer) {
        // Disconnect all clients
        for (QTcpSocket *client : clients) {
            client->disconnectFromHost();
            client->deleteLater();
        }
        clients.clear();

        tcpServer->close();
        tcpServer->deleteLater();
        tcpServer = nullptr;
        qDebug() << "MyWhooshLink: Server stopped";
    }
}

void MyWhooshLink::onNewConnection() {
    while (tcpServer->hasPendingConnections()) {
        QTcpSocket *client = tcpServer->nextPendingConnection();
        clients.append(client);

        connect(client, &QTcpSocket::disconnected, this, &MyWhooshLink::onClientDisconnected);
        connect(client, &QTcpSocket::readyRead, this, &MyWhooshLink::onReadyRead);

        qDebug() << "MyWhooshLink: Client connected from" << client->peerAddress().toString();
    }
}

void MyWhooshLink::onClientDisconnected() {
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (client) {
        qDebug() << "MyWhooshLink: Client disconnected" << client->peerAddress().toString();
        clients.removeAll(client);
        client->deleteLater();
    }
}

void MyWhooshLink::onReadyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (client) {
        QByteArray data = client->readAll();
        qDebug() << "MyWhooshLink: Received data from client:" << data;
        processIncomingData(data);
    }
}

void MyWhooshLink::processIncomingData(const QByteArray &data) {
    receiveBuffer.append(data);

    int newlineIndex = receiveBuffer.indexOf('\n');
    while (newlineIndex >= 0) {
        QByteArray line = receiveBuffer.left(newlineIndex).trimmed();
        receiveBuffer.remove(0, newlineIndex + 1);

        if (!line.isEmpty()) {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(line, &error);
            if (error.error == QJsonParseError::NoError && !doc.isNull()) {
                logIncomingJson(doc);
            } else {
                qDebug() << "MyWhooshLink: Incoming non-JSON message:" << line;
            }
        }

        newlineIndex = receiveBuffer.indexOf('\n');
    }
}

void MyWhooshLink::logIncomingJson(const QJsonDocument &doc) const {
    if (doc.isObject()) {
        QJsonObject object = doc.object();
        qDebug() << "MyWhooshLink: Incoming JSON object:" << QJsonDocument(object).toJson(QJsonDocument::Compact);
        if (object.contains(QStringLiteral("MessageType"))) {
            qDebug() << "MyWhooshLink: MessageType =" << object.value(QStringLiteral("MessageType")).toVariant().toString();
        }
        logInterestingJsonObject(object);
    } else if (doc.isArray()) {
        QJsonArray array = doc.array();
        qDebug() << "MyWhooshLink: Incoming JSON array:" << QJsonDocument(array).toJson(QJsonDocument::Compact);
        for (int i = 0; i < array.size(); ++i) {
            logInterestingJsonValue(QStringLiteral("[%1]").arg(i), array.at(i));
        }
    }
}

void MyWhooshLink::logInterestingJsonValue(const QString &path, const QJsonValue &value) const {
    if (value.isObject()) {
        logInterestingJsonObject(value.toObject(), path);
        return;
    }

    if (value.isArray()) {
        QJsonArray array = value.toArray();
        for (int i = 0; i < array.size(); ++i) {
            logInterestingJsonValue(path + QStringLiteral("[%1]").arg(i), array.at(i));
        }
        return;
    }

    if (!path.isEmpty() && isInterestingField(path.section('.', -1))) {
        qDebug() << "MyWhooshLink: Interesting incoming field" << path << "=" << value.toVariant();
    }
}

void MyWhooshLink::logInterestingJsonObject(const QJsonObject &object, const QString &prefix) const {
    for (auto it = object.begin(); it != object.end(); ++it) {
        const QString path = prefix.isEmpty() ? it.key() : prefix + QStringLiteral(".") + it.key();
        if (isInterestingField(it.key()) && !it.value().isObject() && !it.value().isArray()) {
            qDebug() << "MyWhooshLink: Interesting incoming field" << path << "=" << it.value().toVariant();
        }
        logInterestingJsonValue(path, it.value());
    }
}

bool MyWhooshLink::isInterestingField(const QString &fieldName) const {
    const QString key = fieldName.toLower();
    return key.contains(QStringLiteral("incl")) ||
           key.contains(QStringLiteral("grade")) ||
           key.contains(QStringLiteral("slope")) ||
           key.contains(QStringLiteral("elev")) ||
           key.contains(QStringLiteral("erg")) ||
           key.contains(QStringLiteral("targetpower")) ||
           key.contains(QStringLiteral("target_power")) ||
           key == QStringLiteral("power") ||
           key == QStringLiteral("watts") ||
           key.contains(QStringLiteral("resistance")) ||
           key.contains(QStringLiteral("cadence")) ||
           key.contains(QStringLiteral("heartrate")) ||
           key.contains(QStringLiteral("heart_rate")) ||
           key == QStringLiteral("hr");
}

void MyWhooshLink::sendJsonMessage(const QJsonObject &message) {
    // Get device from bluetooth manager if not already set
    if (!device && bluetoothManager && bluetoothManager->device()) {
        device = bluetoothManager->device();
        qDebug() << "MyWhooshLink: Device connected";
    }

    if (!isRunning()) {
        return;
    }

    QJsonDocument doc(message);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    jsonData.append('\n'); // MyWhoosh expects newline-terminated messages

    for (QTcpSocket *client : clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            qint64 written = client->write(jsonData);
            client->flush();
            qDebug() << "MyWhooshLink: Sent to client:" << jsonData << "bytes:" << written;
        }
    }
}

QString MyWhooshLink::actionToJsonField(Action action) const {
    switch (action) {
        case GearUp:
        case GearDown:
            return QStringLiteral("GearShifting");
        case SteerLeft:
        case SteerRight:
            return QStringLiteral("Steering");
        case UTurn:
            return QStringLiteral("UTurn");
        case CameraAngle:
            return QStringLiteral("CameraAngle");
        case Emote:
            return QStringLiteral("Emote");
        case Tuck:
            return QStringLiteral("Tuck");
        default:
            return QString();
    }
}

QString MyWhooshLink::actionToJsonValue(Action action, bool keyDown) const {
    switch (action) {
        case GearUp:
            return QStringLiteral("1");
        case GearDown:
            return QStringLiteral("-1");
        case SteerLeft:
            return keyDown ? QStringLiteral("-1") : QStringLiteral("0");
        case SteerRight:
            return keyDown ? QStringLiteral("1") : QStringLiteral("0");
        case UTurn:
            return QStringLiteral("true");
        case CameraAngle:
            return QString::number(currentCameraAngle);
        case Emote:
            return QString::number(currentEmote);
        case Tuck:
            return QStringLiteral("true");
        default:
            return QString();
    }
}

void MyWhooshLink::sendAction(Action action, bool keyDown) {
    if (action == Disabled) {
        return;
    }

    const bool isContinuousAction = action == SteerLeft || action == SteerRight;
    if (!keyDown && !isContinuousAction) {
        return;
    }

    QString field = actionToJsonField(action);
    QString value = actionToJsonValue(action, keyDown);

    if (field.isEmpty() || value.isEmpty()) {
        return;
    }

    QJsonObject inGameControls;
    inGameControls[field] = value;

    QJsonObject message;
    message[QStringLiteral("MessageType")] = QStringLiteral("Controls");
    message[QStringLiteral("InGameControls")] = inGameControls;

    sendJsonMessage(message);

    // Cycle values for camera and emote
    if (keyDown) {
        if (action == CameraAngle) {
            cycleCameraAngle();
        } else if (action == Emote) {
            cycleEmote();
        }
    }
}

void MyWhooshLink::sendSteering(int value) {
    QJsonObject inGameControls;

    if (value < 0) {
        inGameControls[QStringLiteral("Steering")] = QStringLiteral("-1");
    } else if (value > 0) {
        inGameControls[QStringLiteral("Steering")] = QStringLiteral("1");
    } else {
        inGameControls[QStringLiteral("Steering")] = QStringLiteral("0");
    }

    QJsonObject message;
    message[QStringLiteral("MessageType")] = QStringLiteral("Controls");
    message[QStringLiteral("InGameControls")] = inGameControls;

    sendJsonMessage(message);
}

void MyWhooshLink::cycleCameraAngle() {
    currentCameraAngle++;
    if (currentCameraAngle > 10) {
        currentCameraAngle = 1;
    }
    settings.setValue(QZSettings::mywhoosh_link_camera_value, currentCameraAngle);
}

void MyWhooshLink::cycleEmote() {
    currentEmote++;
    if (currentEmote > 6) {
        currentEmote = 1;
    }
    settings.setValue(QZSettings::mywhoosh_link_emote_value, currentEmote);
}

void MyWhooshLink::handleGearUp(bool pressed) {
    sendAction(GearUp, pressed);
}

void MyWhooshLink::handleGearDown(bool pressed) {
    sendAction(GearDown, pressed);
}

// Zwift Play button handlers
void MyWhooshLink::handleLeftUp(bool pressed) {
    sendAction(leftUpAction, pressed);
}

void MyWhooshLink::handleLeftDown(bool pressed) {
    sendAction(leftDownAction, pressed);
}

void MyWhooshLink::handleLeftLeft(bool pressed) {
    sendAction(leftLeftAction, pressed);
}

void MyWhooshLink::handleLeftRight(bool pressed) {
    sendAction(leftRightAction, pressed);
}

void MyWhooshLink::handleLeftShoulder(bool pressed) {
    sendAction(leftShoulderAction, pressed);
}

void MyWhooshLink::handleLeftPower(bool pressed) {
    sendAction(leftPowerAction, pressed);
}

void MyWhooshLink::handleLeftPaddle(int value) {
    // Left paddle is always mapped to steer left (fixed, not configurable)
    if (value < 0 && !leftPaddlePressed) {
        // Pressed
        leftPaddlePressed = true;
        sendSteering(-1);
    } else if (value >= 0 && leftPaddlePressed) {
        // Released
        leftPaddlePressed = false;
        sendSteering(0);
    }
}

void MyWhooshLink::handleRightY(bool pressed) {
    sendAction(rightYAction, pressed);
}

void MyWhooshLink::handleRightA(bool pressed) {
    sendAction(rightAAction, pressed);
}

void MyWhooshLink::handleRightB(bool pressed) {
    sendAction(rightBAction, pressed);
}

void MyWhooshLink::handleRightZ(bool pressed) {
    sendAction(rightZAction, pressed);
}

void MyWhooshLink::handleRightShoulder(bool pressed) {
    sendAction(rightShoulderAction, pressed);
}

void MyWhooshLink::handleRightPower(bool pressed) {
    sendAction(rightPowerAction, pressed);
}

void MyWhooshLink::handleRightPaddle(int value) {
    // Right paddle is always mapped to steer right (fixed, not configurable)
    if (value > 0 && !rightPaddlePressed) {
        // Pressed
        rightPaddlePressed = true;
        sendSteering(1);
    } else if (value <= 0 && rightPaddlePressed) {
        // Released
        rightPaddlePressed = false;
        sendSteering(0);
    }
}

void MyWhooshLink::checkServerStatus() {
    if (tcpServer && tcpServer->isListening()) {
        qDebug() << "MyWhooshLink: Server is ACTIVE on port" << PORT
                 << "- Clients connected:" << clients.size();
    } else {
        qDebug() << "MyWhooshLink: WARNING - Server is NOT listening!";
    }
}
