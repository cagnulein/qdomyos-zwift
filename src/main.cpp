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
#include <QTranslator>
#include <QLocale>
#include "logwriter.h"
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
#include <QList>
#ifdef CHARTJS
#include <QtWebView/QtWebView>
#endif

#include "mqttpublisher.h"
#include "androidstatusbar.h"
#include "fontmanager.h"
#include "filesearcher.h"

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

#include "osc.h"

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
QString mqtt_host = "";
int mqtt_port = -1;
QString mqtt_username = "";
QString mqtt_password = "";
QString mqtt_deviceid = "";
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
bool horizon_treadmill_7_8 = false;
bool horizon_treadmill_force_ftms = false;
bool nordictrack_10_treadmill = false;
bool proform_performance_300i_treadmill = false;
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
QString power_sensor_name = QStringLiteral("Disabled");
bool power_sensor_as_treadmill = false;
QString logfilename = QStringLiteral("debug-") +
                      QDateTime::currentDateTime()
                          .toString()
                          .replace(QStringLiteral(":"), QStringLiteral("_"))
                          .replace(QStringLiteral(" "), QStringLiteral("_"))
                          .replace(QStringLiteral("."), QStringLiteral("_")) +
                      QStringLiteral(".log");
QUrl profileToLoad;
static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler(0);

// Function to display help information and exit
void displayHelp() {
    // Test string for translation workflow - will be extracted by lupdate
    QString testTranslation = QCoreApplication::translate("main", "QDomyos-Zwift - Fitness Equipment Bridge");
    Q_UNUSED(testTranslation); // Suppress unused variable warning

    printf("qDomyos-Zwift Usage:\n");
    printf("General options:\n");
    printf("  -h, --help                    Display this help message and exit\n");
    printf("  -no-gui                       Run in non-GUI mode\n");
    printf("  -qml                          Force QML mode\n");
    printf("  -noqml                        Disable QML mode\n");
    printf("  -miles                        Use miles instead of kilometers\n");
    printf("  -no-console                   Disable console output\n");
    printf("  -no-log                       Disable logging\n");
    printf("  -profile <name>               Load specific profile\n");

    printf("\nDevice configuration:\n");
    printf("  -name <device_name>           Set device name\n");
    printf("  -poll-device-time <ms>        Set device polling time in milliseconds\n");
    printf("  -no-write-resistance          Disable resistance writing\n");
    printf("  -no-heart-service             Disable heart rate service\n");
    printf("  -heart-service                Enable heart rate service\n");
    printf("  -no-virtual-device-bluetooth  Disable virtual device bluetooth\n");

    printf("\nBike specific options:\n");
    printf("  -only-virtualbike             Run only virtual bike mode\n");
    printf("  -bike-resistance-gain <value> Set bike resistance gain\n");
    printf("  -bike-resistance-offset <value> Set bike resistance offset\n");
    printf("  -bike-cadence-sensor          Enable bike cadence sensor\n");
    printf("  -bike-power-sensor            Enable bike power sensor\n");
    printf("  -bike-wheel-revs              Enable bike wheel revolution tracking\n");
    printf("  -power-sensor-name <name>     Set power sensor name\n");
    printf("  -power-sensor-as-treadmill    Use power sensor as treadmill\n");

    printf("\nTreadmill specific options:\n");
    printf("  -only-virtualtreadmill        Run only virtual treadmill mode\n");
    printf("  -run-cadence-sensor           Enable run cadence sensor\n");
    printf("  -horizon-treadmill-7-8        Enable Horizon 7.8 treadmill support\n");
    printf("  -horizon-treadmill-force-ftms Force FTMS for Horizon treadmill\n");
    printf("  -nordictrack-10-treadmill     Enable NordicTrack 10 treadmill support\n");
    printf("  -proform-perf-300i-treadmill  Enable Proform Performance 300i support\n");
    printf("  -reebok_fr30_treadmill        Enable Reebok FR30 treadmill support\n");

    printf("\nBluetooth options:\n");
    printf("  -no-reconnection              Disable bluetooth reconnection\n");
    printf("  -bluetooth_relaxed            Enable relaxed bluetooth mode\n");
    printf("  -battery-service              Enable battery service\n");
    printf("  -service-changed              Enable service changed notifications\n");
    printf("  -bluetooth-event-gear-device <device> Set bluetooth event gear device\n");

    printf("\nIntegration options:\n");
    printf("  -zwift_play                   Enable Zwift Play\n");
    printf("  -zwift_click                  Enable Zwift Click\n");
    printf("  -zwift_play_emulator          Enable Zwift Play emulator\n");
    printf("  -test-peloton                 Enable Peloton test mode\n");
    printf("  -test-hfb                     Enable Home Fitness Buddy test mode\n");
    printf("  -test-pzp                     Enable Power Zone Pack test mode\n");
    printf("  -train <program>              Specify training program\n");

    printf("\nPeloton options:\n");
    printf("  -peloton-username <username>  Set Peloton username\n");
    printf("  -peloton-password <password>  Set Peloton password\n");

    printf("\nPower Zone Pack options:\n");
    printf("  -pzp-username <username>      Set Power Zone Pack username\n");
    printf("  -pzp-password <password>      Set Power Zone Pack password\n");

    printf("\nMQTT options:\n");
    printf("  -mqtt-host <hostname>         Set MQTT broker hostname\n");
    printf("  -mqtt-port <port>             Set MQTT broker port (default: 1883)\n");
    printf("  -mqtt-username <username>     Set MQTT username\n");
    printf("  -mqtt-password <password>     Set MQTT password\n");
    printf("  -mqtt-deviceid <deviceid>     Set MQTT device ID\n");

    printf("\nOther options:\n");
    printf("  -test-resistance              Enable resistance testing\n");
    printf("  -fit-file-saved-on-quit       Save FIT file on application quit\n");

    exit(0);
}


#ifdef Q_CC_MSVC
#include <windows.h>
#include <dbghelp.h>
#include <rtcapi.h>
#include <cstdio>

void PrintStack() {
    CONTEXT context = {};
    RtlCaptureContext(&context);

    STACKFRAME64 stackFrame = {};
    stackFrame.AddrPC.Offset = context.Rip;  // Per x64, usa Rip
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = context.Rbp;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = context.Rsp;
    stackFrame.AddrStack.Mode = AddrModeFlat;

    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    SymInitialize(process, NULL, TRUE);

    while (StackWalk64(
        IMAGE_FILE_MACHINE_AMD64, process, thread, &stackFrame, &context,
        NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
        printf("Address: 0x%llx\n", stackFrame.AddrPC.Offset);
    }

    SymCleanup(process);
}

int __cdecl CustomRTCErrorHandler(int errorType, const wchar_t* filename, int linenumber, 
                           const wchar_t* moduleName, const wchar_t* format, ...)
{
    // Buffer for the formatted error message
    wchar_t errorMessage[512];
    va_list args;
    
    // Format the error message using varargs
    va_start(args, format);
    vswprintf(errorMessage, sizeof(errorMessage)/sizeof(wchar_t), format, args);
    va_end(args);
    
    // Print complete error information
    fwprintf(stderr, L"Runtime Error Check Failed!\n");
    fwprintf(stderr, L"Error Type: %d\n", errorType);
    fwprintf(stderr, L"File: %ls\n", filename ? filename : L"Unknown");
    fwprintf(stderr, L"Line: %d\n", linenumber);
    fwprintf(stderr, L"Module: %ls\n", moduleName ? moduleName : L"Unknown");
    fwprintf(stderr, L"Error Message: %ls\n", errorMessage);
    fwprintf(stderr, L"----------------------------------------\n");
    
    #ifdef _DEBUG
        __debugbreak();  // Break into debugger in debug builds
    #endif
  
    PrintStack();

    return 1;  // Return non-zero to indicate error was handled    
}
#endif

QCoreApplication *createApplication(int &argc, char *argv[]) {

    QSettings settings;
    bool nogui = false;

    for (int i = 1; i < argc; ++i) {
        if (!qstrcmp(argv[i], "-h") || !qstrcmp(argv[i], "--help")) {
            displayHelp();
            // displayHelp() will exit the program
        }
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
        if (!qstrcmp(argv[i], "-horizon-treadmill-7-8"))
            horizon_treadmill_7_8 = true; 
        if (!qstrcmp(argv[i], "-horizon-treadmill-force-ftms"))
            horizon_treadmill_force_ftms = true; 
        if (!qstrcmp(argv[i], "-nordictrack-10-treadmill"))
            nordictrack_10_treadmill = true;
        if (!qstrcmp(argv[i], "-proform-perf-300i-treadmill"))
            proform_performance_300i_treadmill = true;
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
        if (!qstrcmp(argv[i], "-power-sensor-name")) {
            power_sensor_name = argv[++i];
        }
        if (!qstrcmp(argv[i], "-power-sensor-as-treadmill")) {
            power_sensor_as_treadmill = true;
        }
        if (!qstrcmp(argv[i], "-mqtt-host")) {
            mqtt_host = argv[++i];
        }
        if (!qstrcmp(argv[i], "-mqtt-port")) {
            mqtt_port = atoi(argv[++i]);
        }
        if (!qstrcmp(argv[i], "-mqtt-username")) {
            mqtt_username = argv[++i];
        }
        if (!qstrcmp(argv[i], "-mqtt-password")) {
            mqtt_password = argv[++i];
        }
        if (!qstrcmp(argv[i], "-mqtt-deviceid")) {
            mqtt_deviceid = argv[++i];
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

// Global thread and writer instance
static QThread *logThread = nullptr;
static LogWriter *logWriter = nullptr;

void initializeLogThread() {
    if (!logThread) {
        logThread = new QThread();
        logWriter = new LogWriter();
        logWriter->moveToThread(logThread);
        logThread->start();
    }
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

    QSettings settings;
    static bool logdebug = settings.value(QZSettings::log_debug, QZSettings::default_log_debug).toBool();
#if defined(Q_OS_LINUX) // Linux OS does not read settings file for now
    if ( (logs == false && !forceQml) || (logdebug == false && forceQml))
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

        // Ensure thread is initialized
        initializeLogThread();

        // Write log in the worker thread
        QMetaObject::invokeMethod(logWriter, "writeLog",
                                 Qt::QueuedConnection,
                                 Q_ARG(QString, path + logfilename),
                                 Q_ARG(QString, txt));

    }
    (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, msg);
}

int main(int argc, char *argv[]) {
#ifdef Q_OS_WIN32
    qputenv("QT_MULTIMEDIA_PREFERRED_PLUGINS", "windowsmediafoundation");
#endif

#ifdef Q_CC_MSVC
  _RTC_SetErrorFuncW(CustomRTCErrorHandler);
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
        settings.setValue(QZSettings::horizon_treadmill_7_8, horizon_treadmill_7_8);
        settings.setValue(QZSettings::horizon_treadmill_force_ftms, horizon_treadmill_force_ftms);
        settings.setValue(QZSettings::nordictrack_10_treadmill, nordictrack_10_treadmill);
        settings.setValue(QZSettings::proform_performance_300i, proform_performance_300i_treadmill);
        settings.setValue(QZSettings::reebok_fr30_treadmill, reebok_fr30_treadmill);
        settings.setValue(QZSettings::zwift_click, zwift_click);
        settings.setValue(QZSettings::zwift_play, zwift_play);
        settings.setValue(QZSettings::zwift_play_emulator, zwift_play_emulator);
        settings.setValue(QZSettings::virtual_device_bluetooth, virtual_device_bluetooth);
        settings.setValue(QZSettings::power_sensor_name, power_sensor_name);
        settings.setValue(QZSettings::power_sensor_as_treadmill, power_sensor_as_treadmill);
        if (mqtt_host.length() > 0) {
            settings.setValue(QZSettings::mqtt_host, mqtt_host);
        }
        if (mqtt_port != -1) {
            settings.setValue(QZSettings::mqtt_port, mqtt_port);
        }
        if (mqtt_username.length() > 0) {
            settings.setValue(QZSettings::mqtt_username, mqtt_username);
        }
        if (mqtt_password.length() > 0) {
            settings.setValue(QZSettings::mqtt_password, mqtt_password);
        }
        if (mqtt_deviceid.length() > 0) {
            settings.setValue(QZSettings::mqtt_deviceid, mqtt_deviceid);
        }
    }
#endif

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::volume_change_gears, QZSettings::default_volume_change_gears).toBool()) {
        qDebug() << "handling volume keys";
        qputenv("QT_ANDROID_VOLUME_KEYS", "1"); // "1" is dummy
    }
#endif
    
    // Register custom meta types used in queued invocations
    qRegisterMetaType<SessionLine>("SessionLine");
    qRegisterMetaType<QList<SessionLine>>("QList<SessionLine>");
    qRegisterMetaType<BLUETOOTH_TYPE>("BLUETOOTH_TYPE");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<FIT_SPORT>("FIT_SPORT");

    qInstallMessageHandler(myMessageOutput);
    qDebug() << QStringLiteral("version ") << app->applicationVersion();
    foreach (QString s, settings.allKeys()) {
        if (!s.contains(QStringLiteral("password")) && !s.contains("user_email") && !s.contains("username") && !s.contains("token") && !s.contains("garmin_device_serial") && !s.contains("garmin_email")) {

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
    qfit::save(path + QDateTime::currentDateTime().toString().replace(":", "_") + ".fit", l, BIKE);
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

    QString OSC_ip = settings.value(QZSettings::OSC_ip, QZSettings::default_OSC_ip).toString();
    if(OSC_ip.length() > 0) {
        OSC* osc = new OSC(&bl);
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
        AndroidStatusBar::registerQmlType();

#ifdef Q_OS_ANDROID
        FontManager fontManager;
        fontManager.initializeEmojiFont();
#endif

        // Load translations based on explicit app setting or system locale.
        QTranslator *translator = new QTranslator(app.data());
        QString configuredLanguage =
            settings.value(QZSettings::app_language, QZSettings::default_app_language).toString().trimmed();
        QString locale = configuredLanguage.compare(QStringLiteral("auto"), Qt::CaseInsensitive) == 0 ||
                                 configuredLanguage.isEmpty()
                             ? QLocale::system().name()
                             : configuredLanguage;
        locale.replace(QLatin1Char('-'), QLatin1Char('_'));

        auto tryLoadTranslation = [&](const QString &localeKey, const QString &sourceLabel) -> bool {
            if (localeKey.isEmpty()) {
                return false;
            }
            if (translator->load(QStringLiteral(":/translations/translations/qdomyos-zwift_") + localeKey)) {
                app->installTranslator(translator);
                qDebug() << "Translation loaded successfully for" << sourceLabel << ":" << localeKey;
                return true;
            }
            return false;
        };

        // "en" is the built-in source language, so we don't load any translator for it.
        if (locale.startsWith(QStringLiteral("en"), Qt::CaseInsensitive)) {
            qDebug() << "Language setting resolves to English. Using built-in source strings.";
        } else {
            bool loaded = tryLoadTranslation(locale, QStringLiteral("locale"));
            if (!loaded && locale.contains(QLatin1Char('_'))) {
                loaded = tryLoadTranslation(locale.section(QLatin1Char('_'), 0, 0), QStringLiteral("language"));
            }
            if (!loaded) {
                qDebug() << "No translation available for locale:" << locale << "- using default (English)";
            }
        }

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
#ifdef Q_OS_ANDROID
        engine.rootContext()->setContextProperty("fontManager", &fontManager);
#endif
        // Expose FileSearcher for fast recursive file searching
        FileSearcher fileSearcher;
        engine.rootContext()->setContextProperty("fileSearcher", &fileSearcher);

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
