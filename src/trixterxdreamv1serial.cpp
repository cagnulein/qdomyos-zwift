#include "trixterxdreamv1serial.h"

#include <QDebug>
#include <QTime>
#include <QSerialPortInfo>

trixterxdreamv1serial::trixterxdreamv1serial(QObject *parent) : QThread(parent){}

trixterxdreamv1serial::~trixterxdreamv1serial() {
    this->quitPending = true;
    this->wait();
}

QList<QSerialPortInfo> trixterxdreamv1serial::availablePorts() {
    return QSerialPortInfo::availablePorts();
}

void trixterxdreamv1serial::open(const QString &portName, QSerialPort::BaudRate baudRate, int waitTimeout) {
    if(this->isRunning())
    {
        qDebug() << "Port is already being monitored.";
        this->error("Port is already being monitored.");
        return;
    }

    const QMutexLocker locker(&this->mutex);
    this->portName = portName;
    this->baudRate = baudRate;
    this->waitTimeout = waitTimeout;
    if (!isRunning())
        start();    
}

void trixterxdreamv1serial::write(const QByteArray& buffer, QString info) {
    qDebug() << "serial >> " << buffer.toHex(' ') << "//" << info;

    // obtain a mutex lock to avoid writing during a read
    QMutexLocker locker(&this->mutex);

    // write the data
    qint64 o = this->serial.write(buffer);

    locker.unlock();

    qDebug() << "serial byte written" << o;
}

void trixterxdreamv1serial::run() {

    serial.setPortName(this->portName);
    serial.setBaudRate(this->baudRate);

    if (!serial.open(QIODevice::ReadWrite)) {
        qDebug() << tr("Can't open %1, error code %2").arg(this->portName).arg(serial.error());
        this->error(tr("Can't open %1, error code %2").arg(this->portName).arg(serial.error()));
        return;
    }
    qDebug() << "Serial port" << this->portName << "opened";

    while (!this->quitPending) {
        QByteArray requestData;

        // Obtain a mutex lock so it's not waiting for ready read while trying to write...
        QMutexLocker locker(&this->mutex);

        // try to read some bytes, but only block for 1ms because a write attempt could come in.
        while (this->serial.waitForReadyRead(1))
            requestData += this->serial.readAll();

        // release the mutex
        locker.unlock();

        if(requestData.length()>0) {

            qDebug() << "serial << " << requestData.toHex(' ');

            // Send the bytes to the client code
            // Unlike the QtSerialPort example that can be found online, this
            // is NOT emitting a signal. This is avoid problems with slots, threads and timers,
            // which seem to require an advanced course to get working together!
            this->receive(requestData);

            qDebug() << requestData.length() << " bytes processed";
        }
    }

    this->serial.close();
    qDebug() << "Serial port closed";
}
