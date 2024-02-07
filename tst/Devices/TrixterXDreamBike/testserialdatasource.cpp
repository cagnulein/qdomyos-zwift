#include "testserialdatasource.h"
#include <QDateTime>
#include <QString>


static uint32_t getTime() {
    auto ms = QDateTime::currentMSecsSinceEpoch();
    return static_cast<uint32_t>(ms);
}

bool TestSerialDatasource::tryPopulate() {
    QMutexLocker locker(&this->mutex);

    uint32_t time = getTime();
    uint32_t last = this->lastReadTime;
    this->lastReadTime = time;
    uint32_t delta = (time-last) / readInterval;

    if(delta==0)
        return false;

    if(delta>100) delta = 100;

    static std::string packet= "6a7f4500000000000000000000005000";

    for(;delta>0; delta--)
        for(size_t i=0; i<packet.length(); i++)
            this->readBuffer.push(packet[i]);

    return true;
}

trixterxdreamv1serial::serialdatasource *TestSerialDatasource::create() { return new TestSerialDatasource(); }



TestSerialDatasource::TestSerialDatasource() : trixterxdreamv1serial::serialdatasource()
{

}

void TestSerialDatasource::appendTestData(const QByteArray &data) {
    QMutexLocker locker(&this->mutex);

    for(int i=0; i<data.size(); i++)
        this->readBuffer.push(data[i]);
}

bool TestSerialDatasource::open() {
    this->lastReadTime = getTime();
    return true;
}

qint64 TestSerialDatasource::write(const QByteArray &data) { bytesWritten.append(data); return data.size(); }

bool TestSerialDatasource::waitForReadyRead() {
    return this->readBufferSize()>0;
}

QByteArray TestSerialDatasource::readAll() {
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



qint64 TestSerialDatasource::readBufferSize() {

    QMutexLocker locker(&this->mutex);
    this->tryPopulate();
    return this->readBuffer.size();
}

QString TestSerialDatasource::error() { return "NoError";}

void TestSerialDatasource::close() { }
