// -----------------------------------------------------------------------------
// QDomyos-Zwift: ANT+ Virtual Footpod Feature
// C++/Python Bridge for ANT+ Broadcasting (Implementation)
//
// Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
// Contributor(s): bassai-sho
// AI analysis tools (Claude, Gemini) were used to assist coding and debugging
// Licensed under GPL-3.0 - see project repository for full license
//
// This file implements the AntWorker class. It runs on a dedicated QThread
// and serves as the bridge between the C++ application and the Python ANT+
// script. It manages the embedded Python interpreter's lifecycle, polls the
// treadmill for speed, and passes data to the Python script.
// -----------------------------------------------------------------------------

#ifdef ANT_LINUX_ENABLED
#include "AntWorker.h"
#include <QDebug>
#include <QThread>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>
#include <cstdlib> // For setenv
#include <chrono>
#include <cmath>
#include <string>
#include <memory>

#ifdef slots
    #undef slots
    #include <pybind11/embed.h>
    #define slots Q_SLOTS
#else
    #include <pybind11/embed.h>
#endif

#include "ant_footpod_script.h"

namespace py = pybind11;

extern bool ant_verbose;
extern int ant_device_id;

// Struct for redirecting Python stdout/stderr to Qt's logging system
struct PythonLogger {
    void write(const std::string &message) {
        QString q_message = QString::fromStdString(message).trimmed();
        if (!q_message.isEmpty()) { qInfo().noquote() << "[ANT+]" << q_message; }
    }
    void flush() {}
};

// Estimates a realistic running cadence (in Steps Per Minute) from speed
int estimateCadence(double speed_kmh) {
    if (speed_kmh < 0.5) {
        return 0;
    }
    double cadence = 0.0;
    if (speed_kmh < 7.0) { // WALKING ZONE
        cadence = (speed_kmh < 3.0) ? (speed_kmh * 30.0) : ((speed_kmh - 3.0) * 12.5 + 90.0);
    } else { // RUNNING ZONE
        cadence = std::min((speed_kmh - 7.0) * 4.0 + 160.0, 200.0);
    }
    return static_cast<int>(cadence + 0.5) & ~1;
}

// This helper function finds the site-packages directory of the user's venv.
// It is the definitive method for locating the required Python libraries.
QString getVenvSitePackages() {
    QByteArray sudoUser = qgetenv("SUDO_USER");
    QByteArray qzUser = qgetenv("QZ_USER");
    QString homePath;

    if (!sudoUser.isEmpty()) {
        homePath = "/home/" + QString::fromLocal8Bit(sudoUser);
    } else if (!qzUser.isEmpty()) {
        homePath = "/home/" + QString::fromLocal8Bit(qzUser);
    } else {
        homePath = QDir::homePath();
    }
    
    QString venvPythonPath = homePath + "/ant_venv/bin/python";
    if (!QFile::exists(venvPythonPath)) {
        qCritical() << "[ANT+] CRITICAL: Virtual environment not found at the expected path:" << venvPythonPath;
        qCritical() << "[ANT+] GUIDANCE: Please create the virtual environment as per the README instructions.";
        return QString();
    }

    qInfo() << "[ANT+] Found virtual environment. Querying for its site-packages directory...";
    QProcess process;
    // This Python command is the most reliable way to get the site-packages path
    process.start(venvPythonPath, QStringList() << "-c" << "import site; print(site.getsitepackages()[0])");
    process.waitForFinished(3000);

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        QString sitePackages = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        if (!sitePackages.isEmpty()) {
            qInfo() << "[ANT+] Found venv site-packages at:" << sitePackages;
            return sitePackages;
        }
    }
    
    qCritical() << "[ANT+] Failed to determine venv site-packages path.";
    return QString();
}

bool AntWorker::initializePython() {
    try {
        // Step 1: Initialize the embedded interpreter. It starts with default system paths.
        m_pyGuard = std::make_unique<py::scoped_interpreter>();

        // Step 2: Find the correct site-packages path for the user's venv.
        QString sitePackagesPath = getVenvSitePackages();
        if (sitePackagesPath.isEmpty()) {
            qCritical() << "[ANT+] Cannot proceed without a valid site-packages path.";
            return false;
        }

        // Step 3: Explicitly add the venv's site-packages to the embedded interpreter's path.
        py::module_ sys = py::module_::import("sys");
        sys.attr("path").attr("append")(sitePackagesPath.toStdString());
        qInfo() << "[ANT+] Successfully added venv site-packages to the embedded Python interpreter's path.";

        // Step 4: Set up the C++ logging redirector.
        py::module_ main_module = py::module_::import("__main__");
        py::class_<PythonLogger>(main_module, "PythonLogger")
            .def(py::init<>())
            .def("write", &PythonLogger::write)
            .def("flush", &PythonLogger::flush);

        sys.attr("stdout") = std::make_shared<PythonLogger>();
        sys.attr("stderr") = std::make_shared<PythonLogger>();
        
        // Step 5: Execute the embedded script and create an instance of the AntBroadcaster class.
        py::exec(ant_footpod_script, main_module.attr("__dict__"));
        m_pyBroadcaster = std::make_unique<py::object>(main_module.attr("AntBroadcaster")());

    } catch (const py::error_already_set& e) {
        qCritical() << "[ANT+] Python initialization failed:" << e.what();
        return false;
    }
    qInfo() << "[ANT+] Python interpreter and ANT module initialized successfully.";
    return true;
}

AntWorker::AntWorker(bluetoothdevice* device, QObject *parent)
    : QObject(parent), m_device(device) {}

AntWorker::~AntWorker() {
    qInfo() << "[ANT+] AntWorker destroyed.";
}

void AntWorker::start() {
    qInfo() << "[ANT+] Worker process started on thread" << QThread::currentThread();
    if (!initializePython()) {
        emit finished();
        return;
    }

    try {
        py::gil_scoped_acquire gil;
        if (!m_pyBroadcaster->attr("start")(ant_device_id, ant_verbose).cast<bool>()) {
            qCritical() << "[ANT+] Python broadcaster's start() method returned false (check Python logs for a specific error).";
            emit finished();
            return;
        }
    } catch (const py::error_already_set& e) {
        qCritical() << "[ANT+] FAILED TO START BROADCASTING. An exception occurred in the Python script:";
        qCritical().noquote() << e.what();
        emit finished();
        return;
    }
    
    qInfo() << "[ANT+] ANT+ broadcasting started. Starting polling timer.";
    m_connectionTimer.start();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &AntWorker::doWork);
    m_timer->start(250);
}

void AntWorker::stop() {
    if (m_stopRequested.exchange(true)) {
        return;
    }
    qInfo() << "[ANT+] AntWorker::stop() called on thread" << QThread::currentThread();
    
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }

    shutdownPython();
    
    qInfo() << "[ANT+] Worker cleanup complete, emitting finished signal";
    emit finished();
}

void AntWorker::doWork() {
    if (m_stopRequested) {
        return;
    }

    double speed_kmh = 0.0;
    int estimated_cadence = 0;
    if (m_device && m_device->connected()) {
        speed_kmh = m_device->currentSpeed().value();
        if (m_connectionTimer.elapsed() < 10000) { speed_kmh = 0.0; }
        if (!std::isfinite(speed_kmh) || speed_kmh < 0) { speed_kmh = 0.0; }
        if (speed_kmh > 0.5) {
            estimated_cadence = estimateCadence(speed_kmh);
        }
    } else {
        m_connectionTimer.restart();
    }

    try {
        py::gil_scoped_acquire gil;
        m_pyBroadcaster->attr("send_ant_data")(speed_kmh / 3.6, estimated_cadence);
    } catch (const py::error_already_set& e) {
        qCritical() << "[ANT+] Python exception in doWork:" << e.what();
        stop();
    }
}

void AntWorker::shutdownPython() {
    if (m_pyBroadcaster) {
        qInfo() << "[ANT+] Shutting down Python broadcaster...";
        try {
            py::gil_scoped_acquire gil;
            m_pyBroadcaster->attr("stop")();
            qInfo() << "[ANT+] Python broadcaster stopped successfully";
        } catch (const py::error_already_set& e) {
            qWarning() << "[ANT+] Error during Python broadcaster stop:" << e.what();
        } catch (...) {
            qWarning() << "[ANT+] Unknown error during Python broadcaster stop";
        }
        m_pyBroadcaster.reset();
    }
    
    if (m_pyGuard) {
        qInfo() << "[ANT+] Finalizing Python interpreter...";
        m_pyGuard.reset();
        qInfo() << "[ANT+] Python interpreter finalized.";
    }
}
#endif // ANT_LINUX_ENABLED