#include "windows_zwift_incline_paddleocr_thread.h"
#include "devices/elliptical.h"
#if __has_include("aiserver.h")
#include "aiserver.h"
#endif
#include "treadmill.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QMetaEnum>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>
using namespace std::chrono_literals;

windows_zwift_incline_paddleocr_thread::windows_zwift_incline_paddleocr_thread(bluetoothdevice *device) {
    this->device = device;
    emit debug("windows_zwift_incline_paddleocr_thread created!");
    process = new QProcess(this);
    connect(process, &QProcess::readyReadStandardOutput, this, &windows_zwift_incline_paddleocr_thread::processOutput);
    connect(process, &QProcess::errorOccurred, this, &windows_zwift_incline_paddleocr_thread::handleError);
}

windows_zwift_incline_paddleocr_thread::~windows_zwift_incline_paddleocr_thread() {
    if (process->state() == QProcess::Running) {
        process->terminate();
        process->waitForFinished(3000);
        if (process->state() == QProcess::Running) {
            process->kill();
        }
    }
    delete process;
}

void windows_zwift_incline_paddleocr_thread::run() {
#ifdef Q_OS_WINDOWS
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString currentPath = env.value("PATH");
    QString updatedPath = currentPath + ";" + QCoreApplication::applicationDirPath();
    env.insert("PATH", updatedPath);
    process->setProcessEnvironment(env);

    emit debug("Starting zwift-metrics-server.exe");
    process->start("zwift-metrics-server.exe", QStringList());

    // Main thread loop - keep checking if process is still running
    while (true) {
        if (process->state() != QProcess::Running) {
            emit debug("zwift-metrics-server.exe stopped, restarting...");
            process->start("zwift-metrics-server.exe", QStringList());
        }
        msleep(1000); // Check once per second if process is still running
    }
#endif
}

void windows_zwift_incline_paddleocr_thread::processOutput() {
    QByteArray output = process->readAllStandardOutput();
    QList<QByteArray> lines = output.split('\n');

    for (const QByteArray &line : lines) {
        if (line.trimmed().isEmpty()) continue;

        QString lineStr = QString::fromUtf8(line.trimmed());
        emit debug("Received: " + lineStr);

        QStringList parts = lineStr.split(';');
        if (parts.size() == 2) {
            QString speedStr = parts[0].trimmed();
            QString inclinationStr = parts[1].trimmed();

            // Process speed if not "None"
            if (speedStr.toUpper() != "NONE") {
                bool ok;
                double speedValue = speedStr.toDouble(&ok);
                if (ok) {
                    emit debug("Emitting speed: " + QString::number(speedValue));
                    emit onSpeed(speedValue);
                    speed = speedValue;
                }
            }

            // Process inclination if not "None"
            if (inclinationStr.toUpper() != "NONE") {
                bool ok;
                double inclinationValue = inclinationStr.toDouble(&ok);
                if (ok) {
                    emit debug("Emitting inclination: " + QString::number(inclinationValue));
                    emit onInclination(inclinationValue, inclinationValue);
                    inclination = inclinationValue;
                }
            }
        }
    }
}

void windows_zwift_incline_paddleocr_thread::handleError(QProcess::ProcessError error) {
    QString errorMessage;
    switch (error) {
    case QProcess::FailedToStart:
        errorMessage = "Failed to start zwift-metrics-server.exe";
        break;
    case QProcess::Crashed:
        errorMessage = "zwift-metrics-server.exe crashed";
        break;
    case QProcess::Timedout:
        errorMessage = "Operation timed out";
        break;
    case QProcess::ReadError:
        errorMessage = "Read error";
        break;
    case QProcess::WriteError:
        errorMessage = "Write error";
        break;
    case QProcess::UnknownError:
        errorMessage = "Unknown error";
        break;
    }
    emit debug("Process error: " + errorMessage);
}
