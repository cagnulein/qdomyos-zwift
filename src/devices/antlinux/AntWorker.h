// =============================================================================
// QDomyos-Zwift: ANT+ Worker Thread (Header)
//
// Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
// Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
//
// QObject managing pybind11-based interaction with embedded Python ANT+
// broadcaster. Runs on a dedicated QThread and polls `bluetoothdevice` for
// speed updates to pass to the Python broadcaster.
//
// Key notes: high-priority thread, cached Python objects, and phased shutdown.
// =============================================================================

#ifdef ANT_LINUX_ENABLED
#pragma once

#include "devices/bluetoothdevice.h"
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <atomic>
#include <memory>

#ifdef slots
    #undef slots
    #include <pybind11/embed.h>
    #define slots Q_SLOTS
#else
    #include <pybind11/embed.h>
#endif

namespace py = pybind11;

class __attribute__((visibility("hidden"))) AntWorker : public QObject
{
    Q_OBJECT

public:
    explicit AntWorker(bluetoothdevice* device, QObject *parent = nullptr);
    ~AntWorker() override;

public Q_SLOTS:
    void start();
    void stop();
    void doWork();

Q_SIGNALS:
    void finished();

private:
    bool initializePython();

    // Core members
    bluetoothdevice* m_device;
    QTimer* m_timer = nullptr;
    std::atomic<bool> m_stopRequested{false};

    // --- Python Integration Members ---
    std::unique_ptr<py::scoped_interpreter> m_pyGuard;
    std::unique_ptr<py::object> m_pyBroadcaster;
    py::object m_pyOriginalStdout;
    py::object m_pyOriginalStderr;
    py::object m_pyLogger;

    // --- Performance Optimization Members ---
    bool m_isVirtualDevice = false;
    QElapsedTimer m_connectionTimer;

    // Cached Python objects for performance
    py::object m_pySendFunc;
    py::object m_pyGcModule;
    py::object m_pySysModule;

    // Rate-limiting for Python calls
    QElapsedTimer m_sendRateTimer;
    int m_sendIntervalMs;

    // Cached state to avoid redundant calculations
    double m_lastSpeedKmh;
    int m_lastEstimatedCadence;
};

#endif // ANT_LINUX_ENABLED