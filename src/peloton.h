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

class peloton : public QObject
{
    Q_OBJECT
public:
    explicit peloton(QObject *parent = nullptr);    

private:
    QNetworkAccessManager * mgr = 0;
    QString user_id;
    QString current_workout_id = "";

    QJsonDocument current_workout;
    QJsonDocument current_workout_summary;
    QJsonDocument workout;

    int total_workout;
    void getWorkoutList(int num);
    void getSummary(QString workout);
    void getWorkout(QString workout);
    void getPerformance(QString workout);

private slots:
    void login_onfinish(QNetworkReply* reply);
    void workoutlist_onfinish(QNetworkReply* reply);
    void summary_onfinish(QNetworkReply* reply);
    void workout_onfinish(QNetworkReply* reply);
    void performance_onfinish(QNetworkReply* reply);

signals:

};

#endif // PELOTON_H
