#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <QDebug>
#include <QSocketNotifier>
#include <QFile>

#ifdef Q_OS_LINUX
#ifndef Q_OS_ANDROID
#include <linux/input.h>
#include "bluetooth.h"

class EventHandler : public QObject
{
    Q_OBJECT

  public:
    EventHandler(const QString& devicePath, QObject* parent = nullptr)
        : QObject(parent), m_devicePath(devicePath), m_notifier(nullptr), m_fd(-1) {}

    ~EventHandler() {
        if (m_fd != -1) {
            ::close(m_fd);
        }
    }

    bool initialize() {
        m_fd = ::open(m_devicePath.toStdString().c_str(), O_RDONLY | O_NONBLOCK);
        if (m_fd == -1) {
            qDebug() << "Failed to open device:" << m_devicePath;
            emit error(QString("Failed to open device: %1").arg(m_devicePath));
            return false;
        }
        m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
        connect(m_notifier, &QSocketNotifier::activated, this, &EventHandler::handleEvent);
        qDebug() << "Device opened successfully:" << m_devicePath;
        return true;
    }

  signals:
    void keyPressed(int keyCode);
    void error(const QString& errorMessage);

  private slots:
    void handleEvent() {
        input_event ev;
        ssize_t bytesRead = ::read(m_fd, &ev, sizeof(ev));

        if (bytesRead == sizeof(ev)) {
            if (ev.type == EV_KEY && ev.value == 1) { // Key press event
                emit keyPressed(ev.code);
            }
        } else if (bytesRead == 0) {
            qDebug() << "End of file reached.";
            m_notifier->setEnabled(false);
        } else if (bytesRead == -1) {
            qDebug() << "Read error:" << strerror(errno);
            emit error(QString("Failed to read from device: %1").arg(strerror(errno)));
        }
    }

  private:
    QString m_devicePath;
    int m_fd;
    QSocketNotifier* m_notifier;
};

class BluetoothHandler : public QObject
{
    Q_OBJECT

  public:
    BluetoothHandler(bluetooth* bl, QString eventDevice, QObject* parent = nullptr)
        : QObject(parent), m_bluetooth(bl)
    {
        m_handler = new EventHandler(eventDevice); // Adjust this path as needed

        if (!m_handler->initialize()) {
            qDebug() << "Failed to initialize EventHandler.";
            return;
        }

        connect(m_handler, &EventHandler::keyPressed, this, &BluetoothHandler::onKeyPressed);
        connect(m_handler, &EventHandler::error, this, &BluetoothHandler::onError);
    }

    ~BluetoothHandler() {
        delete m_handler;
    }

  private slots:
    void onKeyPressed(int keyCode)
    {
        qDebug() << "Key pressed:" << keyCode;
        if (m_bluetooth && m_bluetooth->device() && m_bluetooth->device()->deviceType() == bluetoothdevice::BIKE) {
            if (keyCode == 115) // up
                ((bike*)m_bluetooth->device())->setGears(((bike*)m_bluetooth->device())->gears() + 1);
            else if (keyCode == 114) // down
                ((bike*)m_bluetooth->device())->setGears(((bike*)m_bluetooth->device())->gears() - 1);
        }
    }

    void onError(const QString& errorMessage)
    {
        qDebug() << "Error:" << errorMessage;
    }

  private:
    EventHandler* m_handler;
    bluetooth* m_bluetooth;
};

#endif // EVENTHANDLER_H
#endif // EVENTHANDLER_H
#endif // EVENTHANDLER_H
