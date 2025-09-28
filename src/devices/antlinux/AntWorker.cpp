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

// This helper function finds the site-packages directory of the user's venv,
// correctly handling the environment when run with `sudo`.
QString findVenvSitePackages() {
    // When run with `sudo`, the `SUDO_USER` variable contains the original user's name.
    QByteArray sudoUser = qgetenv("SUDO_USER");
    QString homePath;

    if (!sudoUser.isEmpty()) {
        // If we know the original user, construct the path to their home directory.
        homePath = "/home/" + QString::fromLocal8Bit(sudoUser);
        qInfo() << "[ANT+] Detected sudo user:" << sudoUser << ", setting home path to" << homePath;
    } else {
        // If not run with sudo (e.g., direct root login), use the current home path.
        homePath = QDir::homePath();
        qInfo() << "[ANT+] No sudo user detected, using default home path:" << homePath;
    }
    
    QString pythonExecutable = homePath + "/ant_venv/bin/python3";

    if (!QFile::exists(pythonExecutable)) {
        qWarning() << "[ANT+] CRITICAL: Python executable not found at the expected path:" << pythonExecutable;
        return QString();
    }

    QProcess process;
    process.start(pythonExecutable, QStringList() << "-c" << "import sys; import site; print(next((p for p in sys.path if 'site-packages' in p), ''))");
    process.waitForFinished(3000);

    if (process.exitCode() == 0) {
        QString sitePackages = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        if (!sitePackages.isEmpty()) {
            qInfo() << "[ANT+] Found venv site-packages at:" << sitePackages;
            return sitePackages;
        }
    }
    
    qWarning() << "[ANT+] Failed to determine venv site-packages path.";
    return QString();
}

bool AntWorker::initializePython() {
    try {
        // 1. Initialize the embedded interpreter.
        m_pyGuard = std::make_unique<py::scoped_interpreter>();

        // 2. Find the correct site-packages path for the user's venv.
        QString sitePackagesPath = findVenvSitePackages();
        if (sitePackagesPath.isEmpty()) {
            qCritical() << "[ANT+] Cannot proceed without a valid site-packages path.";
            return false;
        }

        // 3. Append the located path to the interpreter's sys.path.
        py::module_ sys = py::module_::import("sys");
        sys.attr("path").attr("append")(sitePackagesPath.toStdString());
        qInfo() << "[ANT+] Appended venv site-packages to Python interpreter path.";

        // 4. Set up the C++ logging redirector.
        py::module_ main_module = py::module_::import("__main__");
        py::class_<PythonLogger>(main_module, "PythonLogger")
            .def(py::init<>())
            .def("write", &PythonLogger::write)
            .def("flush", &PythonLogger::flush);

        sys.attr("stdout") = std::make_shared<PythonLogger>();
        sys.attr("stderr") = std::make_shared<PythonLogger>();
        
        // 5. Execute the embedded script and create an instance of the AntBroadcaster class.
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
    // Safety net - but cleanup should already be done in stop()
    if (m_pyBroadcaster || m_pyGuard) {
        qWarning() << "[ANT+] Destructor cleanup - Python resources still active";
        shutdownPython();
    }
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

    // CRITICAL: Clean up Python resources on worker thread
    shutdownPython();
    
    qInfo() << "[ANT+] Worker cleanup complete, emitting finished signal";
    emit finished();
}

void AntWorker::doWork() {
    if (m_stopRequested) {
        return;
    }

    double speed_kmh = 0.0;
    if (m_device && m_device->connected()) {
        speed_kmh = m_device->currentSpeed().value();
        if (m_connectionTimer.elapsed() < 10000) { speed_kmh = 0.0; }
        if (!std::isfinite(speed_kmh) || speed_kmh < 0) { speed_kmh = 0.0; }
    } else {
        m_connectionTimer.restart();
    }

    try {
        py::gil_scoped_acquire gil;
        m_pyBroadcaster->attr("send_ant_data")(speed_kmh / 3.6);
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