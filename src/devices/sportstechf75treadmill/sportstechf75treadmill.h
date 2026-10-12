#ifndef SPORTSTECHF75TREADMILL_H
#define SPORTSTECHF75TREADMILL_H

#include <QByteArray>
#include <QTimer>

#include "devices/treadmill.h"

class sportstechf75treadmill : public treadmill {
    Q_OBJECT
  public:
    sportstechf75treadmill();
    ~sportstechf75treadmill() override;

    bool connected() override;
    double minStepInclination() override { return 1.0; }
    double minStepSpeed() override { return 1.0; }

  private slots:
    void update();

  private:
    void openSerial();
    void closeSerial();
    void readSerial();
    void parseFrames();
    void handleFrame(const QByteArray &frame);
    void sendCommand(int command, int data1, int data2);
    QByteArray commandFrame(int command, int data1, int data2) const;
    bool serialAvailable() const;

    QTimer *refresh = nullptr;
    QByteArray receiveBuffer;
    bool serialOpened = false;
    bool commandActive = false;
    int lastCommandSpeed = -1;
    int lastCommandIncline = -100;
};

#endif // SPORTSTECHF75TREADMILL_H
