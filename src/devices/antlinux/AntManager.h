#ifndef ANTMANAGER_H
#define ANTMANAGER_H

#ifdef ANT_LINUX_ENABLED
#include <QObject>
#include "devices/bluetoothdevice.h"

class AntWorker; // Forward declaration
class QThread;

class AntManager : public QObject {
    Q_OBJECT
public:
    static AntManager& instance();

    void startForDevice(bluetoothdevice* device);
    void stopForDevice(bluetoothdevice* device);

private slots:
    void onWorkerFinished();

private:
    AntManager(QObject* parent = nullptr);
    ~AntManager();
    AntManager(const AntManager&) = delete;
    AntManager& operator=(const AntManager&) = delete;

    QThread* m_workerThread = nullptr;
    AntWorker* m_worker = nullptr;
    bluetoothdevice* m_currentDevice = nullptr;
};

#endif // ANT_LINUX_ENABLED
#endif // ANTMANAGER_H