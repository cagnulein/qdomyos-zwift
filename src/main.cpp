#include <QApplication>
#include <QStyleFactory>
#include "virtualtreadmill.h"
#include "domyostreadmill.h"
#include "bluetooth.h"
#include "mainwindow.h"

bool nologs = false;
bool noWriteResistance = false;
bool noHeartService = false;
bool noConsole = false;
bool onlyVirtualBike = false;
bool onlyVirtualTreadmill = false;
QString trainProgram;
QString deviceName = "";
uint32_t pollDeviceTime = 200;

QCoreApplication* createApplication(int &argc, char *argv[])
{
    bool nogui = false;

    for (int i = 1; i < argc; ++i) {
        if (!qstrcmp(argv[i], "-no-gui"))
            nogui = true;        
        if (!qstrcmp(argv[i], "-no-console"))
            noConsole = true;
        if (!qstrcmp(argv[i], "-no-log"))
            nologs = true;
        if (!qstrcmp(argv[i], "-no-write-resistance"))
            noWriteResistance = true;
        if (!qstrcmp(argv[i], "-no-heart-service"))
            noHeartService = true;        
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

int main(int argc, char *argv[])
{
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));

    if(onlyVirtualBike)
    {
        virtualbike* V = new virtualbike(new bike());
        return app->exec();
    }
    else if(onlyVirtualTreadmill)
    {
        virtualtreadmill* V = new virtualtreadmill(new treadmill());
        return app->exec();
    }

    bluetooth* bl = new bluetooth(!nologs, deviceName, noWriteResistance, noHeartService, pollDeviceTime);

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
}
