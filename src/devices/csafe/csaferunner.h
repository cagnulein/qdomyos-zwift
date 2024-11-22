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

class CsafeRunnerThread : public QThread {
    Q_OBJECT

  public:
    explicit CsafeRunnerThread();
    explicit CsafeRunnerThread(QString deviceFileName, int sleepTime = 200);
    void setDevice(const QString &device);
    void setBaudRate(unsigned int baudRate = 9600);
    void setSleepTime(int time);
    void setRefreshCommands(const QStringList &commands);
    void run();

  public slots:
    void sendCommand(const QStringList &commands);

  signals:
    void onCsafeFrame(const QVariantMap &frame);
    void portAvailable(bool available);

  private:
    QString deviceName;
    unsigned int baudRate = 9600;
    int sleepTime = 200;
    QStringList refreshCommands;
    QQueue<QStringList> commandQueue;
    QMutex mutex;
};
