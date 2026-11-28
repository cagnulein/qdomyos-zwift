#include "trixterxdreamserial.h"

#include <QDebug>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>
#include <memory>

#include "serialdatasource.h"

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
#include "qserialdatasource.h"
#endif

std::function<serialdatasource*(QObject *)> trixterxdreamserial::serialDataSourceFactory =
#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
        [](QObject * parent) { return new qserialdatasource(parent); };
#else
        nullptr;
#endif

trixterxdreamserial::trixterxdreamserial(QObject * parent) : QThread(parent){}

trixterxdreamserial::~trixterxdreamserial() {
    this->quitPending = true;
    this->wait();
}

QStringList trixterxdreamserial::availablePorts() {
    QStringList result;

    if(serialDataSourceFactory==nullptr)
        return QStringList();

    auto serialDataSource = std::unique_ptr<serialdatasource>(serialDataSourceFactory(nullptr));

    return serialDataSource->get_availablePorts();
}

void trixterxdreamserial::receive(const QByteArray &bytes) {
    if(this->receiveBytes)
        this->receiveBytes(bytes);
}

bool trixterxdreamserial::open(const QString &portName) {

    QMutexLocker locker(&this->mutex);

    if(this->isRunning())
    {
        qDebug() << "Port is already being monitored.";
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

void trixterxdreamserial::write(const QByteArray& buffer) {
    QMutexLocker locker(&this->writeBufferMutex);

    this->writeBuffer = buffer;
    this->writePending = 1;
}

void trixterxdreamserial::set_pulse(std::function<void ()> function, uint32_t pulseIntervalMilliseconds) {
    this->pulse = function;
    this->pulseIntervalMilliseconds = pulseIntervalMilliseconds;
}

void trixterxdreamserial::set_getTime(std::function<uint32_t ()> get_time_ms) {
    this->getTime = get_time_ms;
}

void trixterxdreamserial::run() {

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
        qDebug() << QStringLiteral("Can't open %1, error code %2").arg(this->portName).arg(serial->error());
        return;
    }

    qDebug() << "Serial port " << this->portName << " opened with read buffer size=" << serial->readBufferSize();

    // turn on log timings for debugging, e.g. to ensure that the outgoing data
    // is being written.
    bool logTimings = false;

    uint32_t pulseDue = 0;
    uint32_t lastWrite = 0;

    QByteArray requestData;
    requestData.reserve(4096);

    while (!this->quitPending) {

        if(this->pulse) {
            // See if the timer is due
            auto t0 = this->getTime();
            if(t0 >= pulseDue) {
                pulseDue = t0 + trixterxdreamserial::pulseIntervalMilliseconds;
                this->pulse();

                if(logTimings) {
                    auto dt = this->getTime() - t0;
                    if(dt > pulseTolerance) {
                        qDebug() << QStringLiteral("WARNING: pulse function took %1ms exceeding tolerance of %2ms")
                                     .arg(dt).arg(trixterxdreamserial::pulseTolerance);
                    }
                }
            }
        }

        if(this->writePending) {
            QMutexLocker locker{&this->writeBufferMutex};
            uint32_t t = this->getTime();
            qint64 bytes = 0;
            try {
                this->writePending = 0;
                bytes = serial->write(this->writeBuffer);
                serial->flush();
            } catch(std::exception const& e) {
                qDebug() <<  "Exception thrown writing to the serial data source : " << e.what();
                throw;
            } catch(...) {
                qDebug() <<  "Error thrown writing to the serial data source";
                throw;
            }

            if(logTimings) {
                uint32_t writeTime = this->getTime() - t;
                uint32_t dt = t - lastWrite;
                lastWrite = t;
                qDebug() << QStringLiteral("Wrote %1 bytes in %2ms after %3ms").arg(bytes).arg(writeTime).arg(dt);
            }
        }

        // try to read some bytes, but only block for 1ms because a write attempt could come in.
        if (!this->quitPending && serial->waitForReadyRead())
            requestData += serial->readAll();

        if(requestData.length()>0) {
            // Send the bytes to the client code
            // Unlike the QSerialPort example that can be found online, this
            // is NOT emitting a signal. This is avoid problems with slots, threads and timers,
            // which seem to require an advanced course to get working together!
            this->receive(requestData);
            requestData.clear();
        }
    }

    serial->close();
}
