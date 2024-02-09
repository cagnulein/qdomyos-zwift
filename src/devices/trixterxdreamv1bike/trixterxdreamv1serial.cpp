#include "trixterxdreamv1serial.h"

#include <QDebug>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>
#include <memory>

#include "serialdatasource.h"

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
#include "qserialdatasource.h"
#endif

std::function<serialdatasource*(QObject *)> trixterxdreamv1serial::serialDataSourceFactory =
#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
        [](QObject * parent) { return new qserialdatasource(parent); };
#else
        nullptr;
#endif

trixterxdreamv1serial::trixterxdreamv1serial(QObject * parent) : QThread(parent){}

trixterxdreamv1serial::~trixterxdreamv1serial() {
    this->quitPending = true;
    this->wait();
}

QStringList trixterxdreamv1serial::availablePorts() {
    QStringList result;

    if(serialDataSourceFactory==nullptr)
        return QStringList();

    auto serialDataSource = std::unique_ptr<serialdatasource>(serialDataSourceFactory(nullptr));

    return serialDataSource->get_availablePorts();
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

void trixterxdreamv1serial::run() {

    if(serialDataSourceFactory==nullptr)
        throw "No serial data source factory configured.";

    auto serial = std::unique_ptr<serialdatasource>(serialDataSourceFactory(nullptr));

    bool openResult = false;

    try {
        openResult = serial->open(this->portName);
        this->openAttemptsPending = 0;
    } catch(...) {
        this->openAttemptsPending = 0;
        throw;
    }

    if (!openResult) {
        qDebug() << tr("Can't open %1, error code %2").arg(this->portName).arg(serial->error());
        this->error(tr("Can't open %1, error code %2").arg(this->portName).arg(serial->error()));
        return;
    }

    qDebug() << "Serial port " << this->portName << " opened with read buffer size=" << serial->readBufferSize();

    while (!this->quitPending) {
        QByteArray requestData;
        requestData.reserve(4096);

        if(this->writePending) {
            QMutexLocker locker{&this->writeBufferMutex};

            try {
                this->writePending = 0;
                serial->write(this->writeBuffer);
            } catch(std::exception const& e) {
                qDebug() <<  "Exception thrown by QSerialPort::write : " << e.what();
                throw;
            } catch(...) {
                qDebug() <<  "Error thrown by QSerialPort::write";
                throw;
            }
        }

        // try to read some bytes, but only block for 1ms because a write attempt could come in.
        while (!this->quitPending && serial->waitForReadyRead())
            requestData += serial->readAll();

        if(requestData.length()>0) {
            // Send the bytes to the client code
            // Unlike the QtSerialPort example that can be found online, this
            // is NOT emitting a signal. This is avoid problems with slots, threads and timers,
            // which seem to require an advanced course to get working together!
            this->receive(requestData);
        }
    }

    serial->close();
}
