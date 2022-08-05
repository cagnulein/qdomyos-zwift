#include "trixterxdreamv1serial.h"

#include <QDebug>
#include <QTime>
#include <QSerialPortInfo>

trixterxdreamv1serial::trixterxdreamv1serial(QObject *parent) : QThread(parent){}

trixterxdreamv1serial::~trixterxdreamv1serial() {
    this->quitPending = true;
    this->wait();
}

QList<QSerialPortInfo> trixterxdreamv1serial::availablePorts()
{
    return QSerialPortInfo::availablePorts();
}

void trixterxdreamv1serial::open(const QString &portName, QSerialPort::BaudRate baudRate, int waitTimeout) {
    const QMutexLocker locker(&this->mutex);
    this->portName = portName;
    this->baudRate = baudRate;
    this->waitTimeout = waitTimeout;
    if (!isRunning())
        start();
}

void trixterxdreamv1serial::write(const QByteArray& buffer, QString info) {
    qDebug() << "serial >> " << buffer.toHex(' ') << "//" << info;
    QMutexLocker locker(&this->mutex);
    qint64 o = serial.write(buffer);
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

        if (this->serial.waitForReadyRead(this->waitTimeout)) {
            QMutexLocker locker(&this->mutex);
            QByteArray requestData = this->serial.readAll();
            while (this->serial.waitForReadyRead(1))
                requestData += this->serial.readAll();
            locker.unlock();
            qDebug() << "serial << " << requestData.toHex(' ');

            // Send the bytes to the client code
            this->receive(requestData);
        }
    }

    this->serial.close();
}
