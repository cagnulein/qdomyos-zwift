#include "androidadblog.h"
#include <QDateTime>
#include <QTimer>

androidadblog::androidadblog() { }

void androidadblog::run() {
    while (1) {
        runAdbTailCommand("logcat");
    }
}

void androidadblog::runAdbTailCommand(QString command) {
#ifdef Q_OS_ANDROID
    found = false;
    auto process = new QProcess;
    QObject::connect(process, &QProcess::readyReadStandardOutput, [process, this]() {
        QString output = process->readAllStandardOutput();
        QStringList olist = output.split('\n', Qt::SplitBehaviorFlags::SkipEmptyParts);        
        foreach(QString o, olist) {
            if(!o.contains("qDomyos-Zwift") && !o.contains("libqdomyos-zwift")) {
                qDebug() << "adbLogCat STDOUT << " << o;
                if(o.contains("V1Callback")) {
                    found = true;
                    QStringList ooList = o.split(' ', Qt::SplitBehaviorFlags::SkipEmptyParts);
                    foreach(QString oo, ooList) {
                        qDebug() << oo;
                    }
                }
            }
        }        
    });
    QObject::connect(process, &QProcess::readyReadStandardError, [process, this]() {
        auto output = process->readAllStandardError();
        qDebug() << "adbLogCat ERROR << " << output;
    });

    QTimer::singleShot(60000,[process, this] () {
        qDebug() << "single shot timer elapsed";
        if(!found) {
            qDebug() << "restarting adb logcat";
            process->kill();
        }
     });

    QStringList arguments;
    //arguments.append("*:e");
    process->start("logcat", arguments);
    process->waitForFinished(-1);
#endif
}
