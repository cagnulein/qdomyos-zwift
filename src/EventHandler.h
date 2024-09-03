#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <QDebug>

#ifdef Q_OS_LINUX
#ifndef Q_OS_ANDROID
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
        QFile inputFile(m_devicePath);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            emit error(QString("Failed to open device: %1").arg(m_devicePath));
            return;
        }

        while (!m_shouldStop) {
            input_event ev;
            if (inputFile.read(reinterpret_cast<char*>(&ev), sizeof(ev)) == sizeof(ev)) {
                qDebug() << "EV_KEY" << ev.type;
                if (ev.type == EV_KEY && ev.value == 1) { // Key press event
                    emit keyPressed(ev.code);
                }
            }
            QThread::msleep(10); // Small delay to prevent busy waiting
        }

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
    BluetoothHandler(QObject* parent = nullptr) : QObject(parent)
    {
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
        // Handle the key press event here
        // For example, you could emit a signal or call a function to change the volume
    }

    void onError(const QString& errorMessage)
    {
        qDebug() << "Error:" << errorMessage;
    }

  private:
    QThread* m_thread;
    EventHandler* m_handler;
};
#endif
#endif

#endif // EVENTHANDLER_H
