#ifndef WINDOWS_ZWIFT_WORKOUT_PADDLEOCR_THREAD_H
#define WINDOWS_ZWIFT_WORKOUT_PADDLEOCR_THREAD_H

#include <QtCore/qbytearray.h>

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include "devices/bluetoothdevice.h"
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QThread>
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

class windows_zwift_workout_paddleocr_thread : public QThread {
    Q_OBJECT

  public:
    explicit windows_zwift_workout_paddleocr_thread(bluetoothdevice *device);

    void run();

  signals:
    void onInclination(double inclination, double grade);
    void onSpeed(double speed);
    void debug(QString string);

  private:
    double inclination = 0;
    double speed = 0;
    bluetoothdevice *device;
    QString runPython(QString command);
};

#endif // WINDOWS_ZWIFT_WORKOUT_PADDLEOCR_THREAD_H
