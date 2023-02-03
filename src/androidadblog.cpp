#include "androidadblog.h"

androidadblog::androidadblog() { }

void androidadblog::run() {
    while (1) {
        runAdbTailCommand("logcat");
    }
}

void androidadblog::runAdbTailCommand(QString command) {
    auto process = new QProcess;
    QObject::connect(process, &QProcess::readyReadStandardOutput, [process, this]() {
        QString output = process->readAllStandardOutput();
        qDebug() << "adbLogCat STDOUT << " << output;
    });
    QObject::connect(process, &QProcess::readyReadStandardError, [process, this]() {
        auto output = process->readAllStandardError();
        qDebug() << "adbLogCat ERROR << " << output;
    });
    QStringList arguments;
    arguments.append("*:e");
    process->start("logcat", arguments);
    process->waitForFinished(-1);
}
