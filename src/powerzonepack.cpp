#include "powerzonepack.h"
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QSettings>
#include <QtXml>

powerzonepack::powerzonepack(bluetooth *bl, QObject *parent) : QObject(parent) {

    QSettings settings;
    bluetoothManager = bl;
    mgr = new QNetworkAccessManager(this);
    QNetworkCookieJar *cookieJar = new QNetworkCookieJar();
    mgr->setCookieJar(cookieJar);

    if (!settings.value(QStringLiteral("pzp_username"), QStringLiteral("username"))
             .toString()
             .compare(QStringLiteral("username"))) {
        qDebug() << QStringLiteral("invalid peloton credentials");
        return;
    }

    startEngine();
}

void powerzonepack::startEngine() {
    if (pzp_credentials_wrong) {
        return;
    }

    QSettings settings;
    connect(mgr, &QNetworkAccessManager::finished, this, &powerzonepack::login_onfinish);
    QUrl url(QStringLiteral("https://pzpack.com/api"));
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    QJsonArray obj;
    obj.append(QStringLiteral("Api_Login"));
    QJsonArray obj2;
    obj2.append(settings.value(QStringLiteral("pzp_username"), QStringLiteral("username")).toString());
    obj2.append(settings.value(QStringLiteral("pzp_password"), QStringLiteral("password")).toString());
    obj.append(obj2);
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    // QNetworkReply *reply = //NOTE: clang-analyzer-deadcode.DeadStores
    mgr->post(request, data); // NOTE: clang-analyzer-deadcode.DeadStores
}

void powerzonepack::error(QNetworkReply::NetworkError code) {
    qDebug() << QStringLiteral("powerzonepack ERROR") << code;
}

void powerzonepack::login_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &powerzonepack::login_onfinish);
    QByteArray payload = reply->readAll(); // JSON

    qDebug() << QStringLiteral("login_onfinish") << payload;

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    QJsonObject json = document.object();
    token = json[QStringLiteral("Right")].toString();

    if (!token.isEmpty()) {
        emit loginState(true);
    }

    // REMOVE IT
    // searchWorkout("d6a54e1ce634437bb172f61eb1588b27");
}

void powerzonepack::searchWorkout(const QString &classid) {
    if (pzp_credentials_wrong) {
        return;
    }

    lastWorkoutID = classid;

    connect(mgr, &QNetworkAccessManager::finished, this, &powerzonepack::search_workout_onfinish);
    QUrl url(QStringLiteral("https://pzpack.com/api"));
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    QJsonArray obj;
    obj.append(QStringLiteral("Api_Authenticated"));
    QJsonArray obj2;
    obj2.append(token);
    QJsonArray obj3;
    obj3.append(QStringLiteral("AuthenticatedApi_GetClassByPeletonId"));
    obj3.append(classid);
    obj2.append(obj3);
    obj.append(obj2);
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    // QNetworkReply *reply = //NOTE: clang-analyzer-deadcode.DeadStores
    mgr->post(request, data); // NOTE: clang-analyzer-deadcode.DeadStores
}

void powerzonepack::search_workout_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &powerzonepack::search_workout_onfinish);
    QByteArray payload = reply->readAll(); // JSON

    qDebug() << QStringLiteral("search_workout_onfinish") << payload;

    QSettings settings;
    // QString difficulty = settings.value(QStringLiteral("peloton_difficulty"), QStringLiteral("lower")).toString();
    // //NOTE: clazy-unused-non-trivial-variable
    QJsonParseError parseError;
    QJsonDocument performance = QJsonDocument::fromJson(payload, &parseError);

    QJsonObject json = performance.object();
    QJsonArray power_graph = json[QStringLiteral("_class_power_graph")].toArray();

    trainrows.clear();
    QTime lastSeconds(0, 0, 0, 0);
    for (int i = 1; i < power_graph.count(); i++) {
        trainrow r;
        int sec = power_graph.at(i).toObject()[QStringLiteral("seconds")].toInt();
        QTime seconds = QTime(sec / 3600, sec / 60, sec % 60, 0);
        r.duration = QTime(0, 0, lastSeconds.msecsTo(seconds) / 1000, 0);
        r.power = power_graph.at(i - 1).toObject()[QStringLiteral("power_ratio")].toDouble() *
                  settings.value(QStringLiteral("ftp"), 200.0).toDouble();
        lastSeconds = seconds;
        trainrows.append(r);
    }

    if (!trainrows.isEmpty()) {
        emit workoutStarted(&trainrows);
    }
}
