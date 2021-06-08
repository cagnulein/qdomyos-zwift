#include <QSettings>
#include <QtXml>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QTextDocument>
#include "homefitnessbuddy.h"
#include "zwiftworkout.h"

homefitnessbuddy::homefitnessbuddy(bluetooth* bl, QObject *parent) : QObject(parent)
{

    QSettings settings;
    bluetoothManager = bl;
    mgr = new QNetworkAccessManager(this);
    QNetworkCookieJar* cookieJar = new QNetworkCookieJar();
    mgr->setCookieJar(cookieJar);

    startEngine();
}

void homefitnessbuddy::startEngine()
{
    QSettings settings;
    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(login_onfinish(QNetworkReply*)));
    QUrl url("https://app.homefitnessbuddy.com/peloton/powerzone/");
    QNetworkRequest request(url);

    //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");

    mgr->get(request);
}

void homefitnessbuddy::error(QNetworkReply::NetworkError code)
{
    qDebug() << "homefitnessbuddy ERROR" << code;
}

void homefitnessbuddy::login_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(login_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON

    qDebug() << "login_onfinish" << payload;

    QTextStream in(payload);
    QStringList fieldNames;
    QJsonObject row;
    QRegularExpression fieldRe("<th(?:\\s+[^>]+)?>([^<]*)</th>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression valueRe("(?:<td(?:\\s+[^>]+)?>([^<]*)</td>|</tr>)", QRegularExpression::CaseInsensitiveOption);
    qDebug() << fieldRe.isValid() << valueRe.isValid();
    while (!in.atEnd())
    {
       QString line = in.readLine();
       //qDebug() << line;
       QRegularExpressionMatch match = fieldRe.match(line);
       if (match.hasMatch()) {
           fieldNames.append(match.captured(1));
       }
       else if (!fieldNames.isEmpty() && line.indexOf("<TR>")>=0){
           QRegularExpressionMatchIterator i = valueRe.globalMatch(line);
           QRegularExpressionMatch vMatch;
           int indexField = -1;
           int nfields = fieldNames.size();
           QString fieldName;
           while (i.hasNext()) {
               vMatch = i.next();
               if (vMatch.lastCapturedIndex() == 1) {
                   indexField++;
                   if (nfields>indexField) {
                       if (!(fieldName = fieldNames.at(indexField)).isEmpty()) {
                           QTextDocument text;
                           text.setHtml(vMatch.captured(1));
                           if(!row[fieldName].toString().length()) // to avoid Coach duplication
                               row[fieldName] = text.toPlainText();
                       }
                   }
                   else {
                       qDebug() << "BUG? "<<nfields <<"<="<<indexField;
                   }
               }
               else if (indexField == nfields-1) {
                   indexField = -1;
                   lessons.append(row);
                   row = QJsonObject();
               }
               else {
                   qDebug() << "BUG2? "<<nfields <<"<="<<indexField;
               }
           }
           break;
       }
    }
    //if(token.length()) emit loginState(true);

    // REMOVE IT
    //searchWorkout(QDate(2021,5,19) ,"Christine D'Ercole");
}

void homefitnessbuddy::searchWorkout(QDate date, QString coach)
{
    foreach(QJsonValue r, lessons) {
        QDate d = QDate::fromString(r.toObject().value("Date").toString(),"MM/dd/yy");
        d = d.addYears(100);
        bool c = !coach.compare(r.toObject().value("Coach").toString());
        if(d == date && c) {
            connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(search_workout_onfinish(QNetworkReply*)));
            QUrl url("https://app.homefitnessbuddy.com/peloton/powerzone/zwift_export.php");
            QUrlQuery query;
            query.addQueryItem("class_id", r.toObject().value("Class ID").toString());
            url.setQuery(query.query());
            QNetworkRequest request(url);

            //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");

            mgr->get(request);
            return;
        }
    }
}

void homefitnessbuddy::search_workout_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(search_workout_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON

    qDebug() << "search_workout_onfinish" << payload;

    QSettings settings;
    QString difficulty = settings.value("peloton_difficulty", "lower").toString();

    trainrows.clear();
    trainrows = zwiftworkout::load(payload);

    if(trainrows.length())
    {
        emit workoutStarted(&trainrows);
    }

}
