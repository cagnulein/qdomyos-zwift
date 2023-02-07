#ifndef ANDROIDADBLOG_H
#define ANDROIDADBLOG_H

#include <QDebug>
#include <QProcess>
#include <QThread>

class androidadblog : public QThread
{   
    Q_OBJECT

  public:
    explicit androidadblog();

    void run();

  private:
    void runAdbTailCommand(QString command);
};

#endif // ANDROIDADBLOG_H
