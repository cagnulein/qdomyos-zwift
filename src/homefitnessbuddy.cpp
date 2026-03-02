#include "homefitnessbuddy.h"
#include "zwiftworkout.h"
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QSettings>
#include <QTextDocument>
#include <QtXml>

using namespace std::chrono_literals;

homefitnessbuddy::homefitnessbuddy(bluetooth *bl, QObject *parent) : QObject(parent) {

    QSettings settings;
    bluetoothManager = bl;
    mgr = new QNetworkAccessManager(this);
    QNetworkCookieJar *cookieJar = new QNetworkCookieJar();
    mgr->setCookieJar(cookieJar);
    retry.setSingleShot(true);
    retry.setInterval(10s);
    connect(&retry, &QTimer::timeout, this, &homefitnessbuddy::startEngine);

    if (!settings.value(QZSettings::peloton_username, QZSettings::default_peloton_username)
             .toString()
             .compare(QStringLiteral("username"))) {
        qDebug() << QStringLiteral("invalid peloton credentials");
        return;
    }

    startEngine();
}

void homefitnessbuddy::startEngine() {
    QSettings settings;
    retry.stop();
    connect(mgr, &QNetworkAccessManager::finished, this, &homefitnessbuddy::login_onfinish);
    QUrl url(QStringLiteral("https://app.homefitnessbuddy.com/peloton/powerzone/"));
    qDebug() << QStringLiteral("Connecting to ") << url.toString();
    QNetworkRequest request(url);

    // request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    mgr->get(request);
}

void homefitnessbuddy::error(QNetworkReply::NetworkError code) {
    qDebug() << QStringLiteral("homefitnessbuddy ERROR") << code;
}

void homefitnessbuddy::login_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &homefitnessbuddy::login_onfinish);
    QByteArray payload = reply->readAll(); // JSON

    qDebug() << QStringLiteral("login_onfinish") << payload;

    if (!payload.length()) {
        qDebug() << QStringLiteral("homefitnessbuddy error, trying again");
        retry.setSingleShot(true);
        retry.start(10s);
        return;
    }

    QTextStream in(payload);
    QStringList fieldNames;
    QJsonObject row;
    QRegularExpression fieldRe(QStringLiteral("<th(?:\\s+[^>]+)?>([^<]*)</th>"),
                               QRegularExpression::CaseInsensitiveOption);
    QRegularExpression valueRe(QStringLiteral("(?:<td(?:\\s+[^>]+)?>([^<]*)</td>|</tr>)"),
                               QRegularExpression::CaseInsensitiveOption);
    qDebug() << fieldRe.isValid() << valueRe.isValid();
    while (!in.atEnd()) {
        QString line = in.readLine();
        // qDebug() << line;
        QRegularExpressionMatch match = fieldRe.match(line);
        if (match.hasMatch()) {
            fieldNames.append(match.captured(1));
        } else if (!fieldNames.isEmpty() && line.indexOf(QStringLiteral("<TR>")) >= 0) {
            QRegularExpressionMatchIterator i = valueRe.globalMatch(line);
            QRegularExpressionMatch vMatch;
            int indexField = -1;
            int nfields = fieldNames.size();
            QString fieldName;
            while (i.hasNext()) {
                vMatch = i.next();
                if (vMatch.lastCapturedIndex() == 1) {
                    indexField++;
                    if (nfields > indexField) {
                        if (!(fieldName = fieldNames.at(indexField)).isEmpty()) {
                            QTextDocument text;
                            text.setHtml(vMatch.captured(1));
                            if (row[fieldName].toString().isEmpty()) { // to avoid Coach duplication
                                row[fieldName] = text.toPlainText();
                            }
                        }
                    } else {
                        qDebug() << QStringLiteral("BUG? ") << nfields << QStringLiteral("<=") << indexField;
                    }
                } else if (indexField == nfields - 1) {
                    indexField = -1;
                    lessons.append(row);
                    row = QJsonObject();
                } else {
                    qDebug() << QStringLiteral("BUG2? ") << nfields << QStringLiteral("<=") << indexField;
                }
            }
            break;
        }
    }
    emit loginState(true);

    // REMOVE IT
    // searchWorkout(QDate(2021,5,19) ,"Christine D'Ercole", 3600, "");
    // searchWorkout(QDate(2020, 1, 18), "Denis & Matt", 3600, ""); //     Multiple Instructors
    // searchWorkout(QDate(2021, 4, 23), "Ben Alldis", 2700, "a5f95a660f5b4a84ac6a86aa4468ea1d");
}

void homefitnessbuddy::searchWorkout(QDate date, const QString &coach, int pedaling_duration, QString class_id) {
    pelotonClassID = class_id;
    int found = 0;
    for (const QJsonValue &r : qAsConst(lessons)) {
        QDate d = QDate::fromString(r.toObject().value(QStringLiteral("Date")).toString(), QStringLiteral("MM/dd/yy"));
        d = d.addYears(100);
        bool c = !coach.compare(r.toObject().value(QStringLiteral("Coach")).toString());
        qDebug()
            << coach.contains('&') << r.toObject().value(QStringLiteral("Coach")).toString()
            << r.toObject().value(QStringLiteral("Coach")).toString().contains(QStringLiteral("Multiple Instructors"))
            << d << date;
        c |= coach.contains('&') &&
             r.toObject().value(QStringLiteral("Coach")).toString().contains(QStringLiteral("Multiple Instructors"));
        if (d == date && c) {
            found++;
        }
    }

    qDebug() << "classes found" << found;

    if (found == 1) {
        for (const QJsonValue &r : qAsConst(lessons)) {
            QDate d =
                QDate::fromString(r.toObject().value(QStringLiteral("Date")).toString(), QStringLiteral("MM/dd/yy"));
            d = d.addYears(100);
            bool c = !coach.compare(r.toObject().value(QStringLiteral("Coach")).toString());
            c |=
                coach.contains('&') &&
                r.toObject().value(QStringLiteral("Coach")).toString().contains(QStringLiteral("Multiple Instructors"));
            if (d == date && c) {
                getClassID(r.toObject().value(QStringLiteral("Class ID")).toString());
                return;
            }
        }
    } else if (found > 1) {
        found = 0;
        QStringList hfbID;
        qDebug() << QStringLiteral("HomeFitnessBuddy found more than one workout with the same date and same "
                                   "Instructor, try to filter this out using the length...");
        for (const QJsonValue &r : qAsConst(lessons)) {
            QDate d =
                QDate::fromString(r.toObject().value(QStringLiteral("Date")).toString(), QStringLiteral("MM/dd/yy"));
            d = d.addYears(100);
            bool c = !coach.compare(r.toObject().value(QStringLiteral("Coach")).toString());
            int len = r.toObject().value(QStringLiteral("Length")).toString().toInt();
            bool duration = (len == (pedaling_duration / 60));
            if (d == date && c && duration) {
                found++;
                hfbID.append(r.toObject().value(QStringLiteral("Class ID")).toString());
            }
        }

        if (found == 1) {
            getClassID(hfbID.first());
        } else {
            for (QString h : hfbID) {
                connect(mgr, &QNetworkAccessManager::finished, this, &homefitnessbuddy::search_detail_onfinish);
                QUrl url(QStringLiteral("https://app.homefitnessbuddy.com/peloton/powerzone/details/") + h);
                qDebug() << "test" << h << url;
                QNetworkRequest request(url);

                // request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
                request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

                mgr->get(request);
            }
        }
    } else {        
        // in case of no results, I'm emitting this anyway in order to show the peloton popup for evaluating the peloton class name on strava
        trainrows.clear();
        emit workoutStarted(&trainrows);
    }
}

void homefitnessbuddy::getClassID(QString id) {
    connect(mgr, &QNetworkAccessManager::finished, this, &homefitnessbuddy::search_workout_onfinish);
    QUrl url(QStringLiteral("https://app.homefitnessbuddy.com/peloton/powerzone/zwift_export.php"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("class_id"), id);
    url.setQuery(query.query());
    QNetworkRequest request(url);

    // request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    mgr->get(request);
}

void homefitnessbuddy::search_workout_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &homefitnessbuddy::search_workout_onfinish);
    QByteArray payload = reply->readAll(); // JSON

    qDebug() << QStringLiteral("search_workout_onfinish") << payload;

    QSettings settings;
    // NOTE: clazy-unused-non-trivial-variable
    // QString difficulty = settings.value(QZSettings::peloton_difficulty,
    // QZSettings::default_peloton_difficulty).toString();

    trainrows.clear();
    trainrows = zwiftworkout::load(payload);

    if (!trainrows.isEmpty()) {
        emit workoutStarted(&trainrows);
    }
}

void homefitnessbuddy::search_detail_onfinish(QNetworkReply *reply) {
    QString payload = reply->readAll(); // JSON

    qDebug() << QStringLiteral("search_detail_onfinish") << payload;

    if (payload.contains(pelotonClassID)) {
        disconnect(mgr, &QNetworkAccessManager::finished, this, &homefitnessbuddy::search_detail_onfinish);
        getClassID(reply->url().toString().split('/').last());
    }
}
