/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _KettlerUSB_h
#define _KettlerUSB_h 1

#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QString>
#include <QThread>

#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#ifndef N_TTY
#define N_TTY 0
#endif
#endif

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#endif

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/* Kettler USB Protocol Constants */

/* read timeouts in microseconds */
#define KETTLER_READTIMEOUT 2000
#define KETTLER_WRITETIMEOUT 2000

/* Control status */
#define KETTLER_RUNNING 0x01
#define KETTLER_PAUSED 0x02

/* default operation mode */
#define DEFAULT_POWER 100.00

class KettlerUSB : public QThread {

  public:
    KettlerUSB(QObject *parent = 0, QString deviceFilename = 0);
    ~KettlerUSB();

    QObject *parent;

    // HIGH-LEVEL FUNCTIONS
    int start();                           // Calls QThread to start
    int restart();                         // restart after paused
    int pause();                           // pauses data collection
    int stop();                            // stops data collection thread
    int quit(int error);                   // called by thread before exiting
    bool discover(QString deviceFilename); // confirm Kettler is attached

    // SET
    void setDevice(QString deviceFilename); // setup the device filename
    void setPower(double power);            // set the target power in Watts

    // GET TELEMETRY AND STATUS
    void getTelemetry(double &Power, double &HeartRate, double &Cadence, double &Speed,
                      double &Distance, int &Status);

  private:
    void run() override; // called by start to kick off the control thread

    // Utility and BG Thread functions
    int openPort();
    int closePort();

    // Protocol encoding/decoding
    int sendCommand(const QString &command);
    QString readResponse();
    void parseStatusResponse(const QString &response);
    void initSequence(); // Send initialization commands

    // Mutex for controlling accessing private data
    QMutex pvars;

    // INBOUND TELEMETRY - all volatile since updated by run() thread
    volatile double devicePower;      // current output power in Watts
    volatile double deviceHeartRate;  // current heartrate in BPM
    volatile double deviceCadence;    // current cadence in RPM
    volatile double deviceSpeed;      // current speed in KPH
    volatile double deviceDistance;   // current distance in meters
    volatile int deviceStatus;        // Device status running, paused, disconnected

    // OUTBOUND COMMANDS - all volatile since updated by GUI thread
    volatile double targetPower;
    volatile bool writePower; // Flag to write power on next poll

    // device port
    QString deviceFilename;
#ifdef WIN32
    HANDLE devicePort;
    DCB deviceSettings;
#else
    int devicePort;
    struct termios deviceSettings;
#endif

    // raw device utils
    int rawWrite(const char *bytes, int size);
    QString rawRead();
    void flushSerialBuffer(); // Flush any pending data in serial buffer

#ifdef Q_OS_ANDROID
    QByteArray rxBuffer;
#endif
};

class KettlerSleeper : public QThread {
  public:
    static void msleep(unsigned long msecs);
};

#endif // _KettlerUSB_h
