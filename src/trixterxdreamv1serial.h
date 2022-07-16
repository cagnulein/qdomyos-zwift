#ifndef TRIXTERXDREAMSERIAL_H
#define TRIXTERXDREAMSERIAL_H

#include <QMutex>
#include <QSerialPort>
#include <QThread>
#include <QWaitCondition>

class trixterxdreamv1serial : public QThread {
    Q_OBJECT

  public:
    explicit trixterxdreamv1serial(QObject *parent = nullptr);
    ~trixterxdreamv1serial();

    /**
     * @brief Opens the port.
     * @param portName The name of the serial port.
     * @param waitTimeout The timeout for the serial port.
     */
    void open(const QString &portName, int waitTimeout);

    /**
     * @brief Writes the array of bytes to the serial port
     * @param buffer The bytes to send.
     * @param len The number of bytes to send.
     * @param info Debug information
     */
    void write(const uint8_t *buffer, int len, QString info);

    /**
     * @brief Sets the callback function to call when bytes have been read.
     * @param bytes_read A callback function to call when a block of bytes has been read.
     */
    void set_bytes_read(std::function<void(QByteArray)> bytes_read) { this->bytes_read = bytes_read; }

    /**
     * @brief availablePorts Returns a list of information objects for the serial ports found in the system.
     */
    static QList<QSerialPortInfo> availablePorts();
signals:
    void request(const QString &s);
    void error(const QString &s);
    void timeout(const QString &s);

  private:
    void run() override;

    QSerialPort serial;
    std::function<void(QByteArray)> bytes_read;
    QString m_portName;
    int m_waitTimeout = 1000;
    QMutex m_mutex;
    bool m_quit = false;
};

#endif // TRIXTERXDREAMSERIAL_H
