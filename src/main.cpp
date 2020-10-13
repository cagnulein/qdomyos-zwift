#include <QApplication>
#include "virtualtreadmill.h"
#include "domyostreadmill.h"
#include "bluetooth.h"
#include "mainwindow.h"

bool nologs = false;

QCoreApplication* createApplication(int &argc, char *argv[])
{
    bool nogui = false;

    for (int i = 1; i < argc; ++i) {
        if (!qstrcmp(argv[i], "-no-gui"))
            nogui = true;
        if (!qstrcmp(argv[i], "-no-log"))
            nologs = true;
    }

    if(nogui)
        return new QCoreApplication(argc, argv);
    else
        return new QApplication(argc, argv);
}

int main(int argc, char *argv[])
{
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));

    //virtualtreadmill* V = new virtualtreadmill();
    bluetooth* bl = new bluetooth(!nologs);

    if (qobject_cast<QApplication *>(app.data())) {
        // start GUI version...
        MainWindow* W = new MainWindow(bl->treadMill());
        W->show();
    } else {
        // start non-GUI version...
    }

    return app->exec();
}
