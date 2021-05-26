#ifndef POWERZONEPACK_H
#define POWERZONEPACK_H

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

class powerzonepack : public QObject
{
    Q_OBJECT
public:
    powerzonepack(bluetooth* bl, QObject *parent);
    void searchWorkout(QString classid);
    QList<trainrow> trainrows;

private:
    const int peloton_workout_second_resolution = 10;
    bool pzp_credentials_wrong = false;

    QNetworkAccessManager * mgr = 0;
    bluetooth* bluetoothManager = 0;
    QString x_xsrf_token;

    void startEngine();

private slots:
    void login_onfinish(QNetworkReply* reply);
    void login_credentials_onfinish(QNetworkReply* reply);
    void search_workout_onfinish(QNetworkReply* reply);
    void workout_onfinish(QNetworkReply* reply);
    void error(QNetworkReply::NetworkError code);

signals:
    void workoutStarted(QList<trainrow>* list);
    void loginState(bool ok);
};

#endif // POWERZONEPACK_H
