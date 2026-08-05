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

#include "IC15DUART.h"
#include <QDebug>

#ifndef WIN32
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#endif

IC15DUART::IC15DUART(QObject *parent, QString deviceFilename) : QThread(parent), deviceFilename(deviceFilename) {}

IC15DUART::~IC15DUART() {}

int IC15DUART::stop() {
    pvars.lock();
    doStop = true;
    pvars.unlock();
    return 0;
}

void IC15DUART::getTelemetry(int &rpm, int &level, bool &valid) {
    pvars.lock();
    rpm = devRpm;
    level = devLevel;
    valid = devValid;
    pvars.unlock();
}

/* ----------------------------------------------------------------------
 * PORT OPEN / CLOSE / READ
 * Receive-only: opened O_RDONLY, no tcflush(), no tcgetattr()/tcsetattr(). The IC15D console's
 * stock app already owns and configures /dev/ttyS2 at 19200 8N1; this driver only listens.
 * ---------------------------------------------------------------------- */
int IC15DUART::openPort() {
#ifndef WIN32
    devicePort = open(deviceFilename.toLatin1().constData(), O_RDONLY | O_NOCTTY | O_NONBLOCK);
    if (devicePort == -1)
        return errno;
    return 0;
#else
    Q_UNUSED(deviceFilename);
    return -1; // IC15D is an Android-console-internal UART; not applicable on Windows.
#endif
}

int IC15DUART::closePort() {
#ifndef WIN32
    if (devicePort == -1)
        return 0;
    int rc = close(devicePort);
    devicePort = -1;
    return rc;
#else
    return 0;
#endif
}

int IC15DUART::rawRead(char *buf, int maxLen, int timeoutMs) {
#ifndef WIN32
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(devicePort, &fds);
    struct timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    int rc = select(devicePort + 1, &fds, NULL, NULL, &tv);
    if (rc > 0)
        return read(devicePort, buf, maxLen);
    return 0;
#else
    Q_UNUSED(buf);
    Q_UNUSED(maxLen);
    Q_UNUSED(timeoutMs);
    return 0;
#endif
}

/* ----------------------------------------------------------------------
 * MAIN RUN LOOP
 * ---------------------------------------------------------------------- */
void IC15DUART::run() {
    if (openPort()) {
        qDebug() << "IC15DUART: failed to open port" << deviceFilename;
        return;
    }

    qDebug() << "IC15DUART: listening (receive-only) on" << deviceFilename;

    while (true) {
        pvars.lock();
        bool stopRequested = doStop;
        pvars.unlock();
        if (stopRequested)
            break;

        char buf[128];
        int n = rawRead(buf, sizeof(buf), 200);
        if (n > 0) {
            rxBuffer.append(buf, n);

            // Drain as many complete frames as are currently buffered.
            while (!rxBuffer.isEmpty()) {
                QByteArray wireFrame;
                int consumed = IC15DProtocol::findWireFrame(rxBuffer, wireFrame);
                if (consumed == 0)
                    break; // wait for more data
                if (consumed < 0) {
                    rxBuffer.remove(0, 1); // resync: discard stray byte before header
                    continue;
                }

                rxBuffer.remove(0, consumed);

                IC15DProtocol::ParsedFrame parsed = IC15DProtocol::parseWireFrame(wireFrame);
                if (!parsed.valid)
                    continue;

                pvars.lock();
                if (parsed.command == IC15DProtocol::CMD_RPM) {
                    devRpm = parsed.value;
                    devValid = true;
                } else if (parsed.command == IC15DProtocol::CMD_LEVEL) {
                    devLevel = parsed.value;
                    devValid = true;
                }
                pvars.unlock();
            }

            // Bound the resync buffer so a stream of garbage cannot grow it forever.
            if (rxBuffer.size() > 4096)
                rxBuffer.clear();
        }
    }

    closePort();
}
