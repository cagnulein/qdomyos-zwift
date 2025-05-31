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
#include <QTextStream>
#include <chrono>
#include <math.h>
using namespace std::chrono_literals;

windows_zwift_incline_paddleocr_thread::windows_zwift_incline_paddleocr_thread(bluetoothdevice *device) {
    this->device = device;
    qDebug() << "windows_zwift_incline_paddleocr_thread created!";
    process = nullptr;
    fileWatcher = nullptr;
    lastFilePosition = 0;
}

windows_zwift_incline_paddleocr_thread::~windows_zwift_incline_paddleocr_thread() {
    // Clean up file watcher and timer if they exist
    if (fileWatcher) {
        delete fileWatcher;
    }

    // Clean up process if it exists (keeping original process logic for compatibility)
    if (process && process->state() == QProcess::Running) {
        process->terminate();
        process->waitForFinished(3000);
        if (process->state() == QProcess::Running) {
            process->kill();
        }
        delete process;
    }
}

void windows_zwift_incline_paddleocr_thread::processError() {
    // Keep original method for compatibility
    if (process) {
        QByteArray error = process->readAllStandardError();
        if (!error.isEmpty()) {
            QString errorStr = QString::fromUtf8(error.trimmed());
            qDebug() << "Error from process: " + errorStr;
        }
    }
}

void windows_zwift_incline_paddleocr_thread::run() {
#ifdef Q_OS_WINDOWS
    // Initialize file monitoring instead of external process
    QString filePath = QCoreApplication::applicationDirPath() + "/zwift-metrics.txt";

    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << "ERROR: zwift-metrics.txt not found at: " + filePath;
        qDebug() << "Creating empty zwift-metrics.txt file...";

        // Create empty file if it doesn't exist
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
        } else {
            qDebug() << "Failed to create zwift-metrics.txt file";
            return;
        }
    }

           // Create timer to periodically check file for new content
    QTimer *fileCheckTimer = new QTimer();
    fileCheckTimer->setInterval(100); // Check every 100ms for responsiveness

           // Connect timer timeout to file reading function
    connect(fileCheckTimer, &QTimer::timeout, this, [this, filePath]() {
        readFileContent(filePath);
    });

    qDebug() << "Starting file monitoring for: " + filePath;

    // Start the timer
    fileCheckTimer->start();

           // Use exec() to start the thread's event loop
           // This will process events, including signals and slots
    exec();

           // Clean up when thread exits
    fileCheckTimer->stop();
    delete fileCheckTimer;
#endif
}

void windows_zwift_incline_paddleocr_thread::readFileContent(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return; // Silently return if file can't be opened
    }

    // Seek to last known position to only read new content
    file.seek(lastFilePosition);

    QTextStream in(&file);
    QString newContent = in.readAll();

    if (!newContent.isEmpty()) {
        // Update last file position
        lastFilePosition = file.pos();

        // Process new lines
        QStringList lines = newContent.split('\n', Qt::SkipEmptyParts);

        for (const QString &line : lines) {
            if (line.trimmed().isEmpty()) continue;

            QString lineStr = line.trimmed();
            qDebug() << "File content received: " + lineStr;

            processDataLine(lineStr);
        }
    }

    file.close();
}

void windows_zwift_incline_paddleocr_thread::processDataLine(const QString &line) {
    QStringList parts = line.split(';');
    if (parts.size() == 2) {
        QString speedStr = parts[0].trimmed();
        QString inclinationStr = parts[1].trimmed();

               // Process speed if not "None"
        if (speedStr.toUpper() != "NONE") {
            bool ok;
            double speedValue = speedStr.toDouble(&ok);
            if (ok) {
                qDebug() << "Emitting speed: " + QString::number(speedValue);
                emit onSpeed(speedValue);
                speed = speedValue;
            }
        }

               // Process inclination if not "None"
        if (inclinationStr.toUpper() != "NONE") {
            bool ok;
            double inclinationValue = inclinationStr.toDouble(&ok);
            if (ok) {
                qDebug() << "Emitting inclination: " + QString::number(inclinationValue);
                emit onInclination(inclinationValue, inclinationValue);
                inclination = inclinationValue;
            }
        }
    }
}

void windows_zwift_incline_paddleocr_thread::processOutput() {
    // Keep original method for compatibility but adapt for file-based approach
    // This method is no longer used in file-based mode but kept for backward compatibility
    if (process) {
        QByteArray output = process->readAllStandardOutput();
        qDebug() << "Raw output received (length: " + QString::number(output.length()) + "): " + QString::fromUtf8(output);
        QList<QByteArray> lines = output.split('\n');

        for (const QByteArray &line : lines) {
            if (line.trimmed().isEmpty()) continue;

            QString lineStr = QString::fromUtf8(line.trimmed());
            qDebug() << "Received: " + lineStr;
            processDataLine(lineStr);
        }
    }
}

void windows_zwift_incline_paddleocr_thread::handleError(QProcess::ProcessError error) {
    // Keep original error handling for process compatibility
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
    qDebug() << "Process error: " + errorMessage;
}
