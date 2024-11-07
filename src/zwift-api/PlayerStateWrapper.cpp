#include "PlayerStateWrapper.h"
#include "../homeform.h"

ZwiftRequest::ZwiftRequest(const QString& getAccessToken) : getAccessToken(getAccessToken) {
    // Load the custom CA certificate
    QFile caFile(homeform::getWritableAppDir() + "ca.pem");
    if(caFile.open(QIODevice::ReadOnly))
        qDebug() << "cert file OK";
    else {
        qDebug() << "cert file ERR";
        return;
    }

    QSslCertificate caCert(&caFile, QSsl::Pem);
    caFile.close();

           // Set up the SSL configuration
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    QList<QSslCertificate> caCerts = sslConfig.caCertificates();
    caCerts.append(caCert);
    sslConfig.setCaCertificates(caCerts);
    QSslConfiguration::setDefaultConfiguration(sslConfig);
}

QString ZwiftRequest::json(const QString& url) {
    QNetworkRequest request(QUrl(BASE_URL + url));
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization", "Bearer " + getAccessToken.toUtf8());

    QNetworkReply* reply = manager.get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error: " << reply->errorString();
        return "";
    }

    return reply->readAll();
}

QByteArray ZwiftRequest::protobuf(const QString& url) {
    QNetworkRequest request(QUrl(BASE_URL + url));
    request.setRawHeader("Accept", "application/x-protobuf-lite");
    request.setRawHeader("Authorization", "Bearer " + getAccessToken.toUtf8());

    QNetworkReply* reply = manager.get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error: " << reply->errorString();
        return QByteArray();
    }

    return reply->readAll();
}
