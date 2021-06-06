#ifndef POWERZONEPACK_H
#define POWERZONEPACK_H

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

class powerzonepack : public QObject {
    Q_OBJECT
  public:
    powerzonepack(bluetooth *bl, QObject *parent);
    void searchWorkout(const QString &classid);
    QList<trainrow> trainrows;

  private:
    const int peloton_workout_second_resolution = 10;
    bool pzp_credentials_wrong = false;

    QNetworkAccessManager *mgr = nullptr;
    bluetooth *bluetoothManager = nullptr;
    QString token;
    QString lastWorkoutID = QLatin1String("");

    void startEngine();

  private slots:
    void login_onfinish(QNetworkReply *reply);
    void search_workout_onfinish(QNetworkReply *reply);
    void error(QNetworkReply::NetworkError code);

  signals:
    void workoutStarted(QList<trainrow> *list);
    void loginState(bool ok);
};

#endif // POWERZONEPACK_H
