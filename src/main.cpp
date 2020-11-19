#include <QApplication>
#include <QStyleFactory>
#include <stdio.h>
#include <stdlib.h>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "virtualtreadmill.h"
#include "domyostreadmill.h"
#include "bluetooth.h"
#include "mainwindow.h"

bool nologs = false;
bool noWriteResistance = false;
bool noHeartService = true;
bool noConsole = false;
bool onlyVirtualBike = false;
bool onlyVirtualTreadmill = false;
bool testResistance = false;
QString trainProgram;
QString deviceName = "";
uint32_t pollDeviceTime = 200;
static QString logfilename = "debug-" + QDateTime::currentDateTime().toString().replace(":", "_") + ".log";

QCoreApplication* createApplication(int &argc, char *argv[])
{
    bool nogui = false;

    for (int i = 1; i < argc; ++i) {
        if (!qstrcmp(argv[i], "-no-gui"))
            nogui = true;        
        if (!qstrcmp(argv[i], "-no-console"))
            noConsole = true;
        if (!qstrcmp(argv[i], "-test-resistance"))
            testResistance = true;
        if (!qstrcmp(argv[i], "-no-log"))
            nologs = true;
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
        if (!qstrcmp(argv[i], "-train"))
        {
            trainProgram = argv[++i];
        }
        if (!qstrcmp(argv[i], "-name"))
        {
            deviceName = argv[++i];
        }
        if (!qstrcmp(argv[i], "-poll-device-time"))
        {
            pollDeviceTime = atol(argv[++i]);
        }
    }

    if(nogui)
        return new QCoreApplication(argc, argv);
    else        
    {
        QApplication* a = new QApplication(argc, argv);

        a->setStyle(QStyleFactory::create("Fusion"));

        /*QFont defaultFont = QApplication::font();
        defaultFont.setPointSize(defaultFont.pointSize()+2);
        qApp->setFont(defaultFont);*/

        // modify palette to dark
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window,QColor(53,53,53));
        darkPalette.setColor(QPalette::WindowText,Qt::white);
        darkPalette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
        darkPalette.setColor(QPalette::Base,QColor(42,42,42));
        darkPalette.setColor(QPalette::AlternateBase,QColor(66,66,66));
        darkPalette.setColor(QPalette::ToolTipBase,Qt::white);
        darkPalette.setColor(QPalette::ToolTipText,Qt::white);
        darkPalette.setColor(QPalette::Text,Qt::white);
        darkPalette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
        darkPalette.setColor(QPalette::Dark,QColor(35,35,35));
        darkPalette.setColor(QPalette::Shadow,QColor(20,20,20));
        darkPalette.setColor(QPalette::Button,QColor(53,53,53));
        darkPalette.setColor(QPalette::ButtonText,Qt::white);
        darkPalette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
        darkPalette.setColor(QPalette::BrightText,Qt::red);
        darkPalette.setColor(QPalette::Link,QColor(42,130,218));
        darkPalette.setColor(QPalette::Highlight,QColor(42,130,218));
        darkPalette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
        darkPalette.setColor(QPalette::HighlightedText,Qt::white);
        darkPalette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));

        qApp->setPalette(darkPalette);

        return a;
    }
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    QString txt;
    switch (type) {
    case QtInfoMsg:
        txt = QString("Info: %1 %2 %3\n").arg(file).arg(function).arg(msg);
        break;
    case QtDebugMsg:
        txt = QString("Debug: %1 %2 %3\n").arg(file).arg(function).arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1 %2 %3\n").arg(file).arg(function).arg(msg);
    break;
    case QtCriticalMsg:
        txt = QString("Critical: %1 %2 %3\n").arg(file).arg(function).arg(msg);
    break;
    case QtFatalMsg:
        txt = QString("Fatal: %1 %2 %3\n").arg(file).arg(function).arg(msg);
        abort();
    }

    if(nologs == false)
    {
        QString path = "";
#ifdef Q_OS_ANDROID
        path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/";
#endif

        QFile outFile(path + logfilename);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt;

        fprintf(stderr, txt.toLocal8Bit());
    }
}

int main(int argc, char *argv[])
{
#if 0
#ifndef Q_OS_ANDROID
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));
    qInstallMessageHandler(myMessageOutput);

    if(onlyVirtualBike)
    {
        virtualbike* V = new virtualbike(new bike(), noWriteResistance, noHeartService);
        return app->exec();
    }
    else if(onlyVirtualTreadmill)
    {
        virtualtreadmill* V = new virtualtreadmill(new treadmill(), noHeartService);
        return app->exec();
    }
#endif
#endif
    bluetooth* bl = new bluetooth(!nologs, deviceName, noWriteResistance, noHeartService, pollDeviceTime, testResistance);

#if 1//def Q_OS_ANDROID
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
#else
    if (qobject_cast<QApplication *>(app.data())) {
        // start GUI version...
        MainWindow* W = 0;
        if(trainProgram.isEmpty())
            W = new MainWindow(bl);
        else
            W = new MainWindow(bl, trainProgram);
        W->show();
    } else {
        // start non-GUI version...
    }
    return app->exec();
#endif
}
