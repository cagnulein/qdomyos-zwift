#include "androidadblog.h"

androidadblog::androidadblog() { }

void androidadblog::run() {
    while (1) {
        runAdbTailCommand("logcat");
    }
}

void androidadblog::runAdbTailCommand(QString command) {
#ifdef Q_OS_ANDROID
    auto process = new QProcess;
    QObject::connect(process, &QProcess::readyReadStandardOutput, [process, this]() {
        QString output = process->readAllStandardOutput();
        QStringList olist = output.split('\n');
        foreach(QString o, olist) {
            if(!o.contains("qDomyos-Zwift")) {
                qDebug() << "adbLogCat STDOUT << " << o;
                if(o.contains("V1Callback")) {
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
    QStringList arguments;
    //arguments.append("*:e");
    process->start("logcat", arguments);
    process->waitForFinished(-1);
#endif
}
