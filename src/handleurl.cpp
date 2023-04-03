#include "handleurl.h"
#include <QDebug>
#include <QUrl>
#ifdef Q_OS_IOS
void HandleURL::handleURL(const QUrl &url)
{
    qDebug() << url;
    h->urlParser(url.toString().toLatin1());
}

HandleURL::HandleURL() {
    h = new lockscreen();
}
#endif
