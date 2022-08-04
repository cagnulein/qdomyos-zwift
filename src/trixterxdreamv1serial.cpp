#include "trixterxdreamv1serial.h"

#include <QDebug>
#include <QTime>
#include <QSerialPortInfo>

trixterxdreamv1serial::trixterxdreamv1serial(QObject *parent) : QThread(parent){}

trixterxdreamv1serial::~trixterxdreamv1serial() {
    this->mutex.lock();
    this->quitPending = true;
    this->mutex.unlock();
    this->wait();
}

QList<QSerialPortInfo> trixterxdreamv1serial::availablePorts()
{
    return QSerialPortInfo::availablePorts();
}

void trixterxdreamv1serial::open(const QString &portName, int waitTimeout) {
    const QMutexLocker locker(&this->mutex);
    this->portName = portName;
    this->waitTimeout = waitTimeout;
    if (!isRunning())
        start();
}

void trixterxdreamv1serial::write(const uint8_t *buffer, int len, QString info) {
    qDebug() << "serial >> " << QByteArray((const char *)buffer, len).toHex(' ') << "//" << info;
    qint64 o = serial.write(QByteArray((const char *)buffer, len));
    qDebug() << "serial byte written" << o;
}

void trixterxdreamv1serial::run() {

    bool currentPortNameChanged = false;

    this->mutex.lock();

    QString currentPortName;
    if (currentPortName != this->portName) {
        currentPortName = this->portName;
        currentPortNameChanged = true;
    }

    int currentWaitTimeout = this->waitTimeout;
    this->mutex.unlock();

    while (!this->quitPending) {
        if (currentPortNameChanged) {
            serial.close();
            serial.setPortName(currentPortName);
            serial.setBaudRate(QSerialPort::Baud115200);

            if (!serial.open(QIODevice::ReadWrite)) {
                qDebug() << tr("Can't open %1, error code %2").arg(this->portName).arg(serial.error());
                this->error(tr("Can't open %1, error code %2").arg(this->portName).arg(serial.error()));
                return;
            }
            qDebug() << "Serial port" << currentPortName << "opened";
        }

        if (serial.waitForReadyRead(currentWaitTimeout)) {
            QByteArray requestData = serial.readAll();
            while (serial.waitForReadyRead(1))
                requestData += serial.readAll();
            qDebug() << "serial << " << requestData.toHex(' ');

            // Send the bytes to the client code
            if(requestData.length()>0) {
                const QString request = QString::fromUtf8(requestData);
                this->receive(request);
            }
        }

        this->mutex.lock();
        if (currentPortName != this->portName) {
            currentPortName = this->portName;
            currentPortNameChanged = true;
        } else {
            currentPortNameChanged = false;
        }
        currentWaitTimeout = this->waitTimeout;
        this->mutex.unlock();
    }
}
