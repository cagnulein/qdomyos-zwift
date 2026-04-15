#ifndef HANDLEURL
#define HANDLEURL

#include <QDesktopServices>
#include <QObject>

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class HandleURL : public QObject
{
    Q_OBJECT

public:
    HandleURL();

private:
#ifdef Q_OS_IOS
    lockscreen* h;
#endif
    
signals:
    void incomingURL(QString path);

public slots:
     void handleURL(const QUrl &url);
};
#endif

