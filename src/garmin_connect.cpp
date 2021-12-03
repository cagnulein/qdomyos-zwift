#include <QAbstractOAuth2>
#include <QApplication>
#include <QByteArray>
#include <QDesktopServices>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QQmlContext>
#include <QQmlFile>

#include <QRandomGenerator>
#include <QSettings>
#include <QStandardPaths>
#include <QTime>
#include <QUrlQuery>
#include <chrono>

using namespace std::chrono_literals;

#include "garmin_connect.h"

garmin_connect::garmin_connect() {
    QSettings settings;
    QNetworkRequest request(QUrl(QStringLiteral("https://sso.garmin.com/sso/signin?service=https://connect.garmin.com/"
                                                "modern&clientId=GarminConnect&gauthHost=https://sso.garmin.com/sso")));
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    /*
    // set params
    QString data;
    data += QStringLiteral("service=https://connect.garmin.com/modern");
    data += QStringLiteral("clientId=GarminConnect");
    data += QStringLiteral("gauthHost=https://sso.garmin.com/sso");
    data += QStringLiteral("consumeServiceTicket=false");
*/

    // make request
    if (manager) {

        delete manager;
        manager = nullptr;
    }
    manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(request);

    // blocking request
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << QStringLiteral("HTTP response code: ") << statusCode;

    // oops, no dice
    if (reply->error() != 0) {
        qDebug() << QStringLiteral("Got error") << reply->errorString().toStdString().c_str();
        return;
    }

    QString data;
    data += QStringLiteral("username=email@gmail.com");
    data += QStringLiteral("&password=password");
    data += QStringLiteral("&_eventId=submit");
    data += QStringLiteral("&embed=true");

    QNetworkRequest request2(
        QUrl(QStringLiteral("https://sso.garmin.com/sso/signin?service=https://connect.garmin.com/"
                            "modern&clientId=GarminConnect&gauthHost=https://sso.garmin.com/sso")));
    request2.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request2.setRawHeader("origin", "https://sso.garmin.com");
    request2.setAttribute(QNetworkRequest::FollowRedirectsAttribute, false);

    QNetworkReply *reply2 = manager->post(request2, data.toLatin1());

    // blocking request
    QEventLoop loop2;
    connect(reply2, &QNetworkReply::finished, &loop2, &QEventLoop::quit);
    loop2.exec();

    statusCode = reply2->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << QStringLiteral("HTTP response code: ") << statusCode;

    // oops, no dice
    if (reply2->error() != 0) {
        qDebug() << QStringLiteral("Got error") << reply2->errorString().toStdString().c_str();
        return;
    }

    QNetworkRequest request3(QUrl(QStringLiteral("https://connect.garmin.com/modern")));
    request3.setAttribute(QNetworkRequest::FollowRedirectsAttribute, false);

    QNetworkReply *reply3 = manager->get(request3);
    // blocking request
    QEventLoop loop3;
    connect(reply3, &QNetworkReply::finished, &loop3, &QEventLoop::quit);
    loop3.exec();

    statusCode = reply3->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << QStringLiteral("HTTP response code: ") << statusCode;

    // oops, no dice
    if (reply3->error() != 0) {
        qDebug() << QStringLiteral("Got error") << reply3->errorString().toStdString().c_str();
        return;
    }

    QString url_prefix = "https://connect.garmin.com";
    int max_redirect_count = 7;
    int current_redirect_count = 1;
    QString url = reply3->rawHeader("location");
    qDebug() << reply3->rawHeader("location");
    while (1) {

        if (url.startsWith("/"))
            url = url_prefix + url;
        url_prefix += url.split("/")[3]; // to be verified
        QNetworkRequest request4(QUrl(url, QUrl::ParsingMode::TolerantMode));
        request4.setAttribute(QNetworkRequest::FollowRedirectsAttribute, false);

        QNetworkReply *reply4 = manager->get(request4);
        // blocking request
        QEventLoop loop4;
        connect(reply4, &QNetworkReply::finished, &loop4, &QEventLoop::quit);
        loop4.exec();

        statusCode = reply4->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << QStringLiteral("HTTP response code: ") << statusCode;

        // oops, no dice
        if (reply4->error() != 0) {
            qDebug() << QStringLiteral("Got error") << reply4->errorString().toStdString().c_str();
            return;
        }
        if (statusCode == 200 || statusCode == 404)
            break;

        current_redirect_count++;
        if (current_redirect_count >= max_redirect_count)
            break;

        url = reply4->rawHeader("location");
        qDebug() << reply4->rawHeader("location");
    }

    QFile *f = new QFile("c:\\a.fit");
    f->open(QFile::ReadOnly);
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"text\""));
    textPart.setBody("a.fit");

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QStringLiteral("application/octet-stream")));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(QStringLiteral("form-data; name=\"data\"; filename=\"") + "a.fit" +
                                QStringLiteral("\"; type=\"application/octet-stream\"")));
    filePart.setBodyDevice(f);
    f->setParent(multiPart);
    multiPart->append(textPart);
    multiPart->append(filePart);

    QNetworkRequest request5(QUrl("https://connect.garmin.com/modern/proxy/upload-service/upload/.fit"));
    request5.setRawHeader("nk", "NT");
    QNetworkReply *reply5 = manager->post(request5, multiPart);
    multiPart->setParent(reply5);

    // blocking request
    QEventLoop loop5;
    connect(reply5, &QNetworkReply::finished, &loop5, &QEventLoop::quit);
    loop5.exec();

    statusCode = reply5->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << QStringLiteral("HTTP response code: ") << statusCode;

    // oops, no dice
    if (reply5->error() != 0) {
        qDebug() << QStringLiteral("Got error") << reply5->errorString().toStdString().c_str();
        return;
    }

    QByteArray r = reply5->readAll();
    qDebug() << QStringLiteral("Got response:") << r.data();

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(r, &parseError);

    // failed to parse result !?
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << tr("JSON parser error") << parseError.errorString();
    }
}
