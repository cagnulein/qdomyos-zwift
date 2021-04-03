#include "peloton.h"

const bool log_request = true;

peloton::peloton(bluetooth* bl, QObject *parent) : QObject(parent)
{
    QSettings settings;
    bluetoothManager = bl;
    mgr = new QNetworkAccessManager(this);
    timer = new QTimer(this);

    if(!settings.value("peloton_username", "username").toString().compare("username"))
    {
        qDebug() << "invalid peloton credentials";
        return;
    }

    connect(timer,SIGNAL(timeout()), this, SLOT(startEngine()));

    startEngine();
}

void peloton::startEngine()
{
    QSettings settings;
    timer->stop();
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

    mgr->post(request, data);
}

void peloton::login_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(login_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);

    if(log_request)
        qDebug() << "login_onfinish" << document;
    else
        qDebug() << "login_onfinish";

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
    QString id = data.at(0)["id"].toString();
    QString status = data.at(0)["status"].toString();    
    current_workout_id = id;
    if(status.toUpper().contains("IN_PROGRESS") && !current_workout_status.contains("IN_PROGRESS"))
    {
        // starting a workout
        qDebug() << "workoutlist_onfinish IN PROGRESS!";

        // peloton bike only
        if(bluetoothManager && bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
        {
            getSummary(id);
            timer->start(60000);  // timeout request
        }
        else
        {
            timer->start(10000);  // check for a status changed
        }
    }
    else
    {
        //getSummary(current_workout_id); // debug
        timer->start(10000);  // check for a status changed
    }
    current_workout_status = status;

    if(log_request)
        qDebug() << "workoutlist_onfinish" << current_workout;
    qDebug() << "current workout id" << current_workout_id;
}

void peloton::summary_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(summary_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    current_workout_summary = QJsonDocument::fromJson(payload, &parseError);

    if(log_request)
        qDebug() << "summary_onfinish" << current_workout_summary;
    else
        qDebug() << "summary_onfinish";

    getWorkout(current_workout_id);
}

void peloton::workout_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(workout_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    workout = QJsonDocument::fromJson(payload, &parseError);
    QJsonObject ride = workout.object()["ride"].toObject();
    current_workout_name = ride["title"].toString();

    if(log_request)
        qDebug() << "workout_onfinish" << workout;
    else
        qDebug() << "workout_onfinish";

    getPerformance(current_workout_id);
}

void peloton::performance_onfinish(QNetworkReply* reply)
{
    disconnect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(performance_onfinish(QNetworkReply*)));
    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    performance = QJsonDocument::fromJson(payload, &parseError);

    QJsonObject json = performance.object();
    QJsonObject target_performance_metrics = json["target_performance_metrics"].toObject();
    QJsonArray target_graph_metrics = target_performance_metrics["target_graph_metrics"].toArray();
    QJsonObject resistances = target_graph_metrics[1].toObject();    
    QJsonObject graph_data_resistances = resistances["graph_data"].toObject();
    QJsonArray lower_resistances = graph_data_resistances["lower"].toArray();
    QJsonObject cadences = target_graph_metrics[0].toObject();
    QJsonObject graph_data_cadences = cadences["graph_data"].toObject();
    QJsonArray lower_cadences = graph_data_cadences["lower"].toArray();

    trainrows.clear();
    for(int i=0; i<lower_resistances.count(); i++)
    {
        trainrow r;
        r.duration = QTime(0,0,peloton_workout_second_resolution,0);
        r.resistance = ((bike*)bluetoothManager->device())->pelotonToBikeResistance(lower_resistances.at(i).toInt());
        r.cadence = lower_cadences.at(i).toInt();
        trainrows.append(r);
    }

    if(log_request)
        qDebug() << "performance_onfinish" << workout;
    else
        qDebug() << "performance_onfinish" << trainrows.length();

    if(trainrows.length())
    {
        emit workoutStarted(current_workout_name);
    }

    timer->start(30000); // check for a status changed
}


void peloton::getPerformance(QString workout)
{
    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(performance_onfinish(QNetworkReply*)));

    QUrl url("https://api.onepeloton.com/api/workout/" + workout + "/performance_graph?every_n=" + QString::number(peloton_workout_second_resolution));
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

    int limit = 1; // for now we don't need more than 1 workout
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
