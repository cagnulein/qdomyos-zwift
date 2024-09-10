#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <QDebug>
#include <QFile>
#include <QThread>

#ifdef Q_OS_LINUX
#ifndef Q_OS_ANDROID
#include <linux/input.h>
#include "bluetooth.h"

class EventHandler : public QObject
{
    Q_OBJECT

  public:
    EventHandler(const QString& devicePath, QObject* parent = nullptr)
        : QObject(parent), m_devicePath(devicePath), m_shouldStop(false) {}

    void stop() { m_shouldStop = true; }

  public slots:
    void run()
    {
        qDebug() << "EventHandler run()";
        QFile inputFile(m_devicePath);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            qDebug() << "EventHandler open error";
            emit error(QString("Failed to open device: %1").arg(m_devicePath));
            return;
        }
        qDebug() << "EventHandler opened correctly";

        while (!m_shouldStop) {
            input_event ev;
            QByteArray buffer;
            qDebug() << "EventHandler read()";
            
            qint64 bytesRead = inputFile.read(reinterpret_cast<char*>(&ev), sizeof(ev));
        
            if (bytesRead == sizeof(ev)) {
                qDebug() << "EV_KEY" << ev.type;
                if (ev.type == EV_KEY && ev.value == 1) { // Key press event
                    emit keyPressed(ev.code);
                }
            } else if (bytesRead == 0) {
                // End of file reached
                qDebug() << "End of file reached";
                break;
            } else if (bytesRead == -1) {
                // Error occurred
                qDebug() << "Read error:" << inputFile.errorString();
                emit error(QString("Failed to read from device: %1").arg(inputFile.errorString()));
                break;
            } else {
                qDebug() << "Incomplete read. Bytes read:" << bytesRead << "Expected:" << sizeof(ev);
            }          
            QThread::msleep(10); // Small delay to prevent busy waiting
        }
        qDebug() << "EventHandler close()";
        
        inputFile.close();
    }

  signals:
    void keyPressed(int keyCode);
    void error(const QString& errorMessage);

  private:
    QString m_devicePath;
    bool m_shouldStop;
};

class BluetoothHandler : public QObject
{
    Q_OBJECT

  public:
    BluetoothHandler(bluetooth* bl, QObject* parent = nullptr) : QObject(parent)
    {
        m_bluetooth = bl;
        m_thread = new QThread(this);
        m_handler = new EventHandler("/dev/input/event2"); // Adjust this path as needed
        m_handler->moveToThread(m_thread);

        connect(m_thread, &QThread::started, m_handler, &EventHandler::run);
        connect(m_handler, &EventHandler::keyPressed, this, &BluetoothHandler::onKeyPressed);
        connect(m_handler, &EventHandler::error, this, &BluetoothHandler::onError);

        m_thread->start();
    }

    ~BluetoothHandler()
    {
        m_handler->stop();
        m_thread->quit();
        m_thread->wait();
    }

  private slots:
    void onKeyPressed(int keyCode)
    {
        qDebug() << "Key pressed:" << keyCode;
        if(m_bluetooth && m_bluetooth->device() && m_bluetooth->device()->deviceType() == bluetoothdevice::BIKE) {
            if(keyCode == 115) // up
                ((bike*)m_bluetooth->device())->setGears(((bike*)m_bluetooth->device())->gears() + 1);
            else if(keyCode == 114) // down
                ((bike*)m_bluetooth->device())->setGears(((bike*)m_bluetooth->device())->gears() - 1);
        }
    }

    void onError(const QString& errorMessage)
    {
        qDebug() << "Error:" << errorMessage;
    }

  private:
    QThread* m_thread;
    EventHandler* m_handler;
    bluetooth* m_bluetooth = 0;
};
#endif
#endif

#endif // EVENTHANDLER_H
