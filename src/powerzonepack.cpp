
#include <QSettings>
#include <QtXml>

#include "powerzonepack.h"

powerzonepack::powerzonepack(bluetooth *bl, QObject *parent) : QObject(parent) {
    QSettings settings;
    bluetoothManager = bl;

    if (!settings.value(QZSettings::pzp_username, QZSettings::default_pzp_username)
             .toString()
             .compare(QZSettings::default_pzp_username)) {
        pzp_credentials_wrong = true;
        qDebug() << QStringLiteral("invalid peloton credentials");
        return;
    }

    startEngine();
}

void powerzonepack::startEngine() {
    if (pzp_credentials_wrong) {
        return;
    }

    connect(&websocket, &QWebSocket::textMessageReceived, this, &powerzonepack::login_onfinish);
    connect(&websocket, &QWebSocket::connected, [&]() {
        QSettings settings;
        websocket.sendTextMessage("[1,[\"Api_Login\",[\"" + settings.value(QZSettings::pzp_username, QZSettings::default_pzp_username).toString() +
                                  "\",\"" + settings.value(QZSettings::pzp_password,  QZSettings::default_pzp_password).toString() + "\"]]]");
    });
    websocket.open(QUrl("wss://pzpack.com/api"));
}

void powerzonepack::error(QAbstractSocket::SocketError error) { qDebug() << "powerzonepack ERROR" << error; }

void powerzonepack::login_onfinish(const QString &message) {
    disconnect(&websocket, &QWebSocket::textMessageReceived, this, &powerzonepack::login_onfinish);
    QByteArray payload = message.toLocal8Bit();

    qDebug() << QStringLiteral("login_onfinish") << payload;

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    QJsonArray json = document.array();
    if (json.count() > 1)
        token = json.at(1)["Right"].toString();

    if (!token.isEmpty()) {
        emit loginState(true);
    }

    // REMOVE IT
    // searchWorkout("d6a54e1ce634437bb172f61eb1588b27");
}

bool powerzonepack::searchWorkout(const QString &classid) {

    qDebug() << "powerzonepack::searchWorkout" << pzp_credentials_wrong << classid;

    if (pzp_credentials_wrong) {
        return false;
    }

    lastWorkoutID = classid;
    connect(&websocket, &QWebSocket::textMessageReceived, this, &powerzonepack::search_workout_onfinish);
    websocket.sendTextMessage("[2,[\"Api_Authenticated\",[\"" + token +
                              "\",[\"AuthenticatedApi_GetClassByPeletonId\",\"" + classid + "\"]]]]");

    return true;
}

void powerzonepack::search_workout_onfinish(const QString &message) {
    disconnect(&websocket, &QWebSocket::binaryMessageReceived, this, &powerzonepack::search_workout_onfinish);
    QByteArray payload = message.toLocal8Bit(); // JSON

    qDebug() << "search_workout_onfinish" << payload;

    QSettings settings;
    QString difficulty = settings.value(QZSettings::peloton_difficulty, QZSettings::default_peloton_difficulty).toString();
    QJsonParseError parseError;
    QJsonDocument performance = QJsonDocument::fromJson(payload, &parseError);

    QJsonArray json = performance.array();
    if (json.count() <= 1)
        return;
    QJsonArray power_graph = json.at(1)["_class_power_graph"].toArray();

    trainrows.clear();
    QTime lastSeconds(0, 0, 0, 0);
    for (int i = 1; i < power_graph.count(); i++) {

        trainrow r;
        double sec = power_graph.at(i).toObject()[QStringLiteral("seconds")].toDouble();
        QTime seconds(0, 0, 0, 0);
        seconds = seconds.addSecs((int)sec);
        r.duration = QTime(0, 0, lastSeconds.msecsTo(seconds) / 1000, 0);
        r.power = power_graph.at(i - 1).toObject()[QStringLiteral("power_ratio")].toDouble() *
                  settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();

        // in order to have compact rows in the training program to have an Reamining Time tile set correctly
        if (i == 1 || (r.power != trainrows.last().power))
            trainrows.append(r);
        else
            trainrows.last().duration = trainrows.last().duration.addSecs(lastSeconds.msecsTo(seconds) / 1000);

        lastSeconds = seconds;
    }

    if (!trainrows.isEmpty()) {

        emit workoutStarted(&trainrows);
    }
}
