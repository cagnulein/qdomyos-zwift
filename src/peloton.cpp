#include "peloton.h"
#include <chrono>

using namespace std::chrono_literals;

const bool log_request = true;

peloton::peloton(bluetooth *bl, QObject *parent) : QObject(parent) {

    QSettings settings;
    bluetoothManager = bl;
    mgr = new QNetworkAccessManager(this);
    timer = new QTimer(this);

    // only for test purpose
    /*
    current_image_downloaded =
        new fileDownloader(QUrl("https://s3.amazonaws.com/peloton-ride-images/fa50b87ea5c44ce078e28a3030b8865b5dbffb35/"
                                "img_1646099287_a620f71b3d6740718457b21769a7ed46.png"));
    */

    if (!settings.value(QZSettings::peloton_username, QZSettings::default_peloton_username)
             .toString()
             .compare(QStringLiteral("username"))) {
        qDebug() << QStringLiteral("invalid peloton credentials");
        return;
    }

    rower_pace_offset = 1;

    rower_pace[0].value = -1;
    rower_pace[0].display_name = QStringLiteral("Recovery");
    rower_pace[0].levels[0].fast_pace = 4.31;
    rower_pace[0].levels[0].slow_pace = 15;
    rower_pace[0].levels[0].display_name = QStringLiteral("Level 1");
    rower_pace[0].levels[0].slug = QStringLiteral("level_1");        
    rower_pace[0].levels[1].fast_pace = 3.58;
    rower_pace[0].levels[1].slow_pace = 15;
    rower_pace[0].levels[1].display_name = QStringLiteral("Level 2");
    rower_pace[0].levels[1].slug = QStringLiteral("level_2");    
    rower_pace[0].levels[2].fast_pace = 3.34;
    rower_pace[0].levels[2].slow_pace = 15;
    rower_pace[0].levels[2].display_name = QStringLiteral("Level 3");
    rower_pace[0].levels[2].slug = QStringLiteral("level_3");        
    rower_pace[0].levels[3].fast_pace = 3.17;
    rower_pace[0].levels[3].slow_pace = 15;
    rower_pace[0].levels[3].display_name = QStringLiteral("Level 4");
    rower_pace[0].levels[3].slug = QStringLiteral("level_4");        
    rower_pace[0].levels[4].fast_pace = 3.03;
    rower_pace[0].levels[4].slow_pace = 15;
    rower_pace[0].levels[4].display_name = QStringLiteral("Level 5");
    rower_pace[0].levels[4].slug = QStringLiteral("level_5");        
    rower_pace[0].levels[5].fast_pace = 2.5;
    rower_pace[0].levels[5].slow_pace = 15;
    rower_pace[0].levels[5].display_name = QStringLiteral("Level 6");
    rower_pace[0].levels[5].slug = QStringLiteral("level_6");        
    rower_pace[0].levels[6].fast_pace = 2.38;
    rower_pace[0].levels[6].slow_pace = 15;
    rower_pace[0].levels[6].display_name = QStringLiteral("Level 7");
    rower_pace[0].levels[6].slug = QStringLiteral("level_7");        
    rower_pace[0].levels[7].fast_pace = 2.28;
    rower_pace[0].levels[7].slow_pace = 15;
    rower_pace[0].levels[7].display_name = QStringLiteral("Level 8");
    rower_pace[0].levels[7].slug = QStringLiteral("level_8");        
    rower_pace[0].levels[8].fast_pace = 2.17;
    rower_pace[0].levels[8].slow_pace = 15;
    rower_pace[0].levels[8].display_name = QStringLiteral("Level 9");
    rower_pace[0].levels[8].slug = QStringLiteral("level_9");        
    rower_pace[0].levels[9].fast_pace = 2.07;
    rower_pace[0].levels[9].slow_pace = 15;
    rower_pace[0].levels[9].display_name = QStringLiteral("Level 10");
    rower_pace[0].levels[9].slug = QStringLiteral("level_10");

    rower_pace[1].value = 0;
    rower_pace[1].display_name = QStringLiteral("Easy");
    rower_pace[1].levels[0].fast_pace = 3.51;
    rower_pace[1].levels[0].slow_pace = 4.31;
    rower_pace[1].levels[0].display_name = QStringLiteral("Level 1");
    rower_pace[1].levels[0].slug = QStringLiteral("level_1");        
    rower_pace[1].levels[1].fast_pace = 3.22;
    rower_pace[1].levels[1].slow_pace = 3.58;
    rower_pace[1].levels[1].display_name = QStringLiteral("Level 2");
    rower_pace[1].levels[1].slug = QStringLiteral("level_2");    
    rower_pace[1].levels[2].fast_pace = 3.02;
    rower_pace[1].levels[2].slow_pace = 3.34;
    rower_pace[1].levels[2].display_name = QStringLiteral("Level 3");
    rower_pace[1].levels[2].slug = QStringLiteral("level_3");        
    rower_pace[1].levels[3].fast_pace = 2.47;
    rower_pace[1].levels[3].slow_pace = 3.17;
    rower_pace[1].levels[3].display_name = QStringLiteral("Level 4");
    rower_pace[1].levels[3].slug = QStringLiteral("level_4");        
    rower_pace[1].levels[4].fast_pace = 2.36;
    rower_pace[1].levels[4].slow_pace = 3.03;
    rower_pace[1].levels[4].display_name = QStringLiteral("Level 5");
    rower_pace[1].levels[4].slug = QStringLiteral("level_5");        
    rower_pace[1].levels[5].fast_pace = 2.24;
    rower_pace[1].levels[5].slow_pace = 2.5;
    rower_pace[1].levels[5].display_name = QStringLiteral("Level 6");
    rower_pace[1].levels[5].slug = QStringLiteral("level_6");        
    rower_pace[1].levels[6].fast_pace = 2.14;
    rower_pace[1].levels[6].slow_pace = 2.38;
    rower_pace[1].levels[6].display_name = QStringLiteral("Level 7");
    rower_pace[1].levels[6].slug = QStringLiteral("level_7");        
    rower_pace[1].levels[7].fast_pace = 2.06;
    rower_pace[1].levels[7].slow_pace = 2.28;
    rower_pace[1].levels[7].display_name = QStringLiteral("Level 8");
    rower_pace[1].levels[7].slug = QStringLiteral("level_8");        
    rower_pace[1].levels[8].fast_pace = 1.56;
    rower_pace[1].levels[8].slow_pace = 2.17;
    rower_pace[1].levels[8].display_name = QStringLiteral("Level 9");
    rower_pace[1].levels[8].slug = QStringLiteral("level_9");        
    rower_pace[1].levels[9].fast_pace = 1.48;
    rower_pace[1].levels[9].slow_pace = 2.07;
    rower_pace[1].levels[9].display_name = QStringLiteral("Level 10");
    rower_pace[1].levels[9].slug = QStringLiteral("level_10");

    rower_pace[2].value = 1;
    rower_pace[2].display_name = QStringLiteral("Moderate");
    rower_pace[2].levels[0].fast_pace = 3.35;
    rower_pace[2].levels[0].slow_pace = 3.51;
    rower_pace[2].levels[0].display_name = QStringLiteral("Level 1");
    rower_pace[2].levels[0].slug = QStringLiteral("level_1");        
    rower_pace[2].levels[1].fast_pace = 3.09;
    rower_pace[2].levels[1].slow_pace = 3.22;
    rower_pace[2].levels[1].display_name = QStringLiteral("Level 2");
    rower_pace[2].levels[1].slug = QStringLiteral("level_2");    
    rower_pace[2].levels[2].fast_pace = 2.5;
    rower_pace[2].levels[2].slow_pace = 3.02;
    rower_pace[2].levels[2].display_name = QStringLiteral("Level 3");
    rower_pace[2].levels[2].slug = QStringLiteral("level_3");        
    rower_pace[2].levels[3].fast_pace = 2.36;
    rower_pace[2].levels[3].slow_pace = 2.47;
    rower_pace[2].levels[3].display_name = QStringLiteral("Level 4");
    rower_pace[2].levels[3].slug = QStringLiteral("level_4");        
    rower_pace[2].levels[4].fast_pace = 2.25;
    rower_pace[2].levels[4].slow_pace = 2.36;
    rower_pace[2].levels[4].display_name = QStringLiteral("Level 5");
    rower_pace[2].levels[4].slug = QStringLiteral("level_5");        
    rower_pace[2].levels[5].fast_pace = 2.15;
    rower_pace[2].levels[5].slow_pace = 2.24;
    rower_pace[2].levels[5].display_name = QStringLiteral("Level 6");
    rower_pace[2].levels[5].slug = QStringLiteral("level_6");        
    rower_pace[2].levels[6].fast_pace = 2.05;
    rower_pace[2].levels[6].slow_pace = 2.14;
    rower_pace[2].levels[6].display_name = QStringLiteral("Level 7");
    rower_pace[2].levels[6].slug = QStringLiteral("level_7");        
    rower_pace[2].levels[7].fast_pace = 1.57;
    rower_pace[2].levels[7].slow_pace = 2.06;
    rower_pace[2].levels[7].display_name = QStringLiteral("Level 8");
    rower_pace[2].levels[7].slug = QStringLiteral("level_8");        
    rower_pace[2].levels[8].fast_pace = 1.49;
    rower_pace[2].levels[8].slow_pace = 1.57;
    rower_pace[2].levels[8].display_name = QStringLiteral("Level 9");
    rower_pace[2].levels[8].slug = QStringLiteral("level_9");        
    rower_pace[2].levels[9].fast_pace = 1.41;
    rower_pace[2].levels[9].slow_pace = 1.48;
    rower_pace[2].levels[9].display_name = QStringLiteral("Level 10");
    rower_pace[2].levels[9].slug = QStringLiteral("level_10");

    rower_pace[3].value = 2;
    rower_pace[3].display_name = QStringLiteral("Challenging");
    rower_pace[3].levels[0].fast_pace = 3.17;
    rower_pace[3].levels[0].slow_pace = 3.35;
    rower_pace[3].levels[0].display_name = QStringLiteral("Level 1");
    rower_pace[3].levels[0].slug = QStringLiteral("level_1");        
    rower_pace[3].levels[1].fast_pace = 2.52;
    rower_pace[3].levels[1].slow_pace = 3.09;
    rower_pace[3].levels[1].display_name = QStringLiteral("Level 2");
    rower_pace[3].levels[1].slug = QStringLiteral("level_2");    
    rower_pace[3].levels[2].fast_pace = 2.35;
    rower_pace[3].levels[2].slow_pace = 2.5;
    rower_pace[3].levels[2].display_name = QStringLiteral("Level 3");
    rower_pace[3].levels[2].slug = QStringLiteral("level_3");        
    rower_pace[3].levels[3].fast_pace = 2.23;
    rower_pace[3].levels[3].slow_pace = 2.36;
    rower_pace[3].levels[3].display_name = QStringLiteral("Level 4");
    rower_pace[3].levels[3].slug = QStringLiteral("level_4");        
    rower_pace[3].levels[4].fast_pace = 2.13;
    rower_pace[3].levels[4].slow_pace = 2.25;
    rower_pace[3].levels[4].display_name = QStringLiteral("Level 5");
    rower_pace[3].levels[4].slug = QStringLiteral("level_5");        
    rower_pace[3].levels[5].fast_pace = 2.03;
    rower_pace[3].levels[5].slow_pace = 2.15;
    rower_pace[3].levels[5].display_name = QStringLiteral("Level 6");
    rower_pace[3].levels[5].slug = QStringLiteral("level_6");        
    rower_pace[3].levels[6].fast_pace = 1.54;
    rower_pace[3].levels[6].slow_pace = 2.05;
    rower_pace[3].levels[6].display_name = QStringLiteral("Level 7");
    rower_pace[3].levels[6].slug = QStringLiteral("level_7");        
    rower_pace[3].levels[7].fast_pace = 1.47;
    rower_pace[3].levels[7].slow_pace = 1.57;
    rower_pace[3].levels[7].display_name = QStringLiteral("Level 8");
    rower_pace[3].levels[7].slug = QStringLiteral("level_8");        
    rower_pace[3].levels[8].fast_pace = 1.4;
    rower_pace[3].levels[8].slow_pace = 1.49;
    rower_pace[3].levels[8].display_name = QStringLiteral("Level 9");
    rower_pace[3].levels[8].slug = QStringLiteral("level_9");        
    rower_pace[3].levels[9].fast_pace = 1.32;
    rower_pace[3].levels[9].slow_pace = 1.41;
    rower_pace[3].levels[9].display_name = QStringLiteral("Level 10");
    rower_pace[3].levels[9].slug = QStringLiteral("level_10");

    rower_pace[4].value = 3;
    rower_pace[4].display_name = QStringLiteral("Max");
    rower_pace[4].levels[0].fast_pace = 3.06;
    rower_pace[4].levels[0].slow_pace = 3.17;
    rower_pace[4].levels[0].display_name = QStringLiteral("Level 1");
    rower_pace[4].levels[0].slug = QStringLiteral("level_1");        
    rower_pace[4].levels[1].fast_pace = 2.42;
    rower_pace[4].levels[1].slow_pace = 2.52;
    rower_pace[4].levels[1].display_name = QStringLiteral("Level 2");
    rower_pace[4].levels[1].slug = QStringLiteral("level_2");    
    rower_pace[4].levels[2].fast_pace = 2.26;
    rower_pace[4].levels[2].slow_pace = 2.35;
    rower_pace[4].levels[2].display_name = QStringLiteral("Level 3");
    rower_pace[4].levels[2].slug = QStringLiteral("level_3");        
    rower_pace[4].levels[3].fast_pace = 2.15;
    rower_pace[4].levels[3].slow_pace = 2.23;
    rower_pace[4].levels[3].display_name = QStringLiteral("Level 4");
    rower_pace[4].levels[3].slug = QStringLiteral("level_4");        
    rower_pace[4].levels[4].fast_pace = 2.05;
    rower_pace[4].levels[4].slow_pace = 2.13;
    rower_pace[4].levels[4].display_name = QStringLiteral("Level 5");
    rower_pace[4].levels[4].slug = QStringLiteral("level_5");        
    rower_pace[4].levels[5].fast_pace = 1.56;
    rower_pace[4].levels[5].slow_pace = 2.03;
    rower_pace[4].levels[5].display_name = QStringLiteral("Level 6");
    rower_pace[4].levels[5].slug = QStringLiteral("level_6");        
    rower_pace[4].levels[6].fast_pace = 1.48;
    rower_pace[4].levels[6].slow_pace = 1.54;
    rower_pace[4].levels[6].display_name = QStringLiteral("Level 7");
    rower_pace[4].levels[6].slug = QStringLiteral("level_7");        
    rower_pace[4].levels[7].fast_pace = 1.41;
    rower_pace[4].levels[7].slow_pace = 1.47;
    rower_pace[4].levels[7].display_name = QStringLiteral("Level 8");
    rower_pace[4].levels[7].slug = QStringLiteral("level_8");        
    rower_pace[4].levels[8].fast_pace = 1.34;
    rower_pace[4].levels[8].slow_pace = 1.4;
    rower_pace[4].levels[8].display_name = QStringLiteral("Level 9");
    rower_pace[4].levels[8].slug = QStringLiteral("level_9");        
    rower_pace[4].levels[9].fast_pace = 1.27;
    rower_pace[4].levels[9].slow_pace = 1.32;
    rower_pace[4].levels[9].display_name = QStringLiteral("Level 10");
    rower_pace[4].levels[9].slug = QStringLiteral("level_10");

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
        settings.value(QZSettings::peloton_username, QZSettings::default_peloton_username).toString();
    obj[QStringLiteral("password")] =
        settings.value(QZSettings::peloton_password, QZSettings::default_peloton_password).toString();
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
        emit loginState(false);
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
    qDebug() << QStringLiteral("peloton::workoutlist_onfinish data") << data;

    if (data.isEmpty()) {
        qDebug() << QStringLiteral(
            "peloton::workoutlist_onfinish Peloton API doesn't answer, trying back in 10 seconds...");
        timer->start(10s);
        return;
    }

    QString id = data.at(0)[QStringLiteral("id")].toString();
    QString status = data.at(0)[QStringLiteral("status")].toString();

    if ((status.contains(QStringLiteral("IN_PROGRESS"),
                         Qt::CaseInsensitive) && // NOTE: removed toUpper because of qstring-insensitive-allocation
         !current_workout_status.contains(QStringLiteral("IN_PROGRESS"))) ||
        (status.contains(QStringLiteral("IN_PROGRESS"), Qt::CaseInsensitive) && id != current_workout_id) ||
        testMode) { // NOTE: removed toUpper because of qstring-insensitive-allocation

        if (testMode)
            id = "eaa6f381891443b995f68f89f9a178be";
        current_workout_id = id;

        // starting a workout
        qDebug() << QStringLiteral("peloton::workoutlist_onfinish workoutlist_onfinish IN PROGRESS!");

        if ((bluetoothManager && bluetoothManager->device()) || testMode) {
            getSummary(id);
            timer->start(1min); // timeout request
            current_workout_status = status;
        } else {
            timer->start(10s); // check for a status changed
            // I don't need to set current_workout_status because the bike was missing and then I didn't set the
            // workout
        }
    } else {

        // getSummary(current_workout_id); // debug
        timer->start(10s); // check for a status changed
        current_workout_status = status;
        current_workout_id = id;
    }

    if (log_request) {
        qDebug() << QStringLiteral("peloton::workoutlist_onfinish") << current_workout;
    }
    qDebug() << QStringLiteral("peloton::workoutlist_onfinish current workout id") << current_workout_id;
}

void peloton::summary_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::summary_onfinish);

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    current_workout_summary = QJsonDocument::fromJson(payload, &parseError);

    if (log_request) {
        qDebug() << QStringLiteral("peloton::summary_onfinish") << current_workout_summary;
    } else {
        qDebug() << QStringLiteral("peloton::summary_onfinish");
    }

    getWorkout(current_workout_id);
}

void peloton::instructor_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::instructor_onfinish);

    QSettings settings;
    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    instructor = QJsonDocument::fromJson(payload, &parseError);
    current_instructor_name = instructor.object()[QStringLiteral("name")].toString();

    if (log_request) {
        qDebug() << QStringLiteral("instructor_onfinish") << instructor;
    } else {
        qDebug() << QStringLiteral("instructor_onfinish");
    }

    QString air_time = current_original_air_time.toString(QStringLiteral("MM/dd/yy"));
    qDebug() << QStringLiteral("air_time ") + air_time;
    QString workout_name = current_workout_name;
    if (settings.value(QZSettings::peloton_date, QZSettings::default_peloton_date)
            .toString()
            .contains(QStringLiteral("Before"))) {
        workout_name = air_time + QStringLiteral(" ") + workout_name;
    } else if (settings.value(QZSettings::peloton_date, QZSettings::default_peloton_date)
                   .toString()
                   .contains(QStringLiteral("After"))) {
        workout_name = workout_name + QStringLiteral(" ") + air_time;
    }
    emit workoutChanged(workout_name, current_instructor_name);

    /*
    if (workout_name.toUpper().contains(QStringLiteral("POWER ZONE"))) {
        qDebug() << QStringLiteral("!!Peloton Power Zone Ride Override!!");
        getPerformance(current_workout_id);
    } else*/
    { getRide(current_ride_id); }
}

void peloton::downloadImage() {
    if (current_image_downloaded) {
        delete current_image_downloaded;
        current_image_downloaded = 0;
    }
    if (!current_image_url.isEmpty()) {
        current_image_downloaded = new fileDownloader(current_image_url);
    }
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
    current_workout_type = ride[QStringLiteral("fitness_discipline")].toString();
    current_pedaling_duration = ride[QStringLiteral("pedaling_duration")].toInt();
    current_image_url = ride[QStringLiteral("image_url")].toString();

    qint64 time = ride[QStringLiteral("original_air_time")].toInt();
    qDebug() << QStringLiteral("original_air_time") << time;
    qDebug() << QStringLiteral("current_pedaling_duration") << current_pedaling_duration;

    current_original_air_time = QDateTime::fromSecsSinceEpoch(time, Qt::UTC);

    if (log_request) {
        qDebug() << QStringLiteral("peloton::workout_onfinish") << workout;
    } else {
        qDebug() << QStringLiteral("peloton::workout_onfinish");
    }

    getInstructor(current_instructor_id);
}

void peloton::ride_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::ride_onfinish);

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    QJsonObject ride = document.object();

    // ride.pedaling_start_offset and instructor_cues[0].offset.start is
    // generally 60s for the intro, but let's ignore this since we assume
    // people are starting the workout after the intro
    QJsonArray instructor_cues = ride[QStringLiteral("instructor_cues")].toArray();

    trainrows.clear();
    if (instructor_cues.count() > 0)
        trainrows.reserve(instructor_cues.count() + 1);

    QSettings settings;
    QString difficulty =
        settings.value(QZSettings::peloton_difficulty, QZSettings::default_peloton_difficulty).toString();
    QJsonObject segments = ride[QStringLiteral("segments")].toObject();
    QJsonArray segments_segment_list = segments[QStringLiteral("segment_list")].toArray();

    for (int i = 0; i < instructor_cues.count(); i++) {
        QJsonObject instructor_cue = instructor_cues.at(i).toObject();
        QJsonObject offsets = instructor_cue[QStringLiteral("offsets")].toObject();
        QJsonObject resistance_range = instructor_cue[QStringLiteral("resistance_range")].toObject();
        QJsonObject cadence_range = instructor_cue[QStringLiteral("cadence_range")].toObject();

        if (resistance_range.count() == 0 && cadence_range.count() == 0) {
            qDebug() << "no resistance and cadence found!";
            continue;
        }

        trainrow r;
        int duration = offsets[QStringLiteral("end")].toInt() - offsets[QStringLiteral("start")].toInt();
        if (i != 0) {
            // offsets have a 1s gap
            duration++;
        }

        r.lower_requested_peloton_resistance = resistance_range[QStringLiteral("lower")].toInt();
        r.upper_requested_peloton_resistance = resistance_range[QStringLiteral("upper")].toInt();

        r.lower_cadence = cadence_range[QStringLiteral("lower")].toInt();
        r.upper_cadence = cadence_range[QStringLiteral("upper")].toInt();

        r.average_requested_peloton_resistance =
            (r.lower_requested_peloton_resistance + r.upper_requested_peloton_resistance) / 2;
        r.average_cadence = (r.lower_cadence + r.upper_cadence) / 2;

        if (bluetoothManager && bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                r.lower_resistance = ((bike *)bluetoothManager->device())
                                         ->pelotonToBikeResistance(resistance_range[QStringLiteral("lower")].toInt());
                r.upper_resistance = ((bike *)bluetoothManager->device())
                                         ->pelotonToBikeResistance(resistance_range[QStringLiteral("upper")].toInt());
                r.average_resistance = ((bike *)bluetoothManager->device())
                                           ->pelotonToBikeResistance(r.average_requested_peloton_resistance);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                r.lower_resistance =
                    ((elliptical *)bluetoothManager->device())
                        ->pelotonToEllipticalResistance(resistance_range[QStringLiteral("lower")].toInt());
                r.upper_resistance =
                    ((elliptical *)bluetoothManager->device())
                        ->pelotonToEllipticalResistance(resistance_range[QStringLiteral("upper")].toInt());
                r.average_resistance = ((elliptical *)bluetoothManager->device())
                                           ->pelotonToEllipticalResistance(r.average_requested_peloton_resistance);
            }
        }

        // Set for compatibility
        if (difficulty == QStringLiteral("average")) {
            r.resistance = r.average_resistance;
            r.requested_peloton_resistance = r.average_requested_peloton_resistance;
            r.cadence = r.average_cadence;
        } else if (difficulty == QStringLiteral("upper")) {
            r.resistance = r.upper_resistance;
            r.requested_peloton_resistance = r.upper_requested_peloton_resistance;
            r.cadence = r.upper_cadence;
        } else { // lower
            r.resistance = r.lower_resistance;
            r.requested_peloton_resistance = r.lower_requested_peloton_resistance;
            r.cadence = r.lower_cadence;
        }

        // in order to have compact rows in the training program to have an Remaining Time tile set correctly
        if (i == 0 ||
            (r.lower_requested_peloton_resistance != trainrows.last().lower_requested_peloton_resistance ||
             r.upper_requested_peloton_resistance != trainrows.last().upper_requested_peloton_resistance ||
             r.lower_cadence != trainrows.last().lower_cadence || r.upper_cadence != trainrows.last().upper_cadence)) {
            r.duration = QTime(0, 0, 0).addSecs(duration);
            trainrows.append(r);
        } else {
            trainrows.last().duration = trainrows.last().duration.addSecs(duration);
        }
    }

    bool atLeastOnePower = false;
    if (trainrows.empty() && !segments_segment_list.isEmpty() &&
        bluetoothManager->device()->deviceType() != bluetoothdevice::ROWING &&
        bluetoothManager->device()->deviceType() != bluetoothdevice::TREADMILL) {
        foreach (QJsonValue o, segments_segment_list) {
            QJsonArray subsegments_v2 = o["subsegments_v2"].toArray();
            if (!subsegments_v2.isEmpty()) {
                foreach (QJsonValue s, subsegments_v2) {
                    trainrow r;
                    QString zone = s["display_name"].toString();
                    int len = s["length"].toInt();
                    if (!zone.toUpper().compare(QStringLiteral("SPIN UPS")) ||
                        !zone.toUpper().compare(QStringLiteral("SPIN-UPS"))) {
                        bool peloton_spinups_autoresistance =
                            settings
                                .value(QZSettings::peloton_spinups_autoresistance,
                                       QZSettings::default_peloton_spinups_autoresistance)
                                .toBool();
                        uint32_t Duration = len;
                        if (peloton_spinups_autoresistance) {
                            double PowerLow = 0.5;
                            double PowerHigh = 0.83;
                            for (uint32_t i = 0; i < Duration; i++) {
                                trainrow row;
                                row.duration = QTime(0, 0, 1, 0);
                                row.rampDuration =
                                    QTime((Duration - i) / 3600, (Duration - i) / 60, (Duration - i) % 60, 0);
                                row.rampElapsed = QTime(i / 3600, i / 60, i % 60, 0);
                                if (PowerHigh > PowerLow) {
                                    row.power = (PowerLow + (((PowerHigh - PowerLow) / Duration) * i)) *
                                                settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                                } else {
                                    row.power = (PowerLow - (((PowerLow - PowerHigh) / Duration) * i)) *
                                                settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                                }
                                qDebug() << row.duration << "power" << row.power << row.rampDuration << row.rampElapsed;
                                trainrows.append(row);
                                atLeastOnePower = true;
                            }
                        } else {
                            r.duration = QTime(0, len / 60, len % 60, 0);
                            r.power = -1;
                            if (r.power != -1) {
                                atLeastOnePower = true;
                            }
                            trainrows.append(r);
                        }
                    } else if (!zone.toUpper().compare(QStringLiteral("DESCENDING RECOVERY"))) {
                        uint32_t Duration = len;
                        double PowerLow = 0.5;
                        double PowerHigh = 0.45;
                        for (uint32_t i = 0; i < Duration; i++) {
                            trainrow row;
                            row.duration = QTime(0, 0, 1, 0);
                            row.rampDuration =
                                QTime((Duration - i) / 3600, (Duration - i) / 60, (Duration - i) % 60, 0);
                            row.rampElapsed = QTime(i / 3600, i / 60, i % 60, 0);
                            if (PowerHigh > PowerLow) {
                                row.power = (PowerLow + (((PowerHigh - PowerLow) / Duration) * i)) *
                                            settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                            } else {
                                row.power = (PowerLow - (((PowerLow - PowerHigh) / Duration) * i)) *
                                            settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                            }
                            qDebug() << row.duration << "power" << row.power << row.rampDuration << row.rampElapsed;
                            trainrows.append(row);
                            atLeastOnePower = true;
                        }
                    } else if (!zone.toUpper().compare(QStringLiteral("RECOVERY"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.45;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("FLAT ROAD"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.50;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("SWEET SPOT"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.91;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("INTERVALS"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.75;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 1"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.50;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 2"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.66;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 3"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.83;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 4"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.98;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 5"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 1.13;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 6"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 1.35;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else if (!zone.toUpper().compare(QStringLiteral("ZONE 7"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 1.5;
                        if (r.power != -1) {
                            atLeastOnePower = true;
                        }
                        trainrows.append(r);
                        qDebug() << r.duration << "power" << r.power;
                    } else {
                        if(len > 0 && atLeastOnePower) {
                            r.duration = QTime(0, len / 60, len % 60, 0);
                            r.power = -1;
                            if (r.power != -1) {
                                atLeastOnePower = true;
                            }
                            qDebug() << "ERROR not handled!" << zone;
                            trainrows.append(r);
                        }
                    }
                }
            }
        }
        // this list doesn't have nothing useful for this session
        if (!atLeastOnePower) {
            trainrows.clear();
        }
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
        QJsonObject target_metrics_data_list = ride[QStringLiteral("target_metrics_data")].toObject();
        QJsonArray pace_intensities_list = target_metrics_data_list[QStringLiteral("pace_intensities")].toArray();

        int pace_count = 0;        
        rower_pace_offset = 0;

        foreach (QJsonValue o, pace_intensities_list) {
            if(o["value"].toInt() < 0) {
                if(abs(o["value"].toInt()) > rower_pace_offset)
                    rower_pace_offset = abs(o["value"].toInt());
            }
        }
        
        qDebug() << "rower_pace_offset" << rower_pace_offset;

        foreach (QJsonValue o, pace_intensities_list) {
            qDebug() << o;
            pace_count = o["value"].toInt() + rower_pace_offset;
            if (pace_count < 5 && pace_count >= 0) {
                rower_pace[pace_count].display_name = o["display_name"].toString();
                rower_pace[pace_count].value = o["value"].toInt();

                QJsonArray levels = o["pace_levels"].toArray();
                if (levels.count() > 10) {
                    qDebug() << "peloton pace levels had been changed!";
                }
                int count = 0;
                foreach (QJsonValue level, levels) {
                    if(level["slug"].toString().split("_").count() > 1 ) {
                        count = level["slug"].toString().split("_")[1].toInt() - 1;
                        if (count >= 0 && count < 11) {
                            rower_pace[pace_count].levels[count].fast_pace = level["fast_pace"].toDouble();
                            rower_pace[pace_count].levels[count].slow_pace = level["slow_pace"].toDouble();
                            rower_pace[pace_count].levels[count].display_name = level["display_name"].toString();
                            rower_pace[pace_count].levels[count].slug = level["slug"].toString();
                            
                            qDebug() << count << level << rower_pace[pace_count].levels[count].display_name
                            << rower_pace[pace_count].levels[count].fast_pace
                            << rower_pace[pace_count].levels[count].slow_pace
                            << rower_pace[pace_count].levels[count].slug;
                        } else {
                            qDebug() << level["slug"].toString() << "slug error";
                        }
                    } else {
                        qDebug() << level["slug"].toString() << "slug count error";
                    }
                }
                qDebug() << pace_count << rower_pace[pace_count].display_name << rower_pace[pace_count].value;
            } else {
                qDebug() << "pace_count error!";
            }
        }
    }

    if (log_request) {
        qDebug() << "peloton::ride_onfinish" << trainrows.length() << ride;
    } else {
        qDebug() << "peloton::ride_onfinish" << trainrows.length();
    }

    if (!trainrows.isEmpty()) {
        emit workoutStarted(current_workout_name, current_instructor_name);
        timer->start(30s); // check for a status changed
    } else {
        // fallback
        getPerformance(current_workout_id);
    }
}

void peloton::performance_onfinish(QNetworkReply *reply) {
    disconnect(mgr, &QNetworkAccessManager::finished, this, &peloton::performance_onfinish);

    QSettings settings;
    QString difficulty =
        settings.value(QZSettings::peloton_difficulty, QZSettings::default_peloton_difficulty).toString();

    QByteArray payload = reply->readAll(); // JSON
    QJsonParseError parseError;
    performance = QJsonDocument::fromJson(payload, &parseError);
    current_api = peloton_api;

    QJsonObject json = performance.object();
    QJsonObject target_performance_metrics = json[QStringLiteral("target_performance_metrics")].toObject();
    QJsonObject target_metrics_performance_data = json[QStringLiteral("target_metrics_performance_data")].toObject();
    QJsonArray segment_list = json[QStringLiteral("segment_list")].toArray();
    trainrows.clear();

    if (!target_metrics_performance_data.isEmpty() && bluetoothManager->device() &&
        bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
        double miles = 1;
        bool treadmill_force_speed =
            settings.value(QZSettings::treadmill_force_speed, QZSettings::default_treadmill_force_speed).toBool();
        QJsonArray target_metrics = target_metrics_performance_data[QStringLiteral("target_metrics")].toArray();
        QJsonObject splits_data = json[QStringLiteral("splits_data")].toObject();
        if (!splits_data[QStringLiteral("distance_marker_display_unit")].toString().toUpper().compare("MI"))
            miles = 1.60934;
        trainrows.reserve(target_metrics.count() + 2);
        for (int i = 0; i < target_metrics.count(); i++) {
            QJsonObject metrics = target_metrics.at(i).toObject();
            QJsonArray metrics_ar = metrics[QStringLiteral("metrics")].toArray();
            QJsonObject offset = metrics[QStringLiteral("offsets")].toObject();
            QString segment_type = metrics[QStringLiteral("segment_type")].toString();
            if (metrics_ar.count() > 1 && !offset.isEmpty()) {
                QJsonObject speed = metrics_ar.at(0).toObject();
                double speed_lower = speed[QStringLiteral("lower")].toDouble();
                double speed_upper = speed[QStringLiteral("upper")].toDouble();
                double speed_average = (((speed_upper - speed_lower) / 2.0) + speed_lower) * miles;
                QJsonObject inc = metrics_ar.at(1).toObject();
                double inc_lower = inc[QStringLiteral("lower")].toDouble();
                double inc_upper = inc[QStringLiteral("upper")].toDouble();
                int offset_start = offset[QStringLiteral("start")].toInt();
                int offset_end = offset[QStringLiteral("end")].toInt();
                double inc_average = ((inc_upper - inc_lower) / 2.0) + inc_lower;
                // keeping the same bike behaviour
                /*if(i == 0 && offset_start > 0) {
                    trainrow r;
                    r.forcespeed = false;
                    r.duration = QTime(0, 0, 0, 0);
                    r.duration = r.duration.addSecs(offset_start);
                    trainrows.append(r);
                    qDebug() << i << r.duration << r.speed << r.inclination;
                }*/
                trainrow r;
                r.forcespeed = treadmill_force_speed;
                r.duration = QTime(0, 0, 0, 0);
                r.duration = r.duration.addSecs((offset_end - offset_start) + 1);
                if (!difficulty.toUpper().compare(QStringLiteral("LOWER"))) {
                    r.speed = speed_lower * miles;
                    r.inclination = inc_lower;
                } else if (!difficulty.toUpper().compare(QStringLiteral("UPPER"))) {
                    r.speed = speed_upper * miles;
                    r.inclination = inc_upper;
                } else {
                    r.speed = (((speed_upper - speed_lower) / 2.0) + speed_lower) * miles;
                    r.inclination = ((inc_upper - inc_lower) / 2.0) + inc_lower;
                }

                double offset =
                    settings.value(QZSettings::zwift_inclination_offset, QZSettings::default_zwift_inclination_offset)
                        .toDouble();
                double gain =
                    settings.value(QZSettings::zwift_inclination_gain, QZSettings::default_zwift_inclination_gain)
                        .toDouble();
                r.inclination *= gain;
                r.inclination += offset;
                r.lower_inclination *= gain;
                r.lower_inclination += offset;
                r.average_inclination *= gain;
                r.average_inclination += offset;
                r.upper_inclination *= gain;
                r.upper_inclination += offset;

                r.lower_speed = speed_lower * miles;
                r.average_speed = speed_average * miles;
                r.upper_speed = speed_upper * miles;
                r.lower_inclination = inc_lower;
                r.average_inclination = inc_average;
                r.upper_inclination = inc_upper;
                trainrows.append(r);
                qDebug() << i << r.duration << r.speed << r.inclination;
            } else if (segment_type.contains("floor") || segment_type.contains("free_mode")) {
                int offset_start = offset[QStringLiteral("start")].toInt();
                int offset_end = offset[QStringLiteral("end")].toInt();
                trainrow r;
                r.duration = QTime(0, 0, 0, 0);
                r.duration = r.duration.addSecs((offset_end - offset_start) + 1);
                trainrows.append(r);
                qDebug() << i << r.duration << r.speed << r.inclination;
            }
        }
    } else if (!target_metrics_performance_data.isEmpty() && bluetoothManager->device() &&
               bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
        QJsonArray target_metrics = target_metrics_performance_data[QStringLiteral("target_metrics")].toArray();
        trainrows.reserve(target_metrics.count() + 2);
        for (int i = 0; i < target_metrics.count(); i++) {
            QJsonObject metrics = target_metrics.at(i).toObject();
            QJsonArray metrics_ar = metrics[QStringLiteral("metrics")].toArray();
            QJsonObject offset = metrics[QStringLiteral("offsets")].toObject();
            QString segment_type = metrics[QStringLiteral("segment_type")].toString();
            if (metrics_ar.count() > 1 && !offset.isEmpty()) {
                QJsonObject strokes_rate = metrics_ar.at(0).toObject();
                QJsonObject pace_intensity = metrics_ar.at(1).toObject();
                int peloton_rower_level =
                    settings.value(QZSettings::peloton_rower_level, QZSettings::default_peloton_rower_level).toInt() -
                    1;
                double strokes_rate_lower = strokes_rate[QStringLiteral("lower")].toDouble();
                double strokes_rate_upper = strokes_rate[QStringLiteral("upper")].toDouble();
                int pace_intensity_lower = pace_intensity[QStringLiteral("lower")].toInt() + rower_pace_offset;
                int pace_intensity_upper = pace_intensity[QStringLiteral("upper")].toInt() + rower_pace_offset;
                int offset_start = offset[QStringLiteral("start")].toInt();
                int offset_end = offset[QStringLiteral("end")].toInt();
                double strokes_rate_average = ((strokes_rate_upper - strokes_rate_lower) / 2.0) + strokes_rate_lower;
                trainrow r;
                r.duration = QTime(0, 0, 0, 0);
                r.duration = r.duration.addSecs((offset_end - offset_start) + 1);
                if (!difficulty.toUpper().compare(QStringLiteral("LOWER"))) {
                    r.cadence = strokes_rate_lower;
                } else if (!difficulty.toUpper().compare(QStringLiteral("UPPER"))) {
                    r.cadence = strokes_rate_upper;
                } else {
                    r.cadence = ((strokes_rate_upper - strokes_rate_lower) / 2.0) + strokes_rate_lower;
                }

                if (pace_intensity_lower >= 0 && pace_intensity_lower < 5) {
                    r.average_speed =
                        (rowerpaceToSpeed(rower_pace[pace_intensity_lower].levels[peloton_rower_level].fast_pace) +
                         rowerpaceToSpeed(rower_pace[pace_intensity_lower].levels[peloton_rower_level].slow_pace)) /
                        2.0;
                    r.upper_speed =
                        rowerpaceToSpeed(rower_pace[pace_intensity_lower].levels[peloton_rower_level].fast_pace);
                    r.lower_speed =
                        rowerpaceToSpeed(rower_pace[pace_intensity_lower].levels[peloton_rower_level].slow_pace);

                    if (!difficulty.toUpper().compare(QStringLiteral("LOWER"))) {
                        r.pace_intensity = pace_intensity_lower;
                        r.speed = r.lower_speed;
                    } else if (!difficulty.toUpper().compare(QStringLiteral("UPPER"))) {
                        r.pace_intensity = pace_intensity_upper;
                        r.speed = r.upper_speed;
                    } else {
                        r.pace_intensity = (pace_intensity_upper + pace_intensity_lower) / 2;
                        r.speed = r.average_speed;
                    }
                    r.forcespeed = 1;
                }                

                r.lower_cadence = strokes_rate_lower;
                r.average_cadence = strokes_rate_average;
                r.upper_cadence = strokes_rate_upper;

                trainrows.append(r);
                qDebug() << i << r.duration << r.cadence << r.speed << r.upper_speed << r.lower_speed;
            } else if (segment_type.contains("floor") || segment_type.contains("free_mode")) {
                int offset_start = offset[QStringLiteral("start")].toInt();
                int offset_end = offset[QStringLiteral("end")].toInt();
                trainrow r;
                r.duration = QTime(0, 0, 0, 0);
                r.duration = r.duration.addSecs((offset_end - offset_start) + 1);
                trainrows.append(r);
                qDebug() << i << r.duration << r.cadence;
            }
        }
    }
    // Target METS it's quite useless so I removed, no one use this
    /* else if (!segment_list.isEmpty() && bluetoothManager->device()->deviceType() != bluetoothdevice::BIKE) {
        trainrows.reserve(segment_list.count() + 1);
        foreach (QJsonValue o, segment_list) {
            int len = o["length"].toInt();
            int mets = o["intensity_in_mets"].toInt();
            if (len > 0) {
                trainrow r;
                r.duration = QTime(0, len / 60, len % 60, 0);
                r.mets = mets;
                trainrows.append(r);
            }
        }
    }*/

    if (log_request) {
        qDebug() << QStringLiteral("peloton::performance_onfinish") << trainrows.length() << performance;
    } else {
        qDebug() << QStringLiteral("peloton::performance_onfinish") << trainrows.length();
    }

    if (!trainrows.isEmpty()) {

        emit workoutStarted(current_workout_name, current_instructor_name);
    } else {

        if (!PZP->searchWorkout(current_ride_id)) {
            current_api = homefitnessbuddy_api;
            HFB->searchWorkout(current_original_air_time.date(), current_instructor_name, current_pedaling_duration,
                               current_ride_id);
        } else {
            current_api = powerzonepack_api;
        }
    }

    timer->start(30s); // check for a status changed
}

double peloton::rowerpaceToSpeed(double pace) {
    float whole, fractional;

    fractional = std::modf(pace, &whole);
    double seconds = whole * 60.0;
    seconds += (fractional * 100.0);
    seconds *= 2.0;

    return 3600.0 / seconds;
}

void peloton::getInstructor(const QString &instructor_id) {
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::instructor_onfinish);

    QUrl url(QStringLiteral("https://api.onepeloton.com/api/instructor/") + instructor_id);
    qDebug() << "peloton::getInstructor" << url;
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    mgr->get(request);
}

void peloton::getRide(const QString &ride_id) {
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::ride_onfinish);

    QUrl url(QStringLiteral("https://api.onepeloton.com/api/ride/") + ride_id +
             QStringLiteral("/details?stream_source=multichannel"));
    qDebug() << "peloton::getRide" << url;
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    mgr->get(request);
}

void peloton::getPerformance(const QString &workout) {
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::performance_onfinish);

    QUrl url(QStringLiteral("https://api.onepeloton.com/api/workout/") + workout +
             QStringLiteral("/performance_graph?every_n=") + QString::number(peloton_workout_second_resolution));
    qDebug() << "peloton::getPerformance" << url;
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    mgr->get(request);
}

void peloton::getWorkout(const QString &workout) {
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::workout_onfinish);

    QUrl url(QStringLiteral("https://api.onepeloton.com/api/workout/") + workout);
    qDebug() << "peloton::getWorkout" << url;
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    mgr->get(request);
}

void peloton::getSummary(const QString &workout) {
    connect(mgr, &QNetworkAccessManager::finished, this, &peloton::summary_onfinish);

    QUrl url(QStringLiteral("https://api.onepeloton.com/api/workout/") + workout + QStringLiteral("/summary"));
    qDebug() << "peloton::getSummary" << url;
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
    qDebug() << "peloton::getWorkoutList" << url;
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("qdomyos-zwift"));

    mgr->get(request);
}

void peloton::setTestMode(bool test) { testMode = test; }
