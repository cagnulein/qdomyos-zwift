#ifndef HOMEFITNESSBUDDY_H
#define HOMEFITNESSBUDDY_H

#include "bluetooth.h"
#include "trainprogram.h"
#include <QAbstractOAuth2>
#include <QDesktopServices>
#include <QHttpMultiPart>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QUrlQuery>

class homefitnessbuddy : public QObject {
    Q_OBJECT

  public:
    homefitnessbuddy(bluetooth *bl, QObject *parent);
    void searchWorkout(QDate date, const QString &coach, int pedaling_duration, QString class_id);
    QList<trainrow> trainrows;

  private:
    QString pelotonClassID;
    void getClassID(QString id);
    const int peloton_workout_second_resolution = 10;

    QNetworkAccessManager *mgr = nullptr;
    bluetooth *bluetoothManager = nullptr;

    QJsonArray lessons;

    QTimer retry;

  private slots:
    void login_onfinish(QNetworkReply *reply);
    void search_workout_onfinish(QNetworkReply *reply);
    void search_detail_onfinish(QNetworkReply *reply);
    void error(QNetworkReply::NetworkError code);
    void startEngine();

  signals:
    void workoutStarted(QList<trainrow> *list);
    void loginState(bool ok);
};

#endif // HOMEFITNESSBUDDY_H
