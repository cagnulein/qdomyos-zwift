// -----------------------------------------------------------------------------
// QDomyos-Zwift: ANT+ Virtual Footpod Feature
// C++/Python Bridge for ANT+ Broadcasting (Header)
//
// Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
// Contributor(s): bassai-sho
// AI analysis tools (Claude, Gemini) were used to assist coding and debugging
// Licensed under GPL-3.0 - see project repository for full license
//
// This file declares the AntWorker class, the QObject responsible for managing
// the pybind11-based interaction with the embedded Python ANT+ broadcaster.
// Optimized for low-CPU devices (Raspberry Pi, etc.)
// -----------------------------------------------------------------------------

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

class AntWorker : public QObject
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