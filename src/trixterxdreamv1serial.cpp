#include "trixterxdreamv1serial.h"

#include <QDebug>
#include <QTime>

#ifndef Q_OS_IOS
#ifndef Q_OS_ANDROID
#include <QSerialPort>
#include <QSerialPortInfo>
#endif
#endif


trixterxdreamv1serial::trixterxdreamv1serial(QObject *parent) : QThread(parent){}

trixterxdreamv1serial::~trixterxdreamv1serial() {
    this->quitPending = true;
    this->wait();
}

QStringList trixterxdreamv1serial::availablePorts(bool debug) {
    QStringList result;

#ifndef Q_OS_IOS
#ifndef Q_OS_ANDROID
    auto ports = QSerialPortInfo::availablePorts();
    for(const auto &port : ports) {
        QString portName = port.portName();

#if defined(Q_OS_LINUX)
        if(!portName.startsWith("ttyUSB"))
        {
            qDebug() << "Skipping port: " << portName << " because it doesn't start with ttyUSB";
            continue;
        }
#endif
        result.push_back(portName);

        if (debug) {
            qDebug() << "Found portName:" << portName
                     << "," << "description:" << port.description()
                     << "," << "vender identifier:" << port.vendorIdentifier()
                     << "," << "manufacturer:" << port.manufacturer()
                     << "," << "product identifier:" << port.productIdentifier()
                     << "," << "system location:" << port.systemLocation()
                     << "," << "isBusy:" << port.isBusy()
                     << "," << "isNull:" << port.isNull()
                     << "," << "serialNumber:" << port.serialNumber();
        }

    }

#endif
#endif
    return result;
}

void trixterxdreamv1serial::receive(const QByteArray &bytes) {
    if(this->receiveBytes)
        this->receiveBytes(bytes);
}

void trixterxdreamv1serial::error(const QString &s) {
    qDebug() << "Error in trixterxdreamv1serial: " << s;
}

bool trixterxdreamv1serial::open(const QString &portName) {

    QMutexLocker locker(&this->mutex);

    if(this->isRunning())
    {
        qDebug() << "Port is already being monitored.";
        this->error("Port is already being monitored.");
        return false;
    }

    this->portName = portName;

    if (!isRunning()) {
        this->openAttemptsPending=1;
        start();
    }

    locker.unlock();

    while(this->openAttemptsPending==1){
        QThread::msleep(10);
    }

    locker.relock();
    return this->isRunning();
}

void trixterxdreamv1serial::write(const QByteArray& buffer) {
    QMutexLocker locker(&this->writeBufferMutex);

    this->writeBuffer = buffer;
    this->writePending = 1;
}

bool trixterxdreamv1serial::get_SendReceiveLog() { return this->sendReceiveLog; }

void trixterxdreamv1serial::set_SendReceiveLog(bool value) { this->sendReceiveLog = value; }

void trixterxdreamv1serial::run() {

#ifdef Q_OS_IOS || Q_OS_ANDROID
    return;
#else

    QSerialPort serial { this };

    serial.setPortName(this->portName);
    serial.setBaudRate(QSerialPort::Baud115200);
    serial.setDataBits(QSerialPort::Data8);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    serial.setParity(QSerialPort::NoParity);
    serial.setReadBufferSize(4096);

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

    qDebug() << "Serial port " << this->portName << " opened with read buffer size=" << serial.readBufferSize();

    while (!this->quitPending) {
        QByteArray requestData;
        requestData.reserve(4096);

        bool log = this->get_SendReceiveLog();

        if(this->writePending) {
            QMutexLocker locker{&this->writeBufferMutex};

            try {
                this->writePending = 0;
                serial.write(this->writeBuffer);
            } catch(std::exception const& e) {
                qDebug() <<  "Exception thrown by QSerialPort::write : " << e.what();
                throw;
            } catch(...) {
                qDebug() <<  "Error thrown by QSerialPort::write";
                throw;
            }

            if(log)
                qDebug() << "write " << this->writeBuffer.size() << " bytes to serial port";
        }

        // try to read some bytes, but only block for 1ms because a write attempt could come in.
        while (!this->quitPending && serial.waitForReadyRead(1))
            requestData += serial.readAll();

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

    serial.close();
    qDebug() << "Serial port closed";
#endif
}
