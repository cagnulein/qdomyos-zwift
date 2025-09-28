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

AntManager::~AntManager() {
    // Safety net in case stopForDevice wasn't called
    if (m_workerThread && m_workerThread->isRunning()) {
        qWarning() << "[ANT+] Destructor cleanup - worker thread still running";
        m_workerThread->quit();
        if (!m_workerThread->wait(1000)) {
            m_workerThread->terminate();
            m_workerThread->wait(500);
        }
    }
}

void AntManager::startForDevice(bluetoothdevice* device) {
    if (m_workerThread) {
        qWarning("[ANT+] Manager: A worker is already running.");
        return;
    }

    qInfo() << "[ANT+] Manager: Received start command. Initializing worker thread...";
    m_currentDevice = device;
    
    // The device is already fully discovered, so we can start the worker immediately.
    m_workerThread = new QThread(this);
    m_workerThread->setObjectName("AntWorkerThread");
    m_worker = new AntWorker(m_currentDevice);
    m_worker->moveToThread(m_workerThread);
    
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
    if (!m_workerThread || !m_workerThread->isRunning() || (device && device != m_currentDevice && device != nullptr)) {
        return;
    }

    qInfo() << "[ANT+] Manager: Stopping ANT+ worker thread...";

    // 1. Signal the worker to begin cleanup
    QMetaObject::invokeMethod(m_worker, "stop", Qt::QueuedConnection);

    // 2. Tell the thread's event loop to exit
    m_workerThread->quit();

    // 3. CRITICAL: Block and wait for thread to actually finish
    if (!m_workerThread->wait(2000)) {
        qWarning() << "[ANT+] Worker thread did not stop gracefully within 2 seconds. Terminating.";
        m_workerThread->terminate();
        // Give terminate a moment to work
        if (!m_workerThread->wait(500)) {
            qCritical() << "[ANT+] Failed to terminate worker thread";
        }
    } else {
        qInfo() << "[ANT+] Worker thread stopped cleanly.";
    }
}

void AntManager::onWorkerFinished() {
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