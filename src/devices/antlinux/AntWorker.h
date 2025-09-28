#ifdef ANT_LINUX_ENABLED
#ifndef ANTWORKER_H
#define ANTWORKER_H
#include <QObject>
#include <QElapsedTimer>
#include <QTimer>
#include <atomic>
#include <memory>
#include "devices/bluetoothdevice.h"

namespace pybind11 { class object; class scoped_interpreter; }

class AntWorker : public QObject {
    Q_OBJECT
public:
    explicit AntWorker(bluetoothdevice* device, QObject *parent = nullptr);
    ~AntWorker();

public slots:
    void start();
    void stop();

signals:
    void finished();

private slots:
    void doWork(); // <-- The new slot for the timer

private:
    bool initializePython();
    void shutdownPython();

    bluetoothdevice* m_device;
    std::atomic<bool> m_stopRequested{false};
    
    std::unique_ptr<pybind11::scoped_interpreter> m_pyGuard;
    std::unique_ptr<pybind11::object> m_pyBroadcaster;

    QElapsedTimer m_connectionTimer;
    QTimer* m_timer = nullptr; // <-- The timer for broadcasting
};
#endif // ANTWORKER_H
#endif // ANT_LINUX_ENABLED