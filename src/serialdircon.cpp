#include "serialdircon.h"

#include <QDebug>
#include <QTime>

serialDircon::serialDircon(QObject *parent) : QThread(parent) {}

serialDircon::~serialDircon() {
    m_mutex.lock();
    m_quit = true;
    m_mutex.unlock();
    wait();
}

void serialDircon::open(const QString &portName, int waitTimeout) {
    const QMutexLocker locker(&m_mutex);
    m_portName = portName;
    m_waitTimeout = waitTimeout;
    if (!isRunning())
        start();
}

void serialDircon::write(char *buffer, int len, QString info) {
    qDebug() << "serial >> " << QByteArray(buffer).toHex(' ') << "//" << info;
    serial.write(buffer, len);
}

void serialDircon::run() {

    const uint8_t init0[] = {0x02, 0x68, 0x04, 0x00, 0x6c, 0x03};
    const uint8_t init1[] = {0x02, 0x68, 0x50, 0x0b, 0x11, 0x00, 0x00, 0x44, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x03};
    const uint8_t init2[] = {0x02, 0x68, 0x50, 0x13, 0x08, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0xb8, 0x6c,
                             0x15, 0x1f, 0x00, 0x00, 0xc7, 0x05, 0x81, 0x0a, 0xf0, 0xa3, 0xd3, 0x03};
    const uint8_t init3[] = {0x02, 0x68, 0x04, 0x00, 0x6c, 0x03};
    const uint8_t init4[] = {0x02, 0x68, 0x04, 0x00, 0x6c, 0x03, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xfc, 0xc0};

    const uint8_t run1[] = {0x02, 0x68, 0x04, 0x09, 0x08, 0x00, 0x17, 0x00, 0x01, 0x01, 0x00, 0x00, 0x13, 0x69, 0x03};
    const uint8_t run2[] = {0x02, 0x68, 0x50, 0x01, 0x00, 0x39, 0x03, 0x02, 0x68, 0x50, 0x01, 0x00, 0x39, 0x03};

    const uint8_t force[] = {0x02, 0xe8, 0x60, 0x09, 0x0f, 0x00, 0x11, 0x00, 0x00, 0xd7, 0xff, 0x28, 0x33, 0xac, 0x03};

    bool initRequest = true;
    uint8_t phase = 0;

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

        if (serial.isOpen()) {
            if (initRequest) {
                switch (phase) {
                case 0:
                    write((char *)init0, sizeof(init0), "init0");
                    break;
                case 1:
                    write((char *)init1, sizeof(init1), "init1");
                    break;
                case 2:
                    write((char *)init2, sizeof(init2), "init2");
                    break;
                case 3:
                    write((char *)init3, sizeof(init3), "init3");
                    break;
                case 4:
                    write((char *)init4, sizeof(init4), "init4");
                    break;
                default:
                    initRequest = false;
                    phase = 0;
                }
            } else {
                switch (phase) {
                case 0:
                    write((char *)run1, sizeof(run1), "run1");
                    break;
                case 1:
                    write((char *)run2, sizeof(run2), "run2");
                    break;
                case 2:
                    write((char *)force, sizeof(force), "force");
                    break;
                default:
                    phase = 0;
                }
            }
        }

        if (serial.waitForReadyRead(currentWaitTimeout)) {
            QByteArray requestData = serial.readAll();
            while (serial.waitForReadyRead(10))
                requestData += serial.readAll();
            qDebug() << "serial << " << requestData.toHex(' ');

            if (requestData.at(0) == 0x02 && requestData.at(1) == 0x68) {
                phase++;
            }

            if (requestData.at(0) == 0x02 && requestData.at(1) == 0x68 && requestData.at(2) == 0x50 &&
                requestData.at(3) == 0x0b && requestData.at(4) == 0x11 && requestData.length() >= 17) {
                uint16_t convertedData = (requestData.at(10) << 8) | ((uint8_t)requestData.at(9));
                double speed = ((double)convertedData) / 100.0;
                convertedData = (requestData.at(8) << 8) | ((uint8_t)requestData.at(7));
                double cadence = ((double)convertedData) / 2.0;
                uint16_t watt = (requestData.at(14) << 8) | ((uint8_t)requestData.at(13));

                qDebug() << "Metrics FROM Serial: Speed" << speed << "Cadence" << cadence << "Watt" << watt;
            }
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
