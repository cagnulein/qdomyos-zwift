#include "handleurl.h"
#include <QDebug>
#include <QUrl>

void HandleURL::handleURL(const QUrl &url)
{
    qDebug() << url;
#ifndef IO_UNDER_QT
    h->urlParser(url.toString().toLatin1());
#endif
}

HandleURL::HandleURL() {
#ifndef IO_UNDER_QT
    h = new lockscreen();
#endif
}

