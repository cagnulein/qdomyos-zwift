#ifndef SERIALDATASOURCE_H
#define SERIALDATASOURCE_H

#include <QByteArray>
#include <QStringList>

/**
 * @brief An interface for a serial I/O device, based on the interface of QSerialPort.
 */
class serialdatasource
{
  protected:
    serialdatasource() {}
    serialdatasource(const serialdatasource& other) { Q_UNUSED(other) }
  public:
    /**
     * @brief Gets a list of available port names.
     */
    virtual QStringList get_availablePorts() = 0;

    /**
     * @brief Tries to open the specified port.
     * @param portName The name of the port to open.
     * @return Boolean indicating success.
     */
    virtual bool open(const QString& portName) = 0;

    /**
     * @brief Writes the specified data to the port.
     * @param data The data to write.
     * @return
     */
    virtual qint64 write(const QByteArray& data)=0;

    /**
     * @brief Flush the port buffer.
     */
    virtual void flush() = 0;

    /**
     * @brief Returns true/false to indicate if data is ready for reading after a maximum of 1ms.
     * @return
     */
    virtual bool waitForReadyRead()=0;

    /**
     * @brief Reads all the data from the buffer.
     * @return
     */
    virtual QByteArray readAll() = 0;

    /**
     * @brief The number of bytes in the buffer.
     * @return
     */
    virtual qint64 readBufferSize() =0;

    /**
     * @brief An error string for debug/logging.
     * @return
     */
    virtual QString error() =0;

    /**
     * @brief Close the port.
     */
    virtual void close()=0;

    virtual ~serialdatasource() = default;
};

#endif // SERIALDATASOURCE_H
