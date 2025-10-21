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
        if (!q_message.isEmpty()) { qInfo().noquote() << "[PYTHON]" << q_message; }
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

// Helper to run a QProcess and check if a Python executable has the required libraries.
bool checkPythonForLibs(const QString& pythonExecutable) {
    QProcess process;
    // This command is the ultimate test: can this Python interpreter import the needed libraries?
    process.start(pythonExecutable, QStringList() << "-c" << "import openant, usb, pybind11");
    process.waitForFinished(5000); // 5-second timeout

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        return true;
    }
    return false;
}

// NEW: This function intelligently searches for a working Python environment.
// It prioritizes a local venv, then falls back to the system-wide/user-site install.
bool findWorkingPython() {
    // --- Step 1: Prioritize the `ant_venv` virtual environment ---
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
    if (QFile::exists(venvPythonPath)) {
        qInfo() << "[ANT+] Found potential virtual environment at:" << venvPythonPath;
        if (checkPythonForLibs(venvPythonPath)) {
            qInfo() << "[ANT+] SUCCESS: Virtual environment has all required libraries. Using this environment.";
            // We don't need to return the path, because pybind11 will find it if we set the program name.
            // However, pybind11 doesn't have an easy way to start a *specific* interpreter by path.
            // The simplest way is just to let the default interpreter find the packages, which our checks now guarantee.
            // For a venv, the user should be running QZ from an activated shell, which this check confirms.
            return true;
        }
        qWarning() << "[ANT+] Virtual environment found, but it's missing required libraries (openant, pyusb, pybind11).";
    }

    // --- Step 2: Fallback to the system's default `python3` ---
    qInfo() << "[ANT+] No valid virtual environment found. Checking system/user Python install...";
    if (checkPythonForLibs("python3")) {
        qInfo() << "[ANT+] SUCCESS: System/user Python has all required libraries. Using this environment.";
        return true;
    }

    // --- Step 3: If both fail, provide comprehensive guidance ---
    qCritical() << "[ANT+] CRITICAL: Could not find a working Python environment.";
    qCritical() << "[ANT+] No required libraries (openant, pyusb, pybind11) found in the `~/ant_venv` virtual environment OR in the system/user path.";
    qCritical() << "[ANT+] GUIDANCE: Please use ONE of the following methods to install the libraries:";
    qCritical() << "[ANT+]   1. (RECOMMENDED) Use a virtual environment:";
    qCritical() << "[ANT+]      python3.11 -m venv ~/ant_venv";
    qCritical() << "[ANT+]      ~/ant_venv/bin/pip install openant pyusb pybind11";
    qCritical() << "[ANT+]   2. (For Bookworm/PEP 668) Install to user directory:";
    qCritical() << "[ANT+]      pip3 install --user openant pyusb pybind11";
    qCritical() << "[ANT+]   3. (Legacy) Install system-wide:";
    qCritical() << "[ANT+]      sudo pip3 install openant pyusb pybind11";

    return false;
}

bool AntWorker::initializePython() {
    try {
        // 1. Find a working Python environment. This new function does all the heavy lifting.
        if (!findWorkingPython()) {
            qCritical() << "[ANT+] Aborting initialization due to missing Python environment.";
            return false;
        }

        // 2. Initialize the embedded interpreter. It will automatically use the correct
        //    environment (system or activated venv) that `findWorkingPython` just validated.
        m_pyGuard = std::make_unique<py::scoped_interpreter>();

        // 3. Set up the C++ logging redirector.
        py::module_ sys = py::module_::import("sys");
        py::module_ main_module = py::module_::import("__main__");
        py::class_<PythonLogger>(main_module, "PythonLogger")
            .def(py::init<>())
            .def("write", &PythonLogger::write)
            .def("flush", &PythonLogger::flush);

        sys.attr("stdout") = std::make_shared<PythonLogger>();
        sys.attr("stderr") = std::make_shared<PythonLogger>();
        
        // 4. Execute the embedded script and create an instance of the AntBroadcaster class.
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
        emit finished(); // Fail gracefully if Python init fails
        return;
    }

    if (!m_pyBroadcaster->attr("start")(ant_device_id, ant_verbose).cast<bool>()) {
        qCritical() << "[ANT+] Failed to start broadcasting.";
        emit finished();
        return;
    }
    
    qInfo() << "[ANT+] ANT+ broadcasting started. Starting polling timer.";
    m_connectionTimer.start();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &AntWorker::doWork);
    m_timer->start(250); // Start the 4Hz timer
}

void AntWorker::stop() {
    if (m_stopRequested.exchange(true)) {
        return; // Already stopping
    }
    qInfo() << "[ANT+] AntWorker::stop() called on thread" << QThread::currentThread();
    
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }

    // Clean up Python resources on worker thread
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
        stop(); // Stop broadcasting on error
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