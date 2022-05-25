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

void serialDircon::write(const uint8_t *buffer, int len, QString info) {
    qDebug() << "serial >> " << QByteArray((const char *)buffer, len).toHex(' ') << "//" << info;
    qint64 o = serial.write(QByteArray((const char *)buffer, len));
    qDebug() << "serial byte written" << o;
}

void serialDircon::run() {

    const uint8_t init0[] = {0x02, 0x81, 0x01, 0x00, 0x80, 0x03};
    const uint8_t init1[] = {0x02, 0x01, 0x01, 0x00, 0x00, 0x03};
    const uint8_t init2[] = {0x02, 0x01, 0x10, 0x03, 0x0C, 0x01, 0x2A, 0x00, 0x01, 0x01,
                             0x00, 0x00, 0x00, 0x08, 0x01, 0x07, 0x10, 0x03, 0x28, 0x03};
    const uint8_t init3[] = {0x02, 0x68, 0x06, 0x26, 0xB4, 0x6F, 0x2D, 0x00, 0x04, 0x4b, 0x34, 0x32, 0x32, 0x30, 0x35,
                             0x30, 0x30, 0x30, 0x31, 0x38, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCE, 0x03};
    const uint8_t init4[] = {0x02, 0x68, 0x04, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x64, 0x03};
    const uint8_t init5[] = {0x02, 0x68, 0x01, 0x10, 0x03, 0x00, 0x08, 0x00, 0x62, 0x03};
    const uint8_t init6[] = {0x02, 0x68, 0x10, 0x02, 0x10, 0x03, 0x00, 0x0A, 0x00, 0x63, 0x03};
    const uint8_t init7[] = {0x02, 0x68, 0x10, 0x02, 0x10, 0x03, 0x00, 0x14, 0x00, 0x7D, 0x03};
    const uint8_t init8[] = {0x02, 0x68, 0x10, 0x02, 0x10, 0x03, 0x00, 0x17, 0x00, 0x7E, 0x03};
    const uint8_t init9[] = {0x02, 0x68, 0x50, 0x01, 0x00, 0x39, 0x03};
    const uint8_t init10[] = {0x02, 0x68, 0x10, 0x03, 0x01, 0x00, 0x6A, 0x03};
    const uint8_t init11[] = {0x02, 0x68, 0x20, 0x01, 0x00, 0x49, 0x03};
    
    const uint8_t run1[] = {0x02, 0x68, 0x04, 0x09, 0x08, 0x17, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x03, 0x78, 0x03};

    //const uint8_t run1[] = {0x02, 0x68, 0x04, 0x09, 0x08, 0x00, 0x17, 0x00, 0x01, 0x01, 0x00, 0x00, 0x13, 0x69, 0x03};
    //const uint8_t run2[] = {0x02, 0x68, 0x50, 0x01, 0x00, 0x39, 0x03, 0x02, 0x68, 0x50, 0x01, 0x00, 0x39, 0x03};

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
                    write(init0, sizeof(init0), "init0");
                    break;
                case 1:
                    write(init1, sizeof(init1), "init1");
                    break;
                case 2:
                    write(init2, sizeof(init2), "init2");
                    break;
                case 3:
                    write(init3, sizeof(init3), "init3");
                    break;
                case 4:
                    write(init4, sizeof(init4), "init4");
                    break;
                case 5:
                    write(init5, sizeof(init5), "init5");
                    break;
                case 6:
                    write(init6, sizeof(init6), "init6");
                    break;
                case 7:
                    write(init7, sizeof(init7), "init7");
                    break;
                case 8:
                    write(init8, sizeof(init8), "init8");
                    break;
                case 9:
                case 10:        
                case 11:
                case 12:        
                case 13:
                case 14:        
                case 15:
                case 16:        
                case 17:
                case 18:        
                case 19:
                case 20:        
                case 21:
                case 22:
                    write(init9, sizeof(init9), "init9");
                    break;                        
                case 23:        
                    write(init10, sizeof(init10), "init10");
                    break;                                  
                case 24:        
                    write(init11, sizeof(init11), "init11");
                    break;                                                                  
                default:
                    initRequest = false;
                    phase = 0;
                }
            } else {/*
                switch (phase) {
                case 0:*/
                    write(run1, sizeof(run1), "run1");
                    //break;
                /*case 1:
                    write(run2, sizeof(run2), "run2");
                    break;*/
                /*case 1:
                    write(force, sizeof(force), "force");
                    break;
                default:
                    phase = 0;
                }*/
            }
        }

        if (serial.waitForReadyRead(currentWaitTimeout)) {
            QByteArray requestData = serial.readAll();
            while (serial.waitForReadyRead(1))
                requestData += serial.readAll();
            qDebug() << "serial << " << requestData.toHex(' ');

            if (requestData.at(0) == 0x02 && requestData.length() >= 6) {
                phase++;
                if (requestData.at(0) == init1[0] && requestData.at(1) == init1[1] && requestData.at(2) == init1[2] &&
                    requestData.at(3) == init1[3] && requestData.at(4) == init1[4] && requestData.at(5) == init1[5])
                    write(init1, sizeof(init1), "init1");
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
