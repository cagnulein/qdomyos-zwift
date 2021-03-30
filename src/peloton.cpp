#include "peloton.h"

peloton::peloton(QObject *parent) : QObject(parent)
{
    QSettings settings;
    mgr = new QNetworkAccessManager(this);
    //connect(mgr,SIGNAL(finished(QNetworkReply*)),mgr,SLOT(deleteLater()));
    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(login_onfinish(QNetworkReply*)));

    QUrl url("https://api.onepeloton.com/auth/login");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");

    QJsonObject obj;
    obj["username_or_email"] = settings.value("peloton_username", "username").toString();
    obj["password"] = settings.value("peloton_password", "password").toString();
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    //mgr->get(QNetworkRequest(QUrl("https://www.qt.io/developers")));
    mgr->post(request, data);
}

void peloton::login_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(login_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);

    qDebug() << "login_onfinish" << document;

    user_id = document["user_id"].toString();
    total_workout = document["user_data"]["total_workouts"].toInt();

    getWorkoutList(1);
}

void peloton::workoutlist_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(workoutlist_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    current_workout = QJsonDocument::fromJson(payload, &parseError);
    QJsonObject json = current_workout.object();
    QJsonArray data = json["data"].toArray();
    qDebug() << "data" << data;
    current_workout_id = data.at(0)["id"].toString();

    qDebug() << "workoutlist_onfinish" << current_workout;
    qDebug() << "current workout id" << current_workout_id;

    getSummary(current_workout_id);
}

void peloton::summary_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(summary_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    current_workout_summary = QJsonDocument::fromJson(payload, &parseError);

    qDebug() << "summary_onfinish" << current_workout_summary;

    getWorkout(current_workout_id);
}

void peloton::workout_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(workout_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    workout = QJsonDocument::fromJson(payload, &parseError);

    qDebug() << "workout_onfinish" << workout;

    getPerformance(current_workout_id);
}

void peloton::performance_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(performance_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    workout = QJsonDocument::fromJson(payload, &parseError);

    qDebug() << "performance_onfinish" << workout;
}


void peloton::getPerformance(QString workout)
{
    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(performance_onfinish(QNetworkReply*)));

    QUrl url("https://api.onepeloton.com/api/workout/" + workout + "/performance_graph");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");

    mgr->get(request);
}

void peloton::getWorkout(QString workout)
{
    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(workout_onfinish(QNetworkReply*)));

    QUrl url("https://api.onepeloton.com/api/workout/" + workout);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");

    mgr->get(request);
}

void peloton::getSummary(QString workout)
{
    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(summary_onfinish(QNetworkReply*)));

    QUrl url("https://api.onepeloton.com/api/workout/" + workout + "/summary");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");

    mgr->get(request);
}

void peloton::getWorkoutList(int num)
{
    if(num == 0)
        num = this->total_workout;

    int limit = 100;
    int pages = num / limit;
    int rem = num % limit;

    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(workoutlist_onfinish(QNetworkReply*)));

    int current_page = 0;

    QUrl url("https://api.onepeloton.com/api/user/" + user_id + "/workouts?sort_by=-created&page=" + QString::number(current_page) + "&limit=" + QString::number(limit));
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "qdomyos-zwift");

    mgr->get(request);
}
