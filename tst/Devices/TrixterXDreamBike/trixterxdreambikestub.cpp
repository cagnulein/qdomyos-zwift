#include "trixterxdreambikestub.h"
#include <QDateTime>
#include <QString>
#include <QThread>


static uint32_t getTime() {
    auto ms = QDateTime::currentMSecsSinceEpoch();
    return static_cast<uint32_t>(ms);
}

bool TrixterXDreamBikeStub::tryPopulate() {
    QMutexLocker locker(&this->mutex);

    uint32_t time = getTime();
    uint32_t last = this->lastAddedData;

    uint32_t delta = (time-last) / readInterval;

    if(delta==0)
        return false;

    this->lastAddedData = time;

    if(delta>100) delta = 100;

    static std::string packet= "6a7f4500000000000000000000005000";

    for(;delta>0; delta--)
        for(size_t i=0; i<packet.length() && this->readBuffer.size()<bufferCapacity; i++)
            this->readBuffer.push(packet[i]);

    while(this->readBuffer.size()>bufferCapacity)
        this->readBuffer.pop();

    return true;
}

serialdatasource *TrixterXDreamBikeStub::create(QObject * parent) { return new TrixterXDreamBikeStub(); }

TrixterXDreamBikeStub::TrixterXDreamBikeStub() : serialdatasource()
{

}

QStringList TrixterXDreamBikeStub::get_availablePorts() {
    return QStringList("stub");
}

bool TrixterXDreamBikeStub::open(const QString& portName) {
    this->lastAddedData = getTime()-readInterval;
    return true;
}

qint64 TrixterXDreamBikeStub::write(const QByteArray &data) {
    bytesWritten.append(data);
    return data.size();
}

void TrixterXDreamBikeStub::flush() {

}

bool TrixterXDreamBikeStub::waitForReadyRead() {
    QThread::msleep(1);
    return this->readBufferSize()>0;
}

QByteArray TrixterXDreamBikeStub::readAll() {
    QByteArray result;
    QMutexLocker locker(&this->mutex);
    auto count = this->readBufferSize();
    result.reserve(count);
    for(int i=0; i<count; i++) {
        result.append(this->readBuffer.front());
        this->readBuffer.pop();
    }
    return result;
}

qint64 TrixterXDreamBikeStub::readBufferSize() {
    QMutexLocker locker(&this->mutex);
    this->tryPopulate();
    return this->readBuffer.size();
}

QString TrixterXDreamBikeStub::error() { return "NoError";}

void TrixterXDreamBikeStub::close() {

}
