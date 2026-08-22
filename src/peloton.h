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
#include <QTcpServer>
#include <QTcpSocket>

#include <QSettings>

#include <QTimer>
#include <QUrlQuery>

#include "filedownloader.h"
#include "homefitnessbuddy.h"

// Include secret.h if it exists
#if __has_include("secret.h")
#include "secret.h"
#endif

// Warn only if PELOTON_SECRET_KEY is not defined
#ifndef PELOTON_SECRET_KEY
#if defined(WIN32)
#pragma message("DEFINE PELOTON_SECRET_KEY!!!")
#else
#warning "DEFINE PELOTON_SECRET_KEY!!!"
#endif
#endif

#define PELOTON_CLIENT_ID_S STRINGIFY(PELOTON_SECRET_KEY)

// Peloton treadmill Bootcamp floor/free-mode segments are emitted by the API as
// timed rows without speed, inclination or other treadmill targets. Keep those
// rows in the program, but make the physical intent explicit: when automatic
// treadmill speed control is enabled, a targetless timed treadmill row means
// stop the belt while the training-program clock keeps advancing.
class PelotonTrainRows : public QList<trainrow> {
  public:
    explicit PelotonTrainRows(bluetooth **bluetoothManager) : m_bluetoothManager(bluetoothManager) {}

    using QList<trainrow>::append;

    void append(const trainrow &input) {
        trainrow row = input;
        if (isTreadmill() && isTargetlessTimedRow(row)) {
            QSettings settings;
            row.speed = 0.0;
            row.lower_speed = 0.0;
            row.average_speed = 0.0;
            row.upper_speed = 0.0;
            row.forcespeed = settings.value(QZSettings::treadmill_force_speed,
                                            QZSettings::default_treadmill_force_speed).toBool();
            qDebug() << row.duration << "Peloton treadmill targetless interval - target speed 0";
        }
        QList<trainrow>::append(row);
    }

  private:
    bool isTreadmill() const {
        return m_bluetoothManager && *m_bluetoothManager && (*m_bluetoothManager)->device() &&
               (*m_bluetoothManager)->device()->deviceType() == TREADMILL;
    }

    static bool isTargetlessTimedRow(const trainrow &row) {
        return row.duration != QTime(0, 0, 0, 0) && row.distance < 0.0 &&
               row.speed < 0.0 && row.lower_speed < 0.0 && row.average_speed < 0.0 && row.upper_speed < 0.0 &&
               row.inclination == -200 && row.lower_inclination == -200 && row.average_inclination == -200 &&
               row.upper_inclination == -200 && row.power < 0 && row.resistance < 0 && row.cadence < 0 &&
               row.pace_intensity < 0;
    }

    bluetooth **m_bluetoothManager = nullptr;
};

class peloton : public QObject {

    Q_OBJECT
  private:
    // Declared before trainrows so the list can safely keep a pointer to this
    // member and observe the device assigned by the constructor body.
    bluetooth *bluetoothManager = nullptr;

  public:
    explicit peloton(bluetooth *bl, QObject *parent = nullptr);
    PelotonTrainRows trainrows{&bluetoothManager};

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

    int getIntroOffset();
    bool isWorkoutInProgress() {
        return current_workout_status.contains(QStringLiteral("IN_PROGRESS"), Qt::CaseInsensitive);
    }
    QString getPelotonWorkoutUrl();

    // Helper function to determine if workout is walking-based
    bool isWalkingWorkout() const {
        // Check if it's a walking discipline or a walking bootcamp
        return current_workout_type == "walking" ||
               (current_workout_type == "circuit" &&
                (current_workout_name.contains("Walk", Qt::CaseInsensitive) ||
                 current_workout_name.contains("Walking", Qt::CaseInsensitive)));
    }

  private:
    _PELOTON_API current_api = peloton_api;
    const int peloton_workout_second_resolution = 1;
    int workout_retry_count = 0;
    bool peloton_credentials_wrong = false;
    bool needsReauth = false;
    QNetworkAccessManager *mgr = nullptr;

    QJsonDocument current_workout;
    QJsonDocument current_workout_summary;
    QJsonDocument workout;
    QJsonDocument instructor;
    QJsonDocument performance;

    QTimer *timer;

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
    QString pelotonPendingState;
    QTcpServer *pelotonDesktopRelayServer = nullptr;
    QOAuth2AuthorizationCodeFlow *peloton_connect();
    void peloton_refreshtoken();    
    QNetworkReply *replyPeloton;
    QAbstractOAuth::ModifyParametersFunction buildModifyParametersFunction(const QUrl &clientIdentifier,
                                                                           const QUrl &clientIdentifierSharedKey);
    bool exchangeAuthorizationCode(const QString &code);
    bool isAcceptedCallbackUrl(const QUrl &url) const;
    void completeOAuthLogin();
#if !defined(Q_OS_ANDROID)
    bool ensureDesktopRelayServer();
    void stopDesktopRelayServer();
    void handleDesktopRelayConnection();
    void handleDesktopRelaySocketReadyRead();
#endif
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
    _peloton_treadmill_pace_intensities walking_pace[5];

    int first_target_metrics_start_offset = 60;

  public slots:
    void peloton_connect_clicked();
    void onUserProfileChanged();
    void peloton_logout();
    void handleOAuthCallbackUrl(const QUrl &url);

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
    void checkWorkoutStatus();

  signals:
    void loginState(bool ok);
    void pzpLoginState(bool ok);
    void workoutStarted(QString name, QString instructor);
    void workoutChanged(QString name, QString instructor);
    void pelotonAuthUrlChanged(QString value);
    void pelotonWebVisibleChanged(bool value);
};

#endif // PELOTON_H