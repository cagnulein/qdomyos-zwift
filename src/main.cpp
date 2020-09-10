#include <QtCore/qcoreapplication.h>
#include "virtualtreadmill.h"

int main(int argc, char *argv[])
{
    //QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
#ifndef Q_OS_ANDROID
    QCoreApplication app(argc, argv);
#else
    QGuiApplication app(argc, argv);
#endif

    virtualtreadmill* V = new virtualtreadmill();

    return app.exec();
}
