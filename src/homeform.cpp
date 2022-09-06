#include "homeform.h"
#include "ios/lockscreen.h"
#include "keepawakehelper.h"
#include "material.h"
#include "qfit.h"
#include "simplecrypt.h"
#include "templateinfosenderbuilder.h"
#include "zwiftworkout.h"

#include <QAbstractOAuth2>
#include <QApplication>
#include <QByteArray>
#include <QDesktopServices>
#include <QFileInfo>
#include <QGeoCoordinate>
#include <QHttpMultiPart>
#include <QImageWriter>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QQmlContext>
#include <QQmlFile>

#include <QRandomGenerator>
#include <QSettings>
#include <QStandardPaths>
#include <QTime>
#include <QUrlQuery>
#include <chrono>

using namespace std::chrono_literals;

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

#ifndef STRAVA_CLIENT_ID
#define STRAVA_CLIENT_ID 7976
#if defined(WIN32)
#pragma message("DEFINE STRAVA_CLIENT_ID!!!")
#else
#warning "DEFINE STRAVA_CLIENT_ID!!!"
#endif
#endif
#define _STR(x) #x
#define STRINGIFY(x) _STR(x)
#define STRAVA_CLIENT_ID_S STRINGIFY(STRAVA_CLIENT_ID)

DataObject::DataObject(const QString &name, const QString &icon, const QString &value, bool writable, const QString &id,
                       int valueFontSize, int labelFontSize, const QString &valueFontColor, const QString &secondLine,
                       const int gridId) {
    m_name = name;
    m_icon = icon;
    m_value = value;
    m_secondLine = secondLine;
    m_writable = writable;
    m_id = id;
    m_valueFontSize = valueFontSize;
    m_valueFontColor = valueFontColor;
    m_labelFontSize = labelFontSize;
    m_gridId = gridId;

    emit plusNameChanged(plusName());   // NOTE: clazy-incorrecrt-emit
    emit minusNameChanged(minusName()); // NOTE: clazy-incorrecrt-emit
}

void DataObject::setName(const QString &v) {
    m_name = v;
    emit nameChanged(m_name);
}
void DataObject::setValue(const QString &v) {
    m_value = v;
    emit valueChanged(m_value);
}
void DataObject::setSecondLine(const QString &value) {
    m_secondLine = value;
    emit secondLineChanged(m_secondLine);
}
void DataObject::setValueFontSize(int value) {
    m_valueFontSize = value;
    emit valueFontSizeChanged(m_valueFontSize);
}
void DataObject::setValueFontColor(const QString &value) {
    m_valueFontColor = value;
    emit valueFontColorChanged(m_valueFontColor);
}
void DataObject::setLabelFontSize(int value) {
    m_labelFontSize = value;
    emit labelFontSizeChanged(m_labelFontSize);
}
void DataObject::setGridId(int id) {
    m_gridId = id;
    emit gridIdChanged(m_gridId);
}
void DataObject::setVisible(bool visible) {
    m_visible = visible;
    emit visibleChanged(m_visible);
}

homeform::homeform(QQmlApplicationEngine *engine, bluetooth *bl) {

    QSettings settings;
    bool miles = settings.value(QStringLiteral("miles_unit"), false).toBool();
    QString unit = QStringLiteral("km");
    QString meters = QStringLiteral("m");
    QString weightLossUnit = QStringLiteral("Kg");
    if (miles) {
        unit = QStringLiteral("mi");
        weightLossUnit = QStringLiteral("Oz");
        meters = QStringLiteral("ft");
    }

#ifdef Q_OS_IOS
    const int labelFontSize = 14;
    const int valueElapsedFontSize = 36;
    const int valueTimeFontSize = 26;
#elif defined Q_OS_ANDROID
    const int labelFontSize = 16;
    const int valueElapsedFontSize = 36;
    const int valueTimeFontSize = 26;
#else
    const int labelFontSize = 10;
    const int valueElapsedFontSize = 30;
    const int valueTimeFontSize = 22;
#endif
    speed = new DataObject(QStringLiteral("Speed (") + unit + QStringLiteral("/h)"),
                           QStringLiteral("icons/icons/speed.png"), QStringLiteral("0.0"), true,
                           QStringLiteral("speed"), 48, labelFontSize);
    inclination = new DataObject(QStringLiteral("Inclination (%)"), QStringLiteral("icons/icons/inclination.png"),
                                 QStringLiteral("0.0"), true, QStringLiteral("inclination"), 48, labelFontSize);
    cadence = new DataObject(QStringLiteral("Cadence (rpm)"), QStringLiteral("icons/icons/cadence.png"),
                             QStringLiteral("0"), false, QStringLiteral("cadence"), 48, labelFontSize);
    elevation = new DataObject(QStringLiteral("Elev. Gain (") + meters + QStringLiteral(")"),
                               QStringLiteral("icons/icons/elevationgain.png"), QStringLiteral("0"), false,
                               QStringLiteral("elevation"), 48, labelFontSize);
    calories = new DataObject(QStringLiteral("Calories (KCal)"), QStringLiteral("icons/icons/kcal.png"),
                              QStringLiteral("0"), false, QStringLiteral("calories"), 48, labelFontSize);
    odometer = new DataObject(QStringLiteral("Odometer (") + unit + QStringLiteral(")"),
                              QStringLiteral("icons/icons/odometer.png"), QStringLiteral("0.0"), false,
                              QStringLiteral("odometer"), 48, labelFontSize);
    pace =
        new DataObject(QStringLiteral("Pace (m/") + unit + QStringLiteral(")"), QStringLiteral("icons/icons/pace.png"),
                       QStringLiteral("0:00"), false, QStringLiteral("pace"), 48, labelFontSize);
    resistance = new DataObject(QStringLiteral("Resistance"), QStringLiteral("icons/icons/resistance.png"),
                                QStringLiteral("0"), true, QStringLiteral("resistance"), 48, labelFontSize);
    peloton_resistance =
        new DataObject(QStringLiteral("Peloton R(%)"), QStringLiteral("icons/icons/resistance.png"),
                       QStringLiteral("0"), true, QStringLiteral("peloton_resistance"), 48, labelFontSize);
    target_resistance =
        new DataObject(QStringLiteral("Target R."), QStringLiteral("icons/icons/resistance.png"), QStringLiteral("0"),
                       true, QStringLiteral("target_resistance"), 48, labelFontSize);
    target_peloton_resistance =
        new DataObject(QStringLiteral("T.Peloton R(%)"), QStringLiteral("icons/icons/resistance.png"),
                       QStringLiteral("0"), false, QStringLiteral("target_peloton_resistance"), 48, labelFontSize);
    target_cadence = new DataObject(QStringLiteral("T.Cadence(rpm)"), QStringLiteral("icons/icons/cadence.png"),
                                    QStringLiteral("0"), false, QStringLiteral("target_cadence"), 48, labelFontSize);
    target_power = new DataObject(QStringLiteral("T.Power(W)"), QStringLiteral("icons/icons/watt.png"),
                                  QStringLiteral("0"), false, QStringLiteral("target_power"), 48, labelFontSize);
    target_zone = new DataObject(QStringLiteral("T.Zone"), QStringLiteral("icons/icons/watt.png"), QStringLiteral("1"),
                                 false, QStringLiteral("target_zone"), 48, labelFontSize);
    target_speed = new DataObject(QStringLiteral("T.Speed (") + unit + QStringLiteral("/h)"),
                                  QStringLiteral("icons/icons/speed.png"), QStringLiteral("0.0"), false,
                                  QStringLiteral("target_speed"), 48, labelFontSize);
    target_incline =
        new DataObject(QStringLiteral("T.Incline (%)"), QStringLiteral("icons/icons/inclination.png"),
                       QStringLiteral("0.0"), false, QStringLiteral("target_inclination"), 48, labelFontSize);

    watt = new DataObject(QStringLiteral("Watt"), QStringLiteral("icons/icons/watt.png"), QStringLiteral("0"), false,
                          QStringLiteral("watt"), 48, labelFontSize);
    weightLoss = new DataObject(QStringLiteral("Weight Loss(") + weightLossUnit + QStringLiteral(")"),
                                QStringLiteral("icons/icons/kcal.png"), QStringLiteral("0"), false,
                                QStringLiteral("weight_loss"), 48, labelFontSize);
    avgWatt = new DataObject(QStringLiteral("AVG Watt"), QStringLiteral("icons/icons/watt.png"), QStringLiteral("0"),
                             false, QStringLiteral("avgWatt"), 48, labelFontSize);
    wattKg = new DataObject(QStringLiteral("Watt/Kg"), QStringLiteral("icons/icons/watt.png"), QStringLiteral("0"),
                            false, QStringLiteral("watt_kg"), 48, labelFontSize);
    ftp = new DataObject(QStringLiteral("FTP Zone"), QStringLiteral("icons/icons/watt.png"), QStringLiteral("0"), false,
                         QStringLiteral("ftp"), 48, labelFontSize);
    heart = new DataObject(QStringLiteral("Heart (bpm)"), QStringLiteral("icons/icons/heart_red.png"),
                           QStringLiteral("0"), false, QStringLiteral("heart"), 48, labelFontSize);
    fan = new DataObject(QStringLiteral("Fan Speed"), QStringLiteral("icons/icons/fan.png"), QStringLiteral("0"), true,
                         QStringLiteral("fan"), 48, labelFontSize);
    jouls = new DataObject(QStringLiteral("KJouls"), QStringLiteral("icons/icons/joul.png"), QStringLiteral("0"), false,
                           QStringLiteral("joul"), 48, labelFontSize);
    elapsed =
        new DataObject(QStringLiteral("Elapsed"), QStringLiteral("icons/icons/clock.png"), QStringLiteral("0:00:00"),
                       false, QStringLiteral("elapsed"), valueElapsedFontSize, labelFontSize);
    moving_time =
        new DataObject(QStringLiteral("Moving T."), QStringLiteral("icons/icons/clock.png"), QStringLiteral("0:00:00"),
                       false, QStringLiteral("moving_time"), valueElapsedFontSize, labelFontSize);
    datetime = new DataObject(QStringLiteral("Clock"), QStringLiteral("icons/icons/clock.png"),
                              QTime::currentTime().toString(QStringLiteral("hh:mm:ss")), false,
                              QStringLiteral("datetime"), valueTimeFontSize, labelFontSize);
    lapElapsed = new DataObject(QStringLiteral("Lap Elapsed"), QStringLiteral("icons/icons/clock.png"),
                                QStringLiteral("0:00:00"), false, QStringLiteral("lapElapsed"), valueElapsedFontSize,
                                labelFontSize);
    remaningTimeTrainingProgramCurrentRow = new DataObject(
        QStringLiteral("Time to Next"), QStringLiteral("icons/icons/clock.png"), QStringLiteral("0:00:00"), true,
        QStringLiteral("remainingtimetrainprogramrow"), valueElapsedFontSize, labelFontSize);

    nextRows =
        new DataObject(QStringLiteral("Next Rows"), QStringLiteral("icons/icons/clock.png"), QStringLiteral("N/A"),
                       false, QStringLiteral("nextrows"), valueElapsedFontSize, labelFontSize);

    mets = new DataObject(QStringLiteral("METS"), QStringLiteral("icons/icons/watt.png"), QStringLiteral("0"), false,
                          QStringLiteral("mets"), 48, labelFontSize);
    targetMets = new DataObject(QStringLiteral("Target METS"), QStringLiteral("icons/icons/watt.png"),
                                QStringLiteral("0"), false, QStringLiteral("targetmets"), 48, labelFontSize);
    steeringAngle = new DataObject(QStringLiteral("Steering"), QStringLiteral("icons/icons/cadence.png"),
                                   QStringLiteral("0"), false, QStringLiteral("steeringangle"), 48, labelFontSize);
    peloton_offset =
        new DataObject(QStringLiteral("Peloton Offset"), QStringLiteral("icons/icons/clock.png"), QStringLiteral("0"),
                       true, QStringLiteral("peloton_offset"), valueElapsedFontSize, labelFontSize);
    peloton_remaining =
        new DataObject(QStringLiteral("Peloton Rem."), QStringLiteral("icons/icons/clock.png"), QStringLiteral("0"),
                       true, QStringLiteral("peloton_remaining"), valueElapsedFontSize, labelFontSize);
    strokesCount = new DataObject(QStringLiteral("Strokes Count"), QStringLiteral("icons/icons/cadence.png"),
                                  QStringLiteral("0"), false, QStringLiteral("strokes_count"), 48, labelFontSize);
    strokesLength = new DataObject(QStringLiteral("Strokes Length"), QStringLiteral("icons/icons/cadence.png"),
                                   QStringLiteral("0"), false, QStringLiteral("strokes_length"), 48, labelFontSize);
    gears = new DataObject(QStringLiteral("Gears"), QStringLiteral("icons/icons/elevationgain.png"),
                           QStringLiteral("0"), true, QStringLiteral("gears"), 48, labelFontSize);
    pidHR = new DataObject(QStringLiteral("PID Heart"), QStringLiteral("icons/icons/heart_red.png"),
                           QStringLiteral("0"), true, QStringLiteral("pid_hr"), 48, labelFontSize);
    extIncline = new DataObject(QStringLiteral("Ext.Inclin.(%)"), QStringLiteral("icons/icons/inclination.png"),
                                QStringLiteral("0.0"), true, QStringLiteral("external_inclination"), 48, labelFontSize);
    instantaneousStrideLengthCM =
        new DataObject(QStringLiteral("Stride L.(cm)"), QStringLiteral("icons/icons/inclination.png"),
                       QStringLiteral("0"), false, QStringLiteral("stride_length"), 48, labelFontSize);
    groundContactMS = new DataObject(QStringLiteral("Ground C.(ms)"), QStringLiteral("icons/icons/inclination.png"),
                                     QStringLiteral("0"), false, QStringLiteral("ground_contact"), 48, labelFontSize);
    verticalOscillationMM =
        new DataObject(QStringLiteral("Vert.Osc.(mm)"), QStringLiteral("icons/icons/inclination.png"),
                       QStringLiteral("0"), false, QStringLiteral("vertical_oscillation"), 48, labelFontSize);

    if (!settings.value(QStringLiteral("top_bar_enabled"), true).toBool()) {

        m_topBarHeight = 0;
        emit topBarHeightChanged(m_topBarHeight); // NOTE: clazy-incorrecrt-emit
        m_info = QLatin1String("");
        emit infoChanged(m_info); // NOTE: clazy-incorrecrt-emit
    }

    stravaPelotonActivityName = QLatin1String("");
    stravaPelotonInstructorName = QLatin1String("");
    activityDescription = QLatin1String("");
    stravaWorkoutName = QLatin1String("");
    movieFileName = QUrl("");

#if defined(Q_OS_WIN) || (defined(Q_OS_MAC) && !defined(Q_OS_IOS))
    connect(engine, &QQmlApplicationEngine::quit, &QGuiApplication::quit);
    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    connect(&tLicense, &QTimer::timeout, this, &homeform::licenseTimeout);
    tLicense.start(600000);
    connect(mgr, &QNetworkAccessManager::finished, this, &homeform::licenseReply);
    QUrl url(QStringLiteral("http://robertoviola.cloud:4010/?supporter=") +
             settings.value("user_email", "").toString());
    QNetworkRequest request(url);
    mgr->get(request);
#endif

    this->bluetoothManager = bl;
    this->engine = engine;
    connect(bluetoothManager, &bluetooth::deviceFound, this, &homeform::deviceFound);
    connect(bluetoothManager, &bluetooth::deviceConnected, this, &homeform::deviceConnected);
    connect(bluetoothManager, &bluetooth::ftmsAccessoryConnected, this, &homeform::ftmsAccessoryConnected);
    connect(bluetoothManager, &bluetooth::deviceConnected, this, &homeform::trainProgramSignals);
    connect(this, &homeform::workoutNameChanged, bluetoothManager->getUserTemplateManager(),
            &TemplateInfoSenderBuilder::onWorkoutNameChanged);
    connect(this, &homeform::workoutStartDateChanged, bluetoothManager->getUserTemplateManager(),
            &TemplateInfoSenderBuilder::onWorkoutStartDate);
    connect(this, &homeform::instructorNameChanged, bluetoothManager->getUserTemplateManager(),
            &TemplateInfoSenderBuilder::onInstructorName);
    connect(this, &homeform::workoutEventStateChanged, bluetoothManager->getUserTemplateManager(),
            &TemplateInfoSenderBuilder::workoutEventStateChanged);
    connect(bluetoothManager->getUserTemplateManager(), &TemplateInfoSenderBuilder::activityDescriptionChanged, this,
            &homeform::setActivityDescription);
    connect(bluetoothManager->getInnerTemplateManager(), &TemplateInfoSenderBuilder::chartSaved, this,
            &homeform::chartSaved);
    connect(this, &homeform::workoutNameChanged, bluetoothManager->getInnerTemplateManager(),
            &TemplateInfoSenderBuilder::onWorkoutNameChanged);
    connect(this, &homeform::workoutStartDateChanged, bluetoothManager->getInnerTemplateManager(),
            &TemplateInfoSenderBuilder::onWorkoutStartDate);
    connect(this, &homeform::instructorNameChanged, bluetoothManager->getInnerTemplateManager(),
            &TemplateInfoSenderBuilder::onInstructorName);
    connect(this, &homeform::workoutEventStateChanged, bluetoothManager->getInnerTemplateManager(),
            &TemplateInfoSenderBuilder::workoutEventStateChanged);
    connect(bluetoothManager->getInnerTemplateManager(), &TemplateInfoSenderBuilder::activityDescriptionChanged, this,
            &homeform::setActivityDescription);
    engine->rootContext()->setContextProperty(QStringLiteral("rootItem"), (QObject *)this);

    this->trainProgram = new trainprogram(QList<trainrow>(), bl);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &homeform::update);
    timer->start(1s);

    backupTimer = new QTimer(this);
    connect(backupTimer, &QTimer::timeout, this, &homeform::backup);
    backupTimer->start(1min);

    QObject *rootObject = engine->rootObjects().constFirst();
    QObject *home = rootObject->findChild<QObject *>(QStringLiteral("home"));
    QObject *stack = rootObject;
    engine->rootContext()->setContextProperty("pathController", &pathController);
    QObject::connect(home, SIGNAL(start_clicked()), this, SLOT(Start()));
    QObject::connect(home, SIGNAL(stop_clicked()), this, SLOT(Stop()));
    QObject::connect(stack, SIGNAL(trainprogram_open_clicked(QUrl)), this, SLOT(trainprogram_open_clicked(QUrl)));
    QObject::connect(stack, SIGNAL(trainprogram_preview(QUrl)), this, SLOT(trainprogram_preview(QUrl)));
    QObject::connect(stack, SIGNAL(gpxpreview_open_clicked(QUrl)), this, SLOT(gpxpreview_open_clicked(QUrl)));
    QObject::connect(stack, SIGNAL(trainprogram_zwo_loaded(QString)), this, SLOT(trainprogram_zwo_loaded(QString)));
    QObject::connect(stack, SIGNAL(gpx_open_clicked(QUrl)), this, SLOT(gpx_open_clicked(QUrl)));
    QObject::connect(stack, SIGNAL(gpx_save_clicked()), this, SLOT(gpx_save_clicked()));
    QObject::connect(stack, SIGNAL(fit_save_clicked()), this, SLOT(fit_save_clicked()));
    QObject::connect(stack, SIGNAL(strava_connect_clicked()), this, SLOT(strava_connect_clicked()));
    QObject::connect(stack, SIGNAL(refresh_bluetooth_devices_clicked()), this,
                     SLOT(refresh_bluetooth_devices_clicked()));
    QObject::connect(home, SIGNAL(lap_clicked()), this, SLOT(Lap()));
    QObject::connect(home, SIGNAL(peloton_start_workout()), this, SLOT(peloton_start_workout()));
    QObject::connect(home, SIGNAL(peloton_abort_workout()), this, SLOT(peloton_abort_workout()));
    QObject::connect(stack, SIGNAL(loadSettings(QUrl)), this, SLOT(loadSettings(QUrl)));
    QObject::connect(stack, SIGNAL(saveSettings(QUrl)), this, SLOT(saveSettings(QUrl)));
    QObject::connect(stack, SIGNAL(deleteSettings(QUrl)), this, SLOT(deleteSettings(QUrl)));
    QObject::connect(stack, SIGNAL(saveProfile(QString)), this, SLOT(saveProfile(QString)));
    QObject::connect(stack, SIGNAL(restart()), this, SLOT(restart()));

    QObject::connect(stack, SIGNAL(volumeUp()), this, SLOT(volumeUp()));
    QObject::connect(stack, SIGNAL(volumeDown()), this, SLOT(volumeDown()));
    QObject::connect(stack, SIGNAL(keyMediaPrevious()), this, SLOT(keyMediaPrevious()));
    QObject::connect(stack, SIGNAL(keyMediaNext()), this, SLOT(keyMediaNext()));

    if (settings.value(QStringLiteral("top_bar_enabled"), true).toBool()) {
        emit stopIconChanged(stopIcon());     // NOTE: clazy-incorrecrt-emit
        emit stopTextChanged(stopText());     // NOTE: clazy-incorrecrt-emit
        emit startIconChanged(startIcon());   // NOTE: clazy-incorrecrt-emit
        emit startTextChanged(startText());   // NOTE: clazy-incorrecrt-emit
        emit startColorChanged(startColor()); // NOTE: clazy-incorrecrt-emit
        emit stopColorChanged(stopColor());   // NOTE: clazy-incorrecrt-emit
    }

    emit tile_orderChanged(tile_order()); // NOTE: clazy-incorrecrt-emit

    pelotonHandler = new peloton(bl);
    connect(pelotonHandler, &peloton::workoutStarted, this, &homeform::pelotonWorkoutStarted);
    connect(pelotonHandler, &peloton::workoutChanged, this, &homeform::pelotonWorkoutChanged);
    connect(pelotonHandler, &peloton::loginState, this, &homeform::pelotonLoginState);
    connect(pelotonHandler, &peloton::pzpLoginState, this, &homeform::pzpLoginState);

    // copying bundles zwo files in the right path if necessesary
    QDirIterator itZwo(":/zwo/");
    QDir().mkdir(getWritableAppDir() + "training/");
    while (itZwo.hasNext()) {
        qDebug() << itZwo.next() << itZwo.fileName();
        if (!QFile(getWritableAppDir() + "training/" + itZwo.fileName()).exists()) {
            QFile::copy(":/zwo/" + itZwo.fileName(), getWritableAppDir() + "training/" + itZwo.fileName());
        }
    }

    QDirIterator itGpx(":/gpx/");
    QDir().mkdir(getWritableAppDir() + "gpx/");
    while (itGpx.hasNext()) {
        qDebug() << itGpx.next() << itGpx.fileName();
        if (!QFile(getWritableAppDir() + "gpx/" + itGpx.fileName()).exists()) {
            QFile::copy(":/gpx/" + itGpx.fileName(), getWritableAppDir() + "gpx/" + itGpx.fileName());
        }
    }

    m_speech.setLocale(QLocale::English);

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    QBluetoothDeviceInfo b;
    deviceConnected(b);
#endif
}

void homeform::setActivityDescription(QString desc) { activityDescription = desc; }

void homeform::chartSaved(QString fileName) {
    if (!stopped)
        return;
    chartImagesFilenames.append(fileName);
    if (chartImagesFilenames.length() >= 7) {
        sendMail();
        chartImagesFilenames.clear();
    }
}

void homeform::keyMediaPrevious() {
    qDebug() << QStringLiteral("keyMediaPrevious");
    QSettings settings;
    if (settings.value(QStringLiteral("volume_change_gears"), false).toBool()) {
        Minus(QStringLiteral("gears"));
        Minus(QStringLiteral("gears"));
        Minus(QStringLiteral("gears"));
        Minus(QStringLiteral("gears"));
        Minus(QStringLiteral("gears"));
    }
}

void homeform::keyMediaNext() {
    qDebug() << QStringLiteral("keyMediaNext");
    QSettings settings;
    if (settings.value(QStringLiteral("volume_change_gears"), false).toBool()) {
        Plus(QStringLiteral("gears"));
        Plus(QStringLiteral("gears"));
        Plus(QStringLiteral("gears"));
        Plus(QStringLiteral("gears"));
        Plus(QStringLiteral("gears"));
    }
}

void homeform::volumeUp() {
    qDebug() << QStringLiteral("volumeUp");
    QSettings settings;
    if (settings.value(QStringLiteral("volume_change_gears"), false).toBool())
        Plus(QStringLiteral("gears"));
}

void homeform::volumeDown() {
    qDebug() << QStringLiteral("volumeDown");
    QSettings settings;
    if (settings.value(QStringLiteral("volume_change_gears"), false).toBool())
        Minus(QStringLiteral("gears"));
}

void homeform::peloton_abort_workout() {
    qDebug() << QStringLiteral("peloton_abort_workout!");
    pelotonAbortedName = pelotonAskedName;
    pelotonAbortedInstructor = pelotonAskedInstructor;
}

void homeform::peloton_start_workout() {
    qDebug() << QStringLiteral("peloton_start_workout!");
    if (pelotonHandler && !pelotonHandler->trainrows.isEmpty()) {
        if (trainProgram) {
            emit trainProgram->stop();

            delete trainProgram;
            trainProgram = nullptr;
        }
        trainProgram = new trainprogram(pelotonHandler->trainrows, bluetoothManager);
        trainProgramSignals();
        trainProgram->restart();
    }
}

void homeform::pzpLoginState(bool ok) {

    m_pzpLoginState = (ok ? 1 : 0);
    emit pzpLoginChanged(m_pzpLoginState);
}

void homeform::pelotonLoginState(bool ok) {

    m_pelotonLoginState = (ok ? 1 : 0);
    emit pelotonLoginChanged(m_pelotonLoginState);
}

void homeform::pelotonWorkoutStarted(const QString &name, const QString &instructor) {
    pelotonAskedName = name;
    pelotonAskedInstructor = instructor;

    if (!pelotonAskedName.compare(pelotonAbortedName) && !pelotonAskedInstructor.compare(pelotonAbortedInstructor)) {
        qDebug() << QStringLiteral("Peloton class aborted before");
        return;
    }
    pelotonAbortedName.clear();
    pelotonAbortedInstructor.clear();

    if (pelotonHandler) {
        switch (pelotonHandler->currentApi()) {
        case peloton::homefitnessbuddy_api:
            m_pelotonProvider = QStringLiteral("Metrics are provided from https://www.homefitnessbuddy.com");
            break;
        case peloton::powerzonepack_api:
            m_pelotonProvider = QStringLiteral("Metrics are provided from https://pzpack.com");
            break;
        default:
            m_pelotonProvider = QStringLiteral("Metrics are provided from https://onepeloton.com");
            break;
        }
    }
    emit changePelotonProvider(pelotonProvider());
    m_pelotonAskStart = true;
    emit changePelotonAskStart(pelotonAskStart());
}

void homeform::pelotonWorkoutChanged(const QString &name, const QString &instructor) {

    QSettings settings;

    stravaPelotonActivityName = name;
    stravaPelotonInstructorName = instructor;
    if (pelotonHandler) {
        if (pelotonHandler->current_workout_type.toLower().startsWith("meditation") ||
            pelotonHandler->current_workout_type.toLower().startsWith("cardio") ||
            pelotonHandler->current_workout_type.toLower().startsWith("circuit") ||
            pelotonHandler->current_workout_type.toLower().startsWith("strength") ||
            pelotonHandler->current_workout_type.toLower().startsWith("stretching") ||
            pelotonHandler->current_workout_type.toLower().startsWith("yoga"))
            stravaPelotonWorkoutType = FIT_SPORT_GENERIC;
        else if (pelotonHandler->current_workout_type.toLower().startsWith("walking"))
            stravaPelotonWorkoutType = FIT_SPORT_WALKING;
        else if (pelotonHandler->current_workout_type.toLower().startsWith("running"))
            stravaPelotonWorkoutType = FIT_SPORT_RUNNING;
        else
            stravaPelotonWorkoutType = FIT_SPORT_INVALID;
    } else {
        stravaPelotonWorkoutType = FIT_SPORT_INVALID;
    }
    emit workoutNameChanged(workoutName());
    emit instructorNameChanged(instructorName());

    if (!settings.value(QStringLiteral("top_bar_enabled"), true).toBool()) {
        return;
    }
    m_info = name;
    emit infoChanged(m_info);
}

QString homeform::getWritableAppDir() {
    QString path = QLatin1String("");
#if defined(Q_OS_ANDROID)
    path = getAndroidDataAppDir() + "/";
#elif defined(Q_OS_MACOS) || defined(Q_OS_OSX)
    path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/";
#elif defined(Q_OS_IOS)
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/";
#endif
    return path;
}

void homeform::backup() {

    static uint8_t index = 0;
    qDebug() << QStringLiteral("saving fit file backup...");

    QString path = getWritableAppDir();
    bluetoothdevice *dev = bluetoothManager->device();
    if (dev) {

        QString filename = path + QString::number(index) + backupFitFileName;
        QFile::remove(filename);
        qfit::save(filename, Session, dev->deviceType(),
                   qobject_cast<m3ibike *>(dev) ? QFIT_PROCESS_DISTANCENOISE : QFIT_PROCESS_NONE,
                   stravaPelotonWorkoutType);

        index++;
        if (index > 1) {
            index = 0;
        }
    }
}

QString homeform::stopColor() { return QStringLiteral("#00000000"); }

QString homeform::startColor() {
    static uint8_t startColorToggle = 0;
    if (paused || stopped) {
        if (startColorToggle) {

            startColorToggle = 0;
            return QStringLiteral("red");
        } else {

            startColorToggle = 1;
            return QStringLiteral("#00000000");
        }
    }
    return QStringLiteral("#00000000");
}

void homeform::refresh_bluetooth_devices_clicked() {

    bluetoothManager->onlyDiscover = true;
    bluetoothManager->restart();
}

homeform::~homeform() {

    gpx_save_clicked();
    fit_save_clicked();
}

void homeform::aboutToQuit() {

    /*if(bluetoothManager->device())
        bluetoothManager->device()->disconnectBluetooth();*/
}

void homeform::trainProgramSignals() {
    if (bluetoothManager->device()) {
        disconnect(trainProgram, &trainprogram::start, bluetoothManager->device(), &bluetoothdevice::start);
        disconnect(trainProgram, &trainprogram::stop, bluetoothManager->device(), &bluetoothdevice::stop);
        disconnect(trainProgram, &trainprogram::lap, this, &homeform::Lap);
        disconnect(trainProgram, &trainprogram::changeSpeed, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeSpeed);
        disconnect(trainProgram, &trainprogram::changeInclination, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeInclination);
        disconnect(trainProgram, &trainprogram::changeNextInclination300Meters, bluetoothManager->device(),
                   &bluetoothdevice::changeNextInclination300Meters);
        disconnect(trainProgram, &trainprogram::changeInclination, ((bike *)bluetoothManager->device()),
                   &bike::changeInclination);
        disconnect(trainProgram, &trainprogram::changeFanSpeed, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeFanSpeed);
        disconnect(trainProgram, &trainprogram::changeSpeedAndInclination, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeSpeedAndInclination);
        disconnect(trainProgram, &trainprogram::changeResistance, ((bike *)bluetoothManager->device()),
                   &bike::changeResistance);
        disconnect(trainProgram, &trainprogram::changeRequestedPelotonResistance, ((bike *)bluetoothManager->device()),
                   &bike::changeRequestedPelotonResistance);
        disconnect(trainProgram, &trainprogram::changeCadence, ((bike *)bluetoothManager->device()),
                   &bike::changeCadence);
        disconnect(trainProgram, &trainprogram::changePower, ((bike *)bluetoothManager->device()), &bike::changePower);
        disconnect(trainProgram, &trainprogram::changePower, ((rower *)bluetoothManager->device()),
                   &rower::changePower);
        disconnect(trainProgram, &trainprogram::changeCadence, ((elliptical *)bluetoothManager->device()),
                   &elliptical::changeCadence);
        disconnect(trainProgram, &trainprogram::changePower, ((elliptical *)bluetoothManager->device()),
                   &elliptical::changePower);
        disconnect(trainProgram, &trainprogram::changeInclination, ((elliptical *)bluetoothManager->device()),
                   &elliptical::changeInclination);
        disconnect(trainProgram, &trainprogram::changeResistance, ((elliptical *)bluetoothManager->device()),
                   &elliptical::changeResistance);
        disconnect(trainProgram, &trainprogram::changeRequestedPelotonResistance,
                   ((elliptical *)bluetoothManager->device()), &elliptical::changeRequestedPelotonResistance);
        disconnect(((treadmill *)bluetoothManager->device()), &treadmill::tapeStarted, trainProgram,
                   &trainprogram::onTapeStarted);
        disconnect(((bike *)bluetoothManager->device()), &bike::bikeStarted, trainProgram,
                   &trainprogram::onTapeStarted);
        disconnect(trainProgram, &trainprogram::changeGeoPosition, bluetoothManager->device(),
                   &bluetoothdevice::changeGeoPosition);
        disconnect(this, &homeform::workoutEventStateChanged, bluetoothManager->device(),
                   &bluetoothdevice::workoutEventStateChanged);
        disconnect(trainProgram, &trainprogram::changeTimestamp, this, &homeform::changeTimestamp);

        Q_ASSERT(connect(trainProgram, &trainprogram::start, bluetoothManager->device(), &bluetoothdevice::start));
        Q_ASSERT(connect(trainProgram, &trainprogram::stop, bluetoothManager->device(), &bluetoothdevice::stop));
        Q_ASSERT(connect(trainProgram, &trainprogram::lap, this, &homeform::Lap));
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            Q_ASSERT(connect(trainProgram, &trainprogram::changeSpeed, ((treadmill *)bluetoothManager->device()),
                             &treadmill::changeSpeed));
            Q_ASSERT(connect(trainProgram, &trainprogram::changeFanSpeed, ((treadmill *)bluetoothManager->device()),
                             &treadmill::changeFanSpeed));
            Q_ASSERT(connect(trainProgram, &trainprogram::changeInclination, ((treadmill *)bluetoothManager->device()),
                             &treadmill::changeInclination));
            Q_ASSERT(connect(trainProgram, &trainprogram::changeSpeedAndInclination,
                             ((treadmill *)bluetoothManager->device()), &treadmill::changeSpeedAndInclination));
            Q_ASSERT(connect(((treadmill *)bluetoothManager->device()), &treadmill::tapeStarted, trainProgram,
                             &trainprogram::onTapeStarted));
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
            Q_ASSERT(connect(trainProgram, &trainprogram::changeCadence, ((bike *)bluetoothManager->device()),
                             &bike::changeCadence));
            Q_ASSERT(connect(trainProgram, &trainprogram::changePower, ((bike *)bluetoothManager->device()),
                             &bike::changePower));
            Q_ASSERT(connect(trainProgram, &trainprogram::changeInclination, ((bike *)bluetoothManager->device()),
                             &bike::changeInclination));
            Q_ASSERT(connect(trainProgram, &trainprogram::changeResistance, ((bike *)bluetoothManager->device()),
                             &bike::changeResistance));
            Q_ASSERT(connect(trainProgram, &trainprogram::changeRequestedPelotonResistance,
                             ((bike *)bluetoothManager->device()), &bike::changeRequestedPelotonResistance));
            Q_ASSERT(connect(((bike *)bluetoothManager->device()), &bike::bikeStarted, trainProgram,
                             &trainprogram::onTapeStarted));
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
            Q_ASSERT(connect(trainProgram, &trainprogram::changeCadence, ((elliptical *)bluetoothManager->device()),
                             &elliptical::changeCadence));
            Q_ASSERT(connect(trainProgram, &trainprogram::changePower, ((elliptical *)bluetoothManager->device()),
                             &elliptical::changePower));
            Q_ASSERT(connect(trainProgram, &trainprogram::changeInclination, ((elliptical *)bluetoothManager->device()),
                             &elliptical::changeInclination));
            Q_ASSERT(connect(trainProgram, &trainprogram::changeResistance, ((elliptical *)bluetoothManager->device()),
                             &elliptical::changeResistance));
            Q_ASSERT(connect(trainProgram, &trainprogram::changeRequestedPelotonResistance,
                             ((elliptical *)bluetoothManager->device()),
                             &elliptical::changeRequestedPelotonResistance));
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
            Q_ASSERT(connect(trainProgram, &trainprogram::changePower, ((rower *)bluetoothManager->device()),
                             &rower::changePower));
        }
        Q_ASSERT(connect(trainProgram, &trainprogram::changeNextInclination300Meters, bluetoothManager->device(),
                         &bluetoothdevice::changeNextInclination300Meters));
        Q_ASSERT(connect(trainProgram, &trainprogram::changeGeoPosition, bluetoothManager->device(),
                         &bluetoothdevice::changeGeoPosition));
        Q_ASSERT(connect(trainProgram, &trainprogram::changeTimestamp, this, &homeform::changeTimestamp));
        Q_ASSERT(connect(this, &homeform::workoutEventStateChanged, bluetoothManager->device(),
                         &bluetoothdevice::workoutEventStateChanged));

        qDebug() << QStringLiteral("trainProgram associated to a device");
    } else {
        qDebug() << QStringLiteral("trainProgram NOT associated to a device");
    }
}

QStringList homeform::tile_order() {

    QStringList r;
    r.reserve(42);
    for (int i = 0; i < 42; i++) {
        r.append(QString::number(i));
    }
    return r;
}

// these events are coming from the SS2K, so when the auto resistance is off, this event shouldn't be processed
void homeform::gearUp() {
    if (autoResistance())
        Plus(QStringLiteral("gears"));
}

void homeform::gearDown() {
    if (autoResistance())
        Minus(QStringLiteral("gears"));
}

void homeform::ftmsAccessoryConnected(smartspin2k *d) {
    connect(this, &homeform::autoResistanceChanged, d, &smartspin2k::autoResistanceChanged);
    connect(d, &smartspin2k::gearUp, this, &homeform::gearUp);
    connect(d, &smartspin2k::gearDown, this, &homeform::gearDown);
}

void homeform::sortTiles() {

    QSettings settings;
    bool pelotoncadence = settings.value(QStringLiteral("bike_cadence_sensor"), false).toBool();

    if (!bluetoothManager || !bluetoothManager->device())
        return;

    dataList.clear();

    if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
        for (int i = 0; i < 100; i++) {
            if (settings.value(QStringLiteral("tile_speed_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_speed_order"), 0).toInt() == i) {

                speed->setGridId(i);
                dataList.append(speed);
            }

            if (settings.value(QStringLiteral("tile_inclination_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_inclination_order"), 0).toInt() == i) {
                inclination->setGridId(i);
                dataList.append(inclination);
            }

            if (settings.value(QStringLiteral("tile_elevation_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_elevation_order"), 0).toInt() == i) {
                elevation->setGridId(i);
                dataList.append(elevation);
            }

            if (settings.value(QStringLiteral("tile_elapsed_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_elapsed_order"), 0).toInt() == i) {
                elapsed->setGridId(i);
                dataList.append(elapsed);
            }

            if (settings.value(QStringLiteral("tile_moving_time_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_moving_time_order"), 19).toInt() == i) {
                moving_time->setGridId(i);
                dataList.append(moving_time);
            }

            if (settings.value(QStringLiteral("tile_peloton_offset_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_peloton_offset_order"), 20).toInt() == i) {
                peloton_offset->setGridId(i);
                dataList.append(peloton_offset);
            }

            if (settings.value(QStringLiteral("tile_peloton_remaining_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_peloton_remaining_order"), 20).toInt() == i) {
                peloton_remaining->setGridId(i);
                dataList.append(peloton_remaining);
            }

            if (settings.value(QStringLiteral("tile_calories_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_calories_order"), 0).toInt() == i) {
                calories->setGridId(i);
                dataList.append(calories);
            }

            if (settings.value(QStringLiteral("tile_odometer_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_odometer_order"), 0).toInt() == i) {
                odometer->setGridId(i);
                dataList.append(odometer);
            }

            if (settings.value(QStringLiteral("tile_pace_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_pace_order"), 0).toInt() == i) {
                pace->setGridId(i);
                dataList.append(pace);
            }

            if (settings.value(QStringLiteral("tile_watt_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_watt_order"), 0).toInt() == i) {
                watt->setGridId(i);
                dataList.append(watt);
            }

            if (settings.value(QStringLiteral("tile_weight_loss_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_weight_loss_order"), 24).toInt() == i) {
                weightLoss->setGridId(i);
                dataList.append(weightLoss);
            }

            if (settings.value(QStringLiteral("tile_avgwatt_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_avgwatt_order"), 0).toInt() == i) {
                avgWatt->setGridId(i);
                dataList.append(avgWatt);
            }

            if (settings.value(QStringLiteral("tile_ftp_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_ftp_order"), 0).toInt() == i) {
                ftp->setGridId(i);
                dataList.append(ftp);
            }

            if (settings.value(QStringLiteral("tile_jouls_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_jouls_order"), 0).toInt() == i) {
                jouls->setGridId(i);
                dataList.append(jouls);
            }

            if (settings.value(QStringLiteral("tile_heart_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_heart_order"), 0).toInt() == i) {
                heart->setGridId(i);
                dataList.append(heart);
            }

            if (settings.value(QStringLiteral("tile_fan_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_fan_order"), 0).toInt() == i) {
                fan->setGridId(i);
                dataList.append(fan);
            }

            if (settings.value(QStringLiteral("tile_datetime_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_datetime_order"), 0).toInt() == i) {
                datetime->setGridId(i);
                dataList.append(datetime);
            }

            if (settings.value(QStringLiteral("tile_lapelapsed_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_lapelapsed_order"), 18).toInt() == i) {
                lapElapsed->setGridId(i);
                dataList.append(lapElapsed);
            }

            if (settings.value(QStringLiteral("tile_watt_kg_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_watt_kg_order"), 24).toInt() == i) {
                wattKg->setGridId(i);
                dataList.append(wattKg);
            }

            if (settings.value(QStringLiteral("tile_remainingtimetrainprogramrow_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_remainingtimetrainprogramrow_order"), 27).toInt() == i) {

                remaningTimeTrainingProgramCurrentRow->setGridId(i);
                dataList.append(remaningTimeTrainingProgramCurrentRow);
            }

            if (settings.value(QStringLiteral("tile_nextrowstrainprogram_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_nextrowtrainprogram_order"), 31).toInt() == i) {

                nextRows->setGridId(i);
                dataList.append(nextRows);
            }

            if (settings.value(QStringLiteral("tile_mets_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_mets_order"), 28).toInt() == i) {

                mets->setGridId(i);
                dataList.append(mets);
            }
            if (settings.value(QStringLiteral("tile_targetmets_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_targetmets_order"), 29).toInt() == i) {

                targetMets->setGridId(i);
                dataList.append(targetMets);
            }

            if (settings.value(QStringLiteral("tile_target_speed_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_target_speed_order"), 28).toInt() == i) {
                target_speed->setGridId(i);
                dataList.append(target_speed);
            }

            if (settings.value(QStringLiteral("tile_target_incline_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_target_incline_order"), 29).toInt() == i) {
                target_incline->setGridId(i);
                dataList.append(target_incline);
            }

            if (settings.value(QStringLiteral("tile_cadence_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_cadence_order"), 30).toInt() == i) {
                cadence->setGridId(i);
                dataList.append(cadence);
            }

            if (settings.value(QStringLiteral("tile_pid_hr_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_pid_hr_order"), 31).toInt() == i) {
                pidHR->setGridId(i);
                dataList.append(pidHR);
            }

            if (settings.value(QStringLiteral("tile_instantaneous_stride_length_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_instantaneous_stride_length_order"), 32).toInt() == i) {
                instantaneousStrideLengthCM->setGridId(i);
                dataList.append(instantaneousStrideLengthCM);
            }

            if (settings.value(QStringLiteral("tile_ground_contact_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_ground_contact_order"), 33).toInt() == i) {
                groundContactMS->setGridId(i);
                dataList.append(groundContactMS);
            }

            if (settings.value(QStringLiteral("tile_vertical_oscillation_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_vertical_oscillation_order"), 34).toInt() == i) {
                verticalOscillationMM->setGridId(i);
                dataList.append(verticalOscillationMM);
            }
        }
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
        for (int i = 0; i < 100; i++) {
            if (settings.value(QStringLiteral("tile_speed_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_speed_order"), 0).toInt() == i) {

                speed->setGridId(i);
                dataList.append(speed);
            }

            if (settings.value(QStringLiteral("tile_cadence_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_cadence_order"), 0).toInt() == i) {
                cadence->setGridId(i);
                dataList.append(cadence);
            }

            if (settings.value(QStringLiteral("tile_elevation_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_elevation_order"), 0).toInt() == i) {
                elevation->setGridId(i);
                dataList.append(elevation);
            }

            if (settings.value(QStringLiteral("tile_elapsed_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_elapsed_order"), 0).toInt() == i) {
                elapsed->setGridId(i);
                dataList.append(elapsed);
            }

            if (settings.value(QStringLiteral("tile_moving_time_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_moving_time_order"), 19).toInt() == i) {
                moving_time->setGridId(i);
                dataList.append(moving_time);
            }

            if (settings.value(QStringLiteral("tile_peloton_offset_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_peloton_offset_order"), 20).toInt() == i) {
                peloton_offset->setGridId(i);
                dataList.append(peloton_offset);
            }

            if (settings.value(QStringLiteral("tile_peloton_remaining_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_peloton_remaining_order"), 20).toInt() == i) {
                peloton_remaining->setGridId(i);
                dataList.append(peloton_remaining);
            }

            if (settings.value(QStringLiteral("tile_calories_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_calories_order"), 0).toInt() == i) {
                calories->setGridId(i);
                dataList.append(calories);
            }

            if (settings.value(QStringLiteral("tile_odometer_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_odometer_order"), 0).toInt() == i) {
                odometer->setGridId(i);
                dataList.append(odometer);
            }

            if (settings.value(QStringLiteral("tile_resistance_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_resistance_order"), 0).toInt() == i) {
                resistance->setGridId(i);
                dataList.append(resistance);
            }

            if (settings.value(QStringLiteral("tile_peloton_resistance_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_peloton_resistance_order"), 0).toInt() == i) {
                peloton_resistance->setGridId(i);
                dataList.append(peloton_resistance);
            }

            if (settings.value(QStringLiteral("tile_watt_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_watt_order"), 0).toInt() == i) {
                watt->setGridId(i);
                dataList.append(watt);
            }

            if (settings.value(QStringLiteral("tile_weight_loss_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_weight_loss_order"), 24).toInt() == i) {
                weightLoss->setGridId(i);
                dataList.append(weightLoss);
            }

            if (settings.value(QStringLiteral("tile_avgwatt_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_avgwatt_order"), 0).toInt() == i) {
                avgWatt->setGridId(i);
                dataList.append(avgWatt);
            }

            if (settings.value(QStringLiteral("tile_ftp_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_ftp_order"), 0).toInt() == i) {
                ftp->setGridId(i);
                dataList.append(ftp);
            }

            if (settings.value(QStringLiteral("tile_jouls_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_jouls_order"), 0).toInt() == i) {
                jouls->setGridId(i);
                dataList.append(jouls);
            }

            if (settings.value(QStringLiteral("tile_heart_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_heart_order"), 0).toInt() == i) {
                heart->setGridId(i);
                dataList.append(heart);
            }

            if (settings.value(QStringLiteral("tile_fan_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_fan_order"), 0).toInt() == i) {
                fan->setGridId(i);
                dataList.append(fan);
            }

            if (settings.value(QStringLiteral("tile_datetime_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_datetime_order"), 0).toInt() == i) {
                datetime->setGridId(i);
                dataList.append(datetime);
            }

            if (settings.value(QStringLiteral("tile_target_resistance_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_target_resistance_order"), 0).toInt() == i) {
                target_resistance->setGridId(i);
                dataList.append(target_resistance);
            }

            if (settings.value(QStringLiteral("tile_target_peloton_resistance_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_target_peloton_resistance_order"), 21).toInt() == i) {
                target_peloton_resistance->setGridId(i);
                dataList.append(target_peloton_resistance);
            }

            if (settings.value(QStringLiteral("tile_target_cadence_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_target_cadence_order"), 19).toInt() == i) {
                target_cadence->setGridId(i);
                dataList.append(target_cadence);
            }

            if (settings.value(QStringLiteral("tile_target_power_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_target_power_order"), 20).toInt() == i) {
                target_power->setGridId(i);
                dataList.append(target_power);
            }

            if (settings.value(QStringLiteral("tile_target_zone_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_target_zone_order"), 24).toInt() == i) {
                target_zone->setGridId(i);
                dataList.append(target_zone);
            }

            if (settings.value(QStringLiteral("tile_lapelapsed_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_lapelapsed_order"), 18).toInt() == i) {
                lapElapsed->setGridId(i);
                dataList.append(lapElapsed);
            }

            if (settings.value(QStringLiteral("tile_watt_kg_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_watt_kg_order"), 24).toInt() == i) {
                wattKg->setGridId(i);
                dataList.append(wattKg);
            }
            if (settings.value(QStringLiteral("tile_gears_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_gears_order"), 25).toInt() == i) {
                gears->setGridId(i);
                dataList.append(gears);
            }

            if (settings.value(QStringLiteral("tile_remainingtimetrainprogramrow_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_remainingtimetrainprogramrow_order"), 27).toInt() == i) {

                remaningTimeTrainingProgramCurrentRow->setGridId(i);
                dataList.append(remaningTimeTrainingProgramCurrentRow);
            }

            if (settings.value(QStringLiteral("tile_nextrowstrainprogram_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_nextrowtrainprogram_order"), 31).toInt() == i) {

                nextRows->setGridId(i);
                dataList.append(nextRows);
            }

            if (settings.value(QStringLiteral("tile_mets_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_mets_order"), 28).toInt() == i) {
                mets->setGridId(i);
                dataList.append(mets);
            }
            if (settings.value(QStringLiteral("tile_targetmets_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_targetmets_order"), 29).toInt() == i) {
                targetMets->setGridId(i);
                dataList.append(targetMets);
            }
            // the proform studio is the only bike managed with an inclination properties.
            // In order to don't break the tiles layout to all the bikes users, i enable this
            // only if this bike is selected
            // since i'm adding the inclination from zwift in this tile, in order to preserve the
            // layour for legacy users, i'm not showing this one if the peloton cadence sensor setting
            // is enabled (assuming that if someone has it, he doesn't want an inclination tile)
            if (!pelotoncadence) {
                if (settings.value(QStringLiteral("tile_inclination_enabled"), true).toBool() &&
                    settings.value(QStringLiteral("tile_inclination_order"), 29).toInt() == i) {
                    inclination->setGridId(i);
                    dataList.append(inclination);
                }
            }
            if (settings.value(QStringLiteral("tile_steering_angle_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_steering_angle_order"), 30).toInt() == i) {

                steeringAngle->setGridId(i);
                dataList.append(steeringAngle);
            }

            if (settings.value(QStringLiteral("tile_pid_hr_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_pid_hr_order"), 31).toInt() == i) {
                pidHR->setGridId(i);
                dataList.append(pidHR);
            }

            if (settings.value(QStringLiteral("tile_ext_incline_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_ext_incline_order"), 32).toInt() == i) {
                extIncline->setGridId(i);
                dataList.append(extIncline);
            }
        }
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
        for (int i = 0; i < 100; i++) {
            if (settings.value(QStringLiteral("tile_speed_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_speed_order"), 0).toInt() == i) {
                speed->setGridId(i);
                dataList.append(speed);
            }

            if (settings.value(QStringLiteral("tile_cadence_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_cadence_order"), 0).toInt() == i) {
                cadence->setGridId(i);
                cadence->setName("Stroke Rate");
                dataList.append(cadence);
            }

            if (settings.value(QStringLiteral("tile_elevation_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_elevation_order"), 0).toInt() == i) {
                elevation->setGridId(i);
                dataList.append(elevation);
            }

            if (settings.value(QStringLiteral("tile_elapsed_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_elapsed_order"), 0).toInt() == i) {
                elapsed->setGridId(i);
                dataList.append(elapsed);
            }

            if (settings.value(QStringLiteral("tile_moving_time_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_moving_time_order"), 19).toInt() == i) {
                moving_time->setGridId(i);
                dataList.append(moving_time);
            }

            if (settings.value(QStringLiteral("tile_peloton_offset_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_peloton_offset_order"), 20).toInt() == i) {
                peloton_offset->setGridId(i);
                dataList.append(peloton_offset);
            }

            if (settings.value(QStringLiteral("tile_peloton_remaining_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_peloton_remaining_order"), 20).toInt() == i) {
                peloton_remaining->setGridId(i);
                dataList.append(peloton_remaining);
            }

            if (settings.value(QStringLiteral("tile_calories_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_calories_order"), 0).toInt() == i) {
                calories->setGridId(i);
                dataList.append(calories);
            }

            if (settings.value(QStringLiteral("tile_odometer_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_odometer_order"), 0).toInt() == i) {
                odometer->setGridId(i);
                odometer->setName("Odometer (m)");
                dataList.append(odometer);
            }

            if (settings.value(QStringLiteral("tile_resistance_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_resistance_order"), 0).toInt() == i) {
                resistance->setGridId(i);
                dataList.append(resistance);
            }

            if (settings.value(QStringLiteral("tile_peloton_resistance_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_peloton_resistance_order"), 0).toInt() == i) {
                peloton_resistance->setGridId(i);
                dataList.append(peloton_resistance);
            }

            if (settings.value(QStringLiteral("tile_watt_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_watt_order"), 0).toInt() == i) {
                watt->setGridId(i);
                dataList.append(watt);
            }

            if (settings.value(QStringLiteral("tile_weight_loss_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_weight_loss_order"), 24).toInt() == i) {
                weightLoss->setGridId(i);
                dataList.append(weightLoss);
            }

            if (settings.value(QStringLiteral("tile_avgwatt_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_avgwatt_order"), 0).toInt() == i) {
                avgWatt->setGridId(i);
                dataList.append(avgWatt);
            }

            if (settings.value(QStringLiteral("tile_ftp_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_ftp_order"), 0).toInt() == i) {
                ftp->setGridId(i);
                dataList.append(ftp);
            }

            if (settings.value(QStringLiteral("tile_jouls_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_jouls_order"), 0).toInt() == i) {
                jouls->setGridId(i);
                dataList.append(jouls);
            }

            if (settings.value(QStringLiteral("tile_heart_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_heart_order"), 0).toInt() == i) {
                heart->setGridId(i);
                dataList.append(heart);
            }

            if (settings.value(QStringLiteral("tile_fan_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_fan_order"), 0).toInt() == i) {
                fan->setGridId(i);
                dataList.append(fan);
            }

            if (settings.value(QStringLiteral("tile_datetime_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_datetime_order"), 0).toInt() == i) {
                datetime->setGridId(i);
                dataList.append(datetime);
            }

            if (settings.value(QStringLiteral("tile_target_resistance_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_target_resistance_order"), 0).toInt() == i) {
                target_resistance->setGridId(i);
                dataList.append(target_resistance);
            }

            if (settings.value(QStringLiteral("tile_target_peloton_resistance_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_target_peloton_resistance_order"), 21).toInt() == i) {
                target_peloton_resistance->setGridId(i);
                dataList.append(target_peloton_resistance);
            }

            if (settings.value(QStringLiteral("tile_target_cadence_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_target_cadence_order"), 19).toInt() == i) {
                target_cadence->setGridId(i);
                dataList.append(target_cadence);
            }

            if (settings.value(QStringLiteral("tile_target_power_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_target_power_order"), 20).toInt() == i) {
                target_power->setGridId(i);
                dataList.append(target_power);
            }

            if (settings.value(QStringLiteral("tile_lapelapsed_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_lapelapsed_order"), 18).toInt() == i) {
                lapElapsed->setGridId(i);
                dataList.append(lapElapsed);
            }

            if (settings.value(QStringLiteral("tile_strokes_length_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_strokes_length_order"), 21).toInt() == i) {
                strokesLength->setGridId(i);
                dataList.append(strokesLength);
            }

            if (settings.value(QStringLiteral("tile_strokes_count_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_strokes_count_order"), 22).toInt() == i) {
                strokesCount->setGridId(i);
                dataList.append(strokesCount);
            }

            if (settings.value(QStringLiteral("tile_pace_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_pace_order"), 0).toInt() == i) {
                pace->setGridId(i);
                pace->setName("Pace (m/500m)");
                dataList.append(pace);
            }

            if (settings.value(QStringLiteral("tile_watt_kg_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_watt_kg_order"), 24).toInt() == i) {
                wattKg->setGridId(i);
                dataList.append(wattKg);
            }

            if (settings.value(QStringLiteral("tile_remainingtimetrainprogramrow_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_remainingtimetrainprogramrow_order"), 27).toInt() == i) {
                remaningTimeTrainingProgramCurrentRow->setGridId(i);
                dataList.append(remaningTimeTrainingProgramCurrentRow);
            }

            if (settings.value(QStringLiteral("tile_nextrowstrainprogram_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_nextrowtrainprogram_order"), 31).toInt() == i) {

                nextRows->setGridId(i);
                dataList.append(nextRows);
            }

            if (settings.value(QStringLiteral("tile_mets_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_mets_order"), 28).toInt() == i) {
                mets->setGridId(i);
                dataList.append(mets);
            }
            if (settings.value(QStringLiteral("tile_targetmets_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_targetmets_order"), 29).toInt() == i) {
                targetMets->setGridId(i);
                dataList.append(targetMets);
            }

            if (settings.value(QStringLiteral("tile_pid_hr_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_pid_hr_order"), 31).toInt() == i) {
                pidHR->setGridId(i);
                dataList.append(pidHR);
            }

            if (settings.value(QStringLiteral("tile_target_zone_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_target_zone_order"), 24).toInt() == i) {
                target_zone->setGridId(i);
                dataList.append(target_zone);
            }
        }
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
        for (int i = 0; i < 100; i++) {
            if (settings.value(QStringLiteral("tile_speed_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_speed_order"), 0).toInt() == i) {
                speed->setGridId(i);
                dataList.append(speed);
            }

            if (settings.value(QStringLiteral("tile_cadence_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_cadence_order"), 0).toInt() == i) {
                cadence->setGridId(i);
                dataList.append(cadence);
            }

            if (settings.value(QStringLiteral("tile_inclination_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_inclination_order"), 0).toInt() == i) {
                inclination->setGridId(i);
                dataList.append(inclination);
            }

            if (settings.value(QStringLiteral("tile_elevation_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_elevation_order"), 0).toInt() == i) {
                elevation->setGridId(i);
                dataList.append(elevation);
            }

            if (settings.value(QStringLiteral("tile_elapsed_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_elapsed_order"), 0).toInt() == i) {
                elapsed->setGridId(i);
                dataList.append(elapsed);
            }

            if (settings.value(QStringLiteral("tile_moving_time_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_moving_time_order"), 19).toInt() == i) {
                moving_time->setGridId(i);
                dataList.append(moving_time);
            }

            if (settings.value(QStringLiteral("tile_peloton_offset_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_peloton_offset_order"), 20).toInt() == i) {
                peloton_offset->setGridId(i);
                dataList.append(peloton_offset);
            }

            if (settings.value(QStringLiteral("tile_peloton_remaining_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_peloton_remaining_order"), 20).toInt() == i) {
                peloton_remaining->setGridId(i);
                dataList.append(peloton_remaining);
            }

            if (settings.value(QStringLiteral("tile_calories_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_calories_order"), 0).toInt() == i) {
                calories->setGridId(i);
                dataList.append(calories);
            }

            if (settings.value(QStringLiteral("tile_odometer_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_odometer_order"), 0).toInt() == i) {
                odometer->setGridId(i);
                dataList.append(odometer);
            }

            if (settings.value(QStringLiteral("tile_resistance_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_resistance_order"), 0).toInt() == i) {
                resistance->setGridId(i);
                dataList.append(resistance);
            }

            if (settings.value(QStringLiteral("tile_peloton_resistance_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_peloton_resistance_order"), 0).toInt() == i) {
                peloton_resistance->setGridId(i);
                dataList.append(peloton_resistance);
            }

            if (settings.value(QStringLiteral("tile_watt_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_watt_order"), 0).toInt() == i) {
                watt->setGridId(i);
                dataList.append(watt);
            }

            if (settings.value(QStringLiteral("tile_weight_loss_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_weight_loss_order"), 24).toInt() == i) {
                weightLoss->setGridId(i);
                dataList.append(weightLoss);
            }

            if (settings.value(QStringLiteral("tile_avgwatt_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_avgwatt_order"), 0).toInt() == i) {
                avgWatt->setGridId(i);
                dataList.append(avgWatt);
            }

            if (settings.value(QStringLiteral("tile_ftp_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_ftp_order"), 0).toInt() == i) {
                ftp->setGridId(i);
                dataList.append(ftp);
            }

            if (settings.value(QStringLiteral("tile_jouls_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_jouls_order"), 0).toInt() == i) {
                jouls->setGridId(i);
                dataList.append(jouls);
            }

            if (settings.value(QStringLiteral("tile_heart_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_heart_order"), 0).toInt() == i) {
                heart->setGridId(i);
                dataList.append(heart);
            }

            if (settings.value(QStringLiteral("tile_fan_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_fan_order"), 0).toInt() == i) {
                fan->setGridId(i);
                dataList.append(fan);
            }

            if (settings.value(QStringLiteral("tile_datetime_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_datetime_order"), 0).toInt() == i) {
                datetime->setGridId(i);
                dataList.append(datetime);
            }

            if (settings.value(QStringLiteral("tile_target_resistance_enabled"), true).toBool() &&
                settings.value(QStringLiteral("tile_target_resistance_order"), 0).toInt() == i) {
                target_resistance->setGridId(i);
                dataList.append(target_resistance);
            }

            if (settings.value(QStringLiteral("tile_lapelapsed_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_lapelapsed_order"), 18).toInt() == i) {
                lapElapsed->setGridId(i);
                dataList.append(lapElapsed);
            }

            if (settings.value(QStringLiteral("tile_watt_kg_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_watt_kg_order"), 24).toInt() == i) {
                wattKg->setGridId(i);
                dataList.append(wattKg);
            }

            if (settings.value(QStringLiteral("tile_remainingtimetrainprogramrow_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_remainingtimetrainprogramrow_order"), 27).toInt() == i) {
                remaningTimeTrainingProgramCurrentRow->setGridId(i);
                dataList.append(remaningTimeTrainingProgramCurrentRow);
            }

            if (settings.value(QStringLiteral("tile_nextrowstrainprogram_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_nextrowtrainprogram_order"), 31).toInt() == i) {

                nextRows->setGridId(i);
                dataList.append(nextRows);
            }

            if (settings.value(QStringLiteral("tile_mets_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_mets_order"), 28).toInt() == i) {
                mets->setGridId(i);
                dataList.append(mets);
            }
            if (settings.value(QStringLiteral("tile_targetmets_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_targetmets_order"), 29).toInt() == i) {
                targetMets->setGridId(i);
                dataList.append(targetMets);
            }

            if (settings.value(QStringLiteral("tile_pid_hr_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_pid_hr_order"), 31).toInt() == i) {
                pidHR->setGridId(i);
                dataList.append(pidHR);
            }

            if (settings.value(QStringLiteral("tile_target_cadence_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_target_cadence_order"), 19).toInt() == i) {
                target_cadence->setGridId(i);
                dataList.append(target_cadence);
            }

            if (settings.value(QStringLiteral("tile_target_speed_enabled"), false).toBool() &&
                settings.value(QStringLiteral("tile_target_speed_order"), 28).toInt() == i) {
                target_speed->setGridId(i);
                dataList.append(target_speed);
            }
        }
    }

    engine->rootContext()->setContextProperty(QStringLiteral("appModel"), QVariant::fromValue(dataList));
}

DataObject *homeform::tileFromName(QString name) {
    foreach (QObject *d, dataList) {
        if (!((DataObject *)d)->name().compare(name)) {
            return (DataObject *)d;
        }
    }
    return nullptr;
}

void homeform::moveTile(QString name, int newIndex, int oldIndex) {
    QSettings settings;
    DataObject *current = tileFromName(name);
    if (current) {
        qDebug() << "moveTile" << name << newIndex << oldIndex;

        foreach (QString s, settings.allKeys()) {
            if (s.contains(QStringLiteral("tile_")) && s.contains(QStringLiteral("_order"))) {

                qDebug() << s << settings.value(s);
            }
        }

        int i = 0;
        foreach (QObject *d, dataList) {
            if (i == newIndex) {
                settings.setValue("tile_" + current->m_id.toLower() + "_order", i);
                i++;
            }
            QString n = ((DataObject *)d)->m_id;
            if (((DataObject *)d)->name().compare(name)) {
                settings.setValue("tile_" + n.toLower() + "_order", i);
                i++;
            }
        }

        foreach (QString s, settings.allKeys()) {
            if (s.contains(QStringLiteral("tile_")) && s.contains(QStringLiteral("_order"))) {

                qDebug() << s << settings.value(s);
            }
        }

        // sortTiles();
        // dataList.move(oldIndex, newIndex);
        // very dirty, but i needed a way to synchronize QML with C++
        QTimer::singleShot(100, this, &homeform::sortTilesTimeout);
    }
}

void homeform::sortTilesTimeout() { sortTiles(); }

void homeform::deviceConnected(QBluetoothDeviceInfo b) {

    qDebug() << "deviceConnected" << bluetoothManager << engine;
    if (bluetoothManager)
        qDebug() << bluetoothManager->device();

    if (bluetoothManager->device() == nullptr)
        return;

    // if the device reconnects in the same session, the tiles shouldn't be created again
    static bool first = false;
    if (first) {
        return;
    }
    first = true;

    if (b.isValid())
        deviceFound(b.name());

    m_labelHelp = false;
    emit changeLabelHelp(m_labelHelp);

    QSettings settings;

    if (settings.value(QStringLiteral("pause_on_start"), false).toBool() &&
        bluetoothManager->device()->deviceType() != bluetoothdevice::TREADMILL) {
        Start();
    } else if (settings.value(QStringLiteral("pause_on_start_treadmill"), false).toBool() &&
               bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
        Start_inner(false); // because if you sent the start command to a treadmill it could start the tape
    }

    sortTiles();

    QObject *rootObject = engine->rootObjects().constFirst();
    QObject *home = rootObject->findChild<QObject *>(QStringLiteral("home"));
    QObject::connect(home, SIGNAL(plus_clicked(QString)), this, SLOT(Plus(QString)));
    QObject::connect(home, SIGNAL(minus_clicked(QString)), this, SLOT(Minus(QString)));

    emit workoutNameChanged(workoutName());
    emit instructorNameChanged(instructorName());
}

void homeform::deviceFound(const QString &name) {
    if (name.trimmed().isEmpty()) {
        return;
    }

    emit bluetoothDevicesChanged(bluetoothDevices());

    if (m_labelHelp == false) {
        return;
    }

    QSettings settings;
    if (!settings.value(QStringLiteral("top_bar_enabled"), true).toBool()) {
        return;
    }
    m_info = name + QStringLiteral(" found");
    emit infoChanged(m_info);
}

void homeform::Plus(const QString &name) {
    QSettings settings;
    bool miles = settings.value(QStringLiteral("miles_unit"), false).toBool();
    qDebug() << QStringLiteral("Plus") << name;
    if (name.contains(QStringLiteral("speed"))) {
        if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            // round up to the next .5 increment (.0 or .5)
            double speed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
            double requestedspeed = ((treadmill *)bluetoothManager->device())->requestedSpeed();
            double targetspeed = ((treadmill *)bluetoothManager->device())->currentTargetSpeed();
            qDebug() << QStringLiteral("Current Speed") << speed << QStringLiteral("Current Requested Speed")
                     << requestedspeed << QStringLiteral("Current Target Speed") << targetspeed;
            if (targetspeed != -1)
                speed = targetspeed;
            if (requestedspeed != -1)
                speed = requestedspeed;
            double minStepSpeed = ((treadmill *)bluetoothManager->device())->minStepSpeed();
            double step = settings.value(QStringLiteral("treadmill_step_speed"), 0.5).toDouble();
            if (!miles)
                step = ((double)qRound(step * 10.0)) / 10.0;
            if (step > minStepSpeed)
                minStepSpeed = step;
            int rest = 0;
            if (!miles)
                rest = (minStepSpeed * 10.0) - (((int)(speed * 10.0)) % (uint8_t)(minStepSpeed * 10.0));
            if (rest == 5 || rest == 0)
                speed = speed + minStepSpeed;
            else
                speed = speed + (((double)rest) / 10.0);

            ((treadmill *)bluetoothManager->device())->changeSpeed(speed);
        }
    } else if (name.contains(QStringLiteral("external_inclination"))) {
        double elite_rizer_gain = settings.value(QStringLiteral("elite_rizer_gain"), 1.0).toDouble();
        elite_rizer_gain = elite_rizer_gain + 0.1;
        settings.setValue(QStringLiteral("elite_rizer_gain"), elite_rizer_gain);
    } else if (name.contains(QStringLiteral("inclination"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                double step = settings.value(QStringLiteral("treadmill_step_incline"), 0.5).toDouble();
                if (step < ((treadmill *)bluetoothManager->device())->minStepInclination())
                    step = ((treadmill *)bluetoothManager->device())->minStepInclination();
                double perc = ((treadmill *)bluetoothManager->device())->currentInclination().value() + step;
                ((treadmill *)bluetoothManager->device())->changeInclination(perc, perc);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                double perc = ((elliptical *)bluetoothManager->device())->currentInclination().value() + 0.5;
                ((elliptical *)bluetoothManager->device())->changeInclination(perc, perc);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                ((bike *)bluetoothManager->device())
                    ->changeInclination(((bike *)bluetoothManager->device())->currentInclination().value() + 0.5,
                                        ((bike *)bluetoothManager->device())->currentInclination().value() + 0.5);
            }
        }
    } else if (name.contains(QStringLiteral("pid_hr"))) {
        if (bluetoothManager->device()) {
            QSettings settings;
            QString zoneS =
                settings.value(QStringLiteral("treadmill_pid_heart_zone"), QStringLiteral("Disabled")).toString();
            uint8_t zone = settings.value(QStringLiteral("treadmill_pid_heart_zone"), QStringLiteral("Disabled"))
                               .toString()
                               .toUInt();

            if (!zoneS.compare(QStringLiteral("Disabled")))
                zone = 0;

            if (zone < 5) {
                zone++;
                settings.setValue(QStringLiteral("treadmill_pid_heart_zone"), QString::number(zone));
            }
        }
    } else if (name.contains("gears")) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                ((bike *)bluetoothManager->device())->setGears(((bike *)bluetoothManager->device())->gears() + 1);
            }
        }
    } else if (name.contains(QStringLiteral("target_resistance"))) {
        if (bluetoothManager->device()) {

            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE ||
                bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL ||
                bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {

                bluetoothManager->device()->setDifficult(bluetoothManager->device()->difficult() + 0.03);
                if (bluetoothManager->device()->difficult() == 0) {
                    bluetoothManager->device()->setDifficult(0.03);
                }

                if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                    ((bike *)bluetoothManager->device())
                        ->changeResistance(((bike *)bluetoothManager->device())->currentResistance().value());
                } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                    ((rower *)bluetoothManager->device())
                        ->changeResistance(((rower *)bluetoothManager->device())->currentResistance().value());
                } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                    ((elliptical *)bluetoothManager->device())
                        ->changeResistance(((elliptical *)bluetoothManager->device())->currentResistance().value());
                }
            }
        }
    } else if (name.contains(QStringLiteral("resistance")) || name.contains(QStringLiteral("peloton_resistance"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                ((bike *)bluetoothManager->device())
                    ->changeResistance(((bike *)bluetoothManager->device())->currentResistance().value() + 1);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                ((rower *)bluetoothManager->device())
                    ->changeResistance(((rower *)bluetoothManager->device())->currentResistance().value() + 1);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                ((elliptical *)bluetoothManager->device())
                    ->changeResistance(((elliptical *)bluetoothManager->device())->currentResistance().value() + 1);
            }
        }
    } else if (name.contains(QStringLiteral("fan"))) {
        QSettings settings;
        if (bluetoothManager->device()) {
            if (settings.value(QStringLiteral("fitmetria_fanfit_enable"), false).toBool() &&
                settings.value(QStringLiteral("fitmetria_fanfit_mode"), QStringLiteral("Heart"))
                    .toString()
                    .compare(QStringLiteral("Manual"))) {
                fanOverride += 10;
            } else if (settings.value(QStringLiteral("fitmetria_fanfit_enable"), false).toBool()) {
                bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() + 10);
            } else
                bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() + 1);
        }
    } else if (name.contains(QStringLiteral("remainingtimetrainprogramrow"))) {
        if (bluetoothManager->device() && trainProgram) {
            trainProgram->increaseElapsedTime(QTime(0, 0, 0).secsTo(trainProgram->currentRowRemainingTime()));
        }
    } else if (name.contains(QStringLiteral("peloton_offset")) || name.contains(QStringLiteral("peloton_remaining"))) {
        if (bluetoothManager->device() && trainProgram) {
            trainProgram->increaseElapsedTime(1);
        }
    } else {
        qDebug() << name << QStringLiteral("not handled");

        qDebug() << "Plus" << name;
    }
}

void homeform::Minus(const QString &name) {
    QSettings settings;
    bool miles = settings.value(QStringLiteral("miles_unit"), false).toBool();
    qDebug() << QStringLiteral("Minus") << name;
    if (name.contains(QStringLiteral("speed"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                // round up to the next .5 increment (.0 or .5)
                double speed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
                double requestedspeed = ((treadmill *)bluetoothManager->device())->requestedSpeed();
                double targetspeed = ((treadmill *)bluetoothManager->device())->currentTargetSpeed();
                qDebug() << QStringLiteral("Current Speed") << speed << QStringLiteral("Current Requested Speed")
                         << requestedspeed << QStringLiteral("Current Target Speed") << targetspeed;
                if (targetspeed != -1)
                    speed = targetspeed;
                if (requestedspeed != -1)
                    speed = requestedspeed;
                double minStepSpeed = ((treadmill *)bluetoothManager->device())->minStepSpeed();
                double step = settings.value(QStringLiteral("treadmill_step_speed"), 0.5).toDouble();
                if (!miles)
                    step = ((double)qRound(step * 10.0)) / 10.0;
                if (step > minStepSpeed)
                    minStepSpeed = step;
                int rest = 0;
                if (!miles)
                    rest = (minStepSpeed * 10.0) - (((int)(speed * 10.0)) % (uint8_t)(minStepSpeed * 10.0));
                if (rest == 5 || rest == 0)
                    speed = speed - minStepSpeed;
                else
                    speed = speed - (((double)rest) / 10.0);
                ((treadmill *)bluetoothManager->device())->changeSpeed(speed);
            }
        }
    } else if (name.contains(QStringLiteral("external_inclination"))) {
        double elite_rizer_gain = settings.value(QStringLiteral("elite_rizer_gain"), 1.0).toDouble();
        if (elite_rizer_gain)
            elite_rizer_gain = elite_rizer_gain - 0.1;
        settings.setValue(QStringLiteral("elite_rizer_gain"), elite_rizer_gain);
    } else if (name.contains(QStringLiteral("inclination"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                double step = settings.value(QStringLiteral("treadmill_step_incline"), 0.5).toDouble();
                if (step < ((treadmill *)bluetoothManager->device())->minStepInclination())
                    step = ((treadmill *)bluetoothManager->device())->minStepInclination();
                double perc = ((treadmill *)bluetoothManager->device())->currentInclination().value() - step;
                ((treadmill *)bluetoothManager->device())->changeInclination(perc, perc);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                double perc = ((elliptical *)bluetoothManager->device())->currentInclination().value() - 0.5;
                ((elliptical *)bluetoothManager->device())->changeInclination(perc, perc);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                ((bike *)bluetoothManager->device())
                    ->changeInclination(((bike *)bluetoothManager->device())->currentInclination().value() - 0.5,
                                        ((bike *)bluetoothManager->device())->currentInclination().value() - 0.5);
            }
        }
    } else if (name.contains(QStringLiteral("pid_hr"))) {
        if (bluetoothManager->device()) {
            QSettings settings;
            uint8_t zone = settings.value(QStringLiteral("treadmill_pid_heart_zone"), QStringLiteral("Disabled"))
                               .toString()
                               .toUInt();
            if (zone > 1) {
                zone--;
                settings.setValue(QStringLiteral("treadmill_pid_heart_zone"), QString::number(zone));
            } else {
                settings.setValue(QStringLiteral("treadmill_pid_heart_zone"), QStringLiteral("Disabled"));
            }
        }
    } else if (name.contains(QStringLiteral("gears"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                ((bike *)bluetoothManager->device())->setGears(((bike *)bluetoothManager->device())->gears() - 1);
            }
        }
    } else if (name.contains(QStringLiteral("target_resistance"))) {
        if (bluetoothManager->device()) {

            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE ||
                bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL ||
                bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {

                bluetoothManager->device()->setDifficult(bluetoothManager->device()->difficult() - 0.03);
                if (bluetoothManager->device()->difficult() == 0) {
                    bluetoothManager->device()->setDifficult(-0.03);
                }

                if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                    ((bike *)bluetoothManager->device())
                        ->changeResistance(((bike *)bluetoothManager->device())->currentResistance().value());
                } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                    ((rower *)bluetoothManager->device())
                        ->changeResistance(((rower *)bluetoothManager->device())->currentResistance().value());
                } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                    ((elliptical *)bluetoothManager->device())
                        ->changeResistance(((elliptical *)bluetoothManager->device())->currentResistance().value());
                }
            }
        }
    } else if (name.contains(QStringLiteral("resistance")) || name.contains(QStringLiteral("peloton_resistance"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                ((bike *)bluetoothManager->device())
                    ->changeResistance(((bike *)bluetoothManager->device())->currentResistance().value() - 1);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                ((rower *)bluetoothManager->device())
                    ->changeResistance(((rower *)bluetoothManager->device())->currentResistance().value() - 1);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                ((elliptical *)bluetoothManager->device())
                    ->changeResistance(((elliptical *)bluetoothManager->device())->currentResistance().value() - 1);
            }
        }
    } else if (name.contains(QStringLiteral("fan"))) {

        if (bluetoothManager->device()) {
            QSettings settings;
            if (settings.value(QStringLiteral("fitmetria_fanfit_enable"), false).toBool() &&
                settings.value(QStringLiteral("fitmetria_fanfit_mode"), QStringLiteral("Heart"))
                    .toString()
                    .compare(QStringLiteral("Manual"))) {
                fanOverride -= 10;
            } else if (settings.value(QStringLiteral("fitmetria_fanfit_enable"), false).toBool()) {
                bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() - 10);
            } else
                bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() - 1);
        }
    } else if (name.contains(QStringLiteral("remainingtimetrainprogramrow"))) {
        if (bluetoothManager->device() && trainProgram) {
            // Offset:
            // 1. To account for current tick
            // 2. To bounce back to previous
            trainProgram->decreaseElapsedTime(QTime(0, 0, 0).secsTo(trainProgram->currentRowElapsedTime()) + 2);
        }
    } else if (name.contains(QStringLiteral("peloton_offset")) || name.contains(QStringLiteral("peloton_remaining"))) {
        if (bluetoothManager->device() && trainProgram) {
            trainProgram->decreaseElapsedTime(1);
        }
    } else {
        qDebug() << name << QStringLiteral("not handled");
        qDebug() << "Minus" << name;
    }
}

void homeform::Start() { Start_inner(true); }

void homeform::Start_inner(bool send_event_to_device) {
    QSettings settings;
    qDebug() << QStringLiteral("Start pressed - paused") << paused << QStringLiteral("stopped") << stopped;

    if (settings.value(QStringLiteral("tts_enabled"), false).toBool())
        m_speech.say("Start pressed");

    if (!paused && !stopped) {

        paused = true;
        if (bluetoothManager->device() && send_event_to_device) {
            bluetoothManager->device()->stop();
        }
        emit workoutEventStateChanged(bluetoothdevice::PAUSED);
    } else {

        if (bluetoothManager->device() && send_event_to_device) {
            bluetoothManager->device()->start();
        }

        if (stopped) {
            trainProgram->restart();
            if (bluetoothManager->device()) {

                bluetoothManager->device()->clearStats();
            }
            Session.clear();
            chartImagesFilenames.clear();

            if (!pelotonHandler || (pelotonHandler && !pelotonHandler->isWorkoutInProgress())) {
                stravaPelotonActivityName = QLatin1String("");
                stravaPelotonInstructorName = QLatin1String("");
                movieFileName = QLatin1String("");
                stravaWorkoutName = QLatin1String("");
                stravaPelotonWorkoutType = FIT_SPORT_INVALID;
                emit workoutNameChanged(workoutName());
                emit instructorNameChanged(instructorName());
            }
            emit workoutEventStateChanged(bluetoothdevice::STARTED);
        } else {
            // if loading a training program (gpx or xml) directly from the startup of QZ, there is no way to start the
            // program otherwise
            if (!trainProgram->isStarted()) {
                qDebug() << QStringLiteral("starting training program from a resume");
                trainProgram->restart();
            }
            emit workoutEventStateChanged(bluetoothdevice::RESUMED);
        }

        paused = false;
        stopped = false;
    }

    if (settings.value(QStringLiteral("top_bar_enabled"), true).toBool()) {

        emit stopIconChanged(stopIcon());
        emit stopTextChanged(stopText());
        emit stopColorChanged(stopColor());
        emit startIconChanged(startIcon());
        emit startTextChanged(startText());
        emit startColorChanged(startColor());
    }

    if (bluetoothManager->device()) {
        bluetoothManager->device()->setPaused(paused | stopped);
    }
}

void homeform::Stop() {
    QSettings settings;
    qDebug() << QStringLiteral("Stop pressed - paused") << paused << QStringLiteral("stopped") << stopped;

    if (settings.value(QStringLiteral("tts_enabled"), false).toBool())
        m_speech.say("Stop pressed");

    if (bluetoothManager->device()) {

        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            QTime zero(0, 0, 0, 0);
            if (bluetoothManager->device()->currentSpeed().value() == 0.0 &&
                zero.secsTo(bluetoothManager->device()->elapsedTime()) == 0) {
                qDebug() << QStringLiteral("Stop pressed - nothing to do. Elapsed time is 0 and current speed is 0");
                return;
            }
        }

        bluetoothManager->device()->stop();
    }

    paused = false;
    stopped = true;

    emit workoutEventStateChanged(bluetoothdevice::STOPPED);

    fit_save_clicked();

    if (bluetoothManager->device()) {
        bluetoothManager->device()->setPaused(paused | stopped);
    }

    if (settings.value(QStringLiteral("top_bar_enabled"), true).toBool()) {

        emit stopIconChanged(stopIcon());
        emit stopTextChanged(stopText());
        emit stopColorChanged(stopColor());
        emit startIconChanged(startIcon());
        emit startTextChanged(startText());
        emit startColorChanged(startColor());
    }

    if (trainProgram) {
        trainProgram->clearRows();
    }
}

void homeform::Lap() {
    qDebug() << QStringLiteral("lap pressed");
    if (bluetoothManager) {
        if (bluetoothManager->device()) {

            bluetoothManager->device()->setLap();
            lapTrigger = true;
        }
    }
}

bool homeform::labelHelp() { return m_labelHelp; }

QString homeform::stopText() {

    QSettings settings;
    if (settings.value(QStringLiteral("top_bar_enabled"), true).toBool()) {
        return QStringLiteral("Stop");
    }
    return QLatin1String("");
}

QString homeform::stopIcon() { return QStringLiteral("icons/icons/stop.png"); }

QString homeform::startText() {

    QSettings settings;
    if (settings.value(QStringLiteral("top_bar_enabled"), true).toBool()) {
        if (paused || stopped) {
            return QStringLiteral("Start");
        } else {
            return QStringLiteral("Pause");
        }
    }
    return QLatin1String("");
}

QString homeform::startIcon() {

    QSettings settings;
    if (settings.value(QStringLiteral("top_bar_enabled"), true).toBool()) {
        if (paused || stopped) {
            return QStringLiteral("icons/icons/start.png");
        } else {
            return QStringLiteral("icons/icons/pause.png");
        }
    }
    return QLatin1String("");
}

QString homeform::signal() {
    if (!bluetoothManager) {
        return QStringLiteral("icons/icons/signal-1.png");
    }

    if (!bluetoothManager->device()) {
        return QStringLiteral("icons/icons/signal-1.png");
    }

    int16_t rssi = bluetoothManager->device()->bluetoothDevice.rssi();
    if (rssi > -40) {
        return QStringLiteral("icons/icons/signal-3.png");
    } else if (rssi > -60) {
        return QStringLiteral("icons/icons/signal-2.png");
    }

    return QStringLiteral("icons/icons/signal-1.png");
}

void homeform::update() {

    QSettings settings;
    double currentHRZone = 1;
    double ftpZone = 1;

    if (settings.status() != QSettings::NoError) {
        qDebug() << "!!!!QSETTINGS ERROR!" << settings.status();
    }

    if ((paused || stopped) && settings.value(QStringLiteral("top_bar_enabled"), true).toBool()) {

        emit stopIconChanged(stopIcon());
        emit stopTextChanged(stopText());
        emit startIconChanged(startIcon());
        emit startTextChanged(startText());
        emit startColorChanged(startColor());
        emit stopColorChanged(stopColor());
    }

    if (bluetoothManager->device()) {

        double inclination = 0;
        double resistance = 0;
        double watts = 0;
        double pace = 0;
        double peloton_resistance = 0;
        uint8_t cadence = 0;
        uint32_t totalStrokes = 0;
        double avgStrokesRate = 0;
        double maxStrokesRate = 0;
        double avgStrokesLength = 0;
        double strideLength = 0;
        double groundContact = 0;
        double verticalOscillation = 0;

        bool miles = settings.value(QStringLiteral("miles_unit"), false).toBool();
        double ftpSetting = settings.value(QStringLiteral("ftp"), 200.0).toDouble();
        double unit_conversion = 1.0;
        double meter_feet_conversion = 1.0;
        bool power5s = settings.value(QStringLiteral("power_avg_5s"), false).toBool();
        uint8_t treadmill_pid_heart_zone =
            settings.value(QStringLiteral("treadmill_pid_heart_zone"), QStringLiteral("Disabled")).toString().toUInt();
        QString treadmill_pid_heart_zone_string =
            settings.value(QStringLiteral("treadmill_pid_heart_zone"), QStringLiteral("Disabled")).toString();
        if (!treadmill_pid_heart_zone_string.compare(QStringLiteral("Disabled")))
            treadmill_pid_heart_zone = 0;

        if (miles) {
            unit_conversion = 0.621371;
            meter_feet_conversion = 3.28084;
        }

        emit signalChanged(signal());
        emit currentSpeedChanged(bluetoothManager->device()->currentSpeed().value());
        speed->setValue(QString::number(bluetoothManager->device()->currentSpeed().value() * unit_conversion, 'f', 1));
        speed->setSecondLine(
            QStringLiteral("AVG: ") +
            QString::number((bluetoothManager->device())->currentSpeed().average() * unit_conversion, 'f', 1) +
            QStringLiteral(" MAX: ") +
            QString::number((bluetoothManager->device())->currentSpeed().max() * unit_conversion, 'f', 1));
        heart->setValue(QString::number(bluetoothManager->device()->currentHeart().value(), 'f', 0));

        calories->setValue(QString::number(bluetoothManager->device()->calories().value(), 'f', 0));
        calories->setSecondLine(QString::number(bluetoothManager->device()->calories().rate1s() * 60.0, 'f', 1) +
                                " /min");
        if (!settings.value(QStringLiteral("fitmetria_fanfit_enable"), false).toBool())
            fan->setValue(QString::number(bluetoothManager->device()->fanSpeed()));
        else
            fan->setValue(QString::number(qRound(((double)bluetoothManager->device()->fanSpeed()) / 10.0) * 10.0));
        jouls->setValue(QString::number(bluetoothManager->device()->jouls().value() / 1000.0, 'f', 1));
        jouls->setSecondLine(QString::number(bluetoothManager->device()->jouls().rate1s() / 1000.0 * 60.0, 'f', 1) +
                             " /min");
        elapsed->setValue(bluetoothManager->device()->elapsedTime().toString(QStringLiteral("h:mm:ss")));
        moving_time->setValue(bluetoothManager->device()->movingTime().toString(QStringLiteral("h:mm:ss")));
        pidHR->setValue(QString::number(treadmill_pid_heart_zone));

        if (trainProgram) {
            peloton_offset->setValue(QString::number(trainProgram->offsetElapsedTime()) + QStringLiteral(" sec."));
            peloton_remaining->setValue(trainProgram->remainingTime().toString("h:mm:ss"));
            peloton_remaining->setSecondLine(QString::number(trainProgram->offsetElapsedTime()) +
                                             QStringLiteral(" sec."));
            remaningTimeTrainingProgramCurrentRow->setValue(
                trainProgram->currentRowRemainingTime().toString(QStringLiteral("h:mm:ss")));
            remaningTimeTrainingProgramCurrentRow->setSecondLine(
                trainProgram->currentRowElapsedTime().toString(QStringLiteral("h:mm:ss")));
            targetMets->setValue(QString::number(trainProgram->currentTargetMets(), 'f', 1));
            trainrow next = trainProgram->getRowFromCurrent(1);
            trainrow next_1 = trainProgram->getRowFromCurrent(2);
            if (next.duration.second() != 0 || next.duration.minute() != 0 || next.duration.hour() != 0) {
                if (next.requested_peloton_resistance != -1)
                    nextRows->setValue(QStringLiteral("PR") + QString::number(next.requested_peloton_resistance) +
                                       QStringLiteral(" ") + next.duration.toString(QStringLiteral("mm:ss")));
                else if (next.resistance != -1)
                    nextRows->setValue(QStringLiteral("R") + QString::number(next.resistance) + QStringLiteral(" ") +
                                       next.duration.toString(QStringLiteral("mm:ss")));
                else if (next.power != -1) {
                    double ftpPerc = (next.power / ftpSetting) * 100.0;
                    uint8_t ftpZone = 1;
                    if (ftpPerc < 56) {
                        ftpZone = 1;
                    } else if (ftpPerc < 76) {
                        ftpZone = 2;
                    } else if (ftpPerc < 91) {
                        ftpZone = 3;
                    } else if (ftpPerc < 106) {
                        ftpZone = 4;
                    } else if (ftpPerc < 121) {
                        ftpZone = 5;
                    } else if (ftpPerc < 151) {
                        ftpZone = 6;
                    } else {
                        ftpZone = 7;
                    }
                    nextRows->setValue(QStringLiteral("Z") + QString::number(ftpZone) + QStringLiteral(" ") +
                                       next.duration.toString(QStringLiteral("mm:ss")));
                    if (next_1.duration.second() != 0 || next_1.duration.minute() != 0 || next_1.duration.hour() != 0) {
                        if (next_1.requested_peloton_resistance != -1)
                            nextRows->setSecondLine(
                                QStringLiteral("PR") + QString::number(next_1.requested_peloton_resistance) +
                                QStringLiteral(" ") + next_1.duration.toString(QStringLiteral("mm:ss")));
                        else if (next_1.resistance != -1)
                            nextRows->setSecondLine(QStringLiteral("R") + QString::number(next_1.resistance) +
                                                    QStringLiteral(" ") +
                                                    next_1.duration.toString(QStringLiteral("mm:ss")));
                        else if (next_1.power != -1) {
                            double ftpPerc = (next_1.power / ftpSetting) * 100.0;
                            uint8_t ftpZone = 1;
                            if (ftpPerc < 56) {
                                ftpZone = 1;
                            } else if (ftpPerc < 76) {
                                ftpZone = 2;
                            } else if (ftpPerc < 91) {
                                ftpZone = 3;
                            } else if (ftpPerc < 106) {
                                ftpZone = 4;
                            } else if (ftpPerc < 121) {
                                ftpZone = 5;
                            } else if (ftpPerc < 151) {
                                ftpZone = 6;
                            } else {
                                ftpZone = 7;
                            }
                            nextRows->setSecondLine(QStringLiteral("Z") + QString::number(ftpZone) +
                                                    QStringLiteral(" ") +
                                                    next_1.duration.toString(QStringLiteral("mm:ss")));
                        }
                    } else {
                        nextRows->setSecondLine(QStringLiteral("N/A"));
                    }
                }
            } else {
                nextRows->setValue(QStringLiteral("N/A"));
            }
        }
        mets->setValue(QString::number(bluetoothManager->device()->currentMETS().value(), 'f', 1));
        mets->setSecondLine(
            QStringLiteral("AVG: ") + QString::number(bluetoothManager->device()->currentMETS().average(), 'f', 1) +
            QStringLiteral("MAX: ") + QString::number(bluetoothManager->device()->currentMETS().max(), 'f', 1));
        lapElapsed->setValue(bluetoothManager->device()->lapElapsedTime().toString(QStringLiteral("h:mm:ss")));
        avgWatt->setValue(QString::number(bluetoothManager->device()->wattsMetric().average(), 'f', 0));
        wattKg->setValue(QString::number(bluetoothManager->device()->wattKg().value(), 'f', 1));
        wattKg->setSecondLine(
            QStringLiteral("AVG: ") + QString::number(bluetoothManager->device()->wattKg().average(), 'f', 1) +
            QStringLiteral("MAX: ") + QString::number(bluetoothManager->device()->wattKg().max(), 'f', 1));
        datetime->setValue(QTime::currentTime().toString(QStringLiteral("hh:mm:ss")));
        if (power5s)
            watts = bluetoothManager->device()->wattsMetric().average5s();
        else
            watts = bluetoothManager->device()->wattsMetric().value();
        watt->setValue(QString::number(watts, 'f', 0));
        weightLoss->setValue(QString::number(miles ? bluetoothManager->device()->weightLoss() * 35.274
                                                   : bluetoothManager->device()->weightLoss(),
                                             'f', 2));

        cadence = bluetoothManager->device()->currentCadence().value();
        this->cadence->setValue(QString::number(cadence));
        this->cadence->setSecondLine(
            QStringLiteral("AVG: ") +
            QString::number(((bike *)bluetoothManager->device())->currentCadence().average(), 'f', 0) +
            QStringLiteral(" MAX: ") +
            QString::number(((bike *)bluetoothManager->device())->currentCadence().max(), 'f', 0));

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        if (settings.value(QStringLiteral("volume_change_gears"), false).toBool()) {
            lockscreen h;
            static double volumeLast = -1;
            double currentVolume = h.getVolume() * 10.0;
            qDebug() << "volume" << volumeLast << currentVolume;
            if (volumeLast == -1)
                qDebug() << "volume init";
            else if (volumeLast > currentVolume) {
                double diff = volumeLast - currentVolume;
                for (int i = 0; i < diff; i++)
                    Minus(QStringLiteral("gears"));
            } else if (volumeLast < currentVolume) {
                double diff = currentVolume - volumeLast;
                for (int i = 0; i < diff; i++)
                    Plus(QStringLiteral("gears"));
            }
            volumeLast = currentVolume;
        }
#endif
#endif

        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {

            odometer->setValue(QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 2));
            if (bluetoothManager->device()->currentSpeed().value()) {
                pace = 10000 / (((treadmill *)bluetoothManager->device())->currentPace().second() +
                                (((treadmill *)bluetoothManager->device())->currentPace().minute() * 60));
                if (pace < 0) {
                    pace = 0;
                }
            } else {

                pace = 0;
            }
            strideLength = ((treadmill *)bluetoothManager->device())->currentStrideLength().value();
            groundContact = ((treadmill *)bluetoothManager->device())->currentGroundContact().value();
            verticalOscillation = ((treadmill *)bluetoothManager->device())->currentVerticalOscillation().value();
            inclination = ((treadmill *)bluetoothManager->device())->currentInclination().value();
            this->pace->setValue(
                ((treadmill *)bluetoothManager->device())->currentPace().toString(QStringLiteral("m:ss")));
            this->pace->setSecondLine(
                QStringLiteral("AVG: ") +
                ((treadmill *)bluetoothManager->device())->averagePace().toString(QStringLiteral("m:ss")) +
                QStringLiteral(" MAX: ") +
                ((treadmill *)bluetoothManager->device())->maxPace().toString(QStringLiteral("m:ss")));
            this->inclination->setValue(QString::number(inclination, 'f', 1));
            this->inclination->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentInclination().average(), 'f', 1) +
                QStringLiteral(" MAX: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentInclination().max(), 'f', 1));
            elevation->setValue(QString::number(((treadmill *)bluetoothManager->device())->elevationGain().value() *
                                                    meter_feet_conversion,
                                                'f', (miles ? 0 : 1)));
            elevation->setSecondLine(
                QString::number(((treadmill *)bluetoothManager->device())->elevationGain().rate1s() * 60.0 *
                                    meter_feet_conversion,
                                'f', (miles ? 0 : 1)) +
                " /min");
            this->instantaneousStrideLengthCM->setValue(QString::number(strideLength, 'f', 0));
            this->instantaneousStrideLengthCM->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentStrideLength().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentStrideLength().max(), 'f', 0));

            this->groundContactMS->setValue(QString::number(groundContact, 'f', 0));
            this->groundContactMS->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentGroundContact().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentGroundContact().max(), 'f', 0));

            this->verticalOscillationMM->setValue(QString::number(verticalOscillation, 'f', 0));
            this->verticalOscillationMM->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentVerticalOscillation().average(), 'f',
                                0) +
                QStringLiteral(" MAX: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentVerticalOscillation().max(), 'f', 0));

            if (bluetoothManager->device()->currentSpeed().value() < 9) {
                speed->setValueFontColor(QStringLiteral("white"));
                this->pace->setValueFontColor(QStringLiteral("white"));
            } else if (bluetoothManager->device()->currentSpeed().value() < 10) {
                speed->setValueFontColor(QStringLiteral("limegreen"));
                this->pace->setValueFontColor(QStringLiteral("limegreen"));
            } else if (bluetoothManager->device()->currentSpeed().value() < 11) {
                speed->setValueFontColor(QStringLiteral("gold"));
                this->pace->setValueFontColor(QStringLiteral("gold"));
            } else if (bluetoothManager->device()->currentSpeed().value() < 12) {
                speed->setValueFontColor(QStringLiteral("orange"));
                this->pace->setValueFontColor(QStringLiteral("orange"));
            } else if (bluetoothManager->device()->currentSpeed().value() < 13) {
                speed->setValueFontColor(QStringLiteral("darkorange"));
                this->pace->setValueFontColor(QStringLiteral("darkorange"));
            } else if (bluetoothManager->device()->currentSpeed().value() < 14) {
                speed->setValueFontColor(QStringLiteral("orangered"));
                this->pace->setValueFontColor(QStringLiteral("orangered"));
            } else {
                speed->setValueFontColor(QStringLiteral("red"));
                this->pace->setValueFontColor(QStringLiteral("red"));
            }

            this->target_speed->setValue(QString::number(
                ((treadmill *)bluetoothManager->device())->lastRequestedSpeed().value() * unit_conversion, 'f', 1));
            this->target_incline->setValue(
                QString::number(((treadmill *)bluetoothManager->device())->lastRequestedInclination().value(), 'f', 1));

            // originally born for #470. When the treadmill reaches the 0 speed it enters in the pause mode
            // so this logic should care about sync the treadmill state to the UI state
            if (((treadmill *)bluetoothManager->device())->autoPauseWhenSpeedIsZero() &&
                bluetoothManager->device()->currentSpeed().value() == 0 && paused == false && stopped == false) {
                qDebug() << QStringLiteral("autoPauseWhenSpeedIsZero!");
                Start_inner(false);
            } else if (((treadmill *)bluetoothManager->device())->autoStartWhenSpeedIsGreaterThenZero() &&
                       bluetoothManager->device()->currentSpeed().value() > 0 && (paused == true || stopped == true)) {
                qDebug() << QStringLiteral("autoStartWhenSpeedIsGreaterThenZero!");
                Start_inner(false);
            }

        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {

            bool pelotoncadence = settings.value(QStringLiteral("bike_cadence_sensor"), false).toBool();

            if (!pelotoncadence) {
                inclination = ((bike *)bluetoothManager->device())->currentInclination().value();
                this->inclination->setValue(QString::number(inclination, 'f', 1));
                this->inclination->setSecondLine(
                    QStringLiteral("AVG: ") +
                    QString::number(((bike *)bluetoothManager->device())->currentInclination().average(), 'f', 1) +
                    QStringLiteral(" MAX: ") +
                    QString::number(((bike *)bluetoothManager->device())->currentInclination().max(), 'f', 1));
            }
            if (bluetoothManager->externalInclination())
                extIncline->setValue(
                    QString::number(bluetoothManager->externalInclination()->currentInclination().value(), 'f', 1));
            double elite_rizer_gain = settings.value(QStringLiteral("elite_rizer_gain"), 1.0).toDouble();
            extIncline->setSecondLine(QStringLiteral("Gain: ") + QString::number(elite_rizer_gain, 'f', 1));
            odometer->setValue(QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 2));
            resistance = ((bike *)bluetoothManager->device())->currentResistance().value();
            peloton_resistance = ((bike *)bluetoothManager->device())->pelotonResistance().value();
            this->peloton_resistance->setValue(QString::number(peloton_resistance, 'f', 0));
            this->target_resistance->setValue(
                QString::number(((bike *)bluetoothManager->device())->lastRequestedResistance().value(), 'f', 0));
            this->target_peloton_resistance->setValue(QString::number(
                ((bike *)bluetoothManager->device())->lastRequestedPelotonResistance().value(), 'f', 0));
            this->target_cadence->setValue(
                QString::number(((bike *)bluetoothManager->device())->lastRequestedCadence().value(), 'f', 0));
            this->target_power->setValue(
                QString::number(((bike *)bluetoothManager->device())->lastRequestedPower().value(), 'f', 0));
            this->resistance->setValue(QString::number(resistance, 'f', 0));
            this->gears->setValue(QString::number(((bike *)bluetoothManager->device())->gears()));

            this->resistance->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((bike *)bluetoothManager->device())->currentResistance().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((bike *)bluetoothManager->device())->currentResistance().max(), 'f', 0));
            this->peloton_resistance->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((bike *)bluetoothManager->device())->pelotonResistance().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((bike *)bluetoothManager->device())->pelotonResistance().max(), 'f', 0));
            this->target_resistance->setSecondLine(
                QString::number(bluetoothManager->device()->difficult() * 100.0, 'f', 0) + QStringLiteral("% @0%=") +
                QString::number(bluetoothManager->device()->difficult() *
                                    settings.value(QStringLiteral("bike_resistance_gain_f"), 1.0).toDouble() *
                                    settings.value(QStringLiteral("bike_resistance_offset"), 4.0).toDouble(),
                                'f', 0));

            elevation->setValue(
                QString::number(((bike *)bluetoothManager->device())->elevationGain().value() * meter_feet_conversion,
                                'f', (miles ? 0 : 1)));
            elevation->setSecondLine(QString::number(((bike *)bluetoothManager->device())->elevationGain().rate1s() *
                                                         60.0 * meter_feet_conversion,
                                                     'f', (miles ? 0 : 1)) +
                                     " /min");

            this->steeringAngle->setValue(
                QString::number(((bike *)bluetoothManager->device())->currentSteeringAngle().value(), 'f', 1));

        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
            if (bluetoothManager->device()->currentSpeed().value()) {
                pace = 10000 / (((rower *)bluetoothManager->device())->currentPace().second() +
                                (((rower *)bluetoothManager->device())->currentPace().minute() * 60));
                if (pace < 0) {
                    pace = 0;
                }
            } else {

                pace = 0;
            }
            this->pace->setValue(((rower *)bluetoothManager->device())->currentPace().toString(QStringLiteral("m:ss")));
            this->pace->setSecondLine(
                QStringLiteral("AVG: ") +
                ((rower *)bluetoothManager->device())->averagePace().toString(QStringLiteral("m:ss")) +
                QStringLiteral(" MAX: ") +
                ((rower *)bluetoothManager->device())->maxPace().toString(QStringLiteral("m:ss")));
            odometer->setValue(QString::number(bluetoothManager->device()->odometer() * 1000.0, 'f', 0));
            resistance = ((rower *)bluetoothManager->device())->currentResistance().value();
            peloton_resistance = ((rower *)bluetoothManager->device())->pelotonResistance().value();
            totalStrokes = ((rower *)bluetoothManager->device())->currentStrokesCount().value();
            avgStrokesRate = ((rower *)bluetoothManager->device())->currentCadence().average();
            maxStrokesRate = ((rower *)bluetoothManager->device())->currentCadence().max();
            avgStrokesLength = ((rower *)bluetoothManager->device())->currentStrokesLength().average();
            this->strokesCount->setValue(
                QString::number(((rower *)bluetoothManager->device())->currentStrokesCount().value(), 'f', 0));
            this->strokesLength->setValue(
                QString::number(((rower *)bluetoothManager->device())->currentStrokesLength().value(), 'f', 1));

            this->peloton_resistance->setValue(QString::number(peloton_resistance, 'f', 0));
            this->target_resistance->setValue(
                QString::number(((rower *)bluetoothManager->device())->lastRequestedResistance().value(), 'f', 0));
            this->target_peloton_resistance->setValue(QString::number(
                ((rower *)bluetoothManager->device())->lastRequestedPelotonResistance().value(), 'f', 0));
            this->target_cadence->setValue(
                QString::number(((rower *)bluetoothManager->device())->lastRequestedCadence().value(), 'f', 0));
            this->target_power->setValue(
                QString::number(((rower *)bluetoothManager->device())->lastRequestedPower().value(), 'f', 0));
            this->resistance->setValue(QString::number(resistance, 'f', 0));

            this->resistance->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((rower *)bluetoothManager->device())->currentResistance().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((rower *)bluetoothManager->device())->currentResistance().max(), 'f', 0));
            this->peloton_resistance->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((rower *)bluetoothManager->device())->pelotonResistance().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((rower *)bluetoothManager->device())->pelotonResistance().max(), 'f', 0));
            this->target_resistance->setSecondLine(
                QString::number(bluetoothManager->device()->difficult() * 100.0, 'f', 0) + QStringLiteral("% @0%=") +
                QString::number(bluetoothManager->device()->difficult() *
                                    settings.value(QStringLiteral("bike_resistance_gain_f"), 1.0).toDouble() *
                                    settings.value(QStringLiteral("bike_resistance_offset"), 4.0).toDouble(),
                                'f', 0));
            this->strokesLength->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((rower *)bluetoothManager->device())->currentStrokesLength().average(), 'f', 1) +
                QStringLiteral(" MAX: ") +
                QString::number(((rower *)bluetoothManager->device())->currentStrokesLength().max(), 'f', 1));
            if (bluetoothManager->device()->currentSpeed().value() < 4) {
                speed->setValueFontColor(QStringLiteral("white"));
                this->pace->setValueFontColor(QStringLiteral("white"));
            } else if (bluetoothManager->device()->currentSpeed().value() < 5) {
                speed->setValueFontColor(QStringLiteral("limegreen"));
                this->pace->setValueFontColor(QStringLiteral("limegreen"));
            } else if (bluetoothManager->device()->currentSpeed().value() < 5.5) {
                speed->setValueFontColor(QStringLiteral("gold"));
                this->pace->setValueFontColor(QStringLiteral("gold"));
            } else if (bluetoothManager->device()->currentSpeed().value() < 6) {
                speed->setValueFontColor(QStringLiteral("orange"));
                this->pace->setValueFontColor(QStringLiteral("orange"));
            } else if (bluetoothManager->device()->currentSpeed().value() < 6.5) {
                speed->setValueFontColor(QStringLiteral("darkorange"));
                this->pace->setValueFontColor(QStringLiteral("darkorange"));
            } else if (bluetoothManager->device()->currentSpeed().value() < 7) {
                speed->setValueFontColor(QStringLiteral("orangered"));
                this->pace->setValueFontColor(QStringLiteral("orangered"));
            } else {
                speed->setValueFontColor(QStringLiteral("red"));
                this->pace->setValueFontColor(QStringLiteral("red"));
            }
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {

            odometer->setValue(QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 2));
            resistance = ((elliptical *)bluetoothManager->device())->currentResistance().value();
            peloton_resistance = ((elliptical *)bluetoothManager->device())->pelotonResistance().value();
            this->peloton_resistance->setValue(QString::number(peloton_resistance, 'f', 0));
            this->target_resistance->setValue(
                QString::number(((elliptical *)bluetoothManager->device())->lastRequestedResistance().value(), 'f', 0));
            this->target_peloton_resistance->setValue(QString::number(
                ((elliptical *)bluetoothManager->device())->lastRequestedPelotonResistance().value(), 'f', 0));
            this->resistance->setValue(QString::number(resistance));
            this->peloton_resistance->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((elliptical *)bluetoothManager->device())->pelotonResistance().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((elliptical *)bluetoothManager->device())->pelotonResistance().max(), 'f', 0));
            this->target_resistance->setSecondLine(
                QString::number(bluetoothManager->device()->difficult() * 100.0, 'f', 0) + QStringLiteral("% @0%=") +
                QString::number(bluetoothManager->device()->difficult() *
                                    settings.value(QStringLiteral("bike_resistance_gain_f"), 1.0).toDouble() *
                                    settings.value(QStringLiteral("bike_resistance_offset"), 4.0).toDouble(),
                                'f', 0));
            inclination = ((elliptical *)bluetoothManager->device())->currentInclination().value();
            this->inclination->setValue(QString::number(inclination, 'f', 1));
            this->inclination->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((elliptical *)bluetoothManager->device())->currentInclination().average(), 'f', 1) +
                QStringLiteral(" MAX: ") +
                QString::number(((elliptical *)bluetoothManager->device())->currentInclination().max(), 'f', 1));
            elevation->setValue(QString::number(((elliptical *)bluetoothManager->device())->elevationGain().value() *
                                                    meter_feet_conversion,
                                                'f', (miles ? 0 : 1)));
            elevation->setSecondLine(
                QString::number(((elliptical *)bluetoothManager->device())->elevationGain().rate1s() * 60.0 *
                                    meter_feet_conversion,
                                'f', (miles ? 0 : 1)) +
                " /min");
            this->target_speed->setValue(QString::number(
                ((elliptical *)bluetoothManager->device())->lastRequestedSpeed().value() * unit_conversion, 'f', 1));

            this->target_cadence->setValue(
                QString::number(((elliptical *)bluetoothManager->device())->lastRequestedCadence().value(), 'f', 0));
        }
        watt->setSecondLine(
            QStringLiteral("AVG: ") + QString::number((bluetoothManager->device())->wattsMetric().average(), 'f', 0) +
            QStringLiteral(" MAX: ") + QString::number((bluetoothManager->device())->wattsMetric().max(), 'f', 0));

        if (trainProgram) {
            int8_t lower_requested_peloton_resistance = trainProgram->currentRow().lower_requested_peloton_resistance;
            int8_t upper_requested_peloton_resistance = trainProgram->currentRow().upper_requested_peloton_resistance;
            if (lower_requested_peloton_resistance != -1) {
                this->target_peloton_resistance->setSecondLine(
                    QStringLiteral("MIN: ") + QString::number(lower_requested_peloton_resistance, 'f', 0) +
                    QStringLiteral(" MAX: ") + QString::number(upper_requested_peloton_resistance, 'f', 0));
            } else {
                this->target_peloton_resistance->setSecondLine(QLatin1String(""));
            }

            if (settings.value(QStringLiteral("tile_peloton_resistance_color_enabled"), false).toBool()) {
                if (lower_requested_peloton_resistance == -1) {
                    this->peloton_resistance->setValueFontColor(QStringLiteral("white"));
                } else if (((int8_t)peloton_resistance) < lower_requested_peloton_resistance) {
                    this->peloton_resistance->setValueFontColor(QStringLiteral("red"));
                } else if (((int8_t)peloton_resistance) <= upper_requested_peloton_resistance) {
                    this->peloton_resistance->setValueFontColor(QStringLiteral("limegreen"));
                } else {
                    this->peloton_resistance->setValueFontColor(QStringLiteral("orange"));
                }
            }

            int16_t lower_cadence = trainProgram->currentRow().lower_cadence;
            int16_t upper_cadence = trainProgram->currentRow().upper_cadence;
            if (lower_cadence != -1) {
                this->target_cadence->setSecondLine(QStringLiteral("MIN: ") + QString::number(lower_cadence, 'f', 0) +
                                                    QStringLiteral(" MAX: ") + QString::number(upper_cadence, 'f', 0));
            } else {
                this->target_cadence->setSecondLine(QLatin1String(""));
            }

            if (settings.value(QStringLiteral("tile_cadence_color_enabled"), false).toBool()) {
                if (lower_cadence == -1) {
                    this->cadence->setValueFontColor(QStringLiteral("white"));
                } else if (cadence < lower_cadence) {
                    this->cadence->setValueFontColor(QStringLiteral("red"));
                } else if (cadence <= upper_cadence) {
                    this->cadence->setValueFontColor(QStringLiteral("limegreen"));
                } else {
                    this->cadence->setValueFontColor(QStringLiteral("orange"));
                }
            }
        }

        double ftpPerc = 0;
        QString ftpMinW = QStringLiteral("0");
        QString ftpMaxW = QStringLiteral("0");
        double requestedPerc = 0;
        double requestedZone = 1;
        QString requestedMinW = QStringLiteral("0");
        QString requestedMaxW = QStringLiteral("0");

        if (ftpSetting > 0) {
            ftpPerc = (watts / ftpSetting) * 100.0;
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                requestedPerc =
                    (((bike *)bluetoothManager->device())->lastRequestedPower().value() / ftpSetting) * 100.0;
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                requestedPerc =
                    (((rower *)bluetoothManager->device())->lastRequestedPower().value() / ftpSetting) * 100.0;
            }
        }
        if (ftpPerc < 56) {
            ftpMinW = QString::number(0, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 0.55, 'f', 0);
            ftpZone = 1;
            ftpZone += (ftpPerc / 56);
            if (ftpZone >= 2) { // double precision could cause unwanted approximation
                ftpZone = 1.9999;
            }
            ftp->setValueFontColor(QStringLiteral("white"));
            watt->setValueFontColor(QStringLiteral("white"));
        } else if (ftpPerc < 76) {

            ftpMinW = QString::number((ftpSetting * 0.55) + 1, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 0.75, 'f', 0);
            ftpZone = 2;
            ftpZone += ((ftpPerc - 56) / 20);
            if (ftpZone >= 3) { // double precision could cause unwanted approximation
                ftpZone = 2.9999;
            }
            ftp->setValueFontColor(QStringLiteral("limegreen"));
            watt->setValueFontColor(QStringLiteral("limegreen"));
        } else if (ftpPerc < 91) {

            ftpMinW = QString::number((ftpSetting * 0.75) + 1, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 0.90, 'f', 0);
            ftpZone = 3;
            ftpZone += ((ftpPerc - 76) / 15);
            if (ftpZone >= 4) { // double precision could cause unwanted approximation
                ftpZone = 3.9999;
            }
            ftp->setValueFontColor(QStringLiteral("gold"));
            watt->setValueFontColor(QStringLiteral("gold"));
        } else if (ftpPerc < 106) {

            ftpMinW = QString::number((ftpSetting * 0.90) + 1, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 1.05, 'f', 0);
            ftpZone = 4;
            ftpZone += ((ftpPerc - 91) / 15);
            if (ftpZone >= 5) { // double precision could cause unwanted approximation
                ftpZone = 4.9999;
            }
            ftp->setValueFontColor(QStringLiteral("orange"));
            watt->setValueFontColor(QStringLiteral("orange"));
        } else if (ftpPerc < 121) {

            ftpMinW = QString::number((ftpSetting * 1.05) + 1, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 1.20, 'f', 0);
            ftpZone = 5;
            ftpZone += ((ftpPerc - 106) / 15);
            if (ftpZone >= 6) { // double precision could cause unwanted approximation
                ftpZone = 5.9999;
            }
            ftp->setValueFontColor(QStringLiteral("darkorange"));
            watt->setValueFontColor(QStringLiteral("darkorange"));
        } else if (ftpPerc < 151) {

            ftpMinW = QString::number((ftpSetting * 1.20) + 1, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 1.50, 'f', 0);
            ftpZone = 6;
            ftpZone += ((ftpPerc - 121) / 30);
            if (ftpZone >= 7) { // double precision could cause unwanted approximation
                ftpZone = 6.9999;
            }
            ftp->setValueFontColor(QStringLiteral("orangered"));
            watt->setValueFontColor(QStringLiteral("orangered"));
        } else {

            ftpMinW = QString::number((ftpSetting * 1.50) + 1, 'f', 0);
            ftpMaxW = QStringLiteral("∞");
            ftpZone = 7;

            ftp->setValueFontColor(QStringLiteral("red"));
            watt->setValueFontColor(QStringLiteral("red"));
        }
        bluetoothManager->device()->setPowerZone(ftpZone);
        ftp->setValue(QStringLiteral("Z") + QString::number(ftpZone, 'f', 1));
        ftp->setSecondLine(ftpMinW + QStringLiteral("-") + ftpMaxW + QStringLiteral("W ") +
                           QString::number(ftpPerc, 'f', 0) + QStringLiteral("%"));

        if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE ||
            bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
            if (requestedPerc < 56) {

                requestedMinW = QString::number(0, 'f', 0);
                requestedMaxW = QString::number(ftpSetting * 0.55, 'f', 0);
                requestedZone = 1;
                requestedZone += (requestedPerc / 56);
                if (requestedZone >= 2) { // double precision could cause unwanted approximation
                    requestedZone = 1.9999;
                }
                target_zone->setValueFontColor(QStringLiteral("white"));
            } else if (requestedPerc < 76) {

                requestedMinW = QString::number((ftpSetting * 0.55) + 1, 'f', 0);
                requestedMaxW = QString::number(ftpSetting * 0.75, 'f', 0);
                requestedZone = 2;
                requestedZone += ((requestedPerc - 56) / 20);
                if (requestedZone >= 3) { // double precision could cause unwanted approximation
                    requestedZone = 2.9999;
                }
                target_zone->setValueFontColor(QStringLiteral("limegreen"));
            } else if (requestedPerc < 91) {

                requestedMinW = QString::number((ftpSetting * 0.75) + 1, 'f', 0);
                requestedMaxW = QString::number(ftpSetting * 0.90, 'f', 0);
                requestedZone = 3;
                requestedZone += ((requestedPerc - 76) / 15);
                if (requestedZone >= 4) { // double precision could cause unwanted approximation
                    requestedZone = 3.9999;
                }
                target_zone->setValueFontColor(QStringLiteral("gold"));
            } else if (requestedPerc < 106) {

                requestedMinW = QString::number((ftpSetting * 0.90) + 1, 'f', 0);
                requestedMaxW = QString::number(ftpSetting * 1.05, 'f', 0);
                requestedZone = 4;
                requestedZone += ((requestedPerc - 91) / 15);
                if (requestedZone >= 5) { // double precision could cause unwanted approximation
                    requestedZone = 4.9999;
                }
                target_zone->setValueFontColor(QStringLiteral("orange"));
            } else if (requestedPerc < 121) {

                requestedMinW = QString::number((ftpSetting * 1.05) + 1, 'f', 0);
                requestedMaxW = QString::number(ftpSetting * 1.20, 'f', 0);
                requestedZone = 5;
                requestedZone += ((requestedPerc - 106) / 15);
                if (requestedZone >= 6) { // double precision could cause unwanted approximation
                    requestedZone = 5.9999;
                }
                target_zone->setValueFontColor(QStringLiteral("darkorange"));
            } else if (requestedPerc < 151) {

                requestedMinW = QString::number((ftpSetting * 1.20) + 1, 'f', 0);
                requestedMaxW = QString::number(ftpSetting * 1.50, 'f', 0);
                requestedZone = 6;
                requestedZone += ((requestedPerc - 121) / 30);
                if (requestedZone >= 7) { // double precision could cause unwanted approximation
                    requestedZone = 6.9999;
                }
                target_zone->setValueFontColor(QStringLiteral("orangered"));
            } else {

                requestedMinW = QString::number((ftpSetting * 1.50) + 1, 'f', 0);
                requestedMaxW = QStringLiteral("∞");
                requestedZone = 7;

                target_zone->setValueFontColor(QStringLiteral("red"));
            }
            target_zone->setValue(QStringLiteral("Z") + QString::number(requestedZone, 'f', 1));
            target_zone->setSecondLine(requestedMinW + QStringLiteral("-") + requestedMaxW + QStringLiteral("W ") +
                                       QString::number(requestedPerc, 'f', 0) + QStringLiteral("%"));
        }

        QString Z;
        double maxHeartRate = heartRateMax();
        double percHeartRate = (bluetoothManager->device()->currentHeart().value() * 100) / maxHeartRate;

        if (percHeartRate < settings.value(QStringLiteral("heart_rate_zone1"), 70.0).toDouble()) {
            currentHRZone = 1;
            currentHRZone += (percHeartRate / settings.value(QStringLiteral("heart_rate_zone1"), 70.0).toDouble());
            if (currentHRZone >= 2) { // double precision could cause unwanted approximation
                currentHRZone = 1.9999;
            }
            heart->setValueFontColor(QStringLiteral("lightsteelblue"));
        } else if (percHeartRate < settings.value(QStringLiteral("heart_rate_zone2"), 80.0).toDouble()) {
            currentHRZone = 2;
            currentHRZone += ((percHeartRate - settings.value(QStringLiteral("heart_rate_zone1"), 70.0).toDouble()) /
                              (settings.value(QStringLiteral("heart_rate_zone2"), 80.0).toDouble() -
                               settings.value(QStringLiteral("heart_rate_zone1"), 70.0).toDouble()));
            if (currentHRZone >= 3) { // double precision could cause unwanted approximation
                currentHRZone = 2.9999;
            }
            heart->setValueFontColor(QStringLiteral("green"));
        } else if (percHeartRate < settings.value(QStringLiteral("heart_rate_zone3"), 90.0).toDouble()) {
            currentHRZone = 3;
            currentHRZone += ((percHeartRate - settings.value(QStringLiteral("heart_rate_zone2"), 80.0).toDouble()) /
                              (settings.value(QStringLiteral("heart_rate_zone3"), 90.0).toDouble() -
                               settings.value(QStringLiteral("heart_rate_zone2"), 80.0).toDouble()));
            if (currentHRZone >= 4) { // double precision could cause unwanted approximation
                currentHRZone = 3.9999;
            }
            heart->setValueFontColor(QStringLiteral("yellow"));
        } else if (percHeartRate < settings.value(QStringLiteral("heart_rate_zone4"), 100.0).toDouble()) {
            currentHRZone = 4;
            currentHRZone += ((percHeartRate - settings.value(QStringLiteral("heart_rate_zone3"), 90.0).toDouble()) /
                              (settings.value(QStringLiteral("heart_rate_zone4"), 100.0).toDouble() -
                               settings.value(QStringLiteral("heart_rate_zone3"), 90.0).toDouble()));
            if (currentHRZone >= 5) { // double precision could cause unwanted approximation
                currentHRZone = 4.9999;
            }
            heart->setValueFontColor(QStringLiteral("orange"));
        } else {
            currentHRZone = 5;
            heart->setValueFontColor(QStringLiteral("red"));
        }
        bluetoothManager->device()->setHeartZone(currentHRZone);
        Z = QStringLiteral("Z") + QString::number(currentHRZone, 'f', 1);
        heart->setSecondLine(Z + QStringLiteral(" AVG: ") +
                             QString::number((bluetoothManager->device())->currentHeart().average(), 'f', 0) +
                             QStringLiteral(" MAX: ") +
                             QString::number((bluetoothManager->device())->currentHeart().max(), 'f', 0));

        /*
                if(trainProgram)
                {
                    trainProgramElapsedTime->setText(trainProgram->totalElapsedTime().toString("hh:mm:ss"));
                    trainProgramCurrentRowElapsedTime->setText(trainProgram->currentRowElapsedTime().toString("hh:mm:ss"));
                    trainProgramDuration->setText(trainProgram->duration().toString("hh:mm:ss"));

                    double distance = trainProgram->totalDistance();
                    if(distance > 0)
                    {
                        trainProgramTotalDistance->setText(QString::number(distance));
                    }
                    else
                        trainProgramTotalDistance->setText("N/A");
                }
        */

#ifdef Q_OS_ANDROID
        if (settings.value("ant_cadence", false).toBool() && KeepAwakeHelper::antObject(false)) {
            KeepAwakeHelper::antObject(false)->callMethod<void>(
                "setCadenceSpeedPower", "(FII)V", (float)bluetoothManager->device()->currentSpeed().value(), (int)watts,
                (int)cadence);
        }
#endif

        if (settings.value(QStringLiteral("trainprogram_random"), false).toBool()) {
            if (!paused && !stopped) {

                static QRandomGenerator r;
                static uint32_t last_seconds = 0;
                uint32_t seconds = bluetoothManager->device()->elapsedTime().second() +
                                   (bluetoothManager->device()->elapsedTime().minute() * 60) +
                                   (bluetoothManager->device()->elapsedTime().hour() * 3600);
                if ((seconds / 60) < settings.value(QStringLiteral("trainprogram_total"), 60).toUInt()) {
                    qDebug() << QStringLiteral("trainprogram random seconds ") + QString::number(seconds) +
                                    QStringLiteral(" last_change ") + last_seconds + QStringLiteral(" period ") +
                                    settings.value(QStringLiteral("trainprogram_period_seconds"), 60).toUInt();
                    if (last_seconds == 0 ||
                        ((seconds - last_seconds) >=
                         settings.value(QStringLiteral("trainprogram_period_seconds"), 60).toUInt())) {
                        bool done = false;

                        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL &&
                            ((treadmill *)bluetoothManager->device())->currentSpeed().value() > 0.0f) {
                            double speed = settings.value(QStringLiteral("trainprogram_speed_min"), 8).toUInt();
                            double incline = settings.value(QStringLiteral("trainprogram_incline_min"), 0).toUInt();
                            if (!speed) {
                                speed = 1.0;
                            }
                            if (settings.value(QStringLiteral("trainprogram_speed_min"), 8).toUInt() != 0 &&
                                settings.value(QStringLiteral("trainprogram_speed_min"), 8).toUInt() <
                                    settings.value(QStringLiteral("trainprogram_speed_max"), 16).toUInt()) {
                                speed =
                                    (double)r.bounded(
                                        settings.value(QStringLiteral("trainprogram_speed_min"), 8).toUInt() * 10,
                                        settings.value(QStringLiteral("trainprogram_speed_max"), 16).toUInt() * 10) /
                                    10.0;
                            }
                            if (settings.value(QStringLiteral("trainprogram_incline_min"), 0).toUInt() <
                                settings.value(QStringLiteral("trainprogram_incline_max"), 15).toUInt()) {
                                incline =
                                    (double)r.bounded(
                                        settings.value(QStringLiteral("trainprogram_incline_min"), 0).toUInt() * 10,
                                        settings.value(QStringLiteral("trainprogram_incline_max"), 15).toUInt() * 10) /
                                    10.0;
                            }
                            ((treadmill *)bluetoothManager->device())->changeSpeedAndInclination(speed, incline);
                            done = true;
                        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                            double resistance =
                                settings.value(QStringLiteral("trainprogram_resistance_min"), 1).toUInt();
                            if (settings.value(QStringLiteral("trainprogram_resistance_min"), 1).toUInt() <
                                settings.value(QStringLiteral("trainprogram_resistance_max"), 32).toUInt()) {
                                resistance = (double)r.bounded(
                                    settings.value(QStringLiteral("trainprogram_resistance_min"), 1).toUInt(),
                                    settings.value(QStringLiteral("trainprogram_resistance_max"), 32).toUInt());
                            }
                            ((bike *)bluetoothManager->device())->changeResistance(resistance);

                            done = true;
                        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                            double resistance =
                                settings.value(QStringLiteral("trainprogram_resistance_min"), 1).toUInt();
                            if (settings.value(QStringLiteral("trainprogram_resistance_min"), 1).toUInt() <
                                settings.value(QStringLiteral("trainprogram_resistance_max"), 32).toUInt()) {
                                resistance = (double)r.bounded(
                                    settings.value(QStringLiteral("trainprogram_resistance_min"), 1).toUInt(),
                                    settings.value(QStringLiteral("trainprogram_resistance_max"), 32).toUInt());
                            }
                            ((rower *)bluetoothManager->device())->changeResistance(resistance);

                            done = true;
                        }

                        if (done) {
                            if (last_seconds == 0) {

                                r.seed(QDateTime::currentDateTime().currentMSecsSinceEpoch());
                                last_seconds = 1; // in order to avoid to re-enter here again if the user doesn't ride
                            } else {

                                last_seconds = seconds;
                            }
                        }
                    }
                } else if (bluetoothManager->device()->currentSpeed().value() > 0) {
                    if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {

                        ((treadmill *)bluetoothManager->device())->changeSpeedAndInclination(0, 0);
                    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {

                        ((bike *)bluetoothManager->device())->changeResistance(1);
                    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {

                        ((rower *)bluetoothManager->device())->changeResistance(1);
                    }
                }
            }
        } else if (!settings.value(QStringLiteral("treadmill_pid_heart_zone"), QStringLiteral("Disabled"))
                        .toString()
                        .contains(QStringLiteral("Disabled")) ||
                   (trainProgram && trainProgram->currentRow().zoneHR > 0)) {
            static uint32_t last_seconds_pid_heart_zone = 0;
            static uint32_t pid_heart_zone_small_inc_counter = 0;
            uint32_t seconds = bluetoothManager->device()->elapsedTime().second() +
                               (bluetoothManager->device()->elapsedTime().minute() * 60) +
                               (bluetoothManager->device()->elapsedTime().hour() * 3600);
            uint8_t delta = 10;
            bool fromTrainProgram = trainProgram && trainProgram->currentRow().zoneHR > 0;
            int8_t maxSpeed = 30;

            if (fromTrainProgram) {
                delta = trainProgram->currentRow().loopTimeHR;
            }

            if (last_seconds_pid_heart_zone == 0 || ((seconds - last_seconds_pid_heart_zone) >= delta)) {

                last_seconds_pid_heart_zone = seconds;
                uint8_t zone = settings.value(QStringLiteral("treadmill_pid_heart_zone"), QStringLiteral("Disabled"))
                                   .toString()
                                   .toUInt();
                if (fromTrainProgram) {
                    zone = trainProgram->currentRow().zoneHR;
                    if (trainProgram->currentRow().maxSpeed > 0) {
                        maxSpeed = trainProgram->currentRow().maxSpeed;
                    }
                }

                if (!stopped && !paused && bluetoothManager->device()->currentHeart().value() &&
                    bluetoothManager->device()->currentSpeed().value() > 0.0f) {
                    if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {

                        const double step = 0.2;
                        double currentSpeed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
                        if (zone < currentHRZone) {
                            ((treadmill *)bluetoothManager->device())
                                ->changeSpeedAndInclination(
                                    currentSpeed - step,
                                    ((treadmill *)bluetoothManager->device())->currentInclination().value());
                            pid_heart_zone_small_inc_counter = 0;
                        } else if (zone > currentHRZone && maxSpeed >= currentSpeed + step) {
                            ((treadmill *)bluetoothManager->device())
                                ->changeSpeedAndInclination(

                                    currentSpeed + step,
                                    ((treadmill *)bluetoothManager->device())->currentInclination().value());
                            pid_heart_zone_small_inc_counter = 0;
                        } else {
                            pid_heart_zone_small_inc_counter++;
                            if (pid_heart_zone_small_inc_counter > 6) {
                                ((treadmill *)bluetoothManager->device())
                                    ->changeSpeedAndInclination(
                                        currentSpeed + step,
                                        ((treadmill *)bluetoothManager->device())->currentInclination().value());
                                pid_heart_zone_small_inc_counter = 0;
                            }
                        }
                    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {

                        const int step = 1;
                        resistance_t currentResistance =
                            ((bike *)bluetoothManager->device())->currentResistance().value();
                        if (zone < currentHRZone) {

                            ((bike *)bluetoothManager->device())->changeResistance(currentResistance - step);
                        } else if (zone > currentHRZone) {

                            ((bike *)bluetoothManager->device())->changeResistance(currentResistance + step);
                        }
                    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {

                        const int step = 1;
                        resistance_t currentResistance =
                            ((rower *)bluetoothManager->device())->currentResistance().value();
                        if (zone < currentHRZone) {

                            ((rower *)bluetoothManager->device())->changeResistance(currentResistance - step);
                        } else if (zone > currentHRZone) {

                            ((rower *)bluetoothManager->device())->changeResistance(currentResistance + step);
                        }
                    }
                }
            }
        }

        if (settings.value(QStringLiteral("fitmetria_fanfit_enable"), false).toBool()) {
            if (!settings.value(QStringLiteral("fitmetria_fanfit_mode"), QStringLiteral("Heart"))
                     .toString()
                     .compare(QStringLiteral("Manual"))) {
                // do nothing here, the user change the fan value with the tile
            } else if (paused || stopped) {
                qDebug() << QStringLiteral("fitmetria_fanfit paused or stopped mode");
                bluetoothManager->device()->changeFanSpeed(0);
            }
            // Heart Mode
            else if (!settings.value(QStringLiteral("fitmetria_fanfit_mode"), QStringLiteral("Heart"))
                          .toString()
                          .compare(QStringLiteral("Heart"))) {
                qDebug() << QStringLiteral("fitmetria_fanfit heart mode")
                         << bluetoothManager->device()->currentHeart().value();
                const uint8_t min = 80;
                uint8_t v = 0;
                if (bluetoothManager->device()->currentHeart().value() > min && maxHeartRate > min)
                    v = ((bluetoothManager->device()->currentHeart().value() - min) * 100.0) /
                        (double)(maxHeartRate - min);
                bluetoothManager->device()->changeFanSpeed(v + fanOverride);
            }
            // Power Mode
            else if (!settings.value(QStringLiteral("fitmetria_fanfit_mode"), QStringLiteral("Heart"))
                          .toString()
                          .compare(QStringLiteral("Power"))) {
                qDebug() << QStringLiteral("fitmetria_fanfit power mode") << watts;
                const double percOverFtp = 1.20;
                const double min = 50;
                uint8_t v = 0;
                double a = (double)(((ftpSetting * percOverFtp) - min));
                if (watts >= min && a > 0)
                    v = ((watts - min) * 100.0) / a;
                bluetoothManager->device()->changeFanSpeed(v + fanOverride);
            }
            // Wind mode
            else if (!settings.value(QStringLiteral("fitmetria_fanfit_mode"), QStringLiteral("Heart"))
                          .toString()
                          .compare(QStringLiteral("Wind"))) {
                // Todo
                qDebug() << QStringLiteral("fitmetria_fanfit wind mode");
                // bluetoothManager->device()->changeFanSpeed((ftpZone - 1) * 1.5);
            }
        }

        if (!stopped && !paused) {
            if (settings.value(QStringLiteral("tts_enabled"), false).toBool()) {
                if (++tts_summary_count >= settings.value(QStringLiteral("tts_summary_sec"), 120).toInt() &&
                    m_speech.state() == QTextToSpeech::Ready) {
                    tts_summary_count = 0;

                    QString s;
                    if (settings.value(QStringLiteral("tts_act_speed"), false).toBool())
                        s.append(tr(", speed ") +
                                 (!miles ? QString::number(bluetoothManager->device()->currentSpeed().value(), 'f', 1) +
                                               tr(" kilometers per hour")
                                         : QString::number(bluetoothManager->device()->currentSpeed().value() *
                                                               unit_conversion,
                                                           'f', 1)) +
                                 tr(" miles per hour"));
                    if (settings.value(QStringLiteral("tts_avg_speed"), true).toBool())
                        s.append(tr(", Average speed ") +
                                 (!miles
                                      ? QString::number(bluetoothManager->device()->currentSpeed().average(), 'f', 1) +
                                            tr("kilometers per hour")
                                      : QString::number(bluetoothManager->device()->currentSpeed().average() *
                                                            unit_conversion,
                                                        'f', 1)) +
                                 tr(" miles per hour"));
                    if (settings.value(QStringLiteral("tts_max_speed"), false).toBool())
                        s.append(
                            tr(", Max speed ") +
                            (!miles ? QString::number(bluetoothManager->device()->currentSpeed().max(), 'f', 1) +
                                          " kilometers per hour"
                                    : QString::number(
                                          bluetoothManager->device()->currentSpeed().max() * unit_conversion, 'f', 1)) +
                            tr(" miles per hour"));
                    if (settings.value(QStringLiteral("tts_act_inclination"), false).toBool())
                        s.append(tr(", inclination ") +
                                 QString::number(bluetoothManager->device()->currentInclination().value(), 'f', 1));
                    if (settings.value(QStringLiteral("tts_act_cadence"), false).toBool())
                        s.append(tr(", cadence ") +
                                 QString::number(bluetoothManager->device()->currentCadence().value(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_avg_cadence"), true).toBool())
                        s.append(tr(", Average cadence ") +
                                 QString::number(bluetoothManager->device()->currentCadence().average(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_max_cadence"), true).toBool())
                        s.append(tr(", Max cadence ") +
                                 QString::number(bluetoothManager->device()->currentCadence().max()));
                    if (settings.value(QStringLiteral("tts_act_elevation"), true).toBool())
                        s.append(tr(", elevation ") +
                                 (!miles
                                      ? QString::number(bluetoothManager->device()->elevationGain().value(), 'f', 1) +
                                            tr(" meters")
                                      : QString::number(bluetoothManager->device()->elevationGain().value() *
                                                            meter_feet_conversion,
                                                        'f', 1)) +
                                 tr(" feet"));
                    if (settings.value(QStringLiteral("tts_act_calories"), true).toBool())
                        s.append(tr(", calories burned ") +
                                 QString::number(bluetoothManager->device()->calories().value(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_act_odometer"), true).toBool())
                        s.append(
                            tr(", distance ") +
                            (!miles
                                 ? QString::number(bluetoothManager->device()->odometer(), 'f', 1) + tr("kilometers")
                                 : QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 1)) +
                            tr(" miles"));
                    if (settings.value(QStringLiteral("tts_act_pace"), false).toBool())
                        s.append(tr(", pace ") +
                                 bluetoothManager->device()->currentPace().toString(QStringLiteral("m:ss")));
                    if (settings.value(QStringLiteral("tts_avg_pace"), true).toBool())
                        s.append(tr(", pace ") +
                                 bluetoothManager->device()->averagePace().toString(QStringLiteral("m:ss")));
                    if (settings.value(QStringLiteral("tts_max_pace"), false).toBool())
                        s.append(tr(", pace ") +
                                 bluetoothManager->device()->maxPace().toString(QStringLiteral("m:ss")));
                    if (settings.value(QStringLiteral("tts_act_resistance"), true).toBool())
                        s.append(tr(", resistance ") +
                                 QString::number(bluetoothManager->device()->currentResistance().value(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_avg_resistance"), true).toBool())
                        s.append(tr(", average resistance ") +
                                 QString::number(bluetoothManager->device()->currentResistance().average(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_max_resistance"), false).toBool())
                        s.append(tr(", max resistance ") +
                                 QString::number(bluetoothManager->device()->currentResistance().max(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_act_watt"), false).toBool())
                        s.append(tr(", watt ") +
                                 QString::number(bluetoothManager->device()->wattsMetric().value(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_avg_watt"), true).toBool())
                        s.append(tr(", average watt ") +
                                 QString::number(bluetoothManager->device()->wattsMetric().average(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_max_watt"), true).toBool())
                        s.append(tr(", max watt ") +
                                 QString::number(bluetoothManager->device()->wattsMetric().max(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_act_ftp"), true).toBool())
                        s.append(tr(", ftp ") + QString::number(ftpZone, 'f', 1));
                    if (settings.value(QStringLiteral("tts_act_heart"), true).toBool())
                        s.append(tr(", heart rate ") +
                                 QString::number(bluetoothManager->device()->currentHeart().value(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_avg_heart"), true).toBool())
                        s.append(tr(", average heart rate ") +
                                 QString::number(bluetoothManager->device()->currentHeart().average(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_max_heart"), false).toBool())
                        s.append(tr(", max heart rate ") +
                                 QString::number(bluetoothManager->device()->currentHeart().max(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_act_jouls"), true).toBool())
                        s.append(tr(", jouls ") + QString::number(bluetoothManager->device()->jouls().max(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_act_elapsed"), true).toBool())
                        s.append(tr(", elapsed ") +
                                 QString::number(bluetoothManager->device()->elapsedTime().minute()) + tr(" minutes ") +
                                 QString::number(bluetoothManager->device()->elapsedTime().second()) + tr(" seconds"));
                    if (settings.value(QStringLiteral("tts_act_peloton_resistance"), false).toBool() &&
                        bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                        s.append(
                            tr(", peloton resistance ") +
                            QString::number(((bike *)bluetoothManager->device())->pelotonResistance().value(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_avg_peloton_resistance"), false).toBool() &&
                        bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                        s.append(tr(", average peloton resistance ") +
                                 QString::number(((bike *)bluetoothManager->device())->pelotonResistance().average(),
                                                 'f', 0));
                    if (settings.value(QStringLiteral("tts_max_peloton_resistance"), false).toBool() &&
                        bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                        s.append(
                            tr(", max peloton resistance ") +
                            QString::number(((bike *)bluetoothManager->device())->pelotonResistance().max(), 'f', 0));
                    if (settings.value(QStringLiteral("tts_act_target_peloton_resistance"), true).toBool() &&
                        bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                        s.append(tr(", target peloton resistance ") +
                                 QString::number(
                                     ((bike *)bluetoothManager->device())->lastRequestedPelotonResistance().value(),
                                     'f', 0));
                    if (settings.value(QStringLiteral("tts_act_target_cadence"), true).toBool() &&
                        bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                        s.append(tr(", target cadence ") +
                                 QString::number(((bike *)bluetoothManager->device())->lastRequestedCadence().value(),
                                                 'f', 0));
                    if (settings.value(QStringLiteral("tts_act_target_power"), true).toBool() &&
                        bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                        s.append(tr(", target power ") +
                                 QString::number(((bike *)bluetoothManager->device())->lastRequestedPower().value(),
                                                 'f', 0));
                    if (settings.value(QStringLiteral("tts_act_target_zone"), true).toBool() &&
                        bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                        s.append(tr(", target zone ") + QString::number(requestedZone, 'f', 1));
                    if (settings.value(QStringLiteral("tts_act_target_speed"), true).toBool() &&
                        bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
                        s.append(tr(", target speed ") +
                                 (!miles ? QString::number(
                                               ((treadmill *)bluetoothManager->device())->lastRequestedSpeed().value(),
                                               'f', 1) +
                                               tr(" kilometers per hour")
                                         : QString::number(
                                               ((treadmill *)bluetoothManager->device())->lastRequestedSpeed().value() *
                                                   unit_conversion,
                                               'f', 1)) +
                                 tr(" miles per hour"));
                    if (settings.value(QStringLiteral("tts_act_target_incline"), true).toBool() &&
                        bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
                        s.append(
                            tr(", target incline ") +
                            QString::number(
                                ((treadmill *)bluetoothManager->device())->lastRequestedInclination().value(), 'f', 1));
                    if (settings.value(QStringLiteral("tts_act_watt_kg"), false).toBool())
                        s.append(tr(", watt for kilograms ") +
                                 QString::number(bluetoothManager->device()->wattKg().value(), 'f', 1));
                    if (settings.value(QStringLiteral("tts_avg_watt_kg"), false).toBool())
                        s.append(tr(", average watt for kilograms") +
                                 QString::number(bluetoothManager->device()->wattKg().average(), 'f', 1));
                    if (settings.value(QStringLiteral("tts_max_watt_kg"), false).toBool())
                        s.append(tr(", max watt for kilograms") +
                                 QString::number(bluetoothManager->device()->wattKg().max(), 'f', 1));

                    qDebug() << "tts" << s;
                    m_speech.say(s);
                }
            }

            SessionLine s(
                bluetoothManager->device()->currentSpeed().value(), inclination, bluetoothManager->device()->odometer(),
                watts, resistance, peloton_resistance, (uint8_t)bluetoothManager->device()->currentHeart().value(),
                pace, cadence, bluetoothManager->device()->calories().value(),
                bluetoothManager->device()->elevationGain().value(),
                bluetoothManager->device()->elapsedTime().second() +
                    (bluetoothManager->device()->elapsedTime().minute() * 60) +
                    (bluetoothManager->device()->elapsedTime().hour() * 3600),

                lapTrigger, totalStrokes, avgStrokesRate, maxStrokesRate, avgStrokesLength,
                bluetoothManager->device()->currentCordinate(), strideLength, groundContact, verticalOscillation);

            Session.append(s);

            if (lapTrigger) {
                lapTrigger = false;
            }
        }
        emit workoutStartDateChanged(workoutStartDate());
    }

    emit changeOfdevice();
    emit changeOflap();
}

bool homeform::getDevice() {

    static bool toggle = false;
    if (!this->bluetoothManager->device()) {

        // toggling the bluetooth icon
        toggle = !toggle;
        return toggle;
    }
    return this->bluetoothManager->device()->connected();
}

bool homeform::getLap() {
    if (!this->bluetoothManager->device()) {

        return false;
    }
    return true;
}

void homeform::trainprogram_open_clicked(const QUrl &fileName) {
    qDebug() << QStringLiteral("trainprogram_open_clicked") << fileName;
    stravaWorkoutName = QFileInfo(fileName.fileName()).baseName();

    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    qDebug() << file.fileName();
    if (!file.fileName().isEmpty()) {
        {
            if (trainProgram) {

                delete trainProgram;
            }
            trainProgram = trainprogram::load(file.fileName(), bluetoothManager);
        }

        trainProgramSignals();
    }
}

void homeform::trainprogram_preview(const QUrl &fileName) {
    qDebug() << QStringLiteral("trainprogram_preview") << fileName;

    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    qDebug() << file.fileName();
    if (!file.fileName().isEmpty()) {
        {
            if (previewTrainProgram) {

                delete previewTrainProgram;
            }
            previewTrainProgram = trainprogram::load(file.fileName(), bluetoothManager);
            emit previewWorkoutPointsChanged(preview_workout_points());
            emit previewWorkoutDescriptionChanged(previewWorkoutDescription());
            emit previewWorkoutTagsChanged(previewWorkoutTags());
        }
    }
}

void homeform::trainprogram_zwo_loaded(const QString &s) {
    qDebug() << QStringLiteral("trainprogram_zwo_loaded") << s;
    trainProgram = new trainprogram(zwiftworkout::loadJSON(s), bluetoothManager);
    if (trainProgram) {
        QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains(QStringLiteral("name"))) {
                stravaPelotonActivityName = obj[QStringLiteral("name")].toString();
                stravaPelotonInstructorName = QStringLiteral("");
                emit workoutNameChanged(workoutName());
                emit instructorNameChanged(instructorName());

                QSettings settings;
                if (!settings.value(QStringLiteral("top_bar_enabled"), true).toBool()) {
                    return;
                }
                m_info = workoutName();
                emit infoChanged(m_info);
            }
        }
    }
    trainProgramSignals();
}

void homeform::gpx_save_clicked() {

    QString path = getWritableAppDir();

    if (bluetoothManager->device()) {
        gpx::save(path + QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
                      QStringLiteral(".gpx"),
                  Session, bluetoothManager->device()->deviceType());
    }
}

void homeform::fit_save_clicked() {

    QString path = getWritableAppDir();
    bluetoothdevice *dev = bluetoothManager->device();
    if (dev) {
        QString filename = path +
                           QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
                           QStringLiteral(".fit");
        qfit::save(filename, Session, dev->deviceType(),
                   qobject_cast<m3ibike *>(dev) ? QFIT_PROCESS_DISTANCENOISE : QFIT_PROCESS_NONE,
                   stravaPelotonWorkoutType);
        lastFitFileSaved = filename;

        QSettings settings;
        if (!settings.value(QStringLiteral("strava_accesstoken"), QLatin1String("")).toString().isEmpty()) {

            QFile f(filename);
            f.open(QFile::OpenModeFlag::ReadOnly);
            QByteArray fitfile = f.readAll();
            strava_upload_file(fitfile, filename);
            f.close();
        }
    }
}

void homeform::gpx_open_clicked(const QUrl &fileName) {
    qDebug() << QStringLiteral("gpx_open_clicked") << fileName;

    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    qDebug() << file.fileName();
    stravaWorkoutName = QFileInfo(file.fileName()).baseName();
    if (!file.fileName().isEmpty()) {
        {
            if (trainProgram) {

                delete trainProgram;
            }

            // KML to GPX https://www.gpsvisualizer.com/elevation
            gpx g;
            QList<trainrow> list;
            auto g_list = g.open(file.fileName());
            if (bluetoothManager->device())
                bluetoothManager->device()->setGPXFile(file.fileName());
            gpx_altitude_point_for_treadmill last;
            quint32 i = 0;
            list.reserve(g_list.size() + 1);
            for (const auto &p : g_list) {
                trainrow r;
                if (p.speed > 0) {
                    r.speed = p.speed;
                    r.duration = QTime(0, 0, 0, 0);
                    r.duration = r.duration.addSecs(p.seconds);
                    r.forcespeed = true;

                    r.altitude = p.elevation;
                    r.inclination = p.inclination;
                    r.latitude = p.latitude;
                    r.longitude = p.longitude;

                    list.append(r);

                } else {
                    if (i > 0) {
                        QGeoCoordinate p1(last.latitude, last.longitude);
                        QGeoCoordinate p2(p.latitude, p.longitude, p.elevation);
                        r.azimuth = p1.azimuthTo(p2);
                        r.distance = p.distance;
                        r.altitude = p.elevation;
                        r.inclination = p.inclination;
                        r.latitude = last.latitude;
                        r.longitude = last.longitude;
                        r.rampElapsed = QTime(0, 0, 0).addSecs(p.seconds);

                        list.append(r);
                        setMapsVisible(true);
                    }
                }

                last = p;
                i++;
            }
            trainProgram = new trainprogram(list, bluetoothManager);

            if (g.getVideoURL().isEmpty() == false) {
                movieFileName = QUrl(g.getVideoURL());
                setVideoVisible(true);
            } else if (QFile::exists(file.fileName().replace(".gpx", ".mp4"))) {
                movieFileName = QUrl::fromLocalFile(file.fileName().replace(".gpx", ".mp4"));
                setVideoVisible(true);
            }
        }

        trainProgramSignals();
    }
}

void homeform::gpxpreview_open_clicked(const QUrl &fileName) {
    qDebug() << QStringLiteral("gpxpreview_open_clicked") << fileName;

    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    qDebug() << file.fileName();

    if (!file.fileName().isEmpty()) {
        gpx g;
        auto g_list = g.open(file.fileName());
        gpx_preview.clearPath();
        for (const auto &p : g_list) {
            gpx_preview.addCoordinate(QGeoCoordinate(p.latitude, p.longitude, p.elevation));
        }
        pathController.setGeoPath(gpx_preview);
        pathController.setCenter(gpx_preview.center());
    }
}

QStringList homeform::bluetoothDevices() {

    QStringList r;
    r.append(QStringLiteral("Disabled"));
    r.append(QStringLiteral("Wifi"));
    for (const QBluetoothDeviceInfo &b : qAsConst(bluetoothManager->devices)) {
        if (!b.name().trimmed().isEmpty()) {

            r.append(b.name());
        }
    }
    return r;
}

QStringList homeform::metrics() { return bluetoothdevice::metrics(); }

struct OAuth2Parameter {
    QString responseType = QStringLiteral("code");
    QString approval_prompt = QStringLiteral("force");

    inline bool isEmpty() const { return responseType.isEmpty() && approval_prompt.isEmpty(); }

    QString toString() const {
        QString msg;
        QTextStream out(&msg);
        out << QStringLiteral("OAuth2Parameter{\n") << QStringLiteral("responseType: ") << this->responseType
            << QStringLiteral("\n") << QStringLiteral("approval_prompt: ") << this->approval_prompt
            << QStringLiteral("\n");
        return msg;
    }
};

QAbstractOAuth::ModifyParametersFunction
homeform::buildModifyParametersFunction(const QUrl &clientIdentifier, const QUrl &clientIdentifierSharedKey) {
    return [clientIdentifier, clientIdentifierSharedKey](QAbstractOAuth::Stage stage, QVariantMap *parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAuthorization) {
            parameters->insert(QStringLiteral("responseType"), QStringLiteral("code")); /* Request refresh token*/
            parameters->insert(QStringLiteral("approval_prompt"),
                               QStringLiteral("force")); /* force user check scope again */
            QByteArray code = parameters->value(QStringLiteral("code")).toByteArray();
            // DON'T TOUCH THIS LINE, THANKS Roberto Viola
            (*parameters)[QStringLiteral("code")] = QUrl::fromPercentEncoding(code); // NOTE: Old code replaced by
        }
        if (stage == QAbstractOAuth::Stage::RefreshingAccessToken) {
            parameters->insert(QStringLiteral("client_id"), clientIdentifier);
            parameters->insert(QStringLiteral("client_secret"), clientIdentifierSharedKey);
        }
    };
}

void homeform::strava_refreshtoken() {

    QSettings settings;
    // QUrlQuery params; //NOTE: clazy-unuse-non-tirial-variable

    if (settings.value(QStringLiteral("strava_refreshtoken")).toString().isEmpty()) {

        strava_connect();
        return;
    }

    QNetworkRequest request(QUrl(QStringLiteral("https://www.strava.com/oauth/token?")));
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    // set params
    QString data;
    data += QStringLiteral("client_id=" STRAVA_CLIENT_ID_S);
#ifdef STRAVA_SECRET_KEY
    data += "&client_secret=";
    data += STRINGIFY(STRAVA_SECRET_KEY);
#endif
    data += QStringLiteral("&refresh_token=") + settings.value(QStringLiteral("strava_refreshtoken")).toString();
    data += QStringLiteral("&grant_type=refresh_token");

    // make request
    if (manager) {

        delete manager;
        manager = nullptr;
    }
    manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->post(request, data.toLatin1());

    // blocking request
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << QStringLiteral("HTTP response code: ") << statusCode;

    // oops, no dice
    if (reply->error() != 0) {
        qDebug() << QStringLiteral("Got error") << reply->errorString().toStdString().c_str();
        return;
    }

    // lets extract the access token, and possibly a new refresh token
    QByteArray r = reply->readAll();
    qDebug() << QStringLiteral("Got response:") << r.data();

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(r, &parseError);

    // failed to parse result !?
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << tr("JSON parser error") << parseError.errorString();
    }

    QString access_token = document[QStringLiteral("access_token")].toString();
    QString refresh_token = document[QStringLiteral("refresh_token")].toString();

    settings.setValue(QStringLiteral("strava_accesstoken"), access_token);
    settings.setValue(QStringLiteral("strava_refreshtoken"), refresh_token);
    settings.setValue(QStringLiteral("strava_lastrefresh"), QDateTime::currentDateTime());
}

bool homeform::strava_upload_file(const QByteArray &data, const QString &remotename) {

    strava_refreshtoken();

    QSettings settings;
    QString token = settings.value(QStringLiteral("strava_accesstoken")).toString();

    // The V3 API doc said "https://api.strava.com" but it is not working yet
    QUrl url = QUrl(QStringLiteral("https://www.strava.com/api/v3/uploads"));
    QNetworkRequest request = QNetworkRequest(url);

    // QString boundary = QString::number(qrand() * (90000000000) / (RAND_MAX + 1) + 10000000000, 16);
    QString boundary = QVariant(QRandomGenerator::global()->generate()).toString() +
                       QVariant(QRandomGenerator::global()->generate()).toString() +
                       QVariant(QRandomGenerator::global()->generate()).toString(); // NOTE: qrand is deprecated

    // MULTIPART *****************

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->setBoundary(boundary.toLatin1());

    QHttpPart accessTokenPart;
    accessTokenPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                              QVariant(QStringLiteral("form-data; name=\"access_token\"")));
    accessTokenPart.setBody(token.toLatin1());
    multiPart->append(accessTokenPart);

    QHttpPart activityTypePart;
    activityTypePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                               QVariant("form-data; name=\"activity_type\""));

    // Map some known sports and default to ride for anything else
    if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
        activityTypePart.setBody("run");
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
        activityTypePart.setBody("rowing");
    } else {
        activityTypePart.setBody("ride");
    }
    multiPart->append(activityTypePart);

    QHttpPart activityNamePart;
    activityNamePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                               QVariant(QStringLiteral("form-data; name=\"name\"")));

    // use metadata config if the user selected it
    QString activityName =
        QStringLiteral(" ") + settings.value(QStringLiteral("strava_suffix"), QStringLiteral("#QZ")).toString();
    if (!stravaPelotonActivityName.isEmpty()) {
        activityName = stravaPelotonActivityName + QStringLiteral(" - ") + stravaPelotonInstructorName + activityName;
        if (pelotonHandler && settings.value(QStringLiteral("peloton_description_link"), true).toBool())
            activityDescription =
                QStringLiteral("https://members.onepeloton.com/classes/cycling?modal=classDetailsModal&classId=") +
                pelotonHandler->current_ride_id;
    } else {
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            activityName = QStringLiteral("Run") + activityName;
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
            activityName = QStringLiteral("Row") + activityName;
        } else {
            activityName = QStringLiteral("Ride") + activityName;
        }
    }
    activityNamePart.setHeader(QNetworkRequest::ContentTypeHeader,
                               QVariant(QStringLiteral("text/plain;charset=utf-8")));
    activityNamePart.setBody(activityName.toUtf8());
    if (activityName != QLatin1String("")) {
        multiPart->append(activityNamePart);
    }

    QHttpPart activityDescriptionPart;
    activityDescriptionPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                      QVariant(QStringLiteral("form-data; name=\"description\"")));
    activityDescriptionPart.setHeader(QNetworkRequest::ContentTypeHeader,
                                      QVariant(QStringLiteral("text/plain;charset=utf-8")));
    activityDescriptionPart.setBody(activityDescription.toUtf8());
    if (activityDescription != QLatin1String("")) {
        multiPart->append(activityDescriptionPart);
    }

    // upload file data
    QString filename = QFileInfo(remotename).baseName();

    QHttpPart dataTypePart;
    dataTypePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QVariant(QStringLiteral("form-data; name=\"data_type\"")));
    dataTypePart.setBody("fit");
    multiPart->append(dataTypePart);

    QHttpPart externalIdPart;
    externalIdPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                             QVariant(QStringLiteral("form-data; name=\"external_id\"")));
    externalIdPart.setBody(filename.toStdString().c_str());
    multiPart->append(externalIdPart);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QStringLiteral("application/octet-stream")));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(QStringLiteral("form-data; name=\"file\"; filename=\"") + remotename +
                                QStringLiteral("\"; type=\"application/octet-stream\"")));
    filePart.setBody(data);
    multiPart->append(filePart);

    // this must be performed asyncronously and call made
    // to notifyWriteCompleted(QString remotename, QString message) when done
    if (manager) {

        delete manager;
        manager = 0;
    }
    manager = new QNetworkAccessManager(this);
    replyStrava = manager->post(request, multiPart);

    // catch finished signal
    connect(replyStrava, &QNetworkReply::finished, this, &homeform::writeFileCompleted);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
    connect(replyStrava, &QNetworkReply::errorOccurred, this, &homeform::errorOccurredUploadStrava);
#endif
    return true;
}

void homeform::errorOccurredUploadStrava(QNetworkReply::NetworkError code) {
    qDebug() << QStringLiteral("strava upload error!") << code;
}

void homeform::writeFileCompleted() {
    qDebug() << QStringLiteral("strava upload completed!");

    QNetworkReply *reply = static_cast<QNetworkReply *>(QObject::sender());

    QString response = reply->readAll();
    // QString uploadError = QStringLiteral("invalid response or parser error");
    // NOTE: clazy-unused-non-trivial-variable

    qDebug() << "reply:" << response;
}

void homeform::onStravaGranted() {

    QSettings settings;
    settings.setValue(QStringLiteral("strava_accesstoken"), strava->token());
    settings.setValue(QStringLiteral("strava_refreshtoken"), strava->refreshToken());
    settings.setValue(QStringLiteral("strava_lastrefresh"), QDateTime::currentDateTime());
    qDebug() << QStringLiteral("strava authenticathed") << strava->token() << strava->refreshToken();
    strava_refreshtoken();
    setGeneralPopupVisible(true);
}

void homeform::onStravaAuthorizeWithBrowser(const QUrl &url) {

    // ui->textBrowser->append(tr("Open with browser:") + url.toString());
    QDesktopServices::openUrl(url);
}

void homeform::replyDataReceived(const QByteArray &v) {

    qDebug() << v;

    QByteArray data;
    QSettings settings;
    QString s(v);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(s.toUtf8());
    settings.setValue(QStringLiteral("strava_accesstoken"), jsonResponse[QStringLiteral("access_token")]);
    settings.setValue(QStringLiteral("strava_refreshtoken"), jsonResponse[QStringLiteral("refresh_token")]);
    settings.setValue(QStringLiteral("strava_expires"), jsonResponse[QStringLiteral("expires_at")]);

    qDebug() << jsonResponse[QStringLiteral("access_token")] << jsonResponse[QStringLiteral("refresh_token")]
             << jsonResponse[QStringLiteral("expires_at")];

    QString urlstr = QStringLiteral("https://www.strava.com/oauth/token?");
    QUrlQuery params;
    params.addQueryItem(QStringLiteral("client_id"), QStringLiteral(STRAVA_CLIENT_ID_S));
#ifdef STRAVA_SECRET_KEY
#define _STR(x) #x
#define STRINGIFY(x) _STR(x)
    params.addQueryItem("client_secret", STRINGIFY(STRAVA_SECRET_KEY));
#endif

    params.addQueryItem(QStringLiteral("code"), strava_code);
    data.append(params.query(QUrl::FullyEncoded).toUtf8());

    // trade-in the temporary access code retrieved by the Call-Back URL for the finale token
    QUrl url = QUrl(urlstr);

    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    // now get the final token - but ignore errors
    if (manager) {

        delete manager;
        manager = 0;
    }
    manager = new QNetworkAccessManager(this);
    // connect(manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )), this,
    // SLOT(onSslErrors(QNetworkReply*, const QList<QSslError> & ))); connect(manager,
    // SIGNAL(finished(QNetworkReply*)), this, SLOT(networkRequestFinished(QNetworkReply*)));
    manager->post(request, data);
}

void homeform::onSslErrors(QNetworkReply *reply, const QList<QSslError> &error) {

    reply->ignoreSslErrors();
    qDebug() << QStringLiteral("homeform::onSslErrors") << error;
}

void homeform::networkRequestFinished(QNetworkReply *reply) {

    QSettings settings;

    // we can handle SSL handshake errors, if we got here then some kind of protocol was agreed
    if (reply->error() == QNetworkReply::NoError || reply->error() == QNetworkReply::SslHandshakeFailedError) {

        QByteArray payload = reply->readAll(); // JSON
        QString refresh_token;
        QString access_token;

        // parse the response and extract the tokens, pretty much the same for all services
        // although polar choose to also pass a user id, which is needed for future calls
        QJsonParseError parseError;
        QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            refresh_token = document[QStringLiteral("refresh_token")].toString();
            access_token = document[QStringLiteral("access_token")].toString();
        }

        settings.setValue(QStringLiteral("strava_accesstoken"), access_token);
        settings.setValue(QStringLiteral("strava_refreshtoken"), refresh_token);
        settings.setValue(QStringLiteral("strava_lastrefresh"), QDateTime::currentDateTime());

        qDebug() << access_token << refresh_token;

    } else {

        // general error getting response
        QString error =
            QString(tr("Error retrieving access token, %1 (%2)")).arg(reply->errorString()).arg(reply->error());
        qDebug() << error << reply->url() << reply->readAll();
    }
}

void homeform::callbackReceived(const QVariantMap &values) {
    qDebug() << QStringLiteral("homeform::callbackReceived") << values;
    if (!values.value(QStringLiteral("code")).toString().isEmpty()) {
        strava_code = values.value(QStringLiteral("code")).toString();

        qDebug() << strava_code;
    }
}

QOAuth2AuthorizationCodeFlow *homeform::strava_connect() {
    if (manager) {

        delete manager;
        manager = nullptr;
    }
    if (strava) {

        delete strava;
        strava = nullptr;
    }
    if (stravaReplyHandler) {

        delete stravaReplyHandler;
        stravaReplyHandler = nullptr;
    }
    manager = new QNetworkAccessManager(this);
    OAuth2Parameter parameter;
    strava = new QOAuth2AuthorizationCodeFlow(manager, this);
    strava->setScope(QStringLiteral("activity:read_all,activity:write"));
    strava->setClientIdentifier(QStringLiteral(STRAVA_CLIENT_ID_S));
    strava->setAuthorizationUrl(QUrl(QStringLiteral("https://www.strava.com/oauth/authorize")));
    strava->setAccessTokenUrl(QUrl(QStringLiteral("https://www.strava.com/oauth/token")));
#ifdef STRAVA_SECRET_KEY
#define _STR(x) #x
#define STRINGIFY(x) _STR(x)
    strava->setClientIdentifierSharedKey(STRINGIFY(STRAVA_SECRET_KEY));
#elif defined(WIN32)
#pragma message("DEFINE STRAVA_SECRET_KEY!!!")
#else
#warning "DEFINE STRAVA_SECRET_KEY!!!"
#endif
    strava->setModifyParametersFunction(
        buildModifyParametersFunction(QUrl(QLatin1String("")), QUrl(QLatin1String(""))));
    stravaReplyHandler = new QOAuthHttpServerReplyHandler(QHostAddress(QStringLiteral("127.0.0.1")), 8091, this);
    connect(stravaReplyHandler, &QOAuthHttpServerReplyHandler::replyDataReceived, this, &homeform::replyDataReceived);
    connect(stravaReplyHandler, &QOAuthHttpServerReplyHandler::callbackReceived, this, &homeform::callbackReceived);

    strava->setReplyHandler(stravaReplyHandler);

    return strava;
}

void homeform::strava_connect_clicked() {
    QLoggingCategory::setFilterRules(QStringLiteral("qt.networkauth.*=true"));

    strava_connect();
    connect(strava, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &homeform::onStravaAuthorizeWithBrowser);
    connect(strava, &QOAuth2AuthorizationCodeFlow::granted, this, &homeform::onStravaGranted);

    strava->grant();
    // qDebug() <<
    // QAbstractOAuth2::post("https://www.strava.com/oauth/authorize?client_id=7976&scope=activity:read_all,activity:write&redirect_uri=http://127.0.0.1&response_type=code&approval_prompt=force");
}

bool homeform::generalPopupVisible() { return m_generalPopupVisible; }

void homeform::setGeneralPopupVisible(bool value) {

    m_generalPopupVisible = value;
    emit generalPopupVisibleChanged(m_generalPopupVisible);
}

bool homeform::licensePopupVisible() { return m_LicensePopupVisible; }

void homeform::setLicensePopupVisible(bool value) {

    m_LicensePopupVisible = value;
    emit licensePopupVisibleChanged(m_LicensePopupVisible);
}

bool homeform::mapsVisible() { return m_MapsVisible; }

void homeform::setMapsVisible(bool value) {

    m_MapsVisible = value;
    emit mapsVisibleChanged(m_MapsVisible);
}

bool homeform::videoVisible() { return m_VideoVisible; }

void homeform::setVideoVisible(bool value) {

    m_VideoVisible = value;
    emit videoVisibleChanged(m_VideoVisible);
}

int homeform::videoPosition() { return m_VideoPosition; }

void homeform::setVideoPosition(int value) {

    m_VideoPosition = value;
    emit videoPositionChanged(m_VideoPosition);
}

double homeform::videoRate() { return m_VideoRate; }

void homeform::setVideoRate(double value) {

    m_VideoRate = value;
    emit videoRateChanged(m_VideoPosition);
}

void homeform::smtpError(SmtpClient::SmtpError e) { qDebug() << QStringLiteral("SMTP ERROR") << e; }

void homeform::sendMail() {

    QSettings settings;

    bool miles = settings.value(QStringLiteral("miles_unit"), false).toBool();
    double unit_conversion = 1.0;
    QString weightLossUnit = QStringLiteral("Kg");
    double WeightLoss = 0;

    // TODO: add a condition to avoid sending mail when the user look at the chart while is riding
    if (settings.value(QStringLiteral("user_email"), "").toString().length() == 0 || !bluetoothManager->device()) {
        return;
    }

    if (miles) {
        unit_conversion = 0.621371; // clang, don't touch it!
        weightLossUnit = QStringLiteral("Oz");
    }
    WeightLoss = (miles ? bluetoothManager->device()->weightLoss() * 35.274 : bluetoothManager->device()->weightLoss());

#ifdef SMTP_SERVER
#define _STR(x) #x
#define STRINGIFY(x) _STR(x)
    SmtpClient smtp(STRINGIFY(SMTP_SERVER), 587, SmtpClient::TlsConnection);
    connect(&smtp, SIGNAL(smtpError(SmtpClient::SmtpError)), this, SLOT(smtpError(SmtpClient::SmtpError)));
#else
#warning "stmp server is unset!"
    SmtpClient smtp(QLatin1String(""), 25, SmtpClient::TlsConnection);
    return;
#endif

// We need to set the username (your email address) and the password
// for smtp authentification.
#ifdef SMTP_PASSWORD
#define _STR(x) #x
#define STRINGIFY(x) _STR(x)
    smtp.setUser(STRINGIFY(SMTP_USERNAME));
#else
#warning "smtp username is unset!"
    return;
#endif
#ifdef SMTP_PASSWORD
#define _STR(x) #x
#define STRINGIFY(x) _STR(x)
    smtp.setPassword(STRINGIFY(SMTP_PASSWORD));
#else
#warning "smtp password is unset!"
    return;
#endif

    // Now we create a MimeMessage object. This will be the email.

    MimeMessage message;

    message.setSender(new EmailAddress(QStringLiteral("no-reply@qzapp.it"), QStringLiteral("QZ")));
    message.addRecipient(new EmailAddress(settings.value(QStringLiteral("user_email"), QLatin1String("")).toString(),
                                          settings.value(QStringLiteral("user_email"), QLatin1String("")).toString()));
    if (!Session.isEmpty()) {
        QString title = Session.constFirst().time.toString();
        if (!stravaPelotonActivityName.isEmpty()) {
            title +=
                QStringLiteral(" ") + stravaPelotonActivityName + QStringLiteral(" - ") + stravaPelotonInstructorName;
        }
        message.setSubject(title);
    } else {
        message.setSubject(QStringLiteral("Test"));
    }

    // Now add some text to the email.
    // First we create a MimeText object.

    MimeText text;

    QString textMessage = QStringLiteral("Great workout!\n\n");

    if (pelotonHandler) {
        if (!pelotonHandler->current_ride_id.isEmpty()) {
            textMessage +=
                stravaPelotonActivityName + QStringLiteral(" - ") + stravaPelotonInstructorName +
                QStringLiteral(" https://members.onepeloton.com/classes/cycling?modal=classDetailsModal&classId=") +
                pelotonHandler->current_ride_id;
        }
    }

    textMessage += '\n';
    textMessage += QStringLiteral("Average Speed: ") +
                   QString::number(bluetoothManager->device()->currentSpeed().average() * unit_conversion, 'f', 1) +
                   QStringLiteral("\n");
    textMessage += QStringLiteral("Max Speed: ") +
                   QString::number(bluetoothManager->device()->currentSpeed().max() * unit_conversion, 'f', 1) +
                   QStringLiteral("\n");
    textMessage += QStringLiteral("Calories burned: ") +
                   QString::number(bluetoothManager->device()->calories().value(), 'f', 0) + QStringLiteral("\n");
    textMessage += QStringLiteral("Distance: ") +
                   QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 1) +
                   QStringLiteral("\n");
    textMessage += QStringLiteral("Average Watt: ") +
                   QString::number(bluetoothManager->device()->wattsMetric().average(), 'f', 0) + QStringLiteral("\n");
    textMessage += QStringLiteral("Max Watt: ") +
                   QString::number(bluetoothManager->device()->wattsMetric().max(), 'f', 0) + QStringLiteral("\n");
    textMessage += QStringLiteral("Average Watt/Kg: ") +
                   QString::number(bluetoothManager->device()->wattKg().average(), 'f', 1) + "\n";
    textMessage +=
        QStringLiteral("Max Watt/Kg: ") + QString::number(bluetoothManager->device()->wattKg().max(), 'f', 1) + "\n";
    textMessage += QStringLiteral("Average Heart Rate: ") +
                   QString::number(bluetoothManager->device()->currentHeart().average(), 'f', 0) + QStringLiteral("\n");
    textMessage += QStringLiteral("Max Heart Rate: ") +
                   QString::number(bluetoothManager->device()->currentHeart().max(), 'f', 0) + QStringLiteral("\n");
    textMessage += QStringLiteral("Total Output: ") +
                   QString::number(bluetoothManager->device()->jouls().max() / 1000.0, 'f', 0) + QStringLiteral("\n");
    textMessage +=
        QStringLiteral("Elapsed Time: ") + bluetoothManager->device()->elapsedTime().toString() + QStringLiteral("\n");
    textMessage +=
        QStringLiteral("Moving Time: ") + bluetoothManager->device()->movingTime().toString() + QStringLiteral("\n");
    textMessage += QStringLiteral("Weight Loss (") + weightLossUnit + "): " + QString::number(WeightLoss, 'f', 2) +
                   QStringLiteral("\n");
    textMessage += QStringLiteral("Estimated VO2Max: ") + QString::number(metric::calculateVO2Max(&Session), 'f', 1) +
                   QStringLiteral("\n");
    if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
        textMessage += QStringLiteral("Average Cadence: ") +
                       QString::number(((bike *)bluetoothManager->device())->currentCadence().average(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Cadence: ") +
                       QString::number(((bike *)bluetoothManager->device())->currentCadence().max(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Average Resistance: ") +
                       QString::number(((bike *)bluetoothManager->device())->currentResistance().average(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Resistance: ") +
                       QString::number(((bike *)bluetoothManager->device())->currentResistance().max(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Average Peloton Resistance: ") +
                       QString::number(((bike *)bluetoothManager->device())->pelotonResistance().average(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Peloton Resistance: ") +
                       QString::number(((bike *)bluetoothManager->device())->pelotonResistance().max(), 'f', 0) +
                       QStringLiteral("\n");
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
        textMessage += QStringLiteral("Average Cadence: ") +
                       QString::number(((rower *)bluetoothManager->device())->currentCadence().average(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Cadence: ") +
                       QString::number(((rower *)bluetoothManager->device())->currentCadence().max(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Average Resistance: ") +
                       QString::number(((rower *)bluetoothManager->device())->currentResistance().average(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Resistance: ") +
                       QString::number(((rower *)bluetoothManager->device())->currentResistance().max(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Average Peloton Resistance: ") +
                       QString::number(((rower *)bluetoothManager->device())->pelotonResistance().average(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Peloton Resistance: ") +
                       QString::number(((rower *)bluetoothManager->device())->pelotonResistance().max(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage +=
            QStringLiteral("Average Strokes Length: ") +
            QString::number(((rower *)bluetoothManager->device())->currentStrokesLength().average(), 'f', 1) + "\n";
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
        // for stryd and similars
        if (bluetoothManager->device()->currentCadence().average() > 0) {
            textMessage += QStringLiteral("Average Cadence: ") +
                           QString::number((bluetoothManager->device())->currentCadence().average(), 'f', 0) +
                           QStringLiteral("\n");
            textMessage += QStringLiteral("Max Cadence: ") +
                           QString::number((bluetoothManager->device())->currentCadence().max(), 'f', 0) +
                           QStringLiteral("\n");
        }
    }
    textMessage += QStringLiteral("\n\nQZ version: ") + QApplication::applicationVersion();
#ifdef Q_OS_ANDROID
    textMessage += " - Android";
#endif
#ifdef Q_OS_IOS
    textMessage += " - iOS";
#endif
    if (bluetoothManager) {
        textMessage += QStringLiteral("\nDevice: ") + bluetoothManager->device()->bluetoothDevice.name();

        if (bluetoothManager->heartRateDevice()) {
            textMessage +=
                QStringLiteral("\nHR Device: ") + bluetoothManager->heartRateDevice()->bluetoothDevice.name();
        }
    }

    text.setText(textMessage);
    message.addPart(&text);

    for (const QString &f : qAsConst(chartImagesFilenames)) {

        // Create a MimeInlineFile object for each image
        MimeInlineFile *image = new MimeInlineFile((new QFile(f)));

        // An unique content id must be setted
        image->setContentId(f);
        image->setContentType(QStringLiteral("image/jpg"));
        message.addPart(image);
    }

    if (!lastFitFileSaved.isEmpty()) {

        // Create a MimeInlineFile object for each image
        MimeInlineFile *fit = new MimeInlineFile((new QFile(lastFitFileSaved)));

        // An unique content id must be setted
        fit->setContentId(lastFitFileSaved);
        fit->setContentType(QStringLiteral("application/octet-stream"));
        message.addPart(fit);
    }
    if (pelotonHandler && pelotonHandler->current_image_downloaded &&
        !pelotonHandler->current_image_downloaded->downloadedData().isEmpty()) {

        QString path = getWritableAppDir();
        QString filename = path +
                           QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
                           QStringLiteral("_peloton_image.png");
        QString filenameJPG =
            path + QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
            QStringLiteral("_peloton_image.jpg");
        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        file.write(pelotonHandler->current_image_downloaded->downloadedData());
        file.close();
        QImage image(filename);
        QImageWriter writer(filename, "png");
        writer.setFileName(filenameJPG);
        writer.setFormat("jpg");
        writer.setQuality(30);
        writer.write(image);
        QFile::remove(filename);

        // Create a MimeInlineFile object for each image
        MimeInlineFile *pelotonImage = new MimeInlineFile((new QFile(filenameJPG)));

        // An unique content id must be setted
        pelotonImage->setContentId(filenameJPG);
        pelotonImage->setContentType(QStringLiteral("image/jpg"));
        message.addPart(pelotonImage);
    }

    /* THE SMTP SERVER DOESN'T LIKE THE ZIP FILE
    extern QString logfilename;
    if (settings.value("log_debug").toBool() && QFile::exists(getWritableAppDir() + logfilename)) {
        QString fileName = getWritableAppDir() + logfilename;
        QFile f(fileName);
        f.open(QIODevice::ReadOnly);
        QTextStream ts(&f);
        QByteArray b = f.readAll();
        f.close();
        QByteArray c = qCompress(b, 9);
        QFile fc(fileName.replace(".log", ".zip"));
        fc.open(QIODevice::WriteOnly);
        c.remove(0, 4);
        fc.write(c);
        fc.close();

        // Create a MimeInlineFile object for each image
        MimeInlineFile *log = new MimeInlineFile((new QFile(fileName)));

        // An unique content id must be setted
        log->setContentId(fileName);
        log->setContentType(QStringLiteral("application/octet-stream"));
        message.addPart(log);
    }*/

    bool r = false;
    uint8_t i = 0;
    while (!r) {
        qDebug() << "trying to send email #" << i;
        r = smtp.connectToHost();
        r = smtp.login();
        r = smtp.sendMail(message);
        if (i++ == 3)
            break;
    }
    smtp.quit();

    // delete image variable TODO
}

#if defined(Q_OS_ANDROID)
QString homeform::getAndroidDataAppDir() {
    static QString path = "";

    if (path.length()) {
        return path;
    }

    QAndroidJniObject filesArr = QtAndroid::androidActivity().callObjectMethod(
        "getExternalFilesDirs", "(Ljava/lang/String;)[Ljava/io/File;", nullptr);
    jobjectArray dataArray = filesArr.object<jobjectArray>();
    QString out;
    if (dataArray) {
        QAndroidJniEnvironment env;
        jsize dataSize = env->GetArrayLength(dataArray);
        if (dataSize) {
            QAndroidJniObject mediaPath;
            QAndroidJniObject file;
            for (int i = 0; i < dataSize; i++) {
                file = env->GetObjectArrayElement(dataArray, i);
                jboolean val = QAndroidJniObject::callStaticMethod<jboolean>(
                    "android/os/Environment", "isExternalStorageRemovable", "(Ljava/io/File;)Z", file.object());
                mediaPath = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
                out = mediaPath.toString();
                if (!val)
                    break;
            }
        }
    }
    path = out;
    return out;
}
#endif

quint64 homeform::cryptoKeySettingsProfiles() {
    QSettings settings;
    quint64 v = settings.value(cryptoKeySettingsProfilesTag, 0).toULongLong();
    if (!v) {
        QRandomGenerator r = QRandomGenerator();
        r.seed(QDateTime::currentMSecsSinceEpoch());
        v = r.generate64();
        settings.setValue(cryptoKeySettingsProfilesTag, v);
    }
    return v;
}

void homeform::saveSettings(const QUrl &filename) {
    Q_UNUSED(filename)
    QString path = getWritableAppDir();

    QDir().mkdir(path + QStringLiteral("settings/"));
    QSettings settings;
    QSettings settings2Save(path + QStringLiteral("settings/settings_") + settings.value("profile_name").toString() +
                                QStringLiteral("_") + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") +
                                QStringLiteral(".qzs"),
                            QSettings::IniFormat);
    auto settigsAllKeys = settings.allKeys();
    for (const QString &s : qAsConst(settigsAllKeys)) {
        if (!s.contains(cryptoKeySettingsProfilesTag)) {
            if (!s.contains(QStringLiteral("password")) && !s.contains(QStringLiteral("token"))) {
                settings2Save.setValue(s, settings.value(s));
            } else {
                SimpleCrypt crypt;
                crypt.setKey(cryptoKeySettingsProfiles());
                settings2Save.setValue(s, crypt.encryptToString(settings.value(s).toString()));
            }
        }
    }
}

void homeform::loadSettings(const QUrl &filename) {
    QSettings settings;
    QSettings settings2Load(filename.toLocalFile(), QSettings::IniFormat);
    auto settings2LoadAllKeys = settings2Load.allKeys();
    for (const QString &s : qAsConst(settings2LoadAllKeys)) {
        if (!s.contains(cryptoKeySettingsProfilesTag)) {
            if (!s.contains(QStringLiteral("password")) && !s.contains(QStringLiteral("token"))) {
                settings.setValue(s, settings2Load.value(s));
            } else {
                SimpleCrypt crypt;
                crypt.setKey(cryptoKeySettingsProfiles());
                settings.setValue(s, crypt.decryptToString(settings2Load.value(s).toString()));
            }
        }
    }
}

void homeform::deleteSettings(const QUrl &filename) { QFile(filename.toLocalFile()).remove(); }

QString homeform::getProfileDir() {
    QString path = getWritableAppDir() + "profiles";
    QDir().mkdir(path);
    return path;
}

void homeform::saveProfile(QString profilename) {
    qDebug() << "homeform::saveProfile";
    QString path = getProfileDir();

    QSettings settings;
    settings.setValue("profile_name", profilename);
    QSettings settings2Save(path + "/" + profilename + QStringLiteral(".qzs"), QSettings::IniFormat);
    auto settigsAllKeys = settings.allKeys();
    for (const QString &s : qAsConst(settigsAllKeys)) {
        if (!s.contains(cryptoKeySettingsProfilesTag)) {
            if (!s.contains(QStringLiteral("password")) && !s.contains(QStringLiteral("token"))) {
                settings2Save.setValue(s, settings.value(s));
            } else {
                SimpleCrypt crypt;
                crypt.setKey(cryptoKeySettingsProfiles());
                settings2Save.setValue(s, crypt.encryptToString(settings.value(s).toString()));
            }
        }
    }
}

void homeform::restart() {
    qApp->quit();
#if !defined(Q_OS_DARWIN) && !defined(Q_OS_IOS) && !defined(Q_OS_WINRT)
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
#endif
}

double homeform::heartRateMax() {
    QSettings settings;
    double maxHeartRate = 220.0 - settings.value(QStringLiteral("age"), 35).toDouble();

    if (settings.value(QStringLiteral("heart_max_override_enable"), false).toBool())
        maxHeartRate = settings.value(QStringLiteral("heart_max_override_value"), 195).toDouble();
    if (maxHeartRate == 0) {
        maxHeartRate = 190.0;
    }
    return maxHeartRate;
}

void homeform::clearFiles() {
    QString path = homeform::getWritableAppDir();
    QDir dir(path);
    QFileInfoList list = dir.entryInfoList(QDir::Files);
    foreach (QFileInfo f, list) {
        if (!f.suffix().toLower().compare("log") || !f.suffix().toLower().compare("jpg") ||
            !f.suffix().toLower().compare("fit") || !f.suffix().toLower().compare("png")) {
            QFile::remove(f.filePath());
        }
    }
}

int homeform::preview_workout_points() {
    if (previewTrainProgram) {
        QTime d = previewTrainProgram->duration();
        return (d.hour() * 3600) + (d.minute() * 60) + d.second();
    }
    return 0;
}

#if defined(Q_OS_WIN) || (defined(Q_OS_MAC) && !defined(Q_OS_IOS))
void homeform::licenseReply(QNetworkReply *reply) {
    QString r = reply->readAll();
    qDebug() << r;
    if (r.contains("OK")) {
        tLicense.stop();
    }
}
void homeform::licenseTimeout() { setLicensePopupVisible(true); }
#endif

void homeform::changeTimestamp(QTime source, QTime actual) {
    static QTime lastSource = QTime(0, 0, 0);
    static QTime lastActual = QTime(0, 0, 0);
    const double filterRate = 0.1;
    QSettings settings;
    int filterSeconds = settings.value(QStringLiteral("video_playback_window_s"), 12).toInt();

    // if a time is smaller then the last one means that the user restart the program
    if (lastSource > source || lastActual > actual) {
        qDebug() << "changeTimestamp resetting timestamp " << source << actual << lastSource << lastActual;
        lastSource = QTime(0, 0, 0);
        lastActual = QTime(0, 0, 0);
    }

    // we need to calculate the rate only if this the first time or every X seconds (in order to have an average)
    if (lastSource.secsTo(QTime(0, 0, 0)) == 0 || lastActual.secsTo(actual) > filterSeconds) {
        double fullRate = (double)QTime(0, 0, 0).secsTo(source) / (double)QTime(0, 0, 0).secsTo(actual);
        double rate = (double)lastSource.secsTo(source) / (double)lastActual.secsTo(actual);
        lastSource = source;
        lastActual = actual;
        qDebug() << "changeTimestamp" << source << actual << fullRate << lastSource << lastActual << rate;
        setVideoPosition(QTime(0, 0, 0).secsTo(source) * 1000);
        if (fabs(videoRate() - rate) > filterRate)
            setVideoRate(rate);
    }
}
