#ifndef SPORTSTECHF75TREADMILL_H
#define SPORTSTECHF75TREADMILL_H

#include <QByteArray>
#include <QTimer>

#include "devices/treadmill.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"

class sportstechf75treadmill : public treadmill {
    Q_OBJECT
  public:
    sportstechf75treadmill(bool noHeartService);
    ~sportstechf75treadmill() override;

    bool connected() override;
    double minStepInclination() override { return 1.0; }
    double minStepSpeed() override { return 0.1; }

  private slots:
    void changeInclinationRequested(double grade, double percentage);
    void update();

  signals:
    void debug(QString string);

  private:
    void openSerial();
    void closeSerial();
    void readSerial();
    void parseFrames();
    void handleFrame(const QByteArray &frame);
    void sendFrame(const QByteArray &frame);
    void sendStartupCommand();
    void sendRunningCommand(double speed, int incline, bool allowZero = false);
    void sendRunningProtocolCommand(int protocolSpeed, int incline, bool allowZero = false);
    void publishCommandedMetrics(double speed, int incline);
    void sendCommand(int command, int data1, int data2);
    QByteArray startupFrame() const;
    QByteArray runningFrame(double speed, int incline, bool allowZero = false) const;
    QByteArray runningProtocolFrame(int protocolSpeed, int incline, bool allowZero = false) const;
    QByteArray commandFrame(int command, int data1, int data2) const;
    bool serialAvailable() const;

    QTimer *refresh = nullptr;
    QByteArray receiveBuffer;
    bool serialOpened = false;
    bool commandActive = false;
    bool noHeartService = false;
    bool firstStateChanged = false;
    int runningCommandTicks = 0;
    int runningBurstFrames = 0;
    int startCountdownValue = -1;
    int startCountdownTicks = 0;
    int stopRunningFrameTicks = 0;
    double lastCommandSpeed = -1.0;
    int lastCommandIncline = -100;
    bool stopRunningFramePending = false;
    bool pauseRampPending = false;
    int pauseRampProtocolSpeed = -1;
    int pauseRampIncline = 0;
    int pauseRampTicks = 0;
};

#endif // SPORTSTECHF75TREADMILL_H
