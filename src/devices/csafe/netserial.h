/*
 * Copyright (c) 2024 Marcel Verpaalen (marcel@verpaalen.com)
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
 *
 *
 */

#ifndef NETSERIAL_H
#define NETSERIAL_H

#include "serialhandler.h"
#include <QString>
#include <QHostAddress>
#include <QTcpSocket>
#include <QDebug>

/**
 * @brief This is a simple implementation of serial port emulation over TCP
 * It emulates a serial port over a network connection.
 * e.g. as created by ser2net or hardware serial to ethernet converters
 */
class NetSerial : public SerialHandler {
  public:
    NetSerial(QString deviceFilename);
    ~NetSerial() ;

    int openPort() override;
    int closePort() override;
    int dataAvailable() override;
    int rawWrite(uint8_t *bytes, int size) override;
    int rawRead(uint8_t bytes[], int size, bool line = false) override;

    bool isOpen() const override;
    void setTimeout(int timeout) override;
    void setEndChar(uint8_t endChar) override;
    void setDevice(const QString &devname) override;

  private:
    QString deviceFilename;
    QString serverAddress;
    quint16 serverPort;
    QTcpSocket *socket;
    int _timeout = 1000; // Timeout in milliseconds
    uint8_t endChar = '\n';
    bool parseDeviceFilename(const QString &filename);
};

#endif // NETSERIAL_H
