#ifndef TRIXTERXDREAMSERIAL_H
#define TRIXTERXDREAMSERIAL_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QAtomicInt>

/**
 * @brief A basic serial port monitoring thread.
 * Avoids using signals to prevent complications with objects, threads and timers.
 */
class trixterxdreamserial : public QThread {
    Q_OBJECT

public:
    static class std::function<class serialdatasource*(QObject *)> serialDataSourceFactory;

    explicit trixterxdreamserial(QObject *parent = nullptr);
    ~trixterxdreamserial();

    /**
     * @brief Opens the port.
     * @param portName The name of the serial port.
     * @returns True if the port was opened, false if the port wasn't opened, or was already open.
     */
    bool open(const QString &portName);

    /**
     * @brief Writes the array of bytes to the serial port
     * @param buffer The bytes to send.
     */
    void write(const QByteArray& buffer);

    /**
     * @brief Set a function pointer to receive bytes. This is an alternative
     * to sublcassing and overrding the virtual receive function.
     * @param value
     */
    void set_receiveBytes(std::function<void(const QByteArray& bytes)> value) { this->receiveBytes = value; }

    /**
     * @brief Sets an optional pulse function - while this object's thread is running, the pulse
     * function is called repeatedly with a period defined by the pulse interval. The pulse function
     * should take less than 1ms.
     * @param function The pulse function.
     * @param pulseIntervalMilliseconds
     */
    void set_pulse(std::function<void()> function, uint32_t pulseIntervalMilliseconds);

    /**
     * @brief Sets the function to get the time in milliseconds since
     * a starting point understood by the client.
     * @param get_time_ms A function to get the time.
     */
    void set_getTime(std::function<uint32_t()> get_time_ms);

    /**
     * @brief availablePorts Returns a list of port names for the serial ports found in the system
     * that could host the bike.     
     */
    static QStringList availablePorts();

protected:
    /**
     * @brief receive Override this to process received data.
     * @param bytes
     */
    virtual void receive(const QByteArray &bytes);

  private:
    void run() override;

    /**
     * @brief The number of milliseconds the pulse function should execute in.
     */
    const uint32_t pulseTolerance = 1;

    QMutex writeBufferMutex;
    QByteArray writeBuffer;
    QAtomicInt writePending {0};
    QString portName;
    QMutex mutex;
    QAtomicInt openAttemptsPending{0};
    QAtomicInt quitPending{0};
    std::function<void(const QByteArray& bytes)> receiveBytes=nullptr;
    std::function<uint32_t()> getTime=nullptr;
    std::function<void()> pulse=nullptr;
    uint32_t pulseIntervalMilliseconds = 10;
};

#endif // TRIXTERXDREAMSERIAL_H
