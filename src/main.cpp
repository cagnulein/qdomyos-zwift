#include <QApplication>
#include "virtualtreadmill.h"
#include "domyostreadmill.h"
#include "mainwindow.h"

QCoreApplication* createApplication(int &argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (!qstrcmp(argv[i], "-no-gui"))
            return new QCoreApplication(argc, argv);
    }
    return new QApplication(argc, argv);
}

int main(int argc, char *argv[])
{
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));

    //virtualtreadmill* V = new virtualtreadmill();
    domyostreadmill* D = new domyostreadmill();

    if (qobject_cast<QApplication *>(app.data())) {
        // start GUI version...
        MainWindow* W = new MainWindow(D);
        W->show();
    } else {
        // start non-GUI version...
    }

    //Q_UNUSED(V);
    Q_UNUSED(D);

    return app->exec();
}
