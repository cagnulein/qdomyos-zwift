#include "inspireic15dserialreader.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QMutexLocker>

#ifndef Q_OS_WIN
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

inspireic15dserialreader::inspireic15dserialreader(QObject *parent, const QString &deviceFilename,
                                                   bool metricPollingEnabled)
    : QThread(parent), deviceFilename(deviceFilename), metricPollingEnabled(metricPollingEnabled) {}

inspireic15dserialreader::~inspireic15dserialreader() {
    requestInterruption();
    wait(1000);
}

void inspireic15dserialreader::snapshot(bool &isOpen, qint64 &bytes, QByteArray &chunk, QString &currentError,
                                        int &currentCadence, int &currentPower, int &currentResistance,
                                        qint64 &frames) const {
    QMutexLocker locker(&stateMutex);
    isOpen = portOpen;
    bytes = totalBytes;
    chunk = lastChunk;
    currentError = error;
    currentCadence = cadence;
    currentPower = power;
    currentResistance = resistance;
    frames = validFrames;
}

int inspireic15dserialreader::findFrameMarker(const QByteArray &buffer) {
    for (int i = 0; i < buffer.size(); ++i) {
        const quint8 byte = static_cast<quint8>(buffer.at(i));
        if (byte == 0xf1 || byte == 0xf3)
            return i;
    }
    return -1;
}

int inspireic15dserialreader::frameSizeAt(const QByteArray &buffer, int start) {
    if (buffer.size() - start < 3)
        return 0;

    const bool negativeAcknowledgement = static_cast<quint8>(buffer.at(start)) == 0xf3;
    const int payloadLength = negativeAcknowledgement ? 0 : static_cast<quint8>(buffer.at(start + 2));
    if (payloadLength > 32)
        return -1;

    return payloadLength + 5;
}

inspireic15dserialreader::ParsedFrame inspireic15dserialreader::parseFrame(const QByteArray &frame) {
    ParsedFrame result;
    if (frame.isEmpty())
        return result;

    const bool negativeAcknowledgement = static_cast<quint8>(frame.at(0)) == 0xf3;
    if (negativeAcknowledgement) {
        result.negativeAcknowledgement = true;
        return result;
    }

    const int frameSize = frame.size();
    const int payloadLength = frameSize - 5;
    if (payloadLength < 0)
        return result;

    if (static_cast<quint8>(frame.at(frameSize - 1)) != 0xf6)
        return result;

    quint8 checksum = 0;
    for (int i = 0; i <= payloadLength + 2; ++i)
        checksum = static_cast<quint8>(checksum + static_cast<quint8>(frame.at(i)));
    if (checksum != static_cast<quint8>(frame.at(payloadLength + 3)))
        return result;

    bool validValue = payloadLength > 0;
    for (int i = 3; i < payloadLength + 3; ++i) {
        const quint8 digit = static_cast<quint8>(frame.at(i));
        if (digit < '0' || digit > '9') {
            validValue = false;
            break;
        }
    }
    if (!validValue)
        return result;

    // The stock app treats the ASCII payload as least-significant digit first.
    int value = 0;
    for (int i = payloadLength + 2; i >= 3; --i)
        value = (value * 10) + (static_cast<quint8>(frame.at(i)) - '0');

    result.valid = true;
    result.type = static_cast<char>(static_cast<quint8>(frame.at(1)));
    result.value = value;
    return result;
}

void inspireic15dserialreader::run() {
#ifdef Q_OS_WIN
    QMutexLocker locker(&stateMutex);
    error = QStringLiteral("IC15D internal serial diagnostics are not supported on Windows");
#else
    // Polling is opt-in. With it disabled this remains a read-only diagnostic.
    const QByteArray filename = deviceFilename.toLocal8Bit();
    const int accessMode = metricPollingEnabled ? O_RDWR : O_RDONLY;
    const int fd = open(filename.constData(), accessMode | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        QMutexLocker locker(&stateMutex);
        error = QString::fromLocal8Bit(strerror(errno));
        qWarning() << "IC15D serial: unable to open" << deviceFilename << error;
        return;
    }

    termios options;
    if (tcgetattr(fd, &options) != 0) {
        QMutexLocker locker(&stateMutex);
        error = QStringLiteral("Unable to read serial settings: ") + QString::fromLocal8Bit(strerror(errno));
        close(fd);
        return;
    }
    cfmakeraw(&options);
    cfsetispeed(&options, B19200);
    cfsetospeed(&options, B19200);
    options.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
#ifdef CRTSCTS
    options.c_cflag &= ~CRTSCTS;
#endif
    options.c_cflag |= CS8 | CLOCAL | CREAD;
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        QMutexLocker locker(&stateMutex);
        error = QStringLiteral("Unable to configure 19200 8-N-1: ") +
                QString::fromLocal8Bit(strerror(errno));
        close(fd);
        return;
    }

    {
        QMutexLocker locker(&stateMutex);
        portOpen = true;
        error.clear();
    }
    qInfo() << "IC15D serial: opened at 19200 8-N-1; metric polling" << metricPollingEnabled
            << deviceFilename;

    const QByteArray readCommands[] = {
        QByteArray::fromHex("e845533032e2f6"), // Stock handshake.
        QByteArray::fromHex("f54136f6"),       // Cadence (RPM).
        QByteArray::fromHex("f5493ef6"),       // Resistance.
        QByteArray::fromHex("f54439f6"),       // Instantaneous power.
        QByteArray::fromHex("f5493ef6")        // Resistance, matching the stock cycle.
    };
    int commandIndex = 0;
    QElapsedTimer pollTimer;
    pollTimer.start();
    QByteArray pending;

    while (!isInterruptionRequested()) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(fd, &readSet);
        timeval timeout = {0, 30000};
        const int selected = select(fd + 1, &readSet, nullptr, nullptr, &timeout);
        if (selected < 0) {
            if (errno == EINTR)
                continue;
            QMutexLocker locker(&stateMutex);
            error = QString::fromLocal8Bit(strerror(errno));
            break;
        }
        if (selected > 0) {
            char buffer[256];
            const ssize_t count = read(fd, buffer, sizeof(buffer));
            if (count > 0) {
                const QByteArray received(buffer, static_cast<int>(count));
                pending.append(received);
                {
                    QMutexLocker locker(&stateMutex);
                    totalBytes += count;
                    lastChunk = received;
                }
                qInfo().noquote() << "IC15D serial RX:" << received.toHex(' ');

                while (!pending.isEmpty()) {
                    const int start = findFrameMarker(pending);
                    if (start < 0) {
                        pending.clear();
                        break;
                    }
                    if (start > 0)
                        pending.remove(0, start);

                    const int frameSize = frameSizeAt(pending, 0);
                    if (frameSize == 0)
                        break; // not enough bytes yet to know the frame size
                    if (frameSize < 0) {
                        pending.remove(0, 1); // implausible payload length; resync
                        continue;
                    }
                    if (pending.size() < frameSize)
                        break;

                    const QByteArray frame = pending.left(frameSize);
                    pending.remove(0, frameSize);

                    const ParsedFrame parsed = parseFrame(frame);
                    if (parsed.negativeAcknowledgement) {
                        qWarning().noquote() << "IC15D serial NAK:" << frame.toHex(' ');
                        continue;
                    }
                    if (!parsed.valid)
                        continue;

                    {
                        QMutexLocker locker(&stateMutex);
                        if (parsed.type == 'A')
                            cadence = parsed.value;
                        else if (parsed.type == 'D')
                            power = parsed.value;
                        else if (parsed.type == 'I')
                            resistance = parsed.value;
                        else
                            continue;
                        ++validFrames;
                    }
                    qInfo() << "IC15D metric" << QChar(parsed.type) << parsed.value;
                }
            } else if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                QMutexLocker locker(&stateMutex);
                error = QString::fromLocal8Bit(strerror(errno));
                break;
            }
        }

        if (metricPollingEnabled && pollTimer.elapsed() >= 200) {
            const QByteArray &command = readCommands[commandIndex];
            const ssize_t written = write(fd, command.constData(), static_cast<size_t>(command.size()));
            if (written != command.size()) {
                QMutexLocker locker(&stateMutex);
                error = QStringLiteral("Metric query write failed: ") + QString::fromLocal8Bit(strerror(errno));
                break;
            }
            qInfo().noquote() << "IC15D metric query TX:" << command.toHex(' ');
            commandIndex = (commandIndex + 1) % (sizeof(readCommands) / sizeof(readCommands[0]));
            pollTimer.restart();
        }
    }

    close(fd);
    QMutexLocker locker(&stateMutex);
    portOpen = false;
#endif
}
