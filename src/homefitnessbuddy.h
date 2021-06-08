#ifndef HOMEFITNESSBUDDY_H
#define HOMEFITNESSBUDDY_H

#include <QObject>
#include <QAbstractOAuth2>
#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QSettings>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include "trainprogram.h"
#include "bluetooth.h"

class homefitnessbuddy : public QObject
{
    Q_OBJECT

public:
    homefitnessbuddy(bluetooth* bl, QObject *parent);
    void searchWorkout(QDate date, QString coach);
    QList<trainrow> trainrows;

private:
    const int peloton_workout_second_resolution = 10;

    QNetworkAccessManager * mgr = 0;
    bluetooth* bluetoothManager = 0;

    void startEngine();

    QJsonArray lessons;

private slots:
    void login_onfinish(QNetworkReply* reply);
    void search_workout_onfinish(QNetworkReply* reply);
    void error(QNetworkReply::NetworkError code);

signals:
    void workoutStarted(QList<trainrow>* list);
    //void loginState(bool ok);
};

#endif // HOMEFITNESSBUDDY_H
