#ifndef MYWHOOSHLINK_H
#define MYWHOOSHLINK_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QList>
#include <QSettings>
#include <QTimer>
#include <qmdnsengine/server.h>
#include <qmdnsengine/hostname.h>
#include <qmdnsengine/provider.h>
#include "qzsettings.h"

class bluetoothdevice;
class bluetooth;

class MyWhooshLink : public QObject {
    Q_OBJECT

public:
    static MyWhooshLink *instance();

    // Kept for backward compatibility with existing settings.
    enum Action {
        Disabled = 0,
        GearUp = 1,
        GearDown = 2,
        SteerLeft = 3,
        SteerRight = 4,
        UTurn = 5,
        CameraAngle = 6,
        Emote = 7,
        Tuck = 8
    };
    Q_ENUM(Action)

    explicit MyWhooshLink(bluetooth *manager = nullptr, QObject *parent = nullptr);
    ~MyWhooshLink();

    bool isEnabled() const;
    bool isRunning() const;
    bool overrideLocalGears() const;
    void start();
    void stop();
    void handleGearUp(bool pressed = true);
    void handleGearDown(bool pressed = true);
    void syncGearValue(int targetGear);

public slots:
    // Zwift Play button handlers
    void handleLeftUp(bool pressed);
    void handleLeftDown(bool pressed);
    void handleLeftLeft(bool pressed);
    void handleLeftRight(bool pressed);
    void handleLeftShoulder(bool pressed);
    void handleLeftPower(bool pressed);
    void handleLeftPaddle(int value);  // Analog: negative=left, 0=neutral, positive=right

    void handleRightY(bool pressed);
    void handleRightA(bool pressed);
    void handleRightB(bool pressed);
    void handleRightZ(bool pressed);
    void handleRightShoulder(bool pressed);
    void handleRightPower(bool pressed);
    void handleRightPaddle(int value); // Analog: negative=left, 0=neutral, positive=right

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onReadyRead();
    void onUdpReadyRead();
    void checkServerStatus();
    void processQueuedGearAction();

private:
    static MyWhooshLink *s_instance;

    void sendAction(Action action, bool keyDown = true);
    void sendSteering(int value);
    void sendButtonStateMessage(const QList<QPair<quint8, quint8>> &actions);
    quint8 actionToButtonId(Action action) const;
    quint8 actionToButtonState(Action action, bool keyDown) const;
    void processIncomingData(const QByteArray &data, const QString &sourceTag);
    void logIncomingMessage(quint8 messageType, const QByteArray &payload, const QString &sourceTag) const;
    void cycleCameraAngle();
    void cycleEmote();
    void initMdnsAdvertising();

    QTcpServer *tcpServer;
    QUdpSocket *udpSocket;
    QList<QTcpSocket *> clients;
    QTimer *statusTimer;
    bluetoothdevice *device;
    bluetooth *bluetoothManager;
    QSettings settings;

    bool enabled;
    bool overrideGears;
    static const int PORT = 21587;

    // Button action mappings
    Action leftUpAction;
    Action leftDownAction;
    Action leftLeftAction;
    Action leftRightAction;
    Action leftShoulderAction;
    Action leftPowerAction;

    Action rightYAction;
    Action rightAAction;
    Action rightBAction;
    Action rightZAction;
    Action rightShoulderAction;
    Action rightPowerAction;

    // Cycling values for camera and emote
    int currentCameraAngle;
    int currentEmote;

    // Paddle state tracking for auto-release
    bool leftPaddlePressed;
    bool rightPaddlePressed;
    QTimer *gearSyncTimer;
    QList<Action> pendingGearActions;
    int estimatedRemoteGear;

    QMdnsEngine::Server *mdnsServer;
    QMdnsEngine::Hostname *mdnsHostname;
    QMdnsEngine::Provider *mdnsProvider;

    void loadSettings();
};

#endif // MYWHOOSHLINK_H
