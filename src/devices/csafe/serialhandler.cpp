#include "serialhandler.h"
#include "netserial.h"
#include "serialport.h"

SerialHandler *SerialHandler::create(const QString &deviceFilename, uint32_t baudRate) {
    if (deviceFilename.contains(':')) {
        qDebug() << "Using NetSerial for device:" << deviceFilename;
        return new NetSerial(deviceFilename);
    } else {
        qDebug() << "Using Serialport for device:" << deviceFilename;
        return new Serialport(deviceFilename, baudRate);
    }
}
