#include "csaferunner.h"

CsafeRunnerThread::CsafeRunnerThread() {}

CsafeRunnerThread::CsafeRunnerThread(QString deviceFileName, int sleepTime) {
    setDevice(deviceFileName);
    setSleepTime(sleepTime);
}

void CsafeRunnerThread::setDevice(const QString &device) { deviceName = device; }

void CsafeRunnerThread::setBaudRate(uint32_t _baudRate) { baudRate = _baudRate; }

void CsafeRunnerThread::setSleepTime(int time) { sleepTime = time; }

void CsafeRunnerThread::addRefreshCommand(const QStringList &commands) {
    mutex.lock();
    refreshCommands.append(commands);
    mutex.unlock();
}

void CsafeRunnerThread::sendCommand(const QStringList &commands) {
    mutex.lock();
    if (commandQueue.size() < MAX_QUEUE_SIZE) {
        commandQueue.enqueue(commands);
        mutex.unlock();
    } else {
        qDebug() << "CSAFE port commands QUEUE FULL. Dropping commands" << commands;
    }
}

void CsafeRunnerThread::run() {

    int rc = 0;

    SerialHandler *serial = SerialHandler::create(deviceName, baudRate);
    serial->setEndChar(0xf2); // end of frame for CSAFE
    serial->setTimeout(1200); // CSAFE spec specifies 1s timeout

    csafe *csafeInstance = new csafe();
    int connectioncounter = 20; // counts timeouts. If 10 timeouts in a row, then the port is closed and reopened
    int refresh_nr = -1;
    QStringList refreshCommand = {};

    while (1) {

        if (connectioncounter > 10 || !serial->isOpen()) {
             serial->closePort();
            rc = serial->openPort();
            if (rc != 0) {
                emit portAvailable(false);
                connectioncounter++;
                qDebug() << "Error opening serial port " << deviceName << "rc=" << rc << " sleeping for "
                         << "5s";
                QThread::msleep(5000);
                continue;
            } else {
                emit portAvailable(true);
                connectioncounter = 0;
            }
        }

        int elapsed = 0;
        while (elapsed < sleepTime || sleepTime == -1) {
            QThread::msleep(50);
            elapsed += 50;
            // TODO: does not seem to work with netsocket as intended. (no data available)
            // Needs further testing, maybe because the port is already closed and needs to remain open.
            // No issue for current implementations as they do not use unsolicited slave data / cmdAutoUpload .
            if (serial->dataAvailable() > 0 || !commandQueue.isEmpty()) {
                qDebug() << "CSAFE port data available. " << serial->dataAvailable() << " bytes"
                         << "commands in queue: " << commandQueue.size();
                break;
            }
        }

        QByteArray ret;
        mutex.lock();
        if (!commandQueue.isEmpty()) {
            ret = csafeInstance->write(commandQueue.dequeue());
            qDebug() << "CSAFE port commands processed from queue. Remaining commands in queue: "
                     << commandQueue.size();
        } else {
            if (!(elapsed < sleepTime) || !refreshCommands.isEmpty()) {
                if (refreshCommands.length() > 0) {
                    refresh_nr++;
                    if (refresh_nr >= refreshCommands.length()) {
                        refresh_nr = 0;
                    }
                    QStringList refreshCommand = refreshCommands[refresh_nr];
                    ret = csafeInstance->write(refreshCommand);
                }
            }
        }
        mutex.unlock();

        if (!ret.isEmpty()) { // we have commands to send
            qDebug() << "CSAFE >> " << ret.toHex(' ');
            rc = serial->rawWrite((uint8_t *)ret.data(), ret.length());
            if (rc < 0) {
                qDebug() << "Error writing serial port " << deviceName << "rc=" << rc;
                connectioncounter++;
                continue;
            }
        } else {
            qDebug() << "CSAFE Slave unsolicited data present.";
        }

        static uint8_t rx[120];
        rc = serial->rawRead(rx, 120, true);
        if (rc > 0) {
            qDebug() << "CSAFE << " << QByteArray::fromRawData((const char *)rx, rc).toHex(' ') << " (" << rc << ")";
            connectioncounter = 0;
        } else {
            qDebug() << "Error reading serial port " << deviceName << " rc=" << rc;
            connectioncounter++;
            continue;
        }

        QVector<quint8> v;
        for (int i = 0; i < rc; i++)
            v.append(rx[i]);
        QVariantMap frame = csafeInstance->read(v);
        emit onCsafeFrame(frame);
        memset(rx, 0x00, sizeof(rx));
    }
    serial->closePort();
}
