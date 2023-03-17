#include "handleurl.h"
#include <QDebug>
#include <QUrl>

void HandleURL::handleURL(const QUrl &url)
{
    qDebug() << url;
    h->urlParser(url.toString().toLatin1());
}

HandleURL::HandleURL() {
    h = new lockscreen();
}
