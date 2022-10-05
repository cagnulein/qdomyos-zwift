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
            "peloton::workoutlist_onfinish Peloton API doens't answer, trying back in 10 seconds...");
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

    if (current_image_downloaded) {
        delete current_image_downloaded;
        current_image_downloaded = 0;
    }
    if (!current_image_url.isEmpty()) {
        current_image_downloaded = new fileDownloader(current_image_url);
    }

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
    QString difficulty = settings.value(QZSettings::peloton_difficulty, QZSettings::default_peloton_difficulty).toString();
    QJsonObject segments = ride[QStringLiteral("segments")].toObject();
    QJsonArray segments_segment_list = segments[QStringLiteral("segment_list")].toArray();

    for (int i = 0; i < instructor_cues.count(); i++) {
        QJsonObject instructor_cue = instructor_cues.at(i).toObject();
        QJsonObject offsets = instructor_cue[QStringLiteral("offsets")].toObject();
        QJsonObject resistance_range = instructor_cue[QStringLiteral("resistance_range")].toObject();
        QJsonObject cadence_range = instructor_cue[QStringLiteral("cadence_range")].toObject();

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
    if (trainrows.empty() && !segments_segment_list.isEmpty()) {
        foreach (QJsonValue o, segments_segment_list) {
            QJsonArray subsegments_v2 = o["subsegments_v2"].toArray();
            if (!subsegments_v2.isEmpty()) {
                foreach (QJsonValue s, subsegments_v2) {
                    trainrow r;
                    QString zone = s["display_name"].toString();
                    int len = s["length"].toInt();
                    if (!zone.toUpper().compare(QStringLiteral("SPIN UPS"))) {
                        uint32_t Duration = len;
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
                    } else if (!zone.toUpper().compare(QStringLiteral("FLAT ROAD"))) {
                        r.duration = QTime(0, len / 60, len % 60, 0);
                        r.power = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 0.50;
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
                    }
                }
            }
        }
        // this list doesn't have nothing useful for this session
        if (!atLeastOnePower) {
            trainrows.clear();
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
    QString difficulty = settings.value(QZSettings::peloton_difficulty, QZSettings::default_peloton_difficulty).toString();

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
        bool treadmill_force_speed = settings.value(QZSettings::treadmill_force_speed, QZSettings::default_treadmill_force_speed).toBool();
        QJsonArray target_metrics = target_metrics_performance_data[QStringLiteral("target_metrics")].toArray();
        QJsonObject splits_data = json[QStringLiteral("splits_data")].toObject();
        if (!splits_data[QStringLiteral("distance_marker_display_unit")].toString().toUpper().compare("MI"))
            miles = 1.60934;
        trainrows.reserve(target_metrics.count() + 2);
        for (int i = 0; i < target_metrics.count(); i++) {
            QJsonObject metrics = target_metrics.at(i).toObject();
            QJsonArray metrics_ar = metrics[QStringLiteral("metrics")].toArray();
            QJsonObject offset = metrics[QStringLiteral("offsets")].toObject();
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
                r.lower_speed = speed_lower * miles;
                r.average_speed = speed_average * miles;
                r.upper_speed = speed_upper * miles;
                r.lower_inclination = inc_lower;
                r.average_inclination = inc_average;
                r.upper_inclination = inc_upper;
                trainrows.append(r);
                qDebug() << i << r.duration << r.speed << r.inclination;
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
            HFB->searchWorkout(current_original_air_time.date(), current_instructor_name, current_pedaling_duration);
        } else {
            current_api = powerzonepack_api;
        }
    }

    timer->start(30s); // check for a status changed
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
