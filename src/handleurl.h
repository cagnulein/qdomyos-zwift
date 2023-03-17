#ifndef HANDLEURL
#define HANDLEURL

#include <QDesktopServices>
#include <QObject>
#include "ios/lockscreen.h"

class HandleURL : public QObject
{
    Q_OBJECT

public:
    HandleURL();
    
private:
    lockscreen* h;
    
signals:
    void incomingURL(QString path);

public slots:
     void handleURL(const QUrl &url);
};

#endif
