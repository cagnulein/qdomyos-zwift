// =============================================================================
// QDomyos-Zwift: ANT+ Lifecycle Manager (Implementation)
//
// Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
// Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
//
// Orchestrates worker thread creation, lifecycle, and destruction. Critical
// shutdown sequence ensures USB dongle release before restart.
//
// See: AntWorker for worker implementation details.
// =============================================================================

#include "AntManager.h"

#ifdef ANT_LINUX_ENABLED
#include "AntWorker.h"
#include <QDebug>
#include <QThread>
#include <QCoreApplication>

AntManager& AntManager::instance() {
    static AntManager manager;
    return manager;
}

AntManager::AntManager(QObject* parent) : QObject(parent) {}

// Destructor is now minimal. All cleanup is explicitly handled in stopForDevice.
AntManager::~AntManager() {
    qInfo() << "[ANT+] AntManager destroyed.";
}

void AntManager::startForDevice(bluetoothdevice* device) {
    if (m_workerThread) {
        qWarning("[ANT+] Manager: A worker is already running.");
        return;
    }

    qInfo() << "[ANT+] Manager: Received start command. Initializing worker thread...";
    m_currentDevice = device;
    
    m_workerThread = new QThread(this);
    m_workerThread->setObjectName("AntWorkerThread");
    m_worker = new AntWorker(m_currentDevice);
    m_worker->moveToThread(m_workerThread);
    
    // This is the correct, thread-safe way to set the thread's priority.
    // It runs in the new thread's context right after it starts.
    connect(m_workerThread, &QThread::started, [this]() {
        qInfo() << "[ANT+] AntWorkerThread has started. Setting to high priority.";
        m_workerThread->setPriority(QThread::TimeCriticalPriority);
    });

    connect(m_workerThread, &QThread::started, m_worker, &AntWorker::start);
    connect(m_worker, &AntWorker::finished, m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished, this, &AntManager::onWorkerFinished);
    
    // Connect device disconnection to our stop function
    connect(device, &bluetoothdevice::disconnected, this, [this, device](){
        this->stopForDevice(device);
    });

    m_workerThread->start();
}

void AntManager::stopForDevice(bluetoothdevice* device) {
    // Check if there is anything to stop.
    if (!m_workerThread || !m_workerThread->isRunning()) {
        return;
    }

    // If called for a specific device, ensure it's the one we are managing.
    if (device && device != m_currentDevice) {
        return;
    }

    qInfo() << "[ANT+] Manager: Stopping ANT+ worker thread...";

    // 1. Signal the worker (in its own thread) to stop its timers and Python script.
    QMetaObject::invokeMethod(m_worker, "stop", Qt::QueuedConnection);

    // 2. Tell the thread's event loop to exit once the worker's tasks are done.
    m_workerThread->quit();

    // 3. Block this (main) thread and wait for the worker thread to completely finish.
    // This is the most critical step to prevent race conditions.
    if (!m_workerThread->wait(3000)) { // Use a generous 3-second timeout
        qWarning() << "[ANT+] Worker thread did not stop gracefully. Terminating.";
        m_workerThread->terminate();
        m_workerThread->wait(500);
    } else {
        qInfo() << "[ANT+] Worker thread stopped cleanly.";
    }
}

void AntManager::onWorkerFinished() {
    // This slot is now called after the thread's event loop has finished.
    qInfo() << "[ANT+] Worker thread has finished. Cleaning up manager resources.";
    
    if (m_currentDevice) {
        disconnect(m_currentDevice, nullptr, this, nullptr);
    }
    
    if (m_worker) m_worker->deleteLater();
    if (m_workerThread) m_workerThread->deleteLater();
    
    m_worker = nullptr;
    m_workerThread = nullptr;
    m_currentDevice = nullptr;
}

#endif // ANT_LINUX_ENABLED