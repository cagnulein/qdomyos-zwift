#include "windows_zwift_workout_paddleocr_thread.h"
#include "devices/elliptical.h"
#include "devices/treadmill.h"
#if __has_include("aiserver.h")
#include "aiserver.h"
#endif
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

windows_zwift_workout_paddleocr_thread::windows_zwift_workout_paddleocr_thread(bluetoothdevice *device) {
    this->device = device;
    emit debug("windows_zwift_workout_paddleocr_thread created!");
}

void windows_zwift_workout_paddleocr_thread::run() {
    float lastInclination = -100;
    float lastSpeed = -100;
    while (1) {
        QString ret = runPython("zwift-workout.py");
        if (ret.length() > 0) {
            QStringList list = ret.split(";");
            if (list.length() >= 2) {
                emit debug("windows_zwift_workout_paddleocr_thread onInclination " + list.at(1) + " onSpeed " +
                           list.at(0).toFloat());
                if (!list.at(1).toUpper().contains("NONE")) {
                    float inc = list.at(1).toFloat();
                    if (inc != lastInclination)
                        emit onInclination(inc, inc);
                    lastInclination = inc;
                }
                if (!list.at(0).toUpper().contains("NONE")) {
                    float speed = list.at(0).toFloat();
                    if (speed != lastSpeed)
                        emit onSpeed(speed);
                    lastSpeed = speed;
                }
            }
        }
    }
}

QString windows_zwift_workout_paddleocr_thread::runPython(QString command) {
#ifdef Q_OS_WINDOWS
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    QString currentPath = env.value("PATH");
    QString updatedPath = currentPath + ";" + QCoreApplication::applicationDirPath() + "\\python\\x64;C:\\Program Files\\CodeProject\\AI\\modules\\OCR\\bin\\windows\\python37\\venv\\Scripts";
    env.insert("PATH", updatedPath);

    QProcess process;
    process.setProcessEnvironment(env);
    //qDebug() << "env >> " << env.value("PATH");
    qDebug() << "run >> " << command;
#ifndef AISERVER    
    process.start("python\\x64\\python.exe", QStringList(command.split(' ')));
#else
    process.start("C:\\Program Files\\CodeProject\\AI\\modules\\OCR\\bin\\windows\\python37\\venv\\Scripts\\python.exe", QStringList(command.split(' ')));
#endif
    process.waitForFinished(-1); // will wait forever until finished

    QString out = process.readAllStandardOutput();
    QString err = process.readAllStandardError();

    emit debug("python << OUT " + out);
    emit debug("python << ERR " + err);
#else
    QString out;
#endif
    return out;
}
