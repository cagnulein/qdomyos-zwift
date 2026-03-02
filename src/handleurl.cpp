#include "handleurl.h"
#include <QDebug>
#include <QUrl>

void HandleURL::handleURL(const QUrl &url)
{
    qDebug() << url;
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
    h->urlParser(url.toString().toLatin1());
#endif
}

HandleURL::HandleURL() {
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
    h = new lockscreen();
#endif
}

