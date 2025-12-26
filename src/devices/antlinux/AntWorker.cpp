// =============================================================================
// QDomyos-Zwift: ANT+ Worker Thread (Implementation)
//
// Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
// Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
//
// Implements the AntWorker class: embedded Python lifecycle, device polling,
// and data transmission. Optimized for low-CPU devices with strict timing.
//
// Key points: high-priority thread, coarse timers, cached Python objects,
// and a phased, synchronous shutdown sequence to safely release USB resources.
// =============================================================================

#ifdef ANT_LINUX_ENABLED
#include "AntWorker.h"
#include <QDebug>
#include <QThread>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>
#include <QMutex>
#include <chrono>
#include <cmath>
#include <string>
#include <memory>

#ifdef slots
    #undef slots
    #include <pybind11/embed.h>
    #include <pybind11/functional.h>
    #define slots Q_SLOTS
#else
    #include <pybind11/embed.h>
    #include <pybind11/functional.h>
#endif

#include "ant_footpod_script.h"

namespace py = pybind11;

extern bool ant_verbose;
extern int ant_device_id;

struct PythonLogger {
    void write(const std::string &message) {
        QString q_message = QString::fromStdString(message).trimmed();
        if (!q_message.isEmpty()) { qInfo().noquote() << "[ANT+]" << q_message; }
    }
    void flush() {}
};

inline int estimateCadence(double speed_kmh) noexcept {
    if (speed_kmh < 0.5) return 0;
    if (speed_kmh < 3.0) return static_cast<int>(speed_kmh * 30.0 + 0.5) & ~1;
    if (speed_kmh < 7.0) return static_cast<int>((speed_kmh - 3.0) * 12.5 + 90.0 + 0.5) & ~1;
    return static_cast<int>(std::min((speed_kmh - 7.0) * 4.0 + 160.0, 200.0) + 0.5) & ~1;
}

QString getVenvSitePackages() {
    // Cache the resolved venv site-packages path to avoid spawning a Python
    // subprocess on every initialization. Use a mutex for thread-safety.
    static QString cachedPath;
    static bool pathResolved = false;
    static QMutex cacheMutex;

    QMutexLocker locker(&cacheMutex);
    if (pathResolved) {
        return cachedPath;
    }

    QByteArray sudoUser = qgetenv("SUDO_USER");
    QByteArray qzUser = qgetenv("QZ_USER");
    QString homePath = (!sudoUser.isEmpty()) ? "/home/" + QString::fromLocal8Bit(sudoUser) :
                      (!qzUser.isEmpty())   ? "/home/" + QString::fromLocal8Bit(qzUser) : QDir::homePath();
    
    QString venvPythonPath = homePath + "/ant_venv/bin/python";
    if (!QFile::exists(venvPythonPath)) {
        qCritical() << "[ANT+] CRITICAL: Virtual environment not found at:" << venvPythonPath;
        return QString();
    }

    QProcess process;
    process.start(venvPythonPath, QStringList() << "-c" << "import site; print(site.getsitepackages()[0])");
    process.waitForFinished(3000);

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        QString sitePackages = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        if (!sitePackages.isEmpty()) {
            cachedPath = sitePackages;
            pathResolved = true;
            return cachedPath;
        }
    }
    qCritical() << "[ANT+] Failed to determine venv site-packages path.";
    return QString();
}

bool AntWorker::initializePython() {
    try {
        m_pyGuard = std::make_unique<py::scoped_interpreter>();
        py::gil_scoped_acquire gil;

        QString sitePackagesPath = getVenvSitePackages();
        if (sitePackagesPath.isEmpty()) return false;

        py::module_ sys = py::module_::import("sys");
        sys.attr("path").attr("append")(sitePackagesPath.toStdString());

        m_pyOriginalStdout = sys.attr("stdout");
        m_pyOriginalStderr = sys.attr("stderr");

        py::module_ main_module = py::module_::import("__main__");
        py::class_<PythonLogger>(main_module, "PythonLogger")
            .def(py::init<>()).def("write", &PythonLogger::write).def("flush", &PythonLogger::flush);

        m_pyLogger = main_module.attr("PythonLogger")();
        sys.attr("stdout") = m_pyLogger;
        sys.attr("stderr") = m_pyLogger;
        
        py::exec(ant_footpod_script, main_module.attr("__dict__"));
        m_pyBroadcaster = std::make_unique<py::object>(main_module.attr("AntBroadcaster")());
        m_pySendFunc = (*m_pyBroadcaster).attr("send_ant_data");

    } catch (const py::error_already_set& e) {
        qCritical() << "[ANT+] Python initialization failed:" << e.what();
        return false;
    }
    qInfo() << "[ANT+] Python interpreter and ANT module initialized successfully.";
    return true;
}

AntWorker::AntWorker(bluetoothdevice* device, QObject *parent) : QObject(parent), m_device(device) {
    if (m_device) {
        m_isVirtualDevice = (m_device->metaObject()->className() == QString("faketreadmill"));
    }
}

AntWorker::~AntWorker() {
    qInfo() << "[ANT+] AntWorker destroyed.";
}

void AntWorker::start() {
    qInfo() << "[ANT+] Worker process started on thread" << QThread::currentThread();
    QThread::currentThread()->setPriority(QThread::HighPriority);

    if (!initializePython()) {
        emit finished();
        return;
    }

    try {
        py::gil_scoped_acquire gil;
        if (!m_pyBroadcaster->attr("start")(ant_device_id, ant_verbose).cast<bool>()) {
            qCritical() << "[ANT+] Python broadcaster's start() method returned false.";
            emit finished();
            return;
        }
    } catch (const py::error_already_set& e) {
        qCritical() << "[ANT+] FAILED TO START BROADCASTING (exception in Python):" << e.what();
        emit finished();
        return;
    }
    
    qInfo() << "[ANT+] ANT+ broadcasting started. Starting polling timer.";
    m_connectionTimer.start();

    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::CoarseTimer);
    connect(m_timer, &QTimer::timeout, this, &AntWorker::doWork);
    // Poll at 4Hz to ensure fresh data is always ready for the Python thread's fastest broadcast rate.
    m_timer->start(250);
}

void AntWorker::stop() {
    if (m_stopRequested.exchange(true)) {
        return;
    }
    qInfo() << "[ANT+] AntWorker::stop() called on thread" << QThread::currentThread();

    if (m_timer) {
        m_timer->stop();
    }

    try {
        py::gil_scoped_acquire gil;
        if (m_pyBroadcaster) {
            (*m_pyBroadcaster).attr("stop")();
        }
        if (!py::module_::import("sys").attr("stdout").is_none()) {
             py::module_::import("sys").attr("stdout") = m_pyOriginalStdout;
        }
        if (!py::module_::import("sys").attr("stderr").is_none()) {
             py::module_::import("sys").attr("stderr") = m_pyOriginalStderr;
        }
        m_pyBroadcaster.reset();
        m_pyLogger = py::object();
    } catch (const std::exception &ex) {
        qWarning() << "[ANT+] C++ exception during Python shutdown:" << ex.what();
    }

    if (m_pyGuard) {
        qInfo() << "[ANT+] Finalizing Python interpreter...";
        m_pyGuard.reset();
        qInfo() << "[ANT+] Python interpreter finalized.";
    }

    qInfo() << "[ANT+] Worker cleanup complete, emitting finished signal.";
    emit finished();
}

void AntWorker::doWork() {
    if (m_stopRequested) return;

    const double KMH_TO_MPS_FACTOR = 1.0 / 3.6;
    double speed_kmh = 0.0;
    int estimated_cadence = 0;

    if (m_device && m_device->connected()) {
        speed_kmh = m_device->currentSpeed().value();
        if (!m_isVirtualDevice && m_connectionTimer.elapsed() < 5000) speed_kmh = 0.0;
        if (!std::isfinite(speed_kmh) || speed_kmh < 0) speed_kmh = 0.0;
    } else {
        m_connectionTimer.restart();
    }

    estimated_cadence = estimateCadence(speed_kmh);

    try {
        py::gil_scoped_acquire gil;
        if (!m_pySendFunc.is_none()) {
            m_pySendFunc(speed_kmh * KMH_TO_MPS_FACTOR, estimated_cadence);
        }
    } catch (const py::error_already_set& e) {
        qCritical() << "[ANT+] Python exception in doWork:" << e.what();
        stop();
    }
}

#endif // ANT_LINUX_ENABLED