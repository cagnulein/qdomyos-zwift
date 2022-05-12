#ifndef SERIALDIRCON_H
#define SERIALDIRCON_H

#include <QMutex>
#include <QSerialPort>
#include <QThread>
#include <QWaitCondition>

class serialDircon : public QThread {
    Q_OBJECT

  public:
    explicit serialDircon(QObject *parent = nullptr);
    ~serialDircon();

    void open(const QString &portName, int waitTimeout);

  signals:
    void request(const QString &s);
    void error(const QString &s);
    void timeout(const QString &s);

  private:
    void run() override;
    void write(char *buffer, int len, QString info);
    QSerialPort serial;

    QString m_portName;
    int m_waitTimeout = 1000;
    QMutex m_mutex;
    bool m_quit = false;
};

#endif // SERIALDIRCON_H
