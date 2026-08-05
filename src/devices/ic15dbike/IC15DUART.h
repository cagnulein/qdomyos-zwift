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

#ifndef IC15DUART_H
#define IC15DUART_H

#include "IC15DProtocol.h"
#include <QByteArray>
#include <QMutex>
#include <QString>
#include <QThread>

// Inspire IC15D internal UART: 19200 8N1 on /dev/ttyS2, owned/configured by the stock
// console app (com.inspire.inspireconsole.cycle).
#define IC15D_DEFAULT_SERIAL_PORT "/dev/ttyS2"
#define IC15D_BAUD 19200

// Receive-only reader for the Inspire IC15D internal UART.
//
// This driver deliberately never flushes, reconfigures (termios), or writes to the port: the
// stock console app already owns and configures /dev/ttyS2, and per GitHub issue #4888 this
// first implementation must only listen, not interfere with the active stock-app session.
// There is intentionally no query/resistance-write API on this class.
class IC15DUART : public QThread {
  public:
    explicit IC15DUART(QObject *parent = nullptr,
                        QString deviceFilename = QStringLiteral(IC15D_DEFAULT_SERIAL_PORT));
    ~IC15DUART();

    int stop();

    // Thread-safe telemetry getters.
    void getTelemetry(int &rpm, int &level, bool &valid);

  private:
    void run() override;

    // Opens the port read-only. No tcflush(), no tcgetattr()/tcsetattr(): the port is left
    // exactly as the stock app configured it.
    int openPort();
    int closePort();
    int rawRead(char *buf, int maxLen, int timeoutMs);

    QMutex pvars;
    volatile bool doStop = false;

    volatile int devRpm = 0;
    volatile int devLevel = 0;
    volatile bool devValid = false;

    QString deviceFilename;
    QByteArray rxBuffer;

    int devicePort = -1;
};

#endif // IC15DUART_H
