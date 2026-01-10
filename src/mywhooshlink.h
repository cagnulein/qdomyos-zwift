#ifndef MYWHOOSHLINK_H
#define MYWHOOSHLINK_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>
#include "qzsettings.h"

class bluetoothdevice;
class bluetooth;

class MyWhooshLink : public QObject {
    Q_OBJECT

public:
    enum Action {
        Disabled = 0,
        GearUp = 1,
        GearDown = 2,
        SteerLeft = 3,
        SteerRight = 4,
        UTurn = 5,
        CameraAngle = 6,
        Emote = 7
        // Space for future actions: PowerUp, RideOn, etc.
    };
    Q_ENUM(Action)

    explicit MyWhooshLink(bluetooth *manager = nullptr, QObject *parent = nullptr);
    ~MyWhooshLink();

    bool isRunning() const;
    void start();
    void stop();

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

private:
    void sendAction(Action action, bool keyDown = true);
    void sendSteering(int value);
    QString actionToJsonField(Action action) const;
    QString actionToJsonValue(Action action, bool keyDown) const;
    void cycleCameraAngle();
    void cycleEmote();

    QTcpServer *tcpServer;
    QList<QTcpSocket *> clients;
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

    void loadSettings();
    void sendJsonMessage(const QJsonObject &message);
};

#endif // MYWHOOSHLINK_H
