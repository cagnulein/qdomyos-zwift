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
}

void windows_zwift_incline_paddleocr_thread::run() {
    while (1) {
        QSettings settings;
        QString ret;
        if (settings.value(QZSettings::zwift_ocr_climb_portal, QZSettings::default_zwift_ocr_climb_portal).toBool())
            ret = runPython("zwift-incline-climb-portal.py");
        else
            ret = runPython("zwift-incline.py");
        if (!ret.toUpper().contains("NONE") && ret.length() > 0) {
            emit debug("windows_zwift_incline_paddleocr_thread onInclination " + QString::number(ret.toFloat()));
            emit onInclination(ret.toFloat(), ret.toFloat());
        }
        msleep(100);
    }
}

QString windows_zwift_incline_paddleocr_thread::runPython(QString command) {
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
