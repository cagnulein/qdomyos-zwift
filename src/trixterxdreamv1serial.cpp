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

bool trixterxdreamv1serial::open(const QString &portName, QSerialPort::BaudRate baudRate) {

    QMutexLocker locker(&this->mutex);

    if(this->isRunning())
    {
        qDebug() << "Port is already being monitored.";
        this->error("Port is already being monitored.");
        return false;
    }

    this->portName = portName;
    this->baudRate = baudRate;


    if (!isRunning()) {
        this->openAttemptsPending=1;
        start();
    }

    locker.unlock();

    while(this->openAttemptsPending==1){
        QThread::msleep(10);
    }

    locker.relock();
    return serial.isOpen();
}

void trixterxdreamv1serial::write(const QByteArray& buffer, QString info) {
    bool log = this->get_SendReceiveLog();

    if(log)
        qDebug() << "serial >> " << buffer.toHex() << "//" << info;

    // obtain a mutex lock to avoid writing during a read
    QMutexLocker locker(&this->mutex);

    // write the data
    qint64 o = this->serial.write(buffer);

    locker.unlock();

    if(log)
        qDebug() << "serial byte written" << o;
}

bool trixterxdreamv1serial::get_SendReceiveLog() { return this->sendReceiveLog; }

void trixterxdreamv1serial::set_SendReceiveLog(bool value) { this->sendReceiveLog = value; }

void trixterxdreamv1serial::run() {

    this->serial.setPortName(this->portName);
    this->serial.setBaudRate(this->baudRate);
    this->serial.setDataBits(QSerialPort::Data8);
    this->serial.setStopBits(QSerialPort::OneStop);
    this->serial.setFlowControl(QSerialPort::NoFlowControl);
    this->serial.setParity(QSerialPort::NoParity);

    bool openResult = false;

    try {
        openResult = serial.open(QIODevice::ReadWrite);
        this->openAttemptsPending = 0;
    } catch(...) {
        this->openAttemptsPending = 0;
        throw;
    }

    if (!openResult) {
        qDebug() << tr("Can't open %1, error code %2").arg(this->portName).arg(serial.error());
        this->error(tr("Can't open %1, error code %2").arg(this->portName).arg(serial.error()));
        return;
    }

    qDebug() << "Serial port" << this->portName << "opened";

    while (!this->quitPending) {
        QByteArray requestData;
        requestData.reserve(4096);

        bool log = this->get_SendReceiveLog();

        // Obtain a mutex lock so it's not waiting for ready read while trying to write...
        QMutexLocker locker(&this->mutex);

        // try to read some bytes, but only block for 1ms because a write attempt could come in.
        int quit = 0;
        while (!(quit=this->quitPending) && this->serial.waitForReadyRead(1))
            requestData += this->serial.readAll();

        // release the mutex
        locker.unlock();

        if(quit) break;

        if(requestData.length()>0) {

            if(log)
                qDebug() << "serial << " << requestData.toHex(' ');

            // Send the bytes to the client code
            // Unlike the QtSerialPort example that can be found online, this
            // is NOT emitting a signal. This is avoid problems with slots, threads and timers,
            // which seem to require an advanced course to get working together!
            this->receive(requestData);

            if(log)
                qDebug() << requestData.length() << " bytes processed";
        }
    }

    this->serial.close();
    qDebug() << "Serial port closed";
}
