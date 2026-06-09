#ifndef CSAFERUNNER_H
#define CSAFERUNNER_H

#include "csafe.h"
#include "qzsettings.h"
#include "serialhandler.h"
#include <QDebug>
#include <QMutex>
#include <QQueue>
#include <QSettings>
#include <QThread>
#include <QVariantMap>
#include <QVector>

#define MAX_QUEUE_SIZE 100
/**
 * @brief The CsafeRunnerThread class is a thread that runs the CSAFE protocol interaction.
 * It periodically sends the refresh commands processes the responses.
 * It also allows sending additional commands to the device.
 */
class CsafeRunnerThread : public QThread {
    Q_OBJECT

  public:
    explicit CsafeRunnerThread();
    explicit CsafeRunnerThread(QString deviceFileName, int sleepTime = 200);
    void setDevice(const QString &device);
    void setBaudRate(uint32_t baudRate = 9600);
    void setSleepTime(int time);
    void addRefreshCommand(const QStringList &commands);
    void run();

  public slots:
    void sendCommand(const QStringList &commands);

  signals:
    void onCsafeFrame(const QVariantMap &frame);
    void portAvailable(bool available);

  private:
    QString deviceName;
    uint32_t baudRate = 9600;
    int sleepTime = 200;
    QList<QStringList> refreshCommands;
    QQueue<QStringList> commandQueue;
    QMutex mutex;
};

#endif // CSAFERUNNER_H
