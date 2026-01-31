/*
 * Copyright (c) 2009 Mark Liversedge (liversedge@gmail.com),
                 2024 Marcel Verpaalen
 *
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

#ifndef _SERIALPORT_h
#define _SERIALPORT_h

#include "serialhandler.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#endif

#include <QString>
#include <QThread>
#include <QFile>
#include <QMutex>
#include <QDebug>

#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#ifndef N_TTY        // for OpenBSD, this is a hack
#define N_TTY 0
#endif
#endif

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

class Serialport : public SerialHandler {
  public:
    Serialport(QString deviceFilename, uint32_t baudRate);
    ~Serialport() override;

    // Device management
    void setDevice(const QString &devname) override;
    void setTimeout(int timeout) override;
    void setEndChar(uint8_t endChar) override;

    // Port control
    int openPort() override;
    int dataAvailable() override;
    int closePort() override;

    // Data transfer
    int rawWrite(uint8_t *bytes, int size) override;
    int rawRead(uint8_t bytes[], int size, bool line = false) override;

    bool isOpen() const override;

  private:
    uint32_t baudRate = 9600;
    uint8_t endChar = 0x0D;
    int _timeout = 1200;
    QString deviceFilename;

    // device port
#ifdef WIN32
    HANDLE devicePort = INVALID_HANDLE_VALUE;  // file descriptor for reading from com3
    DCB deviceSettings; // serial port settings baud rate et al
#else
    int devicePort;                // unix!!
    struct termios deviceSettings; // unix!!
#endif

#ifdef Q_OS_ANDROID
    QList<jbyte> bufRX;
    bool cleanFrame = false;
#endif
};

#endif // _SERIALPORT_h
