#include <QSettings>
#include <QtXml>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include "powerzonepack.h"

powerzonepack::powerzonepack(bluetooth* bl, QObject *parent) : QObject(parent)
{

    QSettings settings;
    bluetoothManager = bl;
    mgr = new QNetworkAccessManager(this);
    QNetworkCookieJar* cookieJar = new QNetworkCookieJar();
    mgr->setCookieJar(cookieJar);

    if(!settings.value("pzp_username", "username").toString().compare("username"))
    {
        qDebug() << "invalid peloton credentials";
        return;
    }

    startEngine();
}

void powerzonepack::startEngine()
{
    if(pzp_credentials_wrong) return;

    QSettings settings;
    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(login_onfinish(QNetworkReply*)));
    QUrl url("https://pzpack.com/api");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");

    QJsonArray obj;
    obj.append("Api_Login");
    QJsonArray obj2;
    obj2.append(settings.value("pzp_username", "username").toString());
    obj2.append(settings.value("pzp_password", "password").toString());
    obj.append(obj2);
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    QNetworkReply* reply = mgr->post(request, data);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &powerzonepack::error);
}

void powerzonepack::error(QNetworkReply::NetworkError code)
{
    qDebug() << "powerzonepack ERROR" << code;
}

void powerzonepack::login_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(login_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON

    qDebug() << "login_onfinish" << payload;

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    QJsonObject json = document.object();
    token = json["Right"].toString();

    if(token.length()) emit loginState(true);

    // REMOVE IT
    //searchWorkout("d6a54e1ce634437bb172f61eb1588b27");
}

void powerzonepack::searchWorkout(QString classid)
{
    if(pzp_credentials_wrong) return;

    lastWorkoutID = classid;

    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(search_workout_onfinish(QNetworkReply*)));
    QUrl url("https://pzpack.com/api");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");

    QJsonArray obj;
    obj.append("Api_Authenticated");
    QJsonArray obj2;
    obj2.append(token);
    QJsonArray obj3;
    obj3.append("AuthenticatedApi_GetClassByPeletonId");
    obj3.append(classid);
    obj2.append(obj3);
    obj.append(obj2);
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    QNetworkReply* reply = mgr->post(request, data);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &powerzonepack::error);

}

void powerzonepack::search_workout_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(search_workout_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON

    qDebug() << "search_workout_onfinish" << payload;

    QSettings settings;
    QString difficulty = settings.value("peloton_difficulty", "lower").toString();
    QJsonParseError parseError;
    QJsonDocument performance = QJsonDocument::fromJson(payload, &parseError);

    QJsonObject json = performance.object();
    QJsonArray power_graph = json["_class_power_graph"].toArray();

    trainrows.clear();
    QTime lastSeconds(0,0,0,0);
    for(int i=1; i<power_graph.count(); i++)
    {
        trainrow r;
        int sec = power_graph.at(i).toObject()["seconds"].toInt();
        QTime seconds = QTime(sec / 3600, sec / 60, sec % 60, 0);
        r.duration = QTime(0,0,lastSeconds.msecsTo(seconds) / 1000,0);
        r.power = power_graph.at(i - 1).toObject()["power_ratio"].toDouble() * settings.value("ftp", 200.0).toDouble();
        lastSeconds = seconds;
        trainrows.append(r);
    }

    if(trainrows.length())
    {
        emit workoutStarted(&trainrows);
    }

}
