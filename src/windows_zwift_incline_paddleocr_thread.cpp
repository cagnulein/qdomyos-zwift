#include "windows_zwift_incline_paddleocr_thread.h"
#include "elliptical.h"
#include "treadmill.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QMetaEnum>
#include <QProcess>
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
    }
}

QString windows_zwift_incline_paddleocr_thread::runPython(QString command) {
#ifdef Q_OS_WINDOWS
    QProcess process;
    qDebug() << "run >> " << command;
    process.start("python\\x64\\python.exe", QStringList(command.split(' ')));
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
