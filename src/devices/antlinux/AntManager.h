// =============================================================================
// QDomyos-Zwift: ANT+ Lifecycle Manager (Header)
//
// Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
// Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
//
// Singleton managing ANT+ worker thread lifecycle. Ensures single broadcaster
// instance and coordinates clean startup/shutdown on device events.
//
// Key contracts: startForDevice(), stopForDevice(), and onWorkerFinished().
// =============================================================================

#ifndef ANTMANAGER_H
#define ANTMANAGER_H

#ifdef ANT_LINUX_ENABLED
#include <QObject>
#include "devices/bluetoothdevice.h"

class AntWorker; // Forward declaration
class QThread;

class __attribute__((visibility("hidden"))) AntManager : public QObject {
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