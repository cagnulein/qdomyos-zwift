#ifndef ANDROIDADBLOG_H
#define ANDROIDADBLOG_H

#include <QDebug>
#include <QProcess>
#include <QThread>
#include <QDateTime>

class androidadblog : public QThread
{   
    Q_OBJECT

  public:
    explicit androidadblog();

    void run();

  private:
    void runAdbTailCommand(QString command);
    bool found = false;
};

#endif // ANDROIDADBLOG_H
