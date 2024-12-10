#include <QApplication>
#include <QStyleFactory>
#include <stdio.h>
#include <stdlib.h>
#ifdef Q_OS_LINUX
#ifndef Q_OS_ANDROID
#include <unistd.h> // getuid
#include "EventHandler.h"
#endif
#endif
#include <QQmlContext>

#include "bluetooth.h"
#include "devices/domyostreadmill/domyostreadmill.h"
#include "homeform.h"
#include "mainwindow.h"
#include "qfit.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QDir>
#include <QGuiApplication>
#include <QOperatingSystemVersion>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QStandardPaths>
#ifdef CHARTJS
#include <QtWebView/QtWebView>
#endif

#include "mqttpublisher.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QtAndroid>
#endif

#ifdef Q_OS_MACOS
#include "macos/lockscreen.h"
#endif

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

#include "handleurl.h"

bool logs = true;
bool noWriteResistance = false;
bool noHeartService = true;
bool noConsole = false;
bool onlyVirtualBike = false;
bool onlyVirtualTreadmill = false;
bool testPeloton = false;
bool testHomeFitnessBudy = false;
bool testPowerZonePack = false;
QString peloton_username = "";
QString peloton_password = "";
QString pzp_username = "";
QString pzp_password = "";
bool fit_file_saved_on_quit = false;
bool testResistance = false;
bool forceQml = true;
bool miles = false;
bool bluetooth_no_reconnection = false;
bool bluetooth_relaxed = false;
bool bike_cadence_sensor = false;
bool bike_power_sensor = false;
bool battery_service = false;
bool service_changed = false;
bool bike_wheel_revs = false;
bool run_cadence_sensor = false;
bool nordictrack_10_treadmill = false;
bool reebok_fr30_treadmill = false;
bool zwift_play = false;
bool zwift_click = false;
bool zwift_play_emulator = false;
bool virtual_device_bluetooth = true;
QString eventGearDevice = QStringLiteral("");
QString trainProgram;
QString deviceName = QLatin1String("");
uint32_t pollDeviceTime = 200;
int8_t bikeResistanceOffset = 4;
double bikeResistanceGain = 1.0;
QString logfilename = QStringLiteral("debug-") +
                      QDateTime::currentDateTime()
                          .toString()
                          .replace(QStringLiteral(":"), QStringLiteral("_"))
                          .replace(QStringLiteral(" "), QStringLiteral("_"))
                          .replace(QStringLiteral("."), QStringLiteral("_")) +
                      QStringLiteral(".log");
QUrl profileToLoad;
static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler(0);

QCoreApplication *createApplication(int &argc, char *argv[]) {

    QSettings settings;
    bool nogui = false;

    for (int i = 1; i < argc; ++i) {
        if (!qstrcmp(argv[i], "-no-gui")) {
            nogui = true;
            forceQml = false;
        }
        if (!qstrcmp(argv[i], "-qml"))
            forceQml = true;
        if (!qstrcmp(argv[i], "-noqml"))
            forceQml = false;
        if (!qstrcmp(argv[i], "-miles"))
            miles = true;
        if (!qstrcmp(argv[i], "-no-console"))
            noConsole = true;
        if (!qstrcmp(argv[i], "-test-resistance"))
            testResistance = true;
        if (!qstrcmp(argv[i], "-no-virtual-device-bluetooth"))
            virtual_device_bluetooth = false;
        if (!qstrcmp(argv[i], "-no-log"))
            logs = false;
        if (!qstrcmp(argv[i], "-no-write-resistance"))
            noWriteResistance = true;
        if (!qstrcmp(argv[i], "-no-heart-service"))
            noHeartService = true;
        if (!qstrcmp(argv[i], "-heart-service"))
            noHeartService = false;
        if (!qstrcmp(argv[i], "-only-virtualbike"))
            onlyVirtualBike = true;
        if (!qstrcmp(argv[i], "-only-virtualtreadmill"))
            onlyVirtualTreadmill = true;
        if (!qstrcmp(argv[i], "-no-reconnection"))
            bluetooth_no_reconnection = true;
        if (!qstrcmp(argv[i], "-bluetooth_relaxed"))
            bluetooth_relaxed = true;
        if (!qstrcmp(argv[i], "-bike-cadence-sensor"))
            bike_cadence_sensor = true;
        if (!qstrcmp(argv[i], "-bike-power-sensor"))
            bike_power_sensor = true;
        if (!qstrcmp(argv[i], "-battery-service"))
            battery_service = true;
        if (!qstrcmp(argv[i], "-service-changed"))
            service_changed = true;
        if (!qstrcmp(argv[i], "-bike-wheel-revs"))
            bike_wheel_revs = true;
        if (!qstrcmp(argv[i], "-run-cadence-sensor"))
            run_cadence_sensor = true;
        if (!qstrcmp(argv[i], "-nordictrack-10-treadmill"))
            nordictrack_10_treadmill = true;
        if (!qstrcmp(argv[i], "-reebok_fr30_treadmill"))
            reebok_fr30_treadmill = true;
        if (!qstrcmp(argv[i], "-zwift_play"))
            zwift_play = true;
        if (!qstrcmp(argv[i], "-zwift_click"))
            zwift_click = true;
        if (!qstrcmp(argv[i], "-zwift_play_emulator"))
            zwift_play_emulator = true;
        if (!qstrcmp(argv[i], "-test-peloton"))
            testPeloton = true;
        if (!qstrcmp(argv[i], "-test-hfb"))
            testHomeFitnessBudy = true;
        if (!qstrcmp(argv[i], "-test-pzp"))
            testPowerZonePack = true;
        if (!qstrcmp(argv[i], "-train")) {

            trainProgram = argv[++i];
        }
        if (!qstrcmp(argv[i], "-name")) {

            deviceName = argv[++i];
        }
        if (!qstrcmp(argv[i], "-bluetooth-event-gear-device")) {

            eventGearDevice = argv[++i];
        }
        if (!qstrcmp(argv[i], "-peloton-username")) {

            peloton_username = argv[++i];
        }
        if (!qstrcmp(argv[i], "-peloton-password")) {

            peloton_password = argv[++i];
        }
        if (!qstrcmp(argv[i], "-pzp-username")) {

            pzp_username = argv[++i];
        }
        if (!qstrcmp(argv[i], "-pzp-password")) {

            pzp_password = argv[++i];
        }
        if (!qstrcmp(argv[i], "-poll-device-time")) {

            pollDeviceTime = atol(argv[++i]);
        }
        if (!qstrcmp(argv[i], "-bike-resistance-gain")) {

            bikeResistanceGain = atof(argv[++i]);
        }
        if (!qstrcmp(argv[i], "-bike-resistance-offset")) {

            bikeResistanceOffset = atoi(argv[++i]);
        }
        if (!qstrcmp(argv[i], "-fit-file-saved-on-quit")) {
            fit_file_saved_on_quit = true;
        }
        if (!qstrcmp(argv[i], "-profile")) {
            QString profileName = argv[++i];
            if (QFile::exists(homeform::getProfileDir() + "/" + profileName + ".qzs")) {
                profileToLoad = QUrl::fromLocalFile(homeform::getProfileDir() + "/" + profileName + ".qzs");
            } else {
                qDebug() << homeform::getProfileDir() + "/" + profileName << "not found!";
            }
        }
    }

    if (nogui) {
        return new QCoreApplication(argc, argv);
    } else if (forceQml) {
        return new QApplication(argc, argv);
    } else {

        QApplication *a = new QApplication(argc, argv);

        a->setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

        /*QFont defaultFont = QApplication::font();
        defaultFont.setPointSize(defaultFont.pointSize()+2);
        qApp->setFont(defaultFont);*/

        // modify palette to dark
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
        darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
        darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
        darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
        darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
        darkPalette.setColor(QPalette::HighlightedText, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

        qApp->setPalette(darkPalette);

        return a;
    }
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

    QSettings settings;
    static bool logdebug = settings.value(QZSettings::log_debug, QZSettings::default_log_debug).toBool();
#if defined(Q_OS_LINUX) // Linux OS does not read settings file for now
    if ((logs == false && !forceQml) || (logdebug == false && forceQml))
#else
    if (logdebug == false)
#endif
        return;

    // QByteArray localMsg = msg.toLocal8Bit(); // NOTE: clazy-unused-non-trivial-variable
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    QString txt = QDateTime::currentDateTime().toString() + QStringLiteral(" ") +
                  QString::number(QDateTime::currentMSecsSinceEpoch()) + QStringLiteral(" ");
    switch (type) {
    case QtInfoMsg:
        txt += QStringLiteral("Info: %1 %2 %3\n").arg(file, function, msg); // NOTE: clazy-qstring-arg
        break;
    case QtDebugMsg:
        txt += QStringLiteral("Debug: %1 %2 %3\n").arg(file, function, msg); // NOTE: clazy-qstring-arg
        break;
    case QtWarningMsg:
        txt += QStringLiteral("Warning: %1 %2 %3\n").arg(file, function, msg); // NOTE: clazy-qstring-arg
        break;
    case QtCriticalMsg:
        txt += QStringLiteral("Critical: %1 %2 %3\n").arg(file, function, msg); // NOTE: clazy-qstring-arg
        break;
    case QtFatalMsg:
        txt += QStringLiteral("Fatal: %1 %2 %3\n").arg(file, function, msg); // NOTE: clazy-qstring-arg
        abort();
    }

    if (logs == true || logdebug == true) {

        QString path = homeform::getWritableAppDir();

        // Linux log files are generated on binary location

        QFile outFile(path + logfilename);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt;
        fprintf(stderr, "%s", txt.toLocal8Bit().constData());
    }
    (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, msg);
}

int main(int argc, char *argv[]) {
#ifdef Q_OS_WIN32
    qputenv("QT_MULTIMEDIA_PREFERRED_PLUGINS", "windowsmediafoundation");
#endif

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));
#else
#ifdef Q_OS_IOS
    HandleURL *URLHandler = new HandleURL();
    QDesktopServices::setUrlHandler("org.cagnulein.ConnectIQComms-ciq", URLHandler, "handleURL");
#endif

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QScopedPointer<QApplication> app(new QApplication(argc, argv));
#endif
#ifdef CHARTJS
    QtWebView::initialize();
#endif

#ifdef Q_OS_LINUX
#ifndef Q_OS_ANDROID
    if (getuid() && !testPeloton && !testHomeFitnessBudy && !testPowerZonePack) {

        printf("Runme as root!\n");
        return -1;
    } else
        printf("%s", "OK, you are root.\n");
#endif
#endif

    app->setOrganizationName(QStringLiteral("Roberto Viola"));
    app->setOrganizationDomain(QStringLiteral("robertoviola.cloud"));
    app->setApplicationName(QStringLiteral("qDomyos-Zwift"));

    QSettings settings;

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    QString profileName = "";
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    profileName = lockscreen::get_action_profile();
    lockscreen::nslog(QString("quick_action profile " + profileName).toLatin1());
#endif
#else
    QAndroidJniObject javaPath = QAndroidJniObject::fromString(homeform::getWritableAppDir());
    QAndroidJniObject r = QAndroidJniObject::callStaticObjectMethod("org/cagnulen/qdomyoszwift/Shortcuts", "getProfileExtras",
                                                "(Landroid/content/Context;)Ljava/lang/String;", QtAndroid::androidContext().object());
    profileName = r.toString();
#endif
    
    QFileInfo pp(profileName);
    profileName = pp.baseName();
    
    if(profileName.count()) {
        if (QFile::exists(homeform::getProfileDir() + "/" + profileName + ".qzs")) {
            profileToLoad = QUrl::fromLocalFile(homeform::getProfileDir() + "/" + profileName + ".qzs");
        } else {
            qDebug() << homeform::getProfileDir() + "/" + profileName << "not found!";
        }
    }
#endif

    if (!profileToLoad.isEmpty()) {
        homeform::loadSettings(profileToLoad);
    }

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)

    if (fit_file_saved_on_quit) {
        settings.setValue(QZSettings::fit_file_saved_on_quit, true);
        qDebug() << "fit_file_saved_on_quit"
                 << settings.value(QZSettings::fit_file_saved_on_quit, QZSettings::default_fit_file_saved_on_quit);
    }

    if (forceQml)
#endif
    {
        bool defaultNoHeartService = !noHeartService;

        // some Android 6 doesn't support wake lock
        if (QOperatingSystemVersion::current() < QOperatingSystemVersion(QOperatingSystemVersion::Android, 7) &&
            !settings.value(QZSettings::android_wakelock).isValid()) {
            settings.setValue(QZSettings::android_wakelock, false);
        }

        noHeartService = settings.value(QZSettings::bike_heartrate_service, defaultNoHeartService).toBool();
        bikeResistanceOffset = settings.value(QZSettings::bike_resistance_offset, bikeResistanceOffset).toInt();
        bikeResistanceGain = settings.value(QZSettings::bike_resistance_gain_f, bikeResistanceGain).toDouble();
        deviceName = settings.value(QZSettings::filter_device, QZSettings::default_filter_device).toString();
        pollDeviceTime = settings.value(QZSettings::poll_device_time, QZSettings::default_poll_device_time).toInt();
    }
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    else {
        settings.setValue(QZSettings::miles_unit, miles);
        settings.setValue(QZSettings::bluetooth_no_reconnection, bluetooth_no_reconnection);
        settings.setValue(QZSettings::bluetooth_relaxed, bluetooth_relaxed);
        settings.setValue(QZSettings::bike_cadence_sensor, bike_cadence_sensor);
        settings.setValue(QZSettings::bike_power_sensor, bike_power_sensor);
        settings.setValue(QZSettings::battery_service, battery_service);
        settings.setValue(QZSettings::service_changed, service_changed);
        settings.setValue(QZSettings::bike_wheel_revs, bike_wheel_revs);
        settings.setValue(QZSettings::run_cadence_sensor, run_cadence_sensor);
        settings.setValue(QZSettings::nordictrack_10_treadmill, nordictrack_10_treadmill);
        settings.setValue(QZSettings::reebok_fr30_treadmill, reebok_fr30_treadmill);
        settings.setValue(QZSettings::zwift_click, zwift_click);
        settings.setValue(QZSettings::zwift_play, zwift_play);
        settings.setValue(QZSettings::zwift_play_emulator, zwift_play_emulator);
        settings.setValue(QZSettings::virtual_device_bluetooth, virtual_device_bluetooth);
    }
#endif

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::volume_change_gears, QZSettings::default_volume_change_gears).toBool()) {
        qDebug() << "handling volume keys";
        qputenv("QT_ANDROID_VOLUME_KEYS", "1"); // "1" is dummy
    }
#endif
    
    qInstallMessageHandler(myMessageOutput);
    qDebug() << QStringLiteral("version ") << app->applicationVersion();
    foreach (QString s, settings.allKeys()) {
        if (!s.contains(QStringLiteral("password")) && !s.contains("user_email") && !s.contains("username")) {

            qDebug() << s << settings.value(s);
        }
    }

#if 0
    qDebug() << "-";
    qDebug() << "Settings from QZSettings";
    QZSettings::qDebugAllSettings();
    qDebug() << "-";
#endif

#if 0 // test gpx or fit export
    QList<SessionLine> l;
    for(int i =0; i< 500; i++)
    {
        QDateTime d = QDateTime::currentDateTime();
        l.append(SessionLine(i%20,i%10,i,i%300,i%10,i%180,i%6,i%120,i,i, d));
    }
    QString path = homeform::getWritableAppDir();
    qfit::save(path + QDateTime::currentDateTime().toString().replace(":", "_") + ".fit", l, bluetoothdevice::BIKE);
    return 0;
#endif

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    if (!forceQml) {
        if (onlyVirtualBike) {
            virtualbike V(new bike(), noWriteResistance,
                          noHeartService); // FIXED: clang-analyzer-cplusplus.NewDeleteLeaks - potential leak

            Q_UNUSED(V)
            return app->exec();
        } else if (onlyVirtualTreadmill) {
            virtualtreadmill V(new treadmill(),
                               noHeartService); // FIXED: clang-analyzer-cplusplus.NewDeleteLeaks - potential leak

            Q_UNUSED(V)
            return app->exec();
        } else if (testPeloton) {
            settings.setValue(QZSettings::peloton_username, peloton_username);
            settings.setValue(QZSettings::peloton_password, peloton_password);
            peloton *p = new peloton(0, 0);
            p->setTestMode(true);
            QObject::connect(p, &peloton::loginState, [&](bool ok) {
                if (ok) {
                } else {
                    exit(1);
                }
            });
            QObject::connect(p, &peloton::workoutStarted,
                             [&](QString workout_name, QString instructor) { app->exit(0); });
            return app->exec();
        } else if (testHomeFitnessBudy) {
            homefitnessbuddy *h = new homefitnessbuddy(0, 0);
            QObject::connect(h, &homefitnessbuddy::loginState, [&](bool ok) {
                if (ok) {
                    h->searchWorkout(QDate(2021, 8, 21), "Matt Wilpers", 2700, "");
                    QObject::connect(h, &homefitnessbuddy::workoutStarted, [&](QList<trainrow> *list) {
                        if (list->length() > 0)
                            app->exit(0);
                        else
                            app->exit(2);
                    });
                } else {
                    exit(1);
                }
            });
            return app->exec();
        } else if (testPowerZonePack) {
            powerzonepack *h = new powerzonepack(0, 0);
            QObject::connect(h, &powerzonepack::loginState, [&](bool ok) {
                if (ok) {
                    h->searchWorkout("d6a54e1ce634437bb172f61eb1588b27");
                    QObject::connect(h, &powerzonepack::workoutStarted, [&](QList<trainrow> *list) {
                        if (list->length() > 0)
                            app->exit(0);
                        else
                            app->exit(2);
                    });
                } else {
                    exit(1);
                }
            });
            return app->exec();
        }
    }
#endif

    settings.setValue(QZSettings::app_opening,
                      settings.value(QZSettings::app_opening, QZSettings::default_app_opening).toInt() + 1);

#if defined(Q_OS_ANDROID)
    auto result = QtAndroid::checkPermission(QString("android.permission.READ_EXTERNAL_STORAGE"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.READ_EXTERNAL_STORAGE"}));
        if (resultHash["android.permission.READ_EXTERNAL_STORAGE"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "READ_EXTERNAL_STORAGE denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.ACCESS_FINE_LOCATION"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.ACCESS_FINE_LOCATION"}));
        if (resultHash["android.permission.ACCESS_FINE_LOCATION"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "ACCESS_FINE_LOCATION denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.BLUETOOTH"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.BLUETOOTH"}));
        if (resultHash["android.permission.BLUETOOTH"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "BLUETOOTH denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.BLUETOOTH_ADMIN"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.BLUETOOTH_ADMIN"}));
        if (resultHash["android.permission.BLUETOOTH_ADMIN"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "BLUETOOTH_ADMIN denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.BLUETOOTH_SCAN"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.BLUETOOTH_SCAN"}));
        if (resultHash["android.permission.BLUETOOTH_SCAN"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "BLUETOOTH_SCAN denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.BLUETOOTH_ADVERTISE"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.BLUETOOTH_ADVERTISE"}));
        if (resultHash["android.permission.BLUETOOTH_ADVERTISE"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "BLUETOOTH_ADVERTISE denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.BLUETOOTH_CONNECT"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.BLUETOOTH_CONNECT"}));
        if (resultHash["android.permission.BLUETOOTH_CONNECT"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "BLUETOOTH_CONNECT denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.POST_NOTIFICATIONS"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.POST_NOTIFICATIONS"}));
        if (resultHash["android.permission.POST_NOTIFICATIONS"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "POST_NOTIFICATIONS denied!";
    }    
#endif

    /* test virtual echelon
     * settings.setValue(QZSettings::virtual_device_echelon, true);
    virtualbike* V = new virtualbike(new bike(), noWriteResistance, noHeartService);
    Q_UNUSED(V)
    return app->exec();*/
    bluetooth bl(logs, deviceName, noWriteResistance, noHeartService, pollDeviceTime, noConsole, testResistance,
                 bikeResistanceOffset,
                 bikeResistanceGain); // FIXED: clang-analyzer-cplusplus.NewDeleteLeaks - potential leak

    QString mqtt_host = settings.value(QZSettings::mqtt_host, QZSettings::default_mqtt_host).toString();
    int mqtt_port = settings.value(QZSettings::mqtt_port, QZSettings::default_mqtt_port).toInt();
    QString mqtt_username = settings.value(QZSettings::mqtt_username, QZSettings::default_mqtt_username).toString();
    QString mqtt_password = settings.value(QZSettings::mqtt_password, QZSettings::default_mqtt_password).toString();
    if(mqtt_host.length() > 0) {
        MQTTPublisher* mqtt = new MQTTPublisher(mqtt_host, mqtt_port, mqtt_username, mqtt_password, &bl);
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    lockscreen h;
    h.request();
#endif
#endif

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    if (forceQml)
#endif
    {
        QQmlApplicationEngine engine;
        const QUrl url(QStringLiteral("qrc:/main.qml"));
        QObject::connect(
            &engine, &QQmlApplicationEngine::objectCreated, qobject_cast<QGuiApplication *>(app.data()),
            [url](QObject *obj, const QUrl &objUrl) {
                if (!obj && url == objUrl)
                    QCoreApplication::exit(-1);
            },
            Qt::QueuedConnection);

#ifdef Q_OS_ANDROID
        engine.rootContext()->setContextProperty("OS_VERSION", QVariant("Android"));
#elif defined(Q_OS_IOS)
        engine.rootContext()->setContextProperty("OS_VERSION", QVariant("iOS"));
#else
        engine.rootContext()->setContextProperty("OS_VERSION", QVariant("Other"));
#endif
#ifdef CHARTJS
        engine.rootContext()->setContextProperty("CHARTJS", QVariant(true));
#else
        engine.rootContext()->setContextProperty("CHARTJS", QVariant(false));
#endif
        engine.load(url);
        homeform *h = new homeform(&engine, &bl);
        QObject::connect(app.data(), &QCoreApplication::aboutToQuit, h,
                         &homeform::aboutToQuit); // NOTE: clazy-unneeded-cast

        {
#ifdef Q_OS_ANDROID
            KeepAwakeHelper helper;
#elif defined Q_OS_MACOS
            lockScreen();
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
            lockscreen yc;
            yc.setTimerDisabled();
#endif
#endif
            // screen and CPU will stay awake during this section
            // lock will be released when helper object goes out of scope
            return app->exec();
        }
#ifdef Q_OS_MACOS
        unlockScreen();
#endif
    }
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    else {
        bl.homeformLoaded = true;
    }
#endif

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    if (qobject_cast<QApplication *>(app.data())) {
        // start GUI version...
        MainWindow *W = 0;
        if (trainProgram.isEmpty()) {
            W = new MainWindow(&bl);
        } else {
            W = new MainWindow(&bl, trainProgram);
        }
        W->show();
    } else {
        // start non-GUI version...
    }

#ifdef Q_OS_LINUX
#ifndef Q_OS_ANDROID
    if(eventGearDevice.length())
        new BluetoothHandler(&bl, eventGearDevice);
#endif
#endif
    return app->exec();
#endif
}
