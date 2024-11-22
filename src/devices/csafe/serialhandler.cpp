#include "serialhandler.h"
#include "netserial.h"
#include "serialport.h"
#include <QDebug>

SerialHandler *SerialHandler::create(const QString &deviceFilename, uint16_t baudRate) {
    if (deviceFilename.contains(':')) {
        qDebug() << "Using NetSerial for device:" << deviceFilename;
        return new NetSerial(deviceFilename);
    } else {
        qDebug() << "Using Serialport for device:" << deviceFilename;
        return new Serialport(deviceFilename, baudRate);
    }
}