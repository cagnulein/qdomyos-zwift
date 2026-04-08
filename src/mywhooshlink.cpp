#include "mywhooshlink.h"
#include "devices/bluetoothdevice.h"
#include "bluetooth.h"

#include <QDebug>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QNetworkDatagram>
#include <QSysInfo>

MyWhooshLink *MyWhooshLink::s_instance = nullptr;

namespace {
constexpr quint8 MessageTypeButtonState = 0x01;
constexpr quint8 MessageTypeDeviceStatus = 0x02;
constexpr quint8 MessageTypeHapticFeedback = 0x03;
constexpr quint8 MessageTypeAppInfo = 0x04;
}

MyWhooshLink *MyWhooshLink::instance() {
    return s_instance;
}

MyWhooshLink::MyWhooshLink(bluetooth *manager, QObject *parent)
    : QObject(parent)
    , tcpServer(nullptr)
    , udpSocket(nullptr)
    , statusTimer(nullptr)
    , device(nullptr)
    , bluetoothManager(manager)
    , enabled(false)
    , overrideGears(false)
    , currentCameraAngle(1)
    , currentEmote(1)
    , leftPaddlePressed(false)
    , rightPaddlePressed(false)
    , mdnsServer(nullptr)
    , mdnsHostname(nullptr)
    , mdnsProvider(nullptr) {
    s_instance = this;
    qDebug() << "MyWhooshLink(OpenBikeControl): constructor";
    loadSettings();

    if (enabled) {
        start();

        statusTimer = new QTimer(this);
        connect(statusTimer, &QTimer::timeout, this, &MyWhooshLink::checkServerStatus);
        statusTimer->start(10000);
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
        qDebug() << "MyWhooshLink(OpenBikeControl): already running";
        return;
    }

    qDebug() << "MyWhooshLink(OpenBikeControl): network interfaces:";
    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            foreach (const QNetworkAddressEntry &entry, interface.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    qDebug() << " " << interface.name() << ":" << entry.ip().toString();
                }
            }
        }
    }

    tcpServer = new QTcpServer(this);
    connect(tcpServer, &QTcpServer::newConnection, this, &MyWhooshLink::onNewConnection);

    bool tcpSuccess = false;
    if (tcpServer->listen(QHostAddress::Any, PORT)) {
        tcpSuccess = true;
        qDebug() << "MyWhooshLink(OpenBikeControl): TCP listening on 0.0.0.0:" << PORT;
    } else if (tcpServer->listen(QHostAddress::AnyIPv6, PORT)) {
        tcpSuccess = true;
        qDebug() << "MyWhooshLink(OpenBikeControl): TCP listening on [::]:" << PORT;
    }

    if (!tcpSuccess) {
        qDebug() << "MyWhooshLink(OpenBikeControl): TCP bind failed:" << tcpServer->errorString();
        delete tcpServer;
        tcpServer = nullptr;
        return;
    }

    tcpServer->setMaxPendingConnections(10);

    udpSocket = new QUdpSocket(this);
    if (udpSocket->bind(QHostAddress::AnyIPv4, PORT,
                        QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        connect(udpSocket, &QUdpSocket::readyRead, this, &MyWhooshLink::onUdpReadyRead);
        qDebug() << "MyWhooshLink(OpenBikeControl): UDP listening on 0.0.0.0:" << PORT;
    } else {
        qDebug() << "MyWhooshLink(OpenBikeControl): UDP bind failed:" << udpSocket->errorString();
        udpSocket->deleteLater();
        udpSocket = nullptr;
    }

    initMdnsAdvertising();
}

void MyWhooshLink::stop() {
    for (QTcpSocket *client : clients) {
        client->disconnectFromHost();
        client->deleteLater();
    }
    clients.clear();

    if (udpSocket) {
        udpSocket->close();
        udpSocket->deleteLater();
        udpSocket = nullptr;
    }

    if (tcpServer) {
        tcpServer->close();
        tcpServer->deleteLater();
        tcpServer = nullptr;
    }

    if (mdnsProvider) {
        mdnsProvider->deleteLater();
        mdnsProvider = nullptr;
    }
    if (mdnsHostname) {
        mdnsHostname->deleteLater();
        mdnsHostname = nullptr;
    }
    if (mdnsServer) {
        mdnsServer->deleteLater();
        mdnsServer = nullptr;
    }

    qDebug() << "MyWhooshLink(OpenBikeControl): stopped";
}

void MyWhooshLink::initMdnsAdvertising() {
    if (mdnsServer) {
        return;
    }

    mdnsServer = new QMdnsEngine::Server(this);
    mdnsHostname = new QMdnsEngine::Hostname(mdnsServer, QByteArrayLiteral("qdomyos-openbikecontrol"), this);
    mdnsProvider = new QMdnsEngine::Provider(mdnsServer, mdnsHostname, this);

    QMdnsEngine::Service mdnsService;
    mdnsService.setType("_openbikecontrol._tcp.local.");
    mdnsService.setName("QDomyos-Zwift OpenBikeControl");
    mdnsService.setPort(PORT);

    const QByteArray id = QSysInfo::machineUniqueId().isEmpty()
                              ? QByteArrayLiteral("qdomyoszwift")
                              : QSysInfo::machineUniqueId().toHex();

    mdnsService.addAttribute(QByteArrayLiteral("version"), QByteArrayLiteral("1"));
    mdnsService.addAttribute(QByteArrayLiteral("id"), id);
    mdnsService.addAttribute(QByteArrayLiteral("name"), QByteArrayLiteral("QDomyos-Zwift OpenBikeControl"));
    mdnsService.addAttribute(QByteArrayLiteral("service-uuids"),
                             QByteArrayLiteral("d273f680-d548-419d-b9d1-fa0472345229"));
    mdnsService.addAttribute(QByteArrayLiteral("manufacturer"), QByteArrayLiteral("QDomyos-Zwift"));
    mdnsService.addAttribute(QByteArrayLiteral("model"), QByteArrayLiteral("VirtualBike"));

    mdnsProvider->update(mdnsService);
    qDebug() << "MyWhooshLink(OpenBikeControl): mDNS published _openbikecontrol._tcp.local. on port" << PORT;
}

void MyWhooshLink::onNewConnection() {
    while (tcpServer->hasPendingConnections()) {
        QTcpSocket *client = tcpServer->nextPendingConnection();
        clients.append(client);

        connect(client, &QTcpSocket::disconnected, this, &MyWhooshLink::onClientDisconnected);
        connect(client, &QTcpSocket::readyRead, this, &MyWhooshLink::onReadyRead);

        qDebug() << "MyWhooshLink(OpenBikeControl): TCP client connected from" << client->peerAddress().toString();

        // Send initial device status (battery unknown, connected/ready).
        QByteArray status;
        status.append(static_cast<char>(MessageTypeDeviceStatus));
        status.append(static_cast<char>(0xFF));
        status.append(static_cast<char>(0x01));
        client->write(status);
        client->flush();
    }
}

void MyWhooshLink::onClientDisconnected() {
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (client) {
        qDebug() << "MyWhooshLink(OpenBikeControl): TCP client disconnected" << client->peerAddress().toString();
        clients.removeAll(client);
        client->deleteLater();
    }
}

void MyWhooshLink::onReadyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (!client) {
        return;
    }

    const QByteArray data = client->readAll();
    if (!data.isEmpty()) {
        processIncomingData(data, QStringLiteral("tcp"));
    }
}

void MyWhooshLink::onUdpReadyRead() {
    if (!udpSocket) {
        return;
    }

    while (udpSocket->hasPendingDatagrams()) {
        const QNetworkDatagram datagram = udpSocket->receiveDatagram();
        if (!datagram.data().isEmpty()) {
            processIncomingData(datagram.data(), QStringLiteral("udp"));
        }
    }
}

void MyWhooshLink::processIncomingData(const QByteArray &data, const QString &sourceTag) {
    if (data.isEmpty()) {
        return;
    }

    const quint8 messageType = static_cast<quint8>(data.at(0));
    const QByteArray payload = data.mid(1);
    logIncomingMessage(messageType, payload, sourceTag);
}

void MyWhooshLink::logIncomingMessage(quint8 messageType, const QByteArray &payload, const QString &sourceTag) const {
    switch (messageType) {
        case MessageTypeButtonState:
            qDebug() << "MyWhooshLink(OpenBikeControl): incoming button-state from" << sourceTag
                     << payload.toHex(' ');
            break;
        case MessageTypeDeviceStatus:
            qDebug() << "MyWhooshLink(OpenBikeControl): incoming device-status from" << sourceTag
                     << payload.toHex(' ');
            break;
        case MessageTypeHapticFeedback:
            qDebug() << "MyWhooshLink(OpenBikeControl): incoming haptic-feedback from" << sourceTag
                     << payload.toHex(' ');
            break;
        case MessageTypeAppInfo:
            qDebug() << "MyWhooshLink(OpenBikeControl): incoming app-info from" << sourceTag
                     << payload.toHex(' ');
            break;
        default:
            qDebug() << "MyWhooshLink(OpenBikeControl): incoming unknown msg type" << messageType
                     << "from" << sourceTag << payload.toHex(' ');
            break;
    }
}

quint8 MyWhooshLink::actionToButtonId(Action action) const {
    switch (action) {
        case GearUp:
            return 0x01;
        case GearDown:
            return 0x02;
        case SteerLeft:
            return 0x18;
        case SteerRight:
            return 0x19;
        case UTurn:
            return 0x80; // app-specific custom action
        case CameraAngle:
            return 0x40;
        case Emote:
            return 0x20;
        case Tuck:
            return 0x81; // app-specific custom action
        case Disabled:
        default:
            return 0x00;
    }
}

quint8 MyWhooshLink::actionToButtonState(Action action, bool keyDown) const {
    if (!keyDown) {
        return 0x00;
    }

    switch (action) {
        case CameraAngle:
            return static_cast<quint8>(qBound(1, currentCameraAngle, 255));
        case Emote:
            return static_cast<quint8>(qBound(1, currentEmote, 255));
        default:
            return 0x01;
    }
}

void MyWhooshLink::sendButtonStateMessage(const QList<QPair<quint8, quint8>> &actions) {
    if (actions.isEmpty() || !isRunning()) {
        return;
    }

    QByteArray message;
    message.append(static_cast<char>(MessageTypeButtonState));
    for (const auto &action : actions) {
        message.append(static_cast<char>(action.first));
        message.append(static_cast<char>(action.second));
    }

    for (QTcpSocket *client : clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(message);
            client->flush();
        }
    }

    qDebug() << "MyWhooshLink(OpenBikeControl): sent button-state" << message.toHex(' ')
             << "to" << clients.size() << "tcp clients";
}

void MyWhooshLink::sendAction(Action action, bool keyDown) {
    if (action == Disabled) {
        return;
    }

    const quint8 buttonId = actionToButtonId(action);
    if (buttonId == 0x00) {
        return;
    }

    sendButtonStateMessage({qMakePair(buttonId, actionToButtonState(action, keyDown))});

    if (keyDown) {
        if (action == CameraAngle) {
            cycleCameraAngle();
        } else if (action == Emote) {
            cycleEmote();
        }
    }
}

void MyWhooshLink::sendSteering(int value) {
    QList<QPair<quint8, quint8>> actions;
    actions.append(qMakePair(static_cast<quint8>(0x18), static_cast<quint8>(value < 0 ? 0x01 : 0x00)));
    actions.append(qMakePair(static_cast<quint8>(0x19), static_cast<quint8>(value > 0 ? 0x01 : 0x00)));
    sendButtonStateMessage(actions);
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


void MyWhooshLink::syncGearValue(int targetGear) {
    if (!isEnabled() || !isRunning()) {
        return;
    }

    const int sanitizedTarget = qMax(0, targetGear);
    static const int hardResetSteps = 64;

    // Absolute sync strategy: force remote gear to minimum, then climb to target.
    for (int i = 0; i < hardResetSteps; ++i) {
        sendAction(GearDown, true);
    }
    for (int i = 0; i < sanitizedTarget; ++i) {
        sendAction(GearUp, true);
    }

    qDebug() << "MyWhooshLink(OpenBikeControl): synced absolute gear to" << sanitizedTarget;
}

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
    if (value < 0 && !leftPaddlePressed) {
        leftPaddlePressed = true;
        sendSteering(-1);
    } else if (value >= 0 && leftPaddlePressed) {
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
    if (value > 0 && !rightPaddlePressed) {
        rightPaddlePressed = true;
        sendSteering(1);
    } else if (value <= 0 && rightPaddlePressed) {
        rightPaddlePressed = false;
        sendSteering(0);
    }
}

void MyWhooshLink::checkServerStatus() {
    if (tcpServer && tcpServer->isListening()) {
        qDebug() << "MyWhooshLink(OpenBikeControl): ACTIVE on port" << PORT
                 << "TCP clients:" << clients.size()
                 << "UDP:" << (udpSocket ? "on" : "off");
    } else {
        qDebug() << "MyWhooshLink(OpenBikeControl): WARNING - TCP server not listening";
    }
}
