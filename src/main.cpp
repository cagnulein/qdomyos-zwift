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
#ifdef Q_OS_ANDROID
#include <unistd.h> // _exit
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
#include <QFileOpenEvent>
#include <QEvent>
#include <QLoggingCategory>
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
#include <QAccessible>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QKeyEvent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSemaphore>
#include <QSharedPointer>
#include <QThread>
#include <QTimer>
#include <QtAndroid>
#include <QtMath>
#include <QVector>
#include <QWindow>
#include <jni.h>
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 9)
#include <QtCore/private/qjni_p.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/private/qhighdpiscaling_p.h>
#include <qpa/qplatformaccessibility.h>
#include <qpa/qplatformintegration.h>
#endif
#endif

#ifdef Q_OS_MACOS
#include "macos/lockscreen.h"
#endif

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

#include "osc.h"

#include "handleurl.h"
#include "mywhooshlink.h"
#include "authutils.h"

class OAuthCallbackEventFilter : public QObject {
  public:
    bool eventFilter(QObject *watched, QEvent *event) override {
#ifdef Q_OS_IOS
        Q_UNUSED(watched)
        if (event->type() == QEvent::FileOpen) {
            auto *fileEvent = static_cast<QFileOpenEvent *>(event);
            const QUrl url = fileEvent->url();
            qDebug() << "QZ iOS FileOpen event received" << sanitizedOAuthCallbackUrl(url);
            if (url.isValid() && url.host() == QStringLiteral("www.qzfitness.com") &&
                url.path().startsWith(QStringLiteral("/peloton/callback")) && homeform::singleton()) {
                qDebug() << "QZ iOS FileOpen matched Peloton callback";
                QMetaObject::invokeMethod(homeform::singleton(), "handleOAuthCallbackUrl", Qt::QueuedConnection,
                                          Q_ARG(QString, url.toString()));
            } else {
                qDebug() << "QZ iOS FileOpen ignored";
            }
        }
#else
        Q_UNUSED(watched)
        Q_UNUSED(event)
#endif
        return false;
    }
};

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
bool smokeTest = false;
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
    printf("  -smoke-test                   Run smoke test (verify Qt loads, print SMOKE_OK, exit)\n");
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
        if (!qstrcmp(argv[i], "-smoke-test")) {
            smokeTest = true;
            nogui = true;
        }
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

    // QSettings only for the first call: this runs for every log line, even with the debug log off
    static bool logdebug = QSettings().value(QZSettings::log_debug, QZSettings::default_log_debug).toBool();
#if defined(Q_OS_LINUX) // Linux OS does not read settings file for now
    if ( (logs == false && !forceQml) || (logdebug == false && forceQml))
#else
    if (logdebug == false)
#endif
    {
        // Qt aborts after a fatal message: print it even with the log off
        if (type == QtFatalMsg)
            (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, msg);
        return;
    }

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
        // the log thread would not get to this line before the abort: write it here
        if (logs == true || logdebug == true)
            LogWriter().writeLog(homeform::getWritableAppDir() + logfilename, txt);
        (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, msg);
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

#ifdef Q_OS_ANDROID
// Android scrolls its own lists to the item TalkBack moves to, Qt 5.15 does not: TalkBack would
// stop on an item below the screen, draw its frame there and miss it with a double tap. Each
// Flickable around the item (a ScrollView, a ListView in it) is scrolled just enough to show it.
static bool scrollItemIntoView(QQuickItem *item) {
    bool scrolled = false;
    for (QQuickItem *flickable = item->parentItem(); flickable; flickable = flickable->parentItem()) {
        if (!flickable->inherits("QQuickFlickable") || !flickable->property("interactive").toBool())
            continue;
        const QRectF rect = item->mapRectToItem(flickable, QRectF(0, 0, item->width(), item->height()));
        const qreal height = flickable->height();
        // as Android's ScrollView: an item that already covers the whole view stays where it is,
        // one taller than the view is shown from its top
        qreal delta = 0;
        if (rect.top() < 0 && rect.bottom() <= height)
            delta = rect.top();
        else if (rect.top() > 0 && rect.bottom() > height)
            delta = qMin(rect.bottom() - height, rect.top());
        if (delta == 0)
            continue;
        const qreal contentY = flickable->property("contentY").toReal();
        const qreal originY = flickable->property("originY").toReal();
        const qreal top = originY - flickable->property("topMargin").toReal();
        const qreal bottom = originY + flickable->property("contentHeight").toReal() +
                             flickable->property("bottomMargin").toReal() - height;
        const qreal target = qMax(top, qMin(contentY + delta, bottom));
        if (target != contentY) {
            flickable->setProperty("contentY", target);
            scrolled = true;
        }
    }
    return scrolled;
}

// CustomQtActivity: TalkBack put its focus on this accessible object (Android UI thread)
extern "C" JNIEXPORT void JNICALL
Java_org_cagnulen_qdomyoszwift_CustomQtActivity_nativeScrollToAccessibleObject(JNIEnv *env, jclass clazz, jint id) {
    Q_UNUSED(env)
    Q_UNUSED(clazz)
    if (!QCoreApplication::instance())
        return;
    QMetaObject::invokeMethod(
        QCoreApplication::instance(),
        [id] {
            QAccessibleInterface *iface = QAccessible::accessibleInterface(QAccessible::Id(id));
            QQuickItem *item = iface && iface->isValid() ? qobject_cast<QQuickItem *>(iface->object()) : nullptr;
            if (item && item->isVisible() && scrollItemIntoView(item))
                QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/CustomQtActivity",
                                                          "accessibilityTreeChanged", "()V");
        },
        Qt::QueuedConnection);
}

// CustomQtActivity: TalkBack's double tap on this accessible object (Android UI thread). Qt 5.15
// does nothing for a button there (QQuickAbstractButton has no accessible press yet) and answers
// false, after which TalkBack taps the middle of the node: that misses the indicator of a switch
// with a MouseArea over it (IndicatorOnlySwitch) and may hit something else on a moved page. A
// button gets the space key a keyboard presses it with instead (nextCheckState() with exclusive
// groups, clicked(), its Action), and true is answered. For anything else, and for a button that
// is disabled, hidden or gone, false is answered and Qt's own press runs, as before.
// The object is looked up and pressed on the Qt thread, where QML items are created and destroyed
// (as Qt 6 does with runInObjectContext()); this thread waits only for the lookup, not for what the
// press runs. Qt 5.15 has no guard against a Qt thread that waits for this one, so the wait is
// limited: when the Qt thread is busy that long, false is answered and this press is not made;
// Qt's own press and TalkBack's tap run instead, as for anything else.
// 2 s: the Qt thread can be busy for well over a second while a device connects, and Android
// waits 5 s for the answer to an accessibility action.
extern "C" JNIEXPORT jboolean JNICALL
Java_org_cagnulen_qdomyoszwift_CustomQtActivity_nativeAccessibleClick(JNIEnv *env, jclass clazz, jint id) {
    Q_UNUSED(env)
    Q_UNUSED(clazz)
    if (!QCoreApplication::instance())
        return false;
    struct Click {
        QSemaphore found;
        QAtomicInt state; // 0 waiting, 1 started on the Qt thread, 2 given up by this thread
        bool button = false;
    };
    const QSharedPointer<Click> click = QSharedPointer<Click>::create();
    QMetaObject::invokeMethod(
        QCoreApplication::instance(),
        [click, id] {
            if (!click->state.testAndSetOrdered(0, 1))
                return;
            QAccessibleInterface *iface = QAccessible::accessibleInterface(QAccessible::Id(id));
            QQuickItem *button = iface && iface->isValid() ? qobject_cast<QQuickItem *>(iface->object()) : nullptr;
            click->button = button && button->inherits("QQuickAbstractButton") && button->isVisible() &&
                            button->isEnabled();
            click->found.release();
            if (!click->button)
                return;
            QKeyEvent press(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, QStringLiteral(" "));
            QKeyEvent release(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, QStringLiteral(" "));
            QCoreApplication::sendEvent(button, &press);
            QCoreApplication::sendEvent(button, &release);
        },
        Qt::QueuedConnection);
    if (!click->found.tryAcquire(1, 2000)) {
        if (click->state.testAndSetOrdered(0, 2))
            return false;
        // started just now: the lookup does not wait for this thread
        click->found.acquire();
    }
    return click->button;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 9)
// Qt 5.15 runs the functions its accessibility layer gives Java (QtNativeAccessibility) on the
// Android UI thread, while the QML items and accessible interfaces they read are made and
// destroyed on the Qt thread. A screen reader that reads a page Qt is building or removing (the
// settings of the tiles: some 90 items) crashes QZ (QTBUG-95764; here in QAccessibleCache), and
// turning accessibility on while QZ is open makes Qt Quick objects on the wrong thread
// (QTBUG-144911). Qt runs them on the Qt thread since 6.2.4 and 5.15.9 (qtbase e0c61193ea), and
// setActive() too since 6.8.10 (4708e204c2). Here they are registered over Qt's own: the code of
// Qt 5.15.0's androidjniaccessibility.cpp, with what touches Qt objects moved to the Qt thread.
// Unlike Qt 6 the wait is limited, as for nativeAccessibleClick() above: Qt 5.15.0's accessibility
// layer takes no guard against a Qt thread that waits for the Android UI thread (QTBUG-105958; Qt 6
// also skips the wait while the event loop is stopped, QTBUG-102594, which QZ's manifest turns off
// with android.app.background_running). After 2 s a read gives up with the answer for an object
// that is gone, the next reads answer that at once until the Qt thread runs again, and then
// TalkBack is told to read the tree anew. A press or scroll waits for the lookup only, not for
// what it runs.
namespace androidAccessibility {

static QAtomicInt qtThreadStuck; // 1 after a read gave up, until the Qt thread runs again

// The result of func run on the Qt thread, or fallback when that thread did not start it within
// 2 s (it is not run then). On the Qt thread func is run directly.
template <typename T, typename Func> static T runOnQtThread(const T &fallback, Func func) {
    QCoreApplication *app = QCoreApplication::instance();
    if (!app)
        return fallback;
    if (QThread::currentThread() == app->thread())
        return func();
    if (qtThreadStuck.loadAcquire())
        return fallback;
    struct Call {
        QSemaphore done;
        QAtomicInt state; // 0 waiting, 1 started on the Qt thread, 2 given up by this thread
        T result;
    };
    const QSharedPointer<Call> call = QSharedPointer<Call>::create();
    call->result = fallback;
    QMetaObject::invokeMethod(
        app,
        [call, func] {
            if (!call->state.testAndSetOrdered(0, 1))
                return;
            call->result = func();
            call->done.release();
        },
        Qt::QueuedConnection);
    if (!call->done.tryAcquire(1, 2000)) {
        if (call->state.testAndSetOrdered(0, 2)) {
            qtThreadStuck.storeRelease(1);
            qWarning() << "accessibility: the Qt thread did not answer in 2 s, answering empty until it runs";
            // queued after the flag is set, so it is cleared once the Qt thread runs again; the
            // empty answers given meanwhile are what TalkBack cached, so it reads the tree anew
            QMetaObject::invokeMethod(
                app,
                [] {
                    if (qtThreadStuck.fetchAndStoreOrdered(0) == 1 && QAccessible::isActive())
                        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/CustomQtActivity",
                                                                  "accessibilityTreeChanged", "()V");
                },
                Qt::QueuedConnection);
            return fallback;
        }
        // started just now: a read of the accessibility tree does not wait for this thread
        call->done.acquire();
    }
    return call->result;
}

// The rest runs on the Qt thread, unless said otherwise

static QAccessibleInterface *interfaceFromId(jint objectId) {
    if (objectId == -1) {
        QWindow *window = QGuiApplication::focusWindow();
        return window ? window->accessibleRoot() : nullptr;
    }
    return QAccessible::accessibleInterface(QAccessible::Id(objectId));
}

// QAccessibleBridgeUtils of Qt 5.15.0: a value control can be stepped up and down
static QStringList effectiveActionNames(QAccessibleInterface *iface) {
    QStringList actions;
    if (QAccessibleActionInterface *actionIface = iface->actionInterface())
        actions = actionIface->actionNames();
    if (iface->valueInterface()) {
        if (!actions.contains(QAccessibleActionInterface::increaseAction()))
            actions << QAccessibleActionInterface::increaseAction();
        if (!actions.contains(QAccessibleActionInterface::decreaseAction()))
            actions << QAccessibleActionInterface::decreaseAction();
    }
    return actions;
}

static void performEffectiveAction(QAccessibleInterface *iface, const QString &actionName) {
    if (QAccessibleActionInterface *actionIface = iface->actionInterface()) {
        if (actionIface->actionNames().contains(actionName)) {
            actionIface->doAction(actionName);
            return;
        }
    }
    QAccessibleValueInterface *valueIface = iface->valueInterface();
    if (!valueIface)
        return;
    bool ok;
    const QVariant current = valueIface->currentValue();
    double step = valueIface->minimumStepSize().toDouble(&ok);
    if (!ok || qFuzzyIsNull(step)) {
        const double min = valueIface->minimumValue().toDouble(&ok);
        if (!ok)
            return;
        const double max = valueIface->maximumValue().toDouble(&ok);
        if (!ok)
            return;
        step = (max - min) / 10;
        if (current.userType() != QMetaType::Float && current.userType() != QMetaType::Double)
            step = qCeil(step);
    }
    const double value = current.toDouble(&ok);
    if (!ok)
        return;
    if (actionName == QAccessibleActionInterface::decreaseAction())
        step = -step;
    valueIface->setCurrentValue(value + step);
}

static QString accessibleDescription(QAccessibleInterface *iface) {
    QString description;
    if (iface && iface->isValid()) {
        description = iface->text(QAccessible::Name);
        if (description.isEmpty())
            description = iface->text(QAccessible::Description);
        if (description.isEmpty()) {
            description = iface->text(QAccessible::Value);
            if (description.isEmpty()) {
                if (QAccessibleValueInterface *valueIface = iface->valueInterface())
                    description = valueIface->currentValue().toString();
            }
        }
    }
    return description;
}

// What populateNode() sets on an AccessibilityNodeInfo
struct NodeData {
    bool valid = false;
    QString description;
    bool hasSelection = false;
    int selectionStart = 0;
    int selectionEnd = 0;
    bool enabled = false;
    bool checkable = false;
    bool checked = false;
    bool focusable = false;
    bool focused = false;
    bool visible = false;
    bool clickable = false;
    bool increase = false;
    bool decrease = false;
};

static NodeData nodeData(jint objectId) {
    NodeData data;
    QAccessibleInterface *iface = interfaceFromId(objectId);
    if (!iface || !iface->isValid())
        return data;
    data.valid = true;
    const QAccessible::State state = iface->state();
    const QStringList actions = effectiveActionNames(iface);
    data.clickable = actions.contains(QAccessibleActionInterface::pressAction()) ||
                     actions.contains(QAccessibleActionInterface::toggleAction());
    data.increase = actions.contains(QAccessibleActionInterface::increaseAction());
    data.decrease = actions.contains(QAccessibleActionInterface::decreaseAction());
    data.description = accessibleDescription(iface);
    if (QAccessibleTextInterface *textIface = iface->textInterface()) {
        if (textIface->selectionCount() > 0) {
            data.hasSelection = true;
            textIface->selection(0, &data.selectionStart, &data.selectionEnd);
        }
    }
    data.enabled = !state.disabled;
    data.checkable = state.checkable;
    data.checked = state.checked;
    data.focusable = state.focusable;
    data.focused = state.focused;
    data.visible = !state.invisible;
    return data;
}

// Runs a press or a scroll later on the Qt thread, as the press of a finger would
static void performLater(jint objectId, const QString &actionName) {
    QMetaObject::invokeMethod(
        QCoreApplication::instance(),
        [objectId, actionName] {
            QAccessibleInterface *iface = interfaceFromId(objectId);
            if (iface && iface->isValid())
                performEffectiveAction(iface, actionName);
        },
        Qt::QueuedConnection);
}

// The functions Java calls, on the Android UI thread

static jclass rectClass = nullptr;
static jmethodID rectConstructor = nullptr;
static jmethodID addActionMethod = nullptr;
static jmethodID setCheckableMethod = nullptr;
static jmethodID setCheckedMethod = nullptr;
static jmethodID setClickableMethod = nullptr;
static jmethodID setContentDescriptionMethod = nullptr;
static jmethodID setEnabledMethod = nullptr;
static jmethodID setFocusableMethod = nullptr;
static jmethodID setFocusedMethod = nullptr;
static jmethodID setScrollableMethod = nullptr;
static jmethodID setTextSelectionMethod = nullptr;
static jmethodID setVisibleToUserMethod = nullptr;

// Not waited for (Qt 6.8.10); on the Qt thread (restoreLostAccessibility()) it applies at once
static void setActive(JNIEnv *, jclass, jboolean active) {
    QCoreApplication *app = QCoreApplication::instance();
    if (!app)
        return;
    const bool on = active;
    const auto apply = [on] {
        QPlatformIntegration *integration = QGuiApplicationPrivate::platformIntegration();
        QPlatformAccessibility *accessibility = integration ? integration->accessibility() : nullptr;
        if (accessibility)
            accessibility->setActive(on);
    };
    if (QThread::currentThread() == app->thread())
        apply();
    else
        QMetaObject::invokeMethod(app, apply, Qt::QueuedConnection);
}

static jintArray childIdListForAccessibleObject(JNIEnv *env, jclass, jint objectId) {
    const QVector<jint> ids = runOnQtThread(QVector<jint>(), [objectId] {
        QVector<jint> ids;
        QAccessibleInterface *iface = interfaceFromId(objectId);
        if (iface && iface->isValid()) {
            const int count = iface->childCount();
            for (int i = 0; i < count; ++i) {
                QAccessibleInterface *child = iface->child(i);
                if (child && child->isValid())
                    ids.append(jint(QAccessible::uniqueId(child)));
            }
        }
        return ids;
    });
    jintArray array = env->NewIntArray(jsize(ids.size()));
    env->SetIntArrayRegion(array, 0, jsize(ids.size()), ids.constData());
    return array;
}

static jint parentId(JNIEnv *, jclass, jint objectId) {
    return runOnQtThread(jint(-1), [objectId] {
        QAccessibleInterface *iface = interfaceFromId(objectId);
        if (iface && iface->isValid()) {
            QAccessibleInterface *parent = iface->parent();
            if (parent && parent->isValid()) {
                if (parent->role() == QAccessible::Application)
                    return jint(-1);
                return jint(QAccessible::uniqueId(parent));
            }
        }
        return jint(-1);
    });
}

static jstring descriptionForAccessibleObject(JNIEnv *env, jclass, jint objectId) {
    const QString description =
        runOnQtThread(QString(), [objectId] { return accessibleDescription(interfaceFromId(objectId)); });
    return env->NewString(reinterpret_cast<const jchar *>(description.constData()), jsize(description.size()));
}

static jobject screenRect(JNIEnv *env, jclass, jint objectId) {
    const QRect rect = runOnQtThread(QRect(), [objectId] {
        QAccessibleInterface *iface = interfaceFromId(objectId);
        if (iface && iface->isValid())
            return QHighDpi::toNativePixels(iface->rect(), iface->window());
        return QRect();
    });
    return env->NewObject(rectClass, rectConstructor, rect.left(), rect.top(), rect.right(), rect.bottom());
}

static jint hitTest(JNIEnv *, jclass, jfloat x, jfloat y) {
    return runOnQtThread(jint(-1), [x, y] {
        QAccessibleInterface *root = interfaceFromId(-1);
        if (root && root->isValid()) {
            const QPoint pos = QHighDpi::fromNativePixels(QPoint(int(x), int(y)), root->window());
            QAccessibleInterface *child = root->childAt(pos.x(), pos.y());
            QAccessibleInterface *lastChild = nullptr;
            while (child && child != lastChild) {
                lastChild = child;
                child = child->childAt(pos.x(), pos.y());
            }
            if (lastChild)
                return jint(QAccessible::uniqueId(lastChild));
        }
        return jint(-1);
    });
}

// Qt 5.15.0 answers false after a press too
static jboolean clickAction(JNIEnv *, jclass, jint objectId) {
    const auto press = [objectId] {
        QAccessibleInterface *iface = interfaceFromId(objectId);
        if (iface && iface->isValid() && iface->actionInterface()) {
            if (iface->actionInterface()->actionNames().contains(QAccessibleActionInterface::pressAction()))
                iface->actionInterface()->doAction(QAccessibleActionInterface::pressAction());
            else
                iface->actionInterface()->doAction(QAccessibleActionInterface::toggleAction());
        }
    };
    QCoreApplication *app = QCoreApplication::instance();
    if (app)
        QMetaObject::invokeMethod(app, press, Qt::QueuedConnection);
    return false;
}

static jboolean scroll(jint objectId, const QString &actionName) {
    const bool scrollable = runOnQtThread(false, [objectId, actionName] {
        QAccessibleInterface *iface = interfaceFromId(objectId);
        return iface && iface->isValid() && effectiveActionNames(iface).contains(actionName);
    });
    if (scrollable)
        performLater(objectId, actionName);
    return scrollable;
}

static jboolean scrollForward(JNIEnv *, jclass, jint objectId) {
    return scroll(objectId, QAccessibleActionInterface::increaseAction());
}

static jboolean scrollBackward(JNIEnv *, jclass, jint objectId) {
    return scroll(objectId, QAccessibleActionInterface::decreaseAction());
}

static jboolean populateNode(JNIEnv *env, jclass, jint objectId, jobject node) {
    const NodeData data = runOnQtThread(NodeData(), [objectId] { return nodeData(objectId); });
    if (!data.valid)
        return false;
    if (data.hasSelection)
        env->CallVoidMethod(node, setTextSelectionMethod, data.selectionStart, data.selectionEnd);
    env->CallVoidMethod(node, setEnabledMethod, jboolean(data.enabled));
    env->CallVoidMethod(node, setCheckableMethod, jboolean(data.checkable));
    env->CallVoidMethod(node, setCheckedMethod, jboolean(data.checked));
    env->CallVoidMethod(node, setFocusableMethod, jboolean(data.focusable));
    env->CallVoidMethod(node, setFocusedMethod, jboolean(data.focused));
    env->CallVoidMethod(node, setVisibleToUserMethod, jboolean(data.visible));
    env->CallVoidMethod(node, setScrollableMethod, jboolean(data.increase || data.decrease));
    env->CallVoidMethod(node, setClickableMethod, jboolean(data.clickable));
    if (data.clickable)
        env->CallVoidMethod(node, addActionMethod, jint(16)); // ACTION_CLICK
    if (data.increase)
        env->CallVoidMethod(node, addActionMethod, jint(4096)); // ACTION_SCROLL_FORWARD
    if (data.decrease)
        env->CallVoidMethod(node, addActionMethod, jint(8192)); // ACTION_SCROLL_BACKWARD
    jstring description =
        env->NewString(reinterpret_cast<const jchar *>(data.description.constData()), jsize(data.description.size()));
    env->CallVoidMethod(node, setContentDescriptionMethod, description);
    env->DeleteLocalRef(description);
    return true;
}

// On the Qt thread in main(), once QApplication exists; Qt's own functions answer until then
static void registerOverQt() {
    QAndroidJniEnvironment env;
    // a lookup that fails leaves an exception, and no JNI call may follow it: each is cleared at once
    const auto findClass = [&env](const char *name) {
        jclass found = env->FindClass(name);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            return jclass(nullptr);
        }
        return found;
    };
    const auto findMethod = [&env](jclass clazz, const char *name, const char *signature) {
        jmethodID found = env->GetMethodID(clazz, name, signature);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            return jmethodID(nullptr);
        }
        return found;
    };
    jclass nativeClass =
        QJNIEnvironmentPrivate::findClass("org/qtproject/qt5/android/accessibility/QtNativeAccessibility", env);
    if (env->ExceptionCheck())
        env->ExceptionClear();
    jclass nodeClass = findClass("android/view/accessibility/AccessibilityNodeInfo");
    jclass rect = findClass("android/graphics/Rect");
    if (!nativeClass || !nodeClass || !rect) {
        qWarning() << "accessibility: Qt's accessibility classes not found, its functions stay on the Android UI thread";
        return;
    }
    rectClass = static_cast<jclass>(env->NewGlobalRef(rect));
    rectConstructor = findMethod(rect, "<init>", "(IIII)V");
    addActionMethod = findMethod(nodeClass, "addAction", "(I)V");
    setCheckableMethod = findMethod(nodeClass, "setCheckable", "(Z)V");
    setCheckedMethod = findMethod(nodeClass, "setChecked", "(Z)V");
    setClickableMethod = findMethod(nodeClass, "setClickable", "(Z)V");
    setContentDescriptionMethod = findMethod(nodeClass, "setContentDescription", "(Ljava/lang/CharSequence;)V");
    setEnabledMethod = findMethod(nodeClass, "setEnabled", "(Z)V");
    setFocusableMethod = findMethod(nodeClass, "setFocusable", "(Z)V");
    setFocusedMethod = findMethod(nodeClass, "setFocused", "(Z)V");
    setScrollableMethod = findMethod(nodeClass, "setScrollable", "(Z)V");
    setTextSelectionMethod = findMethod(nodeClass, "setTextSelection", "(II)V");
    setVisibleToUserMethod = findMethod(nodeClass, "setVisibleToUser", "(Z)V");
    env->DeleteLocalRef(nodeClass);
    env->DeleteLocalRef(rect);
    if (!rectConstructor || !addActionMethod || !setCheckableMethod || !setCheckedMethod || !setClickableMethod ||
        !setContentDescriptionMethod || !setEnabledMethod || !setFocusableMethod || !setFocusedMethod ||
        !setScrollableMethod || !setTextSelectionMethod || !setVisibleToUserMethod) {
        qWarning() << "accessibility: a method of AccessibilityNodeInfo not found, Qt's functions stay on the Android UI thread";
        return;
    }
    static const JNINativeMethod methods[] = {
        {"setActive", "(Z)V", reinterpret_cast<void *>(setActive)},
        {"childIdListForAccessibleObject", "(I)[I", reinterpret_cast<void *>(childIdListForAccessibleObject)},
        {"parentId", "(I)I", reinterpret_cast<void *>(parentId)},
        {"descriptionForAccessibleObject", "(I)Ljava/lang/String;",
         reinterpret_cast<void *>(descriptionForAccessibleObject)},
        {"screenRect", "(I)Landroid/graphics/Rect;", reinterpret_cast<void *>(screenRect)},
        {"hitTest", "(FF)I", reinterpret_cast<void *>(hitTest)},
        {"populateNode", "(ILandroid/view/accessibility/AccessibilityNodeInfo;)Z", reinterpret_cast<void *>(populateNode)},
        {"clickAction", "(I)Z", reinterpret_cast<void *>(clickAction)},
        {"scrollForward", "(I)Z", reinterpret_cast<void *>(scrollForward)},
        {"scrollBackward", "(I)Z", reinterpret_cast<void *>(scrollBackward)},
    };
    if (env->RegisterNatives(nativeClass, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        env->ExceptionClear();
        qWarning() << "accessibility: RegisterNatives failed, Qt's functions stay on the Android UI thread";
        return;
    }
    qDebug() << "accessibility: Qt's accessibility functions run on the Qt thread";
}

} // namespace androidAccessibility
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 3)
// Before 5.15.3 Qt sends a screen reader on Android no event at all (its notifyAccessibilityUpdate()
// is a FIXME), so TalkBack keeps the nodes it cached: after a page is pushed it still walks the page
// under it, after a scroll it still has the rows where they were. Two things change what TalkBack
// should see. A page or a popup that shows or hides: Qt reports ObjectShow/ObjectHide for its items
// to the update handler below, which replaces Qt's own (empty) one. A list that stops scrolling: a
// Flickable reports nothing, but emits movementEnded(), so when a finger is lifted each Flickable
// that is moving is connected to it. Both start one zero timer, so the many items a page shows at
// once give CustomQtActivity a single call. None of this runs while accessibility is off.
// The timer is made in main() on the Qt thread; Qt 5.15 runs some accessibility actions on the
// Android UI thread (unless androidAccessibility::registerOverQt() moved them to the Qt thread),
// so a show or hide that comes from there starts it through a queued call.
static QTimer *accessibilityRefreshTimer() {
    static QTimer *timer = nullptr;
    if (!timer) {
        timer = new QTimer(QCoreApplication::instance());
        timer->setSingleShot(true);
        timer->setInterval(0);
        QObject::connect(timer, &QTimer::timeout, [] {
            if (QAccessible::isActive())
                QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/CustomQtActivity",
                                                          "accessibilityTreeChanged", "()V");
        });
    }
    return timer;
}

// Accessible.ignored: a decorative item, e.g. a tile icon that blinks every 500 ms on the home page
static bool accessibleIgnored(QObject *object) {
    const QObjectList children = object->children();
    for (QObject *child : children) {
        if (child->inherits("QQuickAccessibleAttached"))
            return child->property("ignored").toBool();
    }
    return false;
}

static void androidAccessibilityUpdate(QAccessibleEvent *event) {
    if (event->type() != QAccessible::ObjectShow && event->type() != QAccessible::ObjectHide)
        return;
    if (!event->object() || accessibleIgnored(event->object()))
        return;
    QTimer *timer = accessibilityRefreshTimer();
    if (QThread::currentThread() == timer->thread())
        timer->start();
    else
        QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);
}

static void watchMovingFlickables(QQuickItem *item) {
    // a hidden page of a StackView cannot be scrolling
    if (!item->isVisible())
        return;
    if (item->inherits("QQuickFlickable") && item->property("moving").toBool())
        QObject::connect(item, SIGNAL(movementEnded()), accessibilityRefreshTimer(), SLOT(start()),
                         Qt::UniqueConnection);
    const QList<QQuickItem *> children = item->childItems();
    for (QQuickItem *child : children)
        watchMovingFlickables(child);
}

// Sees a finger lifted from the window before its items do, while a dragged Flickable still moves
class ScrollEndWatcher : public QObject {
  public:
    using QObject::QObject;

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if ((event->type() == QEvent::TouchEnd || event->type() == QEvent::MouseButtonRelease) &&
            QAccessible::isActive()) {
            QQuickWindow *window = qobject_cast<QQuickWindow *>(watched);
            if (window && window->contentItem())
                watchMovingFlickables(window->contentItem());
        }
        return false;
    }
};
#endif

// Qt 5.15 turns its accessibility on when Android reports it enabled. The report comes on the
// Android UI thread, and when it comes before Qt published its platform integration (a service
// such as TalkBack already on at launch), Qt logs "Could not activate platform accessibility" and
// stays off. Qt Quick Controls then set no role or name: buttons are missing from TalkBack's tree,
// a group box has no title, a spin box takes no focus (QTBUG-77320, fixed in later Qt the same way:
// keep the report and apply it once the integration exists). Qt publishes the integration from an
// event it posts while QApplication is constructed, so a call queued after that runs once it is
// published: either Qt has got the report by then, or Android already says accessibility is on
// and one setActive() is enough.
static void restoreLostAccessibility() {
    if (QAccessible::isActive())
        return;
    if (QAndroidJniObject::callStaticMethod<jboolean>("org/cagnulen/qdomyoszwift/CustomQtActivity",
                                                      "accessibilityOn", "(Landroid/content/Context;)Z",
                                                      QtAndroid::androidContext().object())) {
        QAndroidJniObject::callStaticMethod<void>("org/qtproject/qt5/android/accessibility/QtNativeAccessibility",
                                                  "setActive", "(Z)V", jboolean(true));
        qDebug() << "accessibility: Qt lost Android's report at launch, active after setActive:" << QAccessible::isActive();
    }
}
#endif

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

    OAuthCallbackEventFilter oauthCallbackEventFilter;
    app->installEventFilter(&oauthCallbackEventFilter);
#ifdef CHARTJS
    QtWebView::initialize();
#endif

#ifdef Q_OS_LINUX
#ifndef Q_OS_ANDROID
    if (getuid() && !testPeloton && !testHomeFitnessBudy && !testPowerZonePack && !smokeTest) {

        printf("Runme as root!\n");
        return -1;
    } else
        printf("%s", "OK, you are root.\n");
#endif
#endif

    if (smokeTest) {
        printf("SMOKE_OK\n");
        return 0;
    }

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
    // Foreground/background switches: Android may throttle or kill QZ in the background, and without this the
    // log can't tell whether a lost connection to Zwift happened while QZ was on screen
    if (QGuiApplication *guiApp = qobject_cast<QGuiApplication *>(app.data())) {
        QObject::connect(guiApp, &QGuiApplication::applicationStateChanged,
                         [](Qt::ApplicationState state) { qDebug() << "applicationStateChanged" << state; });
        // the signal only reports changes: log the state QZ starts in (background when Android restarts it)
        qDebug() << "applicationState at start" << guiApp->applicationState();
    }

    // myMessageOutput() drops every message when the log is off (same condition as there), so don't
    // build the messages nobody reads: the dump of all the settings and the Qt Bluetooth debug output,
    // which Qt formats for every packet once its category is enabled
    const bool logdebug = settings.value(QZSettings::log_debug, QZSettings::default_log_debug).toBool();
#if defined(Q_OS_LINUX) // Linux OS does not read settings file for now
    const bool logEnabled = forceQml ? logdebug : logs;
#else
    const bool logEnabled = logdebug;
#endif
    if (logEnabled) {
        QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
        foreach (QString s, settings.allKeys()) {
            if (!QZSettings::isSensitiveSettingKey(s)) {
                qDebug() << s << settings.value(s);
            }
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

    // Android 11 and earlier require Location permission for Bluetooth scanning.
    // The patched Qt backend uses Nearby devices alone on Android 12+ when
    // BLUETOOTH_SCAN is declared with neverForLocation.
    const int androidSdkVersion =
        QAndroidJniObject::getStaticField<jint>("android/os/Build$VERSION", "SDK_INT");
    if (androidSdkVersion <= 30) {
        const QString locationPermission = QStringLiteral("android.permission.ACCESS_FINE_LOCATION");
        if (QtAndroid::checkPermission(locationPermission) == QtAndroid::PermissionResult::Denied) {
            const QtAndroid::PermissionResultMap locationResult =
                QtAndroid::requestPermissionsSync(QStringList({locationPermission}));
            if (locationResult.value(locationPermission) == QtAndroid::PermissionResult::Denied)
                qWarning() << "Bluetooth scanning may be blocked by missing legacy Location permission:"
                           << locationPermission;
        }
    }

    // BLUETOOTH and BLUETOOTH_ADMIN are legacy, install-time-only permissions.
    // They are not available on API 31+ once maxSdkVersion=30 is set.
    if (androidSdkVersion <= 30) {
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

    // API 37 blocks direct LAN sockets (including TCP, UDP and mDNS) unless the
    // Nearby devices / Local network permission is granted. Request it before
    // starting the device managers, which may publish or discover LAN services.
    // Earlier Android versions have implicit LAN access through INTERNET.
    if (QAndroidJniObject::getStaticField<jint>("android/os/Build$VERSION", "SDK_INT") >= 37) {
        const QString localNetworkPermission = QStringLiteral("android.permission.ACCESS_LOCAL_NETWORK");
        if (QtAndroid::checkPermission(localNetworkPermission) == QtAndroid::PermissionResult::Denied) {
            const QtAndroid::PermissionResultMap localResult =
                QtAndroid::requestPermissionsSync(QStringList({localNetworkPermission}));
            if (localResult.value(localNetworkPermission) == QtAndroid::PermissionResult::Denied)
                qWarning() << "ACCESS_LOCAL_NETWORK denied: LAN-based equipment, DIRCON, and"
                           << "network discovery may not work. Grant Nearby devices in Android settings.";
        }
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
        new MQTTPublisher(mqtt_host, mqtt_port, mqtt_username, mqtt_password, &bl, &bl);
    }

    QString OSC_ip = settings.value(QZSettings::OSC_ip, QZSettings::default_OSC_ip).toString();
    if(OSC_ip.length() > 0) {
        OSC* osc = new OSC(&bl);
    }

    // MyWhoosh Link integration
    bool mywhoosh_link_enabled = settings.value(QZSettings::mywhoosh_link_enabled, QZSettings::default_mywhoosh_link_enabled).toBool();
    if(mywhoosh_link_enabled) {
        MyWhooshLink* mywhooshLink = new MyWhooshLink(&bl);
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

        // Standard dialog buttons (MessageDialog Yes/No/Abort...) are labelled by the Qt platform
        // theme, and Qt's own translations are not shipped with the app. Listing the theme's
        // strings here lets lupdate put them into our .ts files, so the app translator covers them.
        static const char *const qtStandardButtonTexts[] = {
            QT_TRANSLATE_NOOP("QAndroidPlatformTheme", "Yes"),
            QT_TRANSLATE_NOOP("QAndroidPlatformTheme", "Yes to All"),
            QT_TRANSLATE_NOOP("QAndroidPlatformTheme", "No"),
            QT_TRANSLATE_NOOP("QAndroidPlatformTheme", "No to All"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "OK"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Save"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Save All"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Open"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "&Yes"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Yes to &All"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "&No"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "N&o to All"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Abort"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Retry"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Ignore"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Close"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Cancel"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Discard"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Help"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Apply"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Reset"),
            QT_TRANSLATE_NOOP("QPlatformTheme", "Restore Defaults"),
        };
        Q_UNUSED(qtStandardButtonTexts);

        // Load translations based on explicit app setting or system locale.
        homeform::loadAppTranslation(
            settings.value(QZSettings::app_language, QZSettings::default_app_language).toString());

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

#if defined(Q_OS_ANDROID) && QT_VERSION < QT_VERSION_CHECK(5, 15, 9)
        androidAccessibility::registerOverQt();
#endif
        engine.load(url);
#ifdef Q_OS_ANDROID
        QTimer::singleShot(0, [] { restoreLostAccessibility(); });
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 3)
        accessibilityRefreshTimer();
        QAccessible::installUpdateHandler(androidAccessibilityUpdate);
        if (!engine.rootObjects().isEmpty()) {
            if (QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst()))
                window->installEventFilter(new ScrollEndWatcher(window));
        }
#endif
#endif
        homeform *h = new homeform(&engine, &bl);
        QObject::connect(app.data(), &QCoreApplication::aboutToQuit, h,
                         &homeform::aboutToQuit); // NOTE: clazy-unneeded-cast
#ifdef Q_OS_ANDROID
        // Qt 5.15.0 calls exit() twice when QZ closes: from its thread after main() returns and from
        // System.exit() in QtActivityDelegate.onDestroy(). Their static destructors abort the process:
        // two at once in the QML type data ("Cannot remove multiple registrations for QtQuick 2"), and
        // any of them while the hwui threads still use their mutex (QTBUG-82617). An atexit() handler
        // registered last runs first: the first exit() ends the process without them, the locals of
        // main() are destroyed by then. Registered at aboutToQuit, not at startup, to come after the
        // static objects of the QML plugins loaded during the run. QT_ANDROID_NO_EXIT_CALL (Qt 5.15.4)
        // skips only the exit() of the Qt thread.
        QObject::connect(app.data(), &QCoreApplication::aboutToQuit, [] { atexit([] { _exit(0); }); });
#endif

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
