#ifndef TRIXTERXDREAMSERIAL_H
#define TRIXTERXDREAMSERIAL_H

#include <QMutex>
#include <QSerialPort>
#include <QThread>
#include <QWaitCondition>
#include <QAtomicInt>

/**
 * @brief A basic serial port monitoring thread.
 * Avoids using signals to prevent complications with objects, threads and timers.
 */
class trixterxdreamv1serial : public QThread {
    Q_OBJECT

public:
    explicit trixterxdreamv1serial(QObject *parent = nullptr);
    ~trixterxdreamv1serial();

    /**
     * @brief Opens the port.
     * @param portName The name of the serial port.
     * @param baudRate The baud rate.
     * @returns True if the port was opened, false if the port wasn't opened, or was already open.
     */
    bool open(const QString &portName, QSerialPort::BaudRate baudRate);

    /**
     * @brief Writes the array of bytes to the serial port
     * @param buffer The bytes to send.
     */
    void write(const QByteArray& buffer);

    /**
     * @brief set_receiveBytes Set a delegate to receive bytes. This is an alternative
     * to sublcassing and overrding the virtual receive function.
     * @param value
     */
    void set_receiveBytes(std::function<void(const QByteArray& bytes)> value) { this->receiveBytes = value; }

    /**
     * @brief get_SendReceiveLog Gets whether or not the bytes sent and received will be written to the debug log.
     * @return
     */
    bool get_SendReceiveLog();

    /**
     * @brief set_SendReceiveLog Sets whether or not the bytes sent and received will be written to the debug log.
     * @param value
     */
    void set_SendReceiveLog(bool value);

    /**
     * @brief availablePorts Returns a list of information objects for the serial ports found in the system.
     */
    static QList<QSerialPortInfo> availablePorts();

protected:
    /**
     * @brief receive Override this to process received data.
     * @param bytes
     */
    virtual void receive(const QByteArray &bytes);

    /**
     * @brief error Log an error.
     * @param s The error text.
     */
    virtual void error(const QString &s);

  private:
    void run() override;

    QMutex writeBufferMutex;
    QByteArray writeBuffer;
    QAtomicInt writePending {0};
    bool sendReceiveLog = false;
    QString portName;
    QSerialPort::BaudRate baudRate;
    QMutex mutex;
    QAtomicInt openAttemptsPending{0};
    QAtomicInt quitPending{0};
    std::function<void(const QByteArray& bytes)> receiveBytes=nullptr;
};

#endif // TRIXTERXDREAMSERIAL_H
