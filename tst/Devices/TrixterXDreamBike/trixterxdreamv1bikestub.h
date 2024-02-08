#pragma once

#include <QByteArray>
#include <QMutex>
#include <queue>

#include "devices/trixterxdreamv1bike/trixterxdreamv1serial.h"


class TrixterXDreamV1BikeStub : public trixterxdreamv1serial::serialdatasource {

    QByteArray bytesWritten;
    std::queue<char> readBuffer;
    QRecursiveMutex mutex;

    ulong lastAddedData = 0;
    const ulong readInterval = 10;

protected:
    bool tryPopulate();

public:

    static trixterxdreamv1serial::serialdatasource* create();

    TrixterXDreamV1BikeStub();

    void appendTestData(const QByteArray& data);

    bool open() override;
    qint64 write(const QByteArray& data) override;
    bool waitForReadyRead() override;
    QByteArray readAll() override;
    qint64 readBufferSize() override;
    QString error() override;
    void close() override;
};
