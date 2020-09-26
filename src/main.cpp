#include <QtCore/qcoreapplication.h>
#include "virtualtreadmill.h"
#include "domyostreadmill.h"

int main(int argc, char *argv[])
{
    //QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
#ifndef Q_OS_ANDROID
    QCoreApplication app(argc, argv);
#else
    QGuiApplication app(argc, argv);
#endif

    //virtualtreadmill* V = new virtualtreadmill();
    domyostreadmill* D = new domyostreadmill();

    //Q_UNUSED(V);
    Q_UNUSED(D);

    return app.exec();
}
