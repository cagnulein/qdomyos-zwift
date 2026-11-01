#pragma once

#include <QObject>
#include <QByteArray>
#include <QMutex>
#include <queue>

#include "devices/trixterxdreambike/serialdatasource.h"

/**
 * @brief Implementation of serialdatasource "interface" for faking a Trixter X-Dream bike for testing.
 * Simulates a serial port that sends a single valid packet repeatedly.
 */
class TrixterXDreamBikeStub : public serialdatasource {

    QByteArray bytesWritten;
    std::queue<char> readBuffer;
    QRecursiveMutex mutex;
    ulong lastAddedData = 0;

    const ulong readInterval = 10;
    const uint32_t bufferCapacity = 4096;

protected:

    /**
     * @brief If it's been more than the readInterval since fake data was put in the buffer,
     * puts a packet in the buffer for every interval that has elapsed.
     * @return True/false depedning on whether data was added.
     */
    bool tryPopulate();

public:

    static serialdatasource* create(QObject * parent);

    TrixterXDreamBikeStub();

    QStringList get_availablePorts() override;

    bool open(const QString& portName) override;
    qint64 write(const QByteArray& data) override;
    void flush() override;
    bool waitForReadyRead() override;
    QByteArray readAll() override;
    qint64 readBufferSize() override;
    QString error() override;
    void close() override;
};
