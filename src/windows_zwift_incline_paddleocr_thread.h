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
#include <QFileSystemWatcher>
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
    void processError();
    void handleError(QProcess::ProcessError error);

  private:
    // Core data members
    double inclination = 0;
    double speed = 0;
    bluetoothdevice *device;

    // Process-related members (kept for compatibility)
    QProcess *process;

    // File monitoring members
    QFileSystemWatcher *fileWatcher;
    qint64 lastFilePosition; // Track last read position in file

    // Helper methods
    void readFileContent(const QString &filePath);
    void processDataLine(const QString &line);
};

#endif // WINDOWS_ZWIFT_INCLINE_PADDLEOCR_THREAD_H
