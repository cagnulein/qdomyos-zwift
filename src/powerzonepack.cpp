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
    QUrl url("https://powerzonepack.com/login");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");

    QNetworkReply* reply = mgr->get(request);
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

    QString token;
    QXmlStreamReader stream(payload);
    while(!stream.atEnd())
    {
        stream.readNext();
        QXmlStreamAttributes atts = stream.attributes();
        if(atts.length())
        {
            if(atts.hasAttribute("name") && !atts.value("name").toString().compare("csrf-token"))
            {
                token = atts.value("content").toString();
            }
        }
    }

    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(login_credentials_onfinish(QNetworkReply*)));
    QUrl url("https://powerzonepack.com/login");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");

    QSettings settings;
    QUrlQuery params;
    params.addQueryItem("email", settings.value("pzp_username", "username").toString());
    params.addQueryItem("password", settings.value("pzp_password", "password").toString());
    params.addQueryItem("_token", token);

    mgr->post(request, params.query().toUtf8());
}

void powerzonepack::login_credentials_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(login_credentials_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON
    foreach(QString r, reply->rawHeaderList())
    {
        qDebug() << r << reply->rawHeader(r.toLocal8Bit());
        if(r.toUpper().contains("SET-COOKIE"))
        {
            QString cookies = reply->rawHeader(r.toLocal8Bit());
            const QString tag = "power_zone_pack_session";
            const QString value = cookies.mid(cookies.indexOf(tag) + tag.length() + 1, cookies.indexOf(";", cookies.indexOf(tag)) - (cookies.indexOf(tag) + tag.length() + 1)).toLocal8Bit();
            QNetworkCookie session(tag.toLocal8Bit(), value.toLocal8Bit());
            bool success = mgr->cookieJar()->insertCookie(session);
            qDebug() << success;
        }
    }

    qDebug() << "login_credentials_onfinish" << payload;

    QXmlStreamReader stream(payload);
    while(!stream.atEnd())
    {
        stream.readNext();
        QXmlStreamAttributes atts = stream.attributes();
        if(atts.length())
        {
            if(atts.hasAttribute("href") && atts.value("href").contains("members"))
            {
                pzp_credentials_wrong = false;
                connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(workout_onfinish(QNetworkReply*)));
                QUrl url("https://powerzonepack.com/members/workouts");
                QNetworkRequest request(url);
                request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");
                mgr->get(request);
                //x_xsrf_token = mgr->cookieJar()->cookiesForUrl(QUrl("https://powerzonepack.com"));                
                return;
            }
        }
    }
    pzp_credentials_wrong = true;
}

void powerzonepack::workout_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(workout_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON
    qDebug() << "workout_onfinish" << payload;
    emit loginState(true);

    // REMOVE IT
    //searchWorkout("d6a54e1ce634437bb172f61eb1588b27");
}

void powerzonepack::searchWorkout(QString classid)
{
    if(pzp_credentials_wrong) return;

    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(search_workout_onfinish(QNetworkReply*)));
    QUrl url("https://powerzonepack.com/api/metrics/ridegraph/" + classid);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");
    request.setRawHeader(QByteArray("Accept"), "application/json");
    foreach(QNetworkCookie c, mgr->cookieJar()->cookiesForUrl(QUrl("https://powerzonepack.com")))
    {
        qDebug() << c;
        if(!c.name().compare("XSRF-TOKEN"))
        {
            x_xsrf_token = c.value().left(328);
//            break;
        }
    }
    request.setRawHeader("x-xsrf-token", x_xsrf_token.toLocal8Bit());
    qDebug() << request.rawHeader("x-xsrf-token");

    mgr->get(request);
}

void powerzonepack::search_workout_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(login_credentials_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON

    qDebug() << "search_workout_onfinish" << payload;

    QSettings settings;
    QString difficulty = settings.value("peloton_difficulty", "lower").toString();
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(performance_onfinish(QNetworkReply*)));
    QJsonParseError parseError;
    QJsonDocument performance = QJsonDocument::fromJson(payload, &parseError);

    QJsonObject json = performance.object();
    QJsonArray time = json["xLabels"].toArray();
    QJsonArray watt = json["yValues"].toArray();

    trainrows.clear();
    QTime lastSeconds(0,0,0,0);
    for(int i=1; i<time.count(); i++)
    {
        trainrow r;
        QTime seconds;
        seconds = QTime::fromString(time.at(i).toString(),"mm:ss");
        r.duration = QTime(0,0,lastSeconds.msecsTo(seconds) / 1000,0);
        r.power = watt.at(i - 1).toInt();
        lastSeconds = seconds;
        trainrows.append(r);
    }

    if(trainrows.length())
    {
        emit workoutStarted(&trainrows);
    }

}
