#include "trixterxdreamv1serial.h"

#include <QDebug>
#include <QTime>
#include <QSerialPortInfo>

trixterxdreamv1serial::trixterxdreamv1serial(QObject *parent) : QThread(parent) {}

trixterxdreamv1serial::~trixterxdreamv1serial() {
    m_mutex.lock();
    m_quit = true;
    m_mutex.unlock();
    wait();
}

QList<QSerialPortInfo> trixterxdreamv1serial::availablePorts()
{
    return QSerialPortInfo::availablePorts();
}

void trixterxdreamv1serial::open(const QString &portName, int waitTimeout) {
    const QMutexLocker locker(&m_mutex);
    m_portName = portName;
    m_waitTimeout = waitTimeout;
    if (!isRunning())
        start();
}

void trixterxdreamv1serial::write(const uint8_t *buffer, int len, QString info) {
    qDebug() << "serial >> " << QByteArray((const char *)buffer, len).toHex(' ') << "//" << info;
    qint64 o = serial.write(QByteArray((const char *)buffer, len));
    qDebug() << "serial byte written" << o;
}

void trixterxdreamv1serial::run() {

    bool currentPortNameChanged = false;

    m_mutex.lock();

    QString currentPortName;
    if (currentPortName != m_portName) {
        currentPortName = m_portName;
        currentPortNameChanged = true;
    }

    int currentWaitTimeout = m_waitTimeout;
    m_mutex.unlock();

    while (!m_quit) {
        if (currentPortNameChanged) {
            serial.close();
            serial.setPortName(currentPortName);
            serial.setBaudRate(QSerialPort::Baud115200);

            if (!serial.open(QIODevice::ReadWrite)) {
                qDebug() << tr("Can't open %1, error code %2").arg(m_portName).arg(serial.error());
                emit error(tr("Can't open %1, error code %2").arg(m_portName).arg(serial.error()));
                return;
            }
            qDebug() << "Serial port" << currentPortName << "opened";
        }

        if (serial.waitForReadyRead(currentWaitTimeout)) {
            QByteArray requestData = serial.readAll();
            while (serial.waitForReadyRead(1))
                requestData += serial.readAll();
            qDebug() << "serial << " << requestData.toHex(' ');

            // Send the bytes to the client code
            if(bytes_read)
                bytes_read(requestData);
        }
        m_mutex.lock();
        if (currentPortName != m_portName) {
            currentPortName = m_portName;
            currentPortNameChanged = true;
        } else {
            currentPortNameChanged = false;
        }
        currentWaitTimeout = m_waitTimeout;
        m_mutex.unlock();
    }
}
