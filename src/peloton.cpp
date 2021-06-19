#include "peloton.h"
#include <chrono>

using namespace std::chrono_literals;

const bool log_request = true;

peloton::peloton(bluetooth *bl, QObject *parent) : QObject(parent) {

    QSettings settings;
    bluetoothManager = bl;
    mgr = new QNetworkAccessManager(this);
    timer = new QTimer(this);

    if (!settings.value(QStringLiteral("peloton_username"), QStringLiteral("username"))
             .toString()
             .compare(QStringLiteral("username"))) {
        qDebug() << QStringLiteral("invalid peloton credentials");
        return;
    }

    connect(timer, &QTimer::timeout, this, &peloton::startEngine);

    PZP = new powerzonepack(bl, this);
    HFB = new homefitnessbuddy(bl, this);

    connect(PZP, &powerzonepack::workoutStarted, this, &peloton::pzp_trainrows);
    connect(PZP, &powerzonepack::loginState, this, &peloton::pzp_loginState);
    connect(HFB, &homefitnessbuddy::workoutStarted, this, &peloton::hfb_trainrows);

    startEngine();
}

void peloton::pzp_loginState(bool ok) { emit pzpLoginState(ok); }

void peloton::hfb_trainrows(QList<trainrow> *list) {
    trainrows.clear();
    for (const trainrow r : qAsConst(*list)) {

        trainrows.append(r);
    }
    if (!trainrows.isEmpty()) {

        emit workoutStarted(current_workout_name, current_instructor_name);
    }
}

void peloton::pzp_trainrows(QList<trainrow> *list) {



    trainrows.clear();
    for (const trainrow &r : qAsConst(*list)) {

        trainrows.append(r);
    }
    if (!trainrows.isEmpty()) {

        emit workoutStarted(current_workout_name, current_instructor_name);
    }
}

void peloton::startEngine() {
    if (peloton_credentials_wrong) {
        return;
    }

    QSettings settings;
    timer->stop();
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::login_onfinish);
    QUrl url(QStringLiteral("https://api.onepeloton.com/auth/login"));
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    QJsonObject obj;
    obj[QStringLiteral("username_or_email")] =
        settings.value(QStringLiteral("peloton_username"), QStringLiteral("username")).toString();
    obj[QStringLiteral("password")] =
        settings.value(QStringLiteral("peloton_password"), QStringLiteral("password")).toString();
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    mgr->post(request, data);
}

void peloton::login_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::login_onfinish);

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    QJsonObject json = document.object();
    int status = json[QStringLiteral("status")].toInt();

    if (log_request) {
        qDebug() << QStringLiteral("login_onfinish") << document;
    } else {
        qDebug() << QStringLiteral("login_onfinish");
    }

    if (status != 0) {


        peloton_credentials_wrong = true;
        qDebug() << QStringLiteral("invalid peloton credentials during login ") << status;
        return;
    }

    user_id = document[QStringLiteral("user_id")].toString();
    total_workout = document[QStringLiteral("user_data")][QStringLiteral("total_workouts")].toInt();

    emit loginState(!user_id.isEmpty());

    getWorkoutList(1);
}

void peloton::workoutlist_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::workoutlist_onfinish);

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    current_workout = QJsonDocument::fromJson(payload, &parseError);
    QJsonObject json = current_workout.object();
    QJsonArray data = json[QStringLiteral("data")].toArray();
    qDebug() << QStringLiteral("data") << data;
    QString id = data.at(0)[QStringLiteral("id")].toString();
    QString status = data.at(0)[QStringLiteral("status")].toString();

    if ((status.contains(QStringLiteral("IN_PROGRESS"),
                         Qt::CaseInsensitive) && // NOTE: removed toUpper because of qstring-insensitive-allocation
         !current_workout_status.contains(QStringLiteral("IN_PROGRESS"))) ||
        (status.contains(QStringLiteral("IN_PROGRESS"), Qt::CaseInsensitive) &&
         id != current_workout_id)) { // NOTE: removed toUpper because of qstring-insensitive-allocation
        current_workout_id = id;

        // starting a workout
        qDebug() << QStringLiteral("workoutlist_onfinish IN PROGRESS!");

        // peloton bike only
        if (bluetoothManager && bluetoothManager->device() &&
            bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
            getSummary(id);
            timer->start(1min); // timeout request
            current_workout_status = status;
        } else {
            timer->start(10s); // check for a status changed
            // i don't need to set current_workout_status because, the bike was missing and than i didn't set the
            // workout
        }
    } else {


        // getSummary(current_workout_id); // debug
        timer->start(10s); // check for a status changed
        current_workout_status = status;
        current_workout_id = id;
    }

    if (log_request) {
        qDebug() << QStringLiteral("workoutlist_onfinish") << current_workout;
    }
    qDebug() << QStringLiteral("current workout id") << current_workout_id;
}

void peloton::summary_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::summary_onfinish);

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    current_workout_summary = QJsonDocument::fromJson(payload, &parseError);

    if (log_request) {
        qDebug() << QStringLiteral("summary_onfinish") << current_workout_summary;
    } else {
        qDebug() << QStringLiteral("summary_onfinish");
    }

    getWorkout(current_workout_id);
}

void peloton::instructor_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::instructor_onfinish);

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    instructor = QJsonDocument::fromJson(payload, &parseError);
    current_instructor_name = instructor.object()[QStringLiteral("name")].toString();

    if (log_request) {
        qDebug() << QStringLiteral("instructor_onfinish") << instructor;
    } else {
        qDebug() << QStringLiteral("instructor_onfinish");
    }

    emit workoutChanged(current_workout_name, current_instructor_name);

    getPerformance(current_workout_id);
}

void peloton::workout_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::workout_onfinish);

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    workout = QJsonDocument::fromJson(payload, &parseError);
    QJsonObject ride = workout.object()[QStringLiteral("ride")].toObject();
    current_workout_name = ride[QStringLiteral("title")].toString();
    current_instructor_id = ride[QStringLiteral("instructor_id")].toString();
    current_ride_id = ride[QStringLiteral("id")].toString();
    qint64 time = ride[QStringLiteral("original_air_time")].toInt();
    qDebug() << QStringLiteral("original_air_time") << time;

    current_original_air_time = QDateTime::fromSecsSinceEpoch(time);

    if (log_request) {
        qDebug() << QStringLiteral("workout_onfinish") << workout;
    } else {
        qDebug() << QStringLiteral("workout_onfinish");
    }

    getInstructor(current_instructor_id);
}

void peloton::performance_onfinish(QNetworkReply *reply) {

    QSettings settings;
    QString difficulty = settings.value(QStringLiteral("peloton_difficulty"), QStringLiteral("lower")).toString();
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::performance_onfinish);
    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    performance = QJsonDocument::fromJson(payload, &parseError);
    current_api = peloton_api;

    QJsonObject json = performance.object();
    QJsonObject target_performance_metrics = json[QStringLiteral("target_performance_metrics")].toObject();
    QJsonArray target_graph_metrics = target_performance_metrics[QStringLiteral("target_graph_metrics")].toArray();
    QJsonObject resistances = target_graph_metrics[1].toObject();
    QJsonObject graph_data_resistances = resistances[QStringLiteral("graph_data")].toObject();
    QJsonArray current_resistances = graph_data_resistances[difficulty].toArray();
    QJsonArray lower_resistances = graph_data_resistances[QStringLiteral("lower")].toArray();
    QJsonArray upper_resistances = graph_data_resistances[QStringLiteral("upper")].toArray();
    QJsonObject cadences = target_graph_metrics[0].toObject();
    QJsonObject graph_data_cadences = cadences[QStringLiteral("graph_data")].toObject();
    QJsonArray current_cadences = graph_data_cadences[difficulty].toArray();
    QJsonArray lower_cadences = graph_data_cadences[QStringLiteral("lower")].toArray();
    QJsonArray upper_cadences = graph_data_cadences[QStringLiteral("upper")].toArray();

    trainrows.clear();
    trainrows.reserve(current_resistances.count() + 1);
    for (int i = 0; i < current_resistances.count(); i++) {
        trainrow r;
        r.duration = QTime(0, 0, peloton_workout_second_resolution, 0);
        r.resistance = ((bike *)bluetoothManager->device())->pelotonToBikeResistance(current_resistances.at(i).toInt());
        r.lower_resistance =
            ((bike *)bluetoothManager->device())->pelotonToBikeResistance(lower_resistances.at(i).toInt());
        r.upper_resistance =
            ((bike *)bluetoothManager->device())->pelotonToBikeResistance(upper_resistances.at(i).toInt());
        r.requested_peloton_resistance = current_resistances.at(i).toInt();
        r.lower_requested_peloton_resistance = lower_resistances.at(i).toInt();
        r.upper_requested_peloton_resistance = upper_resistances.at(i).toInt();
        r.cadence = current_cadences.at(i).toInt();
        r.lower_cadence = lower_cadences.at(i).toInt();
        r.upper_cadence = upper_cadences.at(i).toInt();
        trainrows.append(r);
    }

    if (log_request) {
        qDebug() << QStringLiteral("performance_onfinish") << performance;
    } else {
        qDebug() << QStringLiteral("performance_onfinish") << trainrows.length();
    }

    if (!trainrows.isEmpty()) {


        emit workoutStarted(current_workout_name, current_instructor_name);
    } else {


        if (!PZP->searchWorkout(current_ride_id)) {
            current_api = homefitnessbuddy_api;
            HFB->searchWorkout(current_original_air_time.date(), current_instructor_name);
        } else {
            current_api = powerzonepack_api;
        }
    }

    timer->start(30s); // check for a status changed
}

void peloton::getInstructor(const QString &instructor_id) {
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::instructor_onfinish);


    QUrl url(QStringLiteral("https://api.onepeloton.com/api/instructor/") + instructor_id);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    mgr->get(request);
}

void peloton::getPerformance(const QString &workout) {
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::performance_onfinish);


    QUrl url(QStringLiteral("https://api.onepeloton.com/api/workout/") + workout +
             QStringLiteral("/performance_graph?every_n=") + QString::number(peloton_workout_second_resolution));
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    mgr->get(request);
}

void peloton::getWorkout(const QString &workout) {
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::workout_onfinish);


    QUrl url(QStringLiteral("https://api.onepeloton.com/api/workout/") + workout);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    mgr->get(request);
}

void peloton::getSummary(const QString &workout) {
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::summary_onfinish);


    QUrl url(QStringLiteral("https://api.onepeloton.com/api/workout/") + workout + QStringLiteral("/summary"));
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    mgr->get(request);
}

void peloton::getWorkoutList(int num) {
    Q_UNUSED(num)
    //    if (num == 0) { //NOTE: clang-analyzer-deadcode.DeadStores
    //        num = this->total_workout;
    //    }

    int limit = 1; // for now we don't need more than 1 workout
    // int pages = num / limit; //NOTE: clang-analyzer-deadcode.DeadStores
    // int rem = num % limit; //NOTE: clang-analyzer-deadcode.DeadStores

    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::workoutlist_onfinish);

    int current_page = 0;

    QUrl url(QStringLiteral("https://api.onepeloton.com/api/user/") + user_id +
             QStringLiteral("/workouts?sort_by=-created&page=") + QString::number(current_page) +
             QStringLiteral("&limit=") + QString::number(limit));
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    mgr->get(request);
}