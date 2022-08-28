#ifndef HOMEFORM_H
#define HOMEFORM_H

#include "PathController.h"
#include "bluetooth.h"
#include "fit_profile.hpp"
#include "gpx.h"
#include "peloton.h"
#include "screencapture.h"
#include "sessionline.h"
#include "smtpclient/src/SmtpMime"
#include "trainprogram.h"
#include <QChart>
#include <QColor>
#include <QGraphicsScene>
#include <QNetworkReply>
#include <QOAuth2AuthorizationCodeFlow>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QTextToSpeech>

class DataObject : public QObject {

    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(int gridId READ gridId NOTIFY gridIdChanged WRITE setGridId)
    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QString secondLine READ secondLine WRITE setSecondLine NOTIFY secondLineChanged)
    Q_PROPERTY(int valueFontSize READ valueFontSize WRITE setValueFontSize NOTIFY valueFontSizeChanged)
    Q_PROPERTY(QString valueFontColor READ valueFontColor WRITE setValueFontColor NOTIFY valueFontColorChanged)
    Q_PROPERTY(int labelFontSize READ labelFontSize WRITE setLabelFontSize NOTIFY labelFontSizeChanged)
    Q_PROPERTY(bool writable READ writable NOTIFY writableChanged)
    Q_PROPERTY(bool visibleItem READ visibleItem NOTIFY visibleChanged)
    Q_PROPERTY(QString plusName READ plusName NOTIFY plusNameChanged)
    Q_PROPERTY(QString minusName READ minusName NOTIFY minusNameChanged)
    Q_PROPERTY(QString identificator READ identificator)

  public:
    DataObject(const QString &name, const QString &icon, const QString &value, bool writable, const QString &id,
               int valueFontSize, int labelFontSize, const QString &valueFontColor = QStringLiteral("white"),
               const QString &secondLine = QLatin1String(""), const int gridId = 0);
    void setName(const QString &value);
    void setValue(const QString &value);
    void setSecondLine(const QString &value);
    void setValueFontSize(int value);
    void setValueFontColor(const QString &value);
    void setLabelFontSize(int value);
    void setVisible(bool visible);
    void setGridId(int id);
    QString name() { return m_name; }
    QString icon() { return m_icon; }
    QString value() { return m_value; }
    QString secondLine() { return m_secondLine; }
    int gridId() { return m_gridId; }
    int valueFontSize() { return m_valueFontSize; }
    QString valueFontColor() { return m_valueFontColor; }
    int labelFontSize() { return m_labelFontSize; }
    bool writable() { return m_writable; }
    bool visibleItem() { return m_visible; }
    QString plusName() { return m_id + QStringLiteral("_plus"); }
    QString minusName() { return m_id + QStringLiteral("_minus"); }
    QString identificator() { return m_id; }

    QString m_id;
    QString m_name;
    QString m_icon;
    QString m_value;
    QString m_secondLine = QLatin1String("");
    int m_valueFontSize;
    int m_gridId;
    QString m_valueFontColor = QStringLiteral("white");
    int m_labelFontSize;
    bool m_writable;
    bool m_visible = true;

  signals:
    void valueChanged(QString value);
    void secondLineChanged(QString value);
    void valueFontSizeChanged(int value);
    void valueFontColorChanged(QString value);
    void labelFontSizeChanged(int value);
    void gridIdChanged(int value);
    void nameChanged(QString value);
    void iconChanged(QString value);
    void writableChanged(bool value);
    void visibleChanged(bool value);
    void plusNameChanged(QString value);
    void minusNameChanged(QString value);
};

class homeform : public QObject {

    Q_OBJECT
    Q_PROPERTY(bool labelHelp READ labelHelp NOTIFY changeLabelHelp)
    Q_PROPERTY(bool device READ getDevice NOTIFY changeOfdevice)
    Q_PROPERTY(bool lap READ getLap NOTIFY changeOflap)
    Q_PROPERTY(bool pelotonAskStart READ pelotonAskStart NOTIFY changePelotonAskStart WRITE setPelotonAskStart)
    Q_PROPERTY(QString pelotonProvider READ pelotonProvider NOTIFY changePelotonProvider WRITE setPelotonProvider)
    Q_PROPERTY(int topBarHeight READ topBarHeight NOTIFY topBarHeightChanged)
    Q_PROPERTY(QString info READ info NOTIFY infoChanged)
    Q_PROPERTY(QString signal READ signal NOTIFY signalChanged)
    Q_PROPERTY(QString startText READ startText NOTIFY startTextChanged)
    Q_PROPERTY(QString startIcon READ startIcon NOTIFY startIconChanged)
    Q_PROPERTY(QString startColor READ startColor NOTIFY startColorChanged)
    Q_PROPERTY(QString stopText READ stopText NOTIFY stopTextChanged)
    Q_PROPERTY(QString stopIcon READ stopIcon NOTIFY stopIconChanged)
    Q_PROPERTY(QString stopColor READ stopColor NOTIFY stopColorChanged)
    Q_PROPERTY(QStringList metrics READ metrics)
    Q_PROPERTY(QStringList bluetoothDevices READ bluetoothDevices NOTIFY bluetoothDevicesChanged)
    Q_PROPERTY(QStringList tile_order READ tile_order NOTIFY tile_orderChanged)
    Q_PROPERTY(bool generalPopupVisible READ generalPopupVisible NOTIFY generalPopupVisibleChanged WRITE
                   setGeneralPopupVisible)
    Q_PROPERTY(bool licensePopupVisible READ licensePopupVisible NOTIFY licensePopupVisibleChanged WRITE
                   setLicensePopupVisible)
    Q_PROPERTY(bool mapsVisible READ mapsVisible NOTIFY mapsVisibleChanged WRITE setMapsVisible)
    Q_PROPERTY(bool videoVisible READ videoVisible NOTIFY videoVisibleChanged WRITE setVideoVisible)
    Q_PROPERTY(QUrl videoPath READ videoPath)
    Q_PROPERTY(int videoPosition READ videoPosition NOTIFY videoPositionChanged WRITE setVideoPosition)
    Q_PROPERTY(double videoRate READ videoRate NOTIFY videoRateChanged WRITE setVideoRate)
    Q_PROPERTY(double currentSpeed READ currentSpeed NOTIFY currentSpeedChanged)
    Q_PROPERTY(int pelotonLogin READ pelotonLogin NOTIFY pelotonLoginChanged)
    Q_PROPERTY(int pzpLogin READ pzpLogin NOTIFY pzpLoginChanged)
    Q_PROPERTY(QString workoutStartDate READ workoutStartDate)
    Q_PROPERTY(QString workoutName READ workoutName)
    Q_PROPERTY(QString instructorName READ instructorName)
    Q_PROPERTY(int workout_sample_points READ workout_sample_points)
    Q_PROPERTY(QList<double> workout_watt_points READ workout_watt_points)
    Q_PROPERTY(QList<double> workout_heart_points READ workout_heart_points)
    Q_PROPERTY(QList<double> workout_cadence_points READ workout_cadence_points)
    Q_PROPERTY(QList<double> workout_peloton_resistance_points READ workout_peloton_resistance_points)
    Q_PROPERTY(QList<double> workout_resistance_points READ workout_resistance_points)
    Q_PROPERTY(double wattMaxChart READ wattMaxChart)
    Q_PROPERTY(bool autoResistance READ autoResistance NOTIFY autoResistanceChanged WRITE setAutoResistance)

    // workout preview
    Q_PROPERTY(int preview_workout_points READ preview_workout_points NOTIFY previewWorkoutPointsChanged)
    Q_PROPERTY(QList<double> preview_workout_watt READ preview_workout_watt)
    Q_PROPERTY(QString previewWorkoutDescription READ previewWorkoutDescription NOTIFY previewWorkoutDescriptionChanged)
    Q_PROPERTY(QString previewWorkoutTags READ previewWorkoutTags NOTIFY previewWorkoutTagsChanged)

    Q_PROPERTY(bool currentCoordinateValid READ currentCoordinateValid)

  public:
    Q_INVOKABLE void save_screenshot() {

        QString path = getWritableAppDir();

        QString filenameScreenshot =
            path + QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
            QStringLiteral(".jpg");
        QObject *rootObject = engine->rootObjects().constFirst();
        QObject *stack = rootObject;
        screenCapture s(reinterpret_cast<QQuickView *>(stack));
        s.capture(filenameScreenshot);
    }

    Q_INVOKABLE void save_screenshot_chart(QQuickItem *item, QString filename) {
        if (!stopped) {
            return;
        }

        QString path = getWritableAppDir();

        QString filenameScreenshot =
            path + QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
            QStringLiteral("_") + filename.replace(QStringLiteral(":"), QStringLiteral("_")) + QStringLiteral(".jpg");
        QSharedPointer<const QQuickItemGrabResult> grabResult = item->grabToImage();

        connect(grabResult.data(), &QQuickItemGrabResult::ready, [=]() { // NOTE: clazy-connect-3arg-lambda
            grabResult->saveToFile(filenameScreenshot);
            // chartImages.append(grabResult->image());
            chartImagesFilenames.append(filenameScreenshot);
        });
    }

    Q_INVOKABLE void update_chart_power(QQuickItem *item) {
        if (QGraphicsScene *scene = item->findChild<QGraphicsScene *>()) {
            auto items_list = scene->items();
            for (QGraphicsItem *it : qAsConst(items_list)) {
                if (QtCharts::QChart *chart = dynamic_cast<QtCharts::QChart *>(it)) {
                    // Customize chart background
                    QLinearGradient backgroundGradient;
                    double maxWatt = wattMaxChart();
                    QSettings settings;
                    double ftpSetting = settings.value(QStringLiteral("ftp"), 200.0).toDouble();
                    /*backgroundGradient.setStart(QPointF(0, 0));
                    backgroundGradient.setFinalStop(QPointF(0, 1));
                    backgroundGradient.setColorAt((maxWatt - (ftpSetting * 0.55)) / maxWatt, QColor("white"));
                    backgroundGradient.setColorAt((maxWatt - (ftpSetting * 0.75)) / maxWatt, QColor("limegreen"));
                    backgroundGradient.setColorAt((maxWatt - (ftpSetting * 0.90)) / maxWatt, QColor("gold"));
                    backgroundGradient.setColorAt((maxWatt - (ftpSetting * 1.05)) / maxWatt, QColor("orange"));
                    backgroundGradient.setColorAt((maxWatt - (ftpSetting * 1.20)) / maxWatt, QColor("darkorange"));
                    backgroundGradient.setColorAt((maxWatt - (ftpSetting * 1.5)) / maxWatt, QColor("orangered"));
                    backgroundGradient.setColorAt(0.0, QColor("red"));*/

                    // backgroundGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
                    // chart->setBackgroundBrush(backgroundGradient);
                    // Customize plot area background
                    QLinearGradient plotAreaGradient;
                    plotAreaGradient.setStart(QPointF(0, 0));
                    plotAreaGradient.setFinalStop(QPointF(0, 1));
                    plotAreaGradient.setColorAt((maxWatt - (ftpSetting * 0.55)) / maxWatt, QColor("white"));
                    plotAreaGradient.setColorAt((maxWatt - (ftpSetting * 0.75)) / maxWatt, QColor("limegreen"));
                    plotAreaGradient.setColorAt((maxWatt - (ftpSetting * 0.90)) / maxWatt, QColor("gold"));
                    plotAreaGradient.setColorAt((maxWatt - (ftpSetting * 1.05)) / maxWatt, QColor("orange"));
                    plotAreaGradient.setColorAt((maxWatt - (ftpSetting * 1.20)) / maxWatt, QColor("darkorange"));
                    plotAreaGradient.setColorAt((maxWatt - (ftpSetting * 1.5)) / maxWatt, QColor("orangered"));
                    plotAreaGradient.setColorAt(0.0, QColor("red"));
                    plotAreaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
                    chart->setPlotAreaBackgroundBrush(plotAreaGradient);
                    chart->setPlotAreaBackgroundVisible(true);
                }
            }
        }
    }
    Q_INVOKABLE void update_chart_heart(QQuickItem *item) {
        if (QGraphicsScene *scene = item->findChild<QGraphicsScene *>()) {
            auto items_list = scene->items();
            for (QGraphicsItem *it : qAsConst(items_list)) {
                if (QtCharts::QChart *chart = dynamic_cast<QtCharts::QChart *>(it)) {
                    // Customize chart background
                    QLinearGradient backgroundGradient;
                    QSettings settings;
                    double maxHeartRate = heartRateMax();
                    /*backgroundGradient.setStart(QPointF(0, 0));
                    backgroundGradient.setFinalStop(QPointF(0, 1));
                    backgroundGradient.setColorAt((220 - (maxHeartRate *
                    settings.value("heart_rate_zone1", 70.0).toDouble() / 100)) / 220, QColor("lightsteelblue"));
                    backgroundGradient.setColorAt((220 - (maxHeartRate *
                    settings.value("heart_rate_zone2", 80.0).toDouble() / 100)) / 220, QColor("green"));
                    backgroundGradient.setColorAt((220 - (maxHeartRate *
                    settings.value("heart_rate_zone3", 90.0).toDouble() / 100)) / 220, QColor("yellow"));
                    backgroundGradient.setColorAt((220 - (maxHeartRate * settings.value("heart_rate_zone4",
                    100.0).toDouble() / 100)) / 220, QColor("orange")); backgroundGradient.setColorAt(0.0,
                    QColor("red"));*/

                    // backgroundGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
                    // chart->setBackgroundBrush(backgroundGradient);
                    // Customize plot area background
                    QLinearGradient plotAreaGradient;
                    plotAreaGradient.setStart(QPointF(0, 0));
                    plotAreaGradient.setFinalStop(QPointF(0, 1));
                    plotAreaGradient.setColorAt(
                        (220 -
                         (maxHeartRate * settings.value(QStringLiteral("heart_rate_zone1"), 70.0).toDouble() / 100)) /
                            160,
                        QColor(QStringLiteral("lightsteelblue")));
                    plotAreaGradient.setColorAt(
                        (220 -
                         (maxHeartRate * settings.value(QStringLiteral("heart_rate_zone2"), 80.0).toDouble() / 100)) /
                            160,
                        QColor(QStringLiteral("green")));
                    plotAreaGradient.setColorAt(
                        (220 -
                         (maxHeartRate * settings.value(QStringLiteral("heart_rate_zone3"), 90.0).toDouble() / 100)) /
                            160,
                        QColor(QStringLiteral("yellow")));
                    plotAreaGradient.setColorAt(
                        (220 -
                         (maxHeartRate * settings.value(QStringLiteral("heart_rate_zone4"), 100.0).toDouble() / 100)) /
                            160,
                        QColor(QStringLiteral("orange")));
                    plotAreaGradient.setColorAt(0.0, QColor(QStringLiteral("red")));
                    plotAreaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
                    chart->setPlotAreaBackgroundBrush(plotAreaGradient);
                    chart->setPlotAreaBackgroundVisible(true);
                }
            }
        }
    }

    Q_INVOKABLE void update_axes(QtCharts::QAbstractAxis *axisX, QtCharts::QAbstractAxis *axisY) {
        if (axisX && axisY) {
            // Customize axis colors
            QPen axisPen(QRgb(0xd18952));
            axisPen.setWidth(2);
            axisX->setLinePen(axisPen);
            axisY->setLinePen(axisPen);
            // Customize grid lines and shades
            axisY->setShadesPen(Qt::NoPen);
            axisY->setShadesBrush(QBrush(QColor(0x99, 0xcc, 0xcc, 0x55)));
        }
    }

    Q_INVOKABLE bool autoInclinationEnabled() {
        QSettings settings;
        bool virtual_bike = settings.value("virtual_device_force_bike", false).toBool();
        return bluetoothManager && bluetoothManager->device() &&
               bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL && !virtual_bike &&
               bluetoothManager->device()->VirtualDevice() &&
               ((virtualtreadmill *)bluetoothManager->device()->VirtualDevice())->autoInclinationEnabled();
    }

    homeform(QQmlApplicationEngine *engine, bluetooth *bl);
    ~homeform();
    int topBarHeight() { return m_topBarHeight; }
    QString info() { return m_info; }
    QString signal();
    QString startText();
    QString startIcon();
    QString startColor();
    QString stopText();
    QString stopIcon();
    QString stopColor();
    QString workoutStartDate() {
        if (!Session.isEmpty()) {
            return Session.constFirst().time.toString();
        } else {
            return QLatin1String("");
        }
    }
    QString workoutName() {
        if (!stravaPelotonActivityName.isEmpty()) {
            return stravaPelotonActivityName;
        } else if (!stravaWorkoutName.isEmpty()) {
            return stravaWorkoutName;
        } else {
            if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                return QStringLiteral("Ride");
            } else if (bluetoothManager->device() &&
                       bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                return QStringLiteral("Row");
            } else {
                return QStringLiteral("Run");
            }
        }
    }
    QString instructorName() { return stravaPelotonInstructorName; }
    int pelotonLogin() { return m_pelotonLoginState; }
    int pzpLogin() { return m_pzpLoginState; }
    bool pelotonAskStart() { return m_pelotonAskStart; }
    void setPelotonAskStart(bool value) { m_pelotonAskStart = value; }
    QString pelotonProvider() { return m_pelotonProvider; }
    void setPelotonProvider(const QString &value) { m_pelotonProvider = value; }
    bool generalPopupVisible();
    bool licensePopupVisible();
    bool mapsVisible();
    bool videoVisible();
    int videoPosition();
    double videoRate();
    double currentSpeed() {
        if (bluetoothManager && bluetoothManager->device())
            return bluetoothManager->device()->currentSpeed().value();
        else
            return 0;
    }
    QUrl videoPath() { return movieFileName; }
    bool labelHelp();
    QStringList metrics();
    QStringList bluetoothDevices();
    QStringList tile_order();
    bool autoResistance() { return m_autoresistance; }
    void setAutoResistance(bool value) {
        m_autoresistance = value;
        emit autoResistanceChanged(value);
        if (bluetoothManager->device()) {
            bluetoothManager->device()->setAutoResistance(value);
        }
    }
    void setLicensePopupVisible(bool value);
    void setVideoVisible(bool value);
    void setVideoPosition(int position);
    void setVideoRate(double rate);
    void setMapsVisible(bool value);
    void setGeneralPopupVisible(bool value);
    int workout_sample_points() { return Session.count(); }
    int preview_workout_points();

#if defined(Q_OS_ANDROID)
    static QString getAndroidDataAppDir();
#endif
    Q_INVOKABLE static QString getWritableAppDir();
    Q_INVOKABLE static QString getProfileDir();
    Q_INVOKABLE static void clearFiles();

    double wattMaxChart() {
        QSettings settings;
        if (bluetoothManager && bluetoothManager->device() &&
            bluetoothManager->device()->wattsMetric().max() >
                (settings.value(QStringLiteral("ftp"), 200.0).toDouble() * 2)) {
            return bluetoothManager->device()->wattsMetric().max();
        } else {
            return settings.value(QStringLiteral("ftp"), 200.0).toDouble() * 2;
        }
    }

    Q_INVOKABLE void sendMail();

    Q_INVOKABLE void sortTiles();
    Q_INVOKABLE void moveTile(QString name, int newIndex, int oldIndex);
    DataObject *tileFromName(QString name);

    QList<double> workout_watt_points() {
        QList<double> l;
        l.reserve(Session.size() + 1);
        for (const SessionLine &s : qAsConst(Session)) {
            l.append(s.watt);
        }
        return l;
    }
    QList<double> workout_heart_points() {
        QList<double> l;
        l.reserve(Session.size() + 1);
        for (const SessionLine &s : qAsConst(Session)) {
            l.append(s.heart);
        }
        return l;
    }
    QList<double> workout_cadence_points() {
        QList<double> l;
        l.reserve(Session.size() + 1);
        for (const SessionLine &s : qAsConst(Session)) {
            l.append(s.cadence);
        }
        return l;
    }
    QList<double> workout_resistance_points() {
        QList<double> l;
        l.reserve(Session.size() + 1);
        for (const SessionLine &s : qAsConst(Session)) {
            l.append(s.resistance);
        }
        return l;
    }
    QList<double> workout_peloton_resistance_points() {
        QList<double> l;
        l.reserve(Session.size() + 1);
        for (const SessionLine &s : qAsConst(Session)) {
            l.append(s.peloton_resistance);
        }
        return l;
    }

    QList<double> preview_workout_watt() {
        QList<double> l;
        if (!previewTrainProgram)
            return l;
        QTime d = previewTrainProgram->duration();
        l.reserve((d.hour() * 3600) + (d.minute() * 60) + d.second() + 1);
        foreach (trainrow r, previewTrainProgram->loadedRows) {
            for (int i = 0; i < (r.duration.hour() * 3600) + (r.duration.minute() * 60) + r.duration.second(); i++) {
                l.append(r.power);
            }
        }
        return l;
    }

    QString previewWorkoutDescription() {
        if (previewTrainProgram) {
            return previewTrainProgram->description;
        }
        return "";
    }

    QString previewWorkoutTags() {
        if (previewTrainProgram) {
            return previewTrainProgram->tags;
        }
        return "";
    }

    bool currentCoordinateValid() {
        if (bluetoothManager && bluetoothManager->device()) {
            return bluetoothManager->device()->currentCordinate().isValid();
        }
        return false;
    }

  private:
    QList<QObject *> dataList;
    QList<SessionLine> Session;
    bluetooth *bluetoothManager;
    QQmlApplicationEngine *engine;
    trainprogram *trainProgram = nullptr;
    trainprogram *previewTrainProgram = nullptr;
    QString backupFitFileName =
        QStringLiteral("QZ-backup-") +
        QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
        QStringLiteral(".fit");

    int m_topBarHeight = 120;
    QString m_info = QStringLiteral("Connecting...");
    bool m_labelHelp = true;
    bool m_generalPopupVisible = false;
    bool m_LicensePopupVisible = false;
    bool m_MapsVisible = false;
    bool m_VideoVisible = false;
    int m_VideoPosition = 0;
    double m_VideoRate = 1;
    QOAuth2AuthorizationCodeFlow *strava = nullptr;
    QNetworkAccessManager *manager = nullptr;
    QOAuthHttpServerReplyHandler *stravaReplyHandler = nullptr;

    bool paused = false;
    bool stopped = false;
    bool lapTrigger = false;

    peloton *pelotonHandler = nullptr;
    bool m_pelotonAskStart = false;
    QString m_pelotonProvider = "";
    int m_pelotonLoginState = -1;
    int m_pzpLoginState = -1;
    QString stravaPelotonActivityName;
    QString stravaPelotonInstructorName;
    QString stravaWorkoutName = "";
    QUrl movieFileName;
    FIT_SPORT stravaPelotonWorkoutType = FIT_SPORT_INVALID;
    QString activityDescription;
    QString pelotonAskedName = QStringLiteral("");
    QString pelotonAskedInstructor = QStringLiteral("");
    QString pelotonAbortedName = QStringLiteral("");
    QString pelotonAbortedInstructor = QStringLiteral("");

    QString lastFitFileSaved = QLatin1String("");

    QList<QString> chartImagesFilenames;

    bool m_autoresistance = true;

    DataObject *speed;
    DataObject *inclination;
    DataObject *cadence;
    DataObject *elevation;
    DataObject *calories;
    DataObject *odometer;
    DataObject *pace;
    DataObject *datetime;
    DataObject *resistance;
    DataObject *watt;
    DataObject *avgWatt;
    DataObject *heart;
    DataObject *fan;
    DataObject *jouls;
    DataObject *peloton_offset;
    DataObject *peloton_remaining;
    DataObject *elapsed;
    DataObject *moving_time;
    DataObject *peloton_resistance;
    DataObject *target_resistance;
    DataObject *target_peloton_resistance;
    DataObject *target_cadence;
    DataObject *target_power;
    DataObject *target_zone;
    DataObject *target_speed;
    DataObject *target_incline;
    DataObject *ftp;
    DataObject *lapElapsed;
    DataObject *weightLoss;
    DataObject *strokesLength;
    DataObject *strokesCount;
    DataObject *wattKg;
    DataObject *gears;
    DataObject *remaningTimeTrainingProgramCurrentRow;
    DataObject *nextRows;
    DataObject *mets;
    DataObject *targetMets;
    DataObject *steeringAngle;
    DataObject *pidHR;
    DataObject *extIncline;
    DataObject *instantaneousStrideLengthCM;
    DataObject *groundContactMS;
    DataObject *verticalOscillationMM;

    QTimer *timer;
    QTimer *backupTimer;

    QString strava_code;
    QOAuth2AuthorizationCodeFlow *strava_connect();
    void strava_refreshtoken();
    QNetworkReply *replyStrava;
    QAbstractOAuth::ModifyParametersFunction buildModifyParametersFunction(const QUrl &clientIdentifier,
                                                                           const QUrl &clientIdentifierSharedKey);
    bool strava_upload_file(const QByteArray &data, const QString &remotename);

    const QString cryptoKeySettingsProfilesTag = QStringLiteral("cryptoKeySettingsProfiles");
    quint64 cryptoKeySettingsProfiles();

    int16_t fanOverride = 0;

    void update();
    double heartRateMax();
    void backup();
    bool getDevice();
    bool getLap();
    void Start_inner(bool send_event_to_device);

    QTextToSpeech m_speech;
    int tts_summary_count = 0;

#if defined(Q_OS_WIN) || (defined(Q_OS_MAC) && !defined(Q_OS_IOS))
    QTimer tLicense;
#endif

    QGeoPath gpx_preview;
    PathController pathController;

  public slots:
    void aboutToQuit();
    void saveSettings(const QUrl &filename);
    void loadSettings(const QUrl &filename);
    void deleteSettings(const QUrl &filename);
    void saveProfile(QString profilename);
    void restart();

  private slots:
    void Start();
    void Stop();
    void Lap();
    void Minus(const QString &);
    void Plus(const QString &);
    void volumeDown();
    void volumeUp();
    void keyMediaPrevious();
    void keyMediaNext();
    void deviceFound(const QString &name);
    void deviceConnected(QBluetoothDeviceInfo b);
    void ftmsAccessoryConnected(smartspin2k *d);
    void trainprogram_open_clicked(const QUrl &fileName);
    void trainprogram_preview(const QUrl &fileName);
    void gpxpreview_open_clicked(const QUrl &fileName);
    void trainprogram_zwo_loaded(const QString &comp);
    void gpx_open_clicked(const QUrl &fileName);
    void gpx_save_clicked();
    void fit_save_clicked();
    void strava_connect_clicked();
    void trainProgramSignals();
    void refresh_bluetooth_devices_clicked();
    void onStravaGranted();
    void onStravaAuthorizeWithBrowser(const QUrl &url);
    void replyDataReceived(const QByteArray &v);
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &error);
    void networkRequestFinished(QNetworkReply *reply);
    void callbackReceived(const QVariantMap &values);
    void writeFileCompleted();
    void errorOccurredUploadStrava(QNetworkReply::NetworkError code);
    void pelotonWorkoutStarted(const QString &name, const QString &instructor);
    void pelotonWorkoutChanged(const QString &name, const QString &instructor);
    void pelotonLoginState(bool ok);
    void pzpLoginState(bool ok);
    void peloton_start_workout();
    void peloton_abort_workout();
    void smtpError(SmtpClient::SmtpError e);
    void setActivityDescription(QString newdesc);
    void chartSaved(QString fileName);
    void sortTilesTimeout();
    void gearUp();
    void gearDown();
    void changeTimestamp(QTime source, QTime actual);

#if defined(Q_OS_WIN) || (defined(Q_OS_MAC) && !defined(Q_OS_IOS))
    void licenseReply(QNetworkReply *reply);
    void licenseTimeout();
#endif

  signals:

    void changeOfdevice();
    void changeOflap();
    void signalChanged(QString value);
    void startTextChanged(QString value);
    void startIconChanged(QString value);
    void startColorChanged(QString value);
    void stopTextChanged(QString value);
    void stopIconChanged(QString value);
    void stopColorChanged(QString value);
    void infoChanged(QString value);
    void topBarHeightChanged(int value);
    void bluetoothDevicesChanged(QStringList value);
    void tile_orderChanged(QStringList value);
    void changeLabelHelp(bool value);
    void changePelotonAskStart(bool value);
    void changePelotonProvider(QString value);
    void generalPopupVisibleChanged(bool value);
    void licensePopupVisibleChanged(bool value);
    void videoVisibleChanged(bool value);
    void videoPositionChanged(int value);
    void videoRateChanged(double value);
    void currentSpeedChanged(double value);
    void mapsVisibleChanged(bool value);
    void autoResistanceChanged(bool value);
    void pelotonLoginChanged(int ok);
    void pzpLoginChanged(int ok);
    void workoutNameChanged(QString name);
    void workoutStartDateChanged(QString name);
    void instructorNameChanged(QString name);

    void previewWorkoutPointsChanged(int value);
    void previewWorkoutDescriptionChanged(QString value);
    void previewWorkoutTagsChanged(QString value);

    void workoutEventStateChanged(bluetoothdevice::WORKOUT_EVENT_STATE state);
};

#endif // HOMEFORM_H
