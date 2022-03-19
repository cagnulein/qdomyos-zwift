#include "filedownloader.h"

fileDownloader::fileDownloader(QUrl imageUrl, QObject *parent) : QObject(parent) {
    connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply *)), this, SLOT(fileDownloaded(QNetworkReply *)));

    QNetworkRequest request(imageUrl);
    m_WebCtrl.get(request);
}

fileDownloader::~fileDownloader() {}

void fileDownloader::fileDownloaded(QNetworkReply *pReply) {
    m_DownloadedData = pReply->readAll();
    // emit a signal
    pReply->deleteLater();
    emit downloaded();
}

QByteArray fileDownloader::downloadedData() const { return m_DownloadedData; }
