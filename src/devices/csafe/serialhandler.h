#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QString>
#include <sys/types.h>

class SerialHandler {
  public:
    virtual ~SerialHandler() = 0; // Pure virtual destructor

    // Factory method to create the appropriate serial handler
    static SerialHandler *create(const QString &deviceFilename, uint32_t baudRate);

    // Abstract interface for serial operations
    virtual int openPort() = 0;
    virtual int closePort() = 0;
    virtual int rawWrite(uint8_t *bytes, int size) = 0;
    virtual int rawRead(uint8_t bytes[], int size, bool line = false) = 0;
    virtual int dataAvailable() = 0;
    virtual bool isOpen() const = 0;

    // Common configuration methods
    virtual void setDevice(const QString &devname) = 0;
    virtual void setTimeout(int timeout) = 0;
    virtual void setEndChar(uint8_t endChar) = 0;

  protected:
    // Protected constructor to prevent direct instantiation of this abstract class
    SerialHandler() = default;
};

inline SerialHandler::~SerialHandler() {} // Definition of the pure virtual destructor

#endif // SERIALHANDLER_H