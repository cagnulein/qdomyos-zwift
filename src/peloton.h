#ifndef PELOTON_H
#define PELOTON_H

#include "bluetooth.h"
#include "powerzonepack.h"
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

#include "filedownloader.h"
#include "homefitnessbuddy.h"

class peloton : public QObject {

    Q_OBJECT
  public:
    explicit peloton(bluetooth *bl, QObject *parent = nullptr);
    QList<trainrow> trainrows;

    enum _PELOTON_API { peloton_api = 0, powerzonepack_api = 1, homefitnessbuddy_api = 2 };

    _PELOTON_API currentApi() { return current_api; }

    QString user_id;
    QString current_workout_id = QLatin1String("");
    QString current_workout_name = QLatin1String("");
    QString current_workout_status = QLatin1String("");
    QString current_workout_type = QLatin1String("");
    QString current_instructor_id = QLatin1String("");
    QString current_instructor_name = QLatin1String("");
    QString current_ride_id = QLatin1String("");
    QString current_image_url = QLatin1String("");
    fileDownloader *current_image_downloaded = nullptr;
    void downloadImage();
    QDateTime current_original_air_time;
    int current_pedaling_duration = 0;

    void setTestMode(bool test);

    bool isWorkoutInProgress() {
        return current_workout_status.contains(QStringLiteral("IN_PROGRESS"), Qt::CaseInsensitive);
    }

  private:
    _PELOTON_API current_api = peloton_api;
    const int peloton_workout_second_resolution = 10;
    bool peloton_credentials_wrong = false;
    QNetworkAccessManager *mgr = nullptr;

    QJsonDocument current_workout;
    QJsonDocument current_workout_summary;
    QJsonDocument workout;
    QJsonDocument instructor;
    QJsonDocument performance;

    QTimer *timer;

    bluetooth *bluetoothManager = nullptr;
    powerzonepack *PZP = nullptr;
    homefitnessbuddy *HFB = nullptr;

    int total_workout;
    void getWorkoutList(int num);
    void getSummary(const QString &workout);
    void getWorkout(const QString &workout);
    void getInstructor(const QString &instructor_id);
    void getRide(const QString &ride_id);
    void getPerformance(const QString &workout);

    bool testMode = false;

    // rowers
    double rowerpaceToSpeed(double pace);
    typedef struct _peloton_rower_pace_intensities_level {
        QString display_name;
        double fast_pace;
        double slow_pace;
        QString slug;
    }_peloton_rower_pace_intensities_level;

    typedef struct _peloton_rower_pace_intensities {
        QString display_name;
        int value;
        _peloton_rower_pace_intensities_level levels[10];
    } _peloton_rower_pace_intensities;

    _peloton_rower_pace_intensities rower_pace[5];
    int rower_pace_offset = 0;

  private slots:
    void login_onfinish(QNetworkReply *reply);
    void workoutlist_onfinish(QNetworkReply *reply);
    void summary_onfinish(QNetworkReply *reply);
    void workout_onfinish(QNetworkReply *reply);
    void instructor_onfinish(QNetworkReply *reply);
    void ride_onfinish(QNetworkReply *reply);
    void performance_onfinish(QNetworkReply *reply);
    void pzp_trainrows(QList<trainrow> *list);
    void hfb_trainrows(QList<trainrow> *list);
    void pzp_loginState(bool ok);

    void startEngine();

  signals:
    void loginState(bool ok);
    void pzpLoginState(bool ok);
    void workoutStarted(QString name, QString instructor);
    void workoutChanged(QString name, QString instructor);
};

#endif // PELOTON_H
