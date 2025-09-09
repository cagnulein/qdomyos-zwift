#ifndef HOMEFORM_H
#define HOMEFORM_H

#include "PathController.h"
#include "bluetooth.h"
#include "fit_profile.hpp"
#include "fitdatabaseprocessor.h"
#include "gpx.h"
#include "OAuth2.h"
#include "peloton.h"
#include "qmdnsengine/browser.h"
#include "qmdnsengine/cache.h"
#include "qmdnsengine/resolver.h"
#include "screencapture.h"
#include "sessionline.h"
#include "smtpclient/src/SmtpMime"
#include "trainprogram.h"
#include "workoutmodel.h"
#include <QChart>
#include <QColor>
#include <QGraphicsScene>
#include <QMediaPlayer>
#include <QNetworkReply>
#include <QOAuth2AuthorizationCodeFlow>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QTextToSpeech>

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif
#ifdef Q_OS_ANDROID
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#endif

#if __has_include("secret.h")
#include "secret.h"
#else
#define STRAVA_SECRET_KEY test
#if defined(WIN32)
#pragma message("DEFINE STRAVA_SECRET_KEY!!!")
#else
#warning "DEFINE STRAVA_SECRET_KEY!!!"
#endif
#endif

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
    Q_PROPERTY(bool largeButton READ largeButton NOTIFY largeButtonChanged)
    Q_PROPERTY(QString largeButtonColor READ largeButtonColor NOTIFY largeButtonColorChanged)
    Q_PROPERTY(QString largeButtonLabel READ largeButtonLabel NOTIFY largeButtonLabelChanged)
    Q_PROPERTY(QString plusName READ plusName NOTIFY plusNameChanged)
    Q_PROPERTY(QString minusName READ minusName NOTIFY minusNameChanged)
    Q_PROPERTY(QString identificator READ identificator NOTIFY identificatorChanged)

  public:
    DataObject(const QString &name, const QString &icon, const QString &value, bool writable, const QString &id,
               int valueFontSize, int labelFontSize, const QString &valueFontColor = QStringLiteral("white"),
               const QString &secondLine = QLatin1String(""), const int gridId = 0, const bool largeButton = false,
               const QString largeButtonLabel = QLatin1String(""),
               const QString largeButtonColor = QZSettings::default_tile_preset_resistance_1_color);
    void setName(const QString &value);
    void setValue(const QString &value);
    void setSecondLine(const QString &value);
    void setValueFontSize(int value);
    void setValueFontColor(const QString &value);
    void setLabelFontSize(int value);
    void setVisible(bool visible);
    void setGridId(int id);
    void setLargeButtonColor(const QString &color);
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
    bool largeButton() { return m_largeButton; }
    QString largeButtonLabel() { return m_largeButtonLabel; }
    QString largeButtonColor() { return m_largeButtonColor; }

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
    bool m_largeButton = false;
    QString m_largeButtonLabel = QLatin1String("");
    QString m_largeButtonColor = QZSettings::default_tile_preset_resistance_1_color;

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
    void identificatorChanged(QString value);
    void largeButtonChanged(bool value);
    void largeButtonLabelChanged(QString value);
    void largeButtonColorChanged(QString value);
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
    Q_PROPERTY(bool pelotonPopupVisible READ pelotonPopupVisible NOTIFY pelotonPopupVisibleChanged WRITE
                   setPelotonPopupVisible)
    Q_PROPERTY(bool licensePopupVisible READ licensePopupVisible NOTIFY licensePopupVisibleChanged WRITE
                   setLicensePopupVisible)
    Q_PROPERTY(bool mapsVisible READ mapsVisible NOTIFY mapsVisibleChanged WRITE setMapsVisible)
    Q_PROPERTY(bool videoIconVisible READ videoIconVisible NOTIFY videoIconVisibleChanged WRITE setVideoIconVisible)
    Q_PROPERTY(bool videoVisible READ videoVisible NOTIFY videoVisibleChanged WRITE setVideoVisible)
    Q_PROPERTY(bool chartIconVisible READ chartIconVisible NOTIFY chartIconVisibleChanged WRITE setChartIconVisible)
    Q_PROPERTY(
        bool chartFooterVisible READ chartFooterVisible NOTIFY chartFooterVisibleChanged WRITE setChartFooterVisible)
    Q_PROPERTY(QUrl videoPath READ videoPath NOTIFY videoPathChanged)
    Q_PROPERTY(int videoPosition READ videoPosition NOTIFY videoPositionChanged WRITE setVideoPosition)
    Q_PROPERTY(double videoRate READ videoRate NOTIFY videoRateChanged WRITE setVideoRate)
    Q_PROPERTY(double currentSpeed READ currentSpeed NOTIFY currentSpeedChanged)
    Q_PROPERTY(int pelotonLogin READ pelotonLogin NOTIFY pelotonLoginChanged)
    Q_PROPERTY(int pzpLogin READ pzpLogin NOTIFY pzpLoginChanged)
    Q_PROPERTY(int zwiftLogin READ zwiftLogin NOTIFY zwiftLoginChanged)
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
    Q_PROPERTY(bool stopRequested READ stopRequested NOTIFY stopRequestedChanged WRITE setStopRequestedChanged)
    Q_PROPERTY(bool startRequested READ startRequested NOTIFY startRequestedChanged WRITE setStartRequestedChanged)
    Q_PROPERTY(QString toastRequested READ toastRequested NOTIFY toastRequestedChanged WRITE setToastRequested)
    Q_PROPERTY(bool stravaUploadRequested READ stravaUploadRequested NOTIFY stravaUploadRequestedChanged WRITE setStravaUploadRequested)

    // workout preview
    Q_PROPERTY(int preview_workout_points READ preview_workout_points NOTIFY previewWorkoutPointsChanged)
    Q_PROPERTY(QList<double> preview_workout_watt READ preview_workout_watt)
    Q_PROPERTY(QString previewWorkoutDescription READ previewWorkoutDescription NOTIFY previewWorkoutDescriptionChanged)
    Q_PROPERTY(QString previewWorkoutTags READ previewWorkoutTags NOTIFY previewWorkoutTagsChanged)

    Q_PROPERTY(bool currentCoordinateValid READ currentCoordinateValid)
    Q_PROPERTY(bool trainProgramLoadedWithVideo READ trainProgramLoadedWithVideo)

    Q_PROPERTY(QString getStravaAuthUrl READ getStravaAuthUrl NOTIFY stravaAuthUrlChanged)
    Q_PROPERTY(bool stravaWebVisible READ stravaWebVisible NOTIFY stravaWebVisibleChanged)

    QString getPelotonAuthUrl() { if(!pelotonHandler) return ""; return pelotonHandler->pelotonAuthUrl; }
    bool pelotonWebVisible() { if(!pelotonHandler) return false; return pelotonHandler->pelotonAuthWebVisible; }
    Q_PROPERTY(QString getPelotonAuthUrl READ getPelotonAuthUrl NOTIFY pelotonAuthUrlChanged)
    Q_PROPERTY(bool pelotonWebVisible READ pelotonWebVisible NOTIFY pelotonWebVisibleChanged)

  public:
    static homeform *singleton() { return m_singleton; }
    bluetooth *bluetoothManager;

    QByteArray currentPelotonImage();
    Q_INVOKABLE void save_screenshot() {

        QString path = getWritableAppDir();

        QString filenameScreenshot =
            path + QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
            QStringLiteral(".jpg");
        QObject *rootObject = engine->rootObjects().constFirst();
        QObject *stack = rootObject;
        screenCapture s(reinterpret_cast<QQuickView *>(stack));
        s.capture(filenameScreenshot);
        chartImagesFilenames.append(filenameScreenshot);
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
                    double ftpSetting = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
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
                    settings.value(QZSettings::heart_rate_zone1, QZSettings::default_heart_rate_zone1).toDouble() /
                    100)) / 220, QColor("lightsteelblue")); backgroundGradient.setColorAt((220 - (maxHeartRate *
                    settings.value(QZSettings::heart_rate_zone2, QZSettings::default_heart_rate_zone2).toDouble() /
                    100)) / 220, QColor("green")); backgroundGradient.setColorAt((220 - (maxHeartRate *
                    settings.value(QZSettings::heart_rate_zone3, QZSettings::default_heart_rate_zone3).toDouble() /
                    100)) / 220, QColor("yellow")); backgroundGradient.setColorAt((220 - (maxHeartRate *
                    settings.value(QZSettings::heart_rate_zone4, QZSettings::default_heart_rate_zone4).toDouble() /
                    100)) / 220, QColor("orange")); backgroundGradient.setColorAt(0.0, QColor("red")); */

                    // backgroundGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
                    // chart->setBackgroundBrush(backgroundGradient);
                    // Customize plot area background
                    QLinearGradient plotAreaGradient;
                    plotAreaGradient.setStart(QPointF(0, 0));
                    plotAreaGradient.setFinalStop(QPointF(0, 1));
                    plotAreaGradient.setColorAt(
                        (220 - (maxHeartRate *
                                settings.value(QZSettings::heart_rate_zone1, QZSettings::default_heart_rate_zone1)
                                    .toDouble() /
                                100)) /
                            160,
                        QColor(QStringLiteral("lightsteelblue")));
                    plotAreaGradient.setColorAt(
                        (220 - (maxHeartRate *
                                settings.value(QZSettings::heart_rate_zone2, QZSettings::default_heart_rate_zone2)
                                    .toDouble() /
                                100)) /
                            160,
                        QColor(QStringLiteral("green")));
                    plotAreaGradient.setColorAt(
                        (220 - (maxHeartRate *
                                settings.value(QZSettings::heart_rate_zone3, QZSettings::default_heart_rate_zone3)
                                    .toDouble() /
                                100)) /
                            160,
                        QColor(QStringLiteral("yellow")));
                    plotAreaGradient.setColorAt(
                        (220 - (maxHeartRate *
                                settings.value(QZSettings::heart_rate_zone4, QZSettings::default_heart_rate_zone4)
                                    .toDouble() /
                                100)) /
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

    Q_INVOKABLE bool firstRun() {
        QSettings settings;
        
        bool android_antbike = settings.value(QZSettings::android_antbike, QZSettings::default_android_antbike).toBool();
        QString proformtdf4ip = settings.value(QZSettings::proformtdf4ip, QZSettings::default_proformtdf4ip).toString();
        QString proformtdf1ip = settings.value(QZSettings::proformtdf1ip, QZSettings::default_proformtdf1ip).toString();
        QString proformtreadmillip = settings.value(QZSettings::proformtreadmillip, QZSettings::default_proformtreadmillip).toString();

        QString nordictrack_2950_ip =
            settings.value(QZSettings::nordictrack_2950_ip, QZSettings::default_nordictrack_2950_ip).toString();
        QString tdf_10_ip = settings.value(QZSettings::tdf_10_ip, QZSettings::default_tdf_10_ip).toString();
        QString proform_elliptical_ip = settings.value(QZSettings::proform_elliptical_ip, QZSettings::default_proform_elliptical_ip).toString();
        bool fake_bike =
            settings.value(QZSettings::applewatch_fakedevice, QZSettings::default_applewatch_fakedevice).toBool();
        bool fakedevice_elliptical =
            settings.value(QZSettings::fakedevice_elliptical, QZSettings::default_fakedevice_elliptical).toBool();
        bool fakedevice_rower = settings.value(QZSettings::fakedevice_rower, QZSettings::default_fakedevice_rower).toBool();
        bool fakedevice_treadmill =
            settings.value(QZSettings::fakedevice_treadmill, QZSettings::default_fakedevice_treadmill).toBool();
        bool antbike =
            settings.value(QZSettings::antbike, QZSettings::default_antbike).toBool();

        return settings.value(QZSettings::bluetooth_lastdevice_name, QZSettings::default_bluetooth_lastdevice_name).toString().isEmpty() && 
                nordictrack_2950_ip.isEmpty() && tdf_10_ip.isEmpty() && !fake_bike && !fakedevice_elliptical &&
                !fakedevice_rower && !fakedevice_treadmill && !antbike && !android_antbike && proform_elliptical_ip.isEmpty() && 
                proformtdf4ip.isEmpty() && proformtdf1ip.isEmpty() && proformtreadmillip.isEmpty();
    }


    Q_INVOKABLE bool autoInclinationEnabled() {
        QSettings settings;
        bool virtual_bike =
            settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                .toBool();
        return bluetoothManager && bluetoothManager->device() &&
               bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL && !virtual_bike &&
               bluetoothManager->device()->VirtualDevice() &&
               ((virtualtreadmill *)bluetoothManager->device()->VirtualDevice())->autoInclinationEnabled();
    }

    Q_INVOKABLE bool locationServices() {
        return m_locationServices;
    }

    Q_INVOKABLE void enableLocationServices() {
#ifdef Q_OS_ANDROID
        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/LocationHelper", "requestPermissions",
                                                  "(Landroid/content/Context;)V", QtAndroid::androidContext().object());
#endif
    }

    homeform(QQmlApplicationEngine *engine, bluetooth *bl);
    ~homeform();
    int topBarHeight() { return m_topBarHeight; }
    bool stopRequested() { return m_stopRequested; }
    bool startRequested() { return m_startRequested; }
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
    QString workoutNameBasedOnBluetoothDevice() {
        if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
            return QStringLiteral("Ride");
        } else if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
            return QStringLiteral("Row");
        } else {
            return QStringLiteral("Run");
        }
    }
    QString workoutName() {
        if (!stravaPelotonActivityName.isEmpty()) {
            return stravaPelotonActivityName;
        } else if (!stravaWorkoutName.isEmpty()) {
            return stravaWorkoutName;
        } else {
            return workoutNameBasedOnBluetoothDevice();
        }
    }
    QString instructorName() { return stravaPelotonInstructorName; }
    int pelotonLogin() { return m_pelotonLoginState; }
    int pzpLogin() { return m_pzpLoginState; }
    int zwiftLogin() { return m_zwiftLoginState; }
    void setPelotonAskStart(bool value) { m_pelotonAskStart = value; }
    QString pelotonProvider() { return m_pelotonProvider; }
    QString toastRequested() { return m_toastRequested; }
    bool stravaUploadRequested() { return m_stravaUploadRequested; }
    void setPelotonProvider(const QString &value) { m_pelotonProvider = value; }
    bool generalPopupVisible();
    bool pelotonPopupVisible();
    bool licensePopupVisible();
    bool mapsVisible();
    bool videoIconVisible();
    bool videoVisible() { return m_VideoVisible; }
    bool chartIconVisible();
    bool chartFooterVisible() { return m_ChartFooterVisible; }
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
    void setStopRequestedChanged(bool value) {
        m_stopRequested = value;
        emit stopRequestedChanged(value);
    }
    void setStartRequestedChanged(bool value) {
        m_startRequested = value;
        emit startRequestedChanged(value);
    }
    void setLicensePopupVisible(bool value);
    void setVideoIconVisible(bool value);
    void setChartIconVisible(bool value);
    void setVideoVisible(bool value) {
        m_VideoVisible = value;
        emit videoVisibleChanged(m_VideoVisible);
    }
    void setChartFooterVisible(bool value) {
        m_ChartFooterVisible = value;
        emit chartFooterVisibleChanged(m_ChartFooterVisible);
    }
    void setVideoPosition(int position); // on startup
    void videoSeekPosition(int ms);      // in realtime
    void setVideoRate(double rate);
    void setMapsVisible(bool value);
    void setToastRequested(QString value) { m_toastRequested = value; emit toastRequestedChanged(value); }
    void setStravaUploadRequested(bool value) {
        m_stravaUploadRequested = value;
    }
    void setGeneralPopupVisible(bool value);
    void setPelotonPopupVisible(bool value);
    int workout_sample_points() { return Session.count(); }
    int preview_workout_points();

#if defined(Q_OS_ANDROID)
    QString getBluetoothName();
    static QString getAndroidDataAppDir();
#endif
    Q_INVOKABLE static QString getWritableAppDir();
    Q_INVOKABLE static QString getProfileDir();
    Q_INVOKABLE static void clearFiles();

    double wattMaxChart() {
        QSettings settings;
        if (bluetoothManager && bluetoothManager->device() &&
            bluetoothManager->device()->wattsMetric().max() >
                (settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 2)) {
            return bluetoothManager->device()->wattsMetric().max();
        } else {
            return settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble() * 2;
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

    bool trainProgramLoadedWithVideo() { return (trainProgram && trainProgram->videoAvailable); }

    QString getStravaAuthUrl() { return stravaAuthUrl; }
    bool stravaWebVisible() { return stravaAuthWebVisible; }
    trainprogram *trainingProgram() { return trainProgram; }
    void updateGearsValue();
    
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
    DataObject *avgWattLap;
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
    DataObject *target_pace;
    DataObject *target_incline;
    DataObject *ftp;
    DataObject *lapElapsed;
    DataObject *weightLoss;
    DataObject *strokesLength;
    DataObject *strokesCount;
    DataObject *wattKg;
    DataObject *gears;
    DataObject *biggearsPlus;
    DataObject *biggearsMinus;
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
    DataObject *preset_resistance_1;
    DataObject *preset_resistance_2;
    DataObject *preset_resistance_3;
    DataObject *preset_resistance_4;
    DataObject *preset_resistance_5;
    DataObject *preset_speed_1;
    DataObject *preset_speed_2;
    DataObject *preset_speed_3;
    DataObject *preset_speed_4;
    DataObject *preset_speed_5;
    DataObject *preset_inclination_1;
    DataObject *preset_inclination_2;
    DataObject *preset_inclination_3;
    DataObject *preset_inclination_4;
    DataObject *preset_inclination_5;
    DataObject *pace_last500m;
    DataObject *stepCount;
    DataObject *ergMode;
    DataObject *rss;
    DataObject *preset_powerzone_1;
    DataObject *preset_powerzone_2;
    DataObject *preset_powerzone_3;
    DataObject *preset_powerzone_4;
    DataObject *preset_powerzone_5;
    DataObject *preset_powerzone_6;
    DataObject *preset_powerzone_7;
    DataObject *tile_hr_time_in_zone_1;
    DataObject *tile_hr_time_in_zone_2;
    DataObject *tile_hr_time_in_zone_3;
    DataObject *tile_hr_time_in_zone_4;
    DataObject *tile_hr_time_in_zone_5;
    DataObject *tile_heat_time_in_zone_1;
    DataObject *tile_heat_time_in_zone_2;
    DataObject *tile_heat_time_in_zone_3;
    DataObject *tile_heat_time_in_zone_4;
    DataObject *coreTemperature;
    DataObject *autoVirtualShiftingCruise;
    DataObject *autoVirtualShiftingClimb;
    DataObject *autoVirtualShiftingSprint;

  private:
    static homeform *m_singleton;
    TemplateInfoSenderBuilder *userTemplateManager = nullptr;
    TemplateInfoSenderBuilder *innerTemplateManager = nullptr;
    QList<QObject *> dataList;
    QList<SessionLine> Session;
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
    bool m_pelotonPopupVisible = false;
    bool m_LicensePopupVisible = false;
    bool m_MapsVisible = false;
    bool m_VideoIconVisible = false;
    bool m_VideoVisible = false;
    bool m_ChartFooterVisible = false;
    bool m_ChartIconVisible = false;
    int m_VideoPosition = 0;
    double m_VideoRate = 1;
    QOAuth2AuthorizationCodeFlow *strava = nullptr;
    QNetworkAccessManager *manager = nullptr;
    QOAuthHttpServerReplyHandler *stravaReplyHandler = nullptr;

    bool paused = false;
    bool stopped = false;
    bool lapTrigger = false;

    // Automatic Virtual Shifting variables
    QDateTime automaticShiftingGearUpStartTime = QDateTime::currentDateTime();
    QDateTime automaticShiftingGearDownStartTime = QDateTime::currentDateTime();

    // Timer jitter detection variables (same logic as trainprogram::scheduler)
    QDateTime lastUpdateCall = QDateTime::currentDateTime();
    qint64 currentUpdateJitter = 0;

    peloton *pelotonHandler = nullptr;
    bool m_pelotonAskStart = false;
    QString m_pelotonProvider = "";
    QString m_toastRequested = "";
    bool m_stravaUploadRequested = false;
    FitDatabaseProcessor *fitProcessor = nullptr;
    WorkoutModel *workoutModel = nullptr;
    int m_pelotonLoginState = -1;
    int m_pzpLoginState = -1;
    int m_zwiftLoginState = -1;
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
    QString lastTrainProgramFileSaved = QLatin1String("");

    QList<QString> chartImagesFilenames;

    bool m_autoresistance = true;
    bool m_stopRequested = false;
    bool m_startRequested = false;
    bool m_overridePower = false;

    QTimer *timer;
    QTimer *backupTimer;
    QTimer *automaticShiftingTimer;

    QString strava_code;
    QOAuth2AuthorizationCodeFlow *strava_connect();
    void strava_refreshtoken();
    QNetworkReply *replyStrava;
    QAbstractOAuth::ModifyParametersFunction buildModifyParametersFunction(const QUrl &clientIdentifier,
                                                                           const QUrl &clientIdentifierSharedKey);
    bool strava_upload_file(const QByteArray &data, const QString &remotename);
    QString stravaAuthUrl;
    bool stravaAuthWebVisible;

    static quint64 cryptoKeySettingsProfiles();

    static QString copyAndroidContentsURI(QUrl file, QString subfolder);
    static QString getFileNameFromContentUri(const QString &uriString);

    int16_t fanOverride = 0;

    void update();
    void ten_hz();
    double heartRateMax();
    void backup();
    bool getDevice();
    bool getLap();
    void Start_inner(bool send_event_to_device);

    QTextToSpeech m_speech;
    int tts_summary_count = 0;

#if defined(Q_OS_WIN) || (defined(Q_OS_MAC) && !defined(Q_OS_IOS)) || (defined(Q_OS_ANDROID) && defined(LICENSE))
    QTimer tLicense;
    QNetworkAccessManager *mgr = nullptr;
    void licenseRequest();
#endif

    QGeoPath gpx_preview;
    PathController pathController;
    bool videoMustBeReset = true;

#ifdef Q_OS_ANDROID
    bool floating_open = false;    
#endif

#ifdef Q_OS_IOS
    lockscreen *h = nullptr;
#endif
    bool m_locationServices = true;

#ifndef Q_OS_IOS
    QMdnsEngine::Browser *iphone_browser = nullptr;
    QMdnsEngine::Resolver *iphone_resolver = nullptr;
    QMdnsEngine::Server iphone_server;
    QMdnsEngine::Cache iphone_cache;
    QTcpSocket *iphone_socket = nullptr;
    QMdnsEngine::Service iphone_service;
    QHostAddress iphone_address;
    
    // Server for sharing metrics with other QZ instances (fakebike clients)
    QTcpServer *metrics_server = nullptr;
    QMdnsEngine::Provider *iphone_mdns_provider = nullptr;
    QMdnsEngine::Hostname *iphone_mdns_hostname = nullptr;
    QList<QTcpSocket*> metrics_clients;
#endif

  public slots:
    void aboutToQuit();
    void saveSettings(const QUrl &filename);
    static void loadSettings(const QUrl &filename);
    void deleteSettings(const QUrl &filename);
    void restoreSettings();
    void saveProfile(QString profilename);
    void restart();
    bool pelotonAskStart() { return m_pelotonAskStart; }
    void Minus(const QString &);
    void Plus(const QString &);
    void trainprogram_open_clicked(const QUrl &fileName);

  private slots:
    void Start();
    void Stop();
    void StopFromTrainProgram(bool paused);
    void StartRequested();
    void StopRequested();
    void Lap();
    void LargeButton(const QString &);
    void volumeDown();
    void volumeUp();
    void keyMediaPrevious();
    void keyMediaNext();
    void floatingOpen();
    void openFloatingWindowBrowser();
    void deviceFound(const QString &name);
    void deviceConnected(QBluetoothDeviceInfo b);
    void ftmsAccessoryConnected(smartspin2k *d);    
    void trainprogram_open_other_folder(const QUrl &fileName);
    void gpx_open_other_folder(const QUrl &fileName);
    void profile_open_clicked(const QUrl &fileName);
    void trainprogram_preview(const QUrl &fileName);
    void gpxpreview_open_clicked(const QUrl &fileName);
    void fitfile_preview_clicked(const QUrl &fileName);
    void trainprogram_zwo_loaded(const QString &comp);
    void gpx_open_clicked(const QUrl &fileName);
    void gpx_save_clicked();
    void fit_save_clicked();
    void saveSessionAsTrainingProgram();
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
    void zwiftLoginState(bool ok);
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
    void pelotonOffset_Plus();
    void pelotonOffset_Minus();
    int pelotonOffset() { return (trainProgram ? trainProgram->offsetElapsedTime() : 0); }
    void bluetoothDeviceConnected(bluetoothdevice *b);
    void bluetoothDeviceDisconnected();
    void onToastRequested(QString message);
    void strava_upload_file_prepare();
    void handleRestoreDefaultWheelDiameter();

#ifndef Q_OS_IOS
    void initMetricsServer();
    void metricsClientConnected();
    void metricsClientDisconnected();
    void sendMetricsToClients();
#endif

#if defined(Q_OS_WIN) || (defined(Q_OS_MAC) && !defined(Q_OS_IOS)) || (defined(Q_OS_ANDROID) && defined(LICENSE))
    void licenseReply(QNetworkReply *reply);
    void licenseTimeout();
#endif

    void toggleAutoResistance() { setAutoResistance(!autoResistance()); }

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
    void toastRequestedChanged(QString value);
    void stravaUploadRequestedChanged(bool value);
    void generalPopupVisibleChanged(bool value);
    void pelotonPopupVisibleChanged(bool value);
    void licensePopupVisibleChanged(bool value);
    void videoIconVisibleChanged(bool value);
    void videoVisibleChanged(bool value);
    void videoPositionChanged(int value);
    void videoPathChanged(QUrl value);
    void videoRateChanged(double value);
    void chartIconVisibleChanged(bool value);
    void chartFooterVisibleChanged(bool value);
    void currentSpeedChanged(double value);
    void mapsVisibleChanged(bool value);
    void autoResistanceChanged(bool value);
    void pelotonLoginChanged(int ok);    
    void pzpLoginChanged(int ok);
    void zwiftLoginChanged(int ok);
    void userProfileChanged();
    void workoutNameChanged(QString name);
    void workoutStartDateChanged(QString name);
    void instructorNameChanged(QString name);
    void startRequestedChanged(bool value);
    void stopRequestedChanged(bool value);

    void previewWorkoutPointsChanged(int value);
    void previewWorkoutDescriptionChanged(QString value);
    void previewWorkoutTagsChanged(QString value);

    void previewFitFile(const QString &filename, const QString &result, const QString &workoutName);

    void stravaAuthUrlChanged(QString value);
    void stravaWebVisibleChanged(bool value);
    void pelotonAuthUrlChanged(QString value);
    void pelotonWebVisibleChanged(bool value);

    void restoreDefaultWheelDiameter();

    void workoutEventStateChanged(bluetoothdevice::WORKOUT_EVENT_STATE state);

    void heartRate(uint8_t heart);
};

#endif // HOMEFORM_H
