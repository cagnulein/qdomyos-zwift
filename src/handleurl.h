#ifndef HANDLEURL
#define HANDLEURL

#include <QDesktopServices>
#include <QObject>

class HandleURL : public QObject
{
    Q_OBJECT

signals:
    void incomingURL(QString path);

public slots:
     void handleURL(const QUrl &url);
};

#endif
