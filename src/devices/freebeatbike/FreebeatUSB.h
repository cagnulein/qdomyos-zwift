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

#ifndef FREEBEATUSB_H
#define FREEBEATUSB_H

#include <QDebug>
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

// Freebeat internal UART: 9600 8N1 on /dev/ttyS4
#define FREEBEAT_BAUD 9600

// Protocol byte constants
#define FREEBEAT_SYNC_DATA    0x55
#define FREEBEAT_SYNC_LED     0xE5
#define FREEBEAT_TYPE_A       0x15
#define FREEBEAT_TYPE_B       0x03
#define FREEBEAT_TYPE_C       0x25
#define FREEBEAT_VERIFY_BYTE  0x3F
#define FREEBEAT_END_BYTE     0xA0

// Command start codes
#define FREEBEAT_CMD_START    0x25
#define FREEBEAT_CMD_LED      0xB5

// Command actions
#define FREEBEAT_ACT_RESISTANCE  0x03
#define FREEBEAT_ACT_DAME_MIX    0x05
#define FREEBEAT_ACT_DAME_MAX    0x06
#define FREEBEAT_ACT_RESET       0x07
#define FREEBEAT_ACT_QUERY       0x08
#define FREEBEAT_ACT_STOP        0x09
#define FREEBEAT_ACT_VERSION     0x2F
#define FREEBEAT_ACT_LED_CHECK   0x08  // startCode=0xB5
#define FREEBEAT_ACT_LED_CHANGE  0x03  // startCode=0xB5

class FreebeatUSB : public QThread {
  public:
    FreebeatUSB(QObject *parent = nullptr, QString deviceFilename = QString(), int baudrate = FREEBEAT_BAUD);
    ~FreebeatUSB();

    int start();
    int stop();

    // Commands
    void setResistance(int resistance); // 1..100
    void sendQuery();
    void sendStop();
    void sendReset();

    // Telemetry getters (thread-safe)
    void getTelemetry(int &rpm, int &resistance, double &speed, double &watt, bool &valid);

  private:
    void run() override;

    int openPort();
    int closePort();

    // Build 5-byte command: [startCode, action, data, checksum, 0xA0]
    QByteArray buildCmd5(uint8_t startCode, uint8_t action, uint8_t data);
    // Build 4-byte LED query: [0xB5, 0x08, 0x08, 0xA0]
    QByteArray buildLedQuery();

    int rawWrite(const char *bytes, int size);
    int rawRead(char *buf, int maxLen, int timeoutMs);

    bool parsePacket(const QByteArray &pkt);

    QMutex pvars;

    // Outbound
    volatile int targetResistance;
    volatile bool writeResistance;
    volatile bool doStop;
    volatile bool doReset;
    volatile bool doQuery;

    // Inbound telemetry
    volatile int devRpm;
    volatile int devResistance;
    volatile double devSpeed;
    volatile double devWatt;
    volatile bool devValid;

    QString deviceFilename;
    int baudrate;

#ifdef WIN32
    HANDLE devicePort;
    DCB deviceSettings;
#else
    int devicePort;
    struct termios deviceSettings;
#endif

#ifdef Q_OS_ANDROID
    QByteArray rxBuffer;
#endif
};

#endif // FREEBEATUSB_H
