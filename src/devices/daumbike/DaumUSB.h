/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef DAUMUSB_H
#define DAUMUSB_H

#include <QByteArray>
#include <QMutex>
#include <QString>
#include <QThread>

#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#else
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#ifndef N_TTY
#define N_TTY 0
#endif
#endif

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#endif

#include <stdint.h>

#define DAUM_RUNNING 0x01
#define DAUM_PAUSED 0x02

class DaumUSB : public QThread {
  public:
    DaumUSB(QObject *parent = nullptr, QString deviceFilename = QString(), int baudrate = 9600);
    ~DaumUSB() override;

    int start();
    int restart();
    int pause();
    int stop();

    void setPower(double power);
    void getTelemetry(double &power, double &heartRate, double &cadence, double &speed,
                      double &distance, int &status);
    bool connected();

  private:
    void run() override;

    int openPort();
    int closePort();
    int rawWrite(const char *bytes, int size);
    QByteArray rawReadBytes();
    QByteArray readResponse(int expectedLength, unsigned long timeoutMs);

    bool detectCockpit();
    bool pollTelemetry();
    bool writePowerTarget(double power);

    QMutex pvars;
    volatile double devicePower = 0.0;
    volatile double deviceHeartRate = 0.0;
    volatile double deviceCadence = 0.0;
    volatile double deviceSpeed = 0.0;
    volatile double deviceDistance = 0.0;
    volatile int deviceStatus = 0;

    volatile double targetPower = 100.0;
    volatile bool writePower = false;

    QString deviceFilename;
    int baudrate = 9600;
    uint8_t cockpitAddress = 0;

#ifdef WIN32
    HANDLE devicePort = INVALID_HANDLE_VALUE;
    DCB deviceSettings;
#else
    int devicePort = -1;
    struct termios deviceSettings;
#endif
};

#endif // DAUMUSB_H
