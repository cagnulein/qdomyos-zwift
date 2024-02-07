#pragma once

#include <QByteArray>
#include <QMutex>
#include <queue>

#include "trixterxdreamv1serial.h"


class TestSerialDatasource : public trixterxdreamv1serial::serialdatasource {

    QByteArray bytesWritten;
    std::queue<char> readBuffer;
    QRecursiveMutex mutex;

    ulong lastReadTime = 0;
    const ulong readInterval = 10;

protected:
    bool tryPopulate();

public:

    static trixterxdreamv1serial::serialdatasource* create();

    TestSerialDatasource();

    void appendTestData(const QByteArray& data);

    bool open() override;
    qint64 write(const QByteArray& data) override;
    bool waitForReadyRead() override;
    QByteArray readAll() override;
    qint64 readBufferSize() override;
    QString error() override;
    void close() override;
};
