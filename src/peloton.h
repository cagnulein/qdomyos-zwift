#ifndef PELOTON_H
#define PELOTON_H

#include "bluetooth.h"
#include "OAuth2.h"
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

#if defined(WIN32)
#pragma message("DEFINE PELOTON_SECRET_KEY!!!")
#else
#warning "DEFINE PELOTON_SECRET_KEY!!!"
#endif

#define PELOTON_CLIENT_ID_S STRINGIFY(PELOTON_SECRET_KEY)

class peloton : public QObject {

    Q_OBJECT
  public:
    explicit peloton(bluetooth *bl, QObject *parent = nullptr);
    QList<trainrow> trainrows;

    enum _PELOTON_API { peloton_api = 0, powerzonepack_api = 1, homefitnessbuddy_api = 2, no_metrics = 3 };

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
    qint64 start_time = 0;

    // OAuth
    QString pelotonAuthUrl;
    bool pelotonAuthWebVisible;

    void setTestMode(bool test);

    bool hasIntro();
    bool isWorkoutInProgress() {
        return current_workout_status.contains(QStringLiteral("IN_PROGRESS"), Qt::CaseInsensitive);
    }

  private:
    _PELOTON_API current_api = peloton_api;
    const int peloton_workout_second_resolution = 1;
    int workout_retry_count = 0;
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

    //OAuth
    QOAuth2AuthorizationCodeFlow *pelotonOAuth = nullptr;
    QNetworkAccessManager *manager = nullptr;
    QOAuthHttpServerReplyHandler *pelotonReplyHandler = nullptr;
    QString peloton_code;    
    QOAuth2AuthorizationCodeFlow *peloton_connect();
    void peloton_refreshtoken();    
    QNetworkReply *replyPeloton;
    QAbstractOAuth::ModifyParametersFunction buildModifyParametersFunction(const QUrl &clientIdentifier,
                                                                           const QUrl &clientIdentifierSharedKey);
    // Save token with user-specific suffix
    QString getPelotonSettingKey(const QString& baseKey, const QString& userId) {
        if (userId.isEmpty()) {
            qDebug() << "ERROR: userid is empty!";
            return baseKey; // If no user ID, use the default key
        }
        return baseKey + "_" + userId;
    }
    void savePelotonTokenForUser(const QString& baseKey, const QVariant& value, const QString& userId) {
        QSettings settings;
        settings.setValue(getPelotonSettingKey(baseKey, userId), value);
    }
    QVariant getPelotonTokenForUser(const QString& baseKey, const QString& userId, const QVariant& defaultValue = "") {
        QSettings settings;
        return settings.value(getPelotonSettingKey(baseKey, userId), defaultValue).toString();
    }
    QString tempAccessToken = QStringLiteral("");
    QString tempRefreshToken = QStringLiteral("");
    QDateTime tempExpiresAt;

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

    typedef struct _peloton_treadmill_pace_intensities_level {
        QString display_name;
        double fast_pace;
        double slow_pace;
        double speed;  // Average of fast_pace and slow_pace
        QString slug;
    }_peloton_treadmill_pace_intensities_level;

    typedef struct _peloton_treadmill_pace_intensities {
        QString display_name;
        int value;
        _peloton_treadmill_pace_intensities_level levels[10];
    } _peloton_treadmill_pace_intensities;

    _peloton_treadmill_pace_intensities treadmill_pace[7];

  public slots:
    void peloton_connect_clicked();

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

    // OAuth
    void onPelotonGranted();
    void onPelotonAuthorizeWithBrowser(const QUrl &url);
    void replyDataReceived(const QByteArray &v);
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &error);
    void networkRequestFinished(QNetworkReply *reply);
    void callbackReceived(const QVariantMap &values);

    void startEngine();

  signals:
    void loginState(bool ok);
    void pzpLoginState(bool ok);
    void workoutStarted(QString name, QString instructor);
    void workoutChanged(QString name, QString instructor);
    void pelotonAuthUrlChanged(QString value);
    void pelotonWebVisibleChanged(bool value);
};

#endif // PELOTON_H
