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
    qDebug() << "windows_zwift_incline_paddleocr_thread created!";
}

void windows_zwift_incline_paddleocr_thread::run() {
    while (1) {
        QString ret = runPython("zwift-incline.py");
        if (!ret.toUpper().contains("NONE")) {
            if (device->deviceType() == bluetoothdevice::TREADMILL)
                ((treadmill *)device)->changeInclination(ret.toFloat(), ret.toFloat());
            else if (device->deviceType() == bluetoothdevice::ELLIPTICAL)
                ((elliptical *)device)->changeInclination(ret.toFloat(), ret.toFloat());
        }
    }
}

QString windows_zwift_incline_paddleocr_thread::runPython(QString command) {
#ifdef Q_OS_WINDOWS
    QProcess process;
    qDebug() << "run >> " << command;
    process.start("python\\python.exe", QStringList(command.split(' ')));
    process.waitForFinished(-1); // will wait forever until finished

    QString out = process.readAllStandardOutput();
    QString err = process.readAllStandardError();

    qDebug() << "python << OUT" << out;
    qDebug() << "python << ERR" << err;
#else
    QString out;
#endif
    return out;
}
