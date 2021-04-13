#ifndef PELOTON_H
#define PELOTON_H

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

class peloton : public QObject
{
    Q_OBJECT
public:
    explicit peloton(bluetooth* bl, QObject *parent = nullptr);
    QList<trainrow> trainrows;

private:
    const int peloton_workout_second_resolution = 10;
    bool peloton_credentials_wrong = false;
    QNetworkAccessManager * mgr = 0;
    QString user_id;
    QString current_workout_id = "";
    QString current_workout_name = "";
    QString current_workout_status = "";
    QString current_instructor_id = "";
    QString current_instructor_name = "";

    QJsonDocument current_workout;
    QJsonDocument current_workout_summary;
    QJsonDocument workout;
    QJsonDocument instructor;
    QJsonDocument performance;

    QTimer* timer;

    bluetooth* bluetoothManager = 0;

    int total_workout;
    void getWorkoutList(int num);
    void getSummary(QString workout);
    void getWorkout(QString workout);
    void getInstructor(QString instructor_id);
    void getPerformance(QString workout);

private slots:
    void login_onfinish(QNetworkReply* reply);
    void workoutlist_onfinish(QNetworkReply* reply);
    void summary_onfinish(QNetworkReply* reply);
    void workout_onfinish(QNetworkReply* reply);
    void performance_onfinish(QNetworkReply* reply);
    void instructor_onfinish(QNetworkReply* reply);

    void startEngine();

signals:
    void loginState(bool ok);
    void workoutStarted(QString name, QString instructor);
};

#endif // PELOTON_H
