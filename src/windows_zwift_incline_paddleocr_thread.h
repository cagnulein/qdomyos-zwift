#ifndef WINDOWS_ZWIFT_INCLINE_PADDLEOCR_THREAD_H
#define WINDOWS_ZWIFT_INCLINE_PADDLEOCR_THREAD_H

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
#include <QProcess>
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

class windows_zwift_incline_paddleocr_thread : public QThread {
    Q_OBJECT
  public:
    explicit windows_zwift_incline_paddleocr_thread(bluetoothdevice *device);
    ~windows_zwift_incline_paddleocr_thread();
    void run();

  signals:
    void onSpeed(double speed);
    void onInclination(double inclination, double grade);
    void debug(QString string);

  private slots:
    void processOutput();
    void handleError(QProcess::ProcessError error);

  private:
    double inclination = 0;
    double speed = 0;
    bluetoothdevice *device;
    QProcess *process;
};

#endif // WINDOWS_ZWIFT_INCLINE_PADDLEOCR_THREAD_H
