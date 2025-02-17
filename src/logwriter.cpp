#include "logwriter.h"

LogWriter::LogWriter(QObject *parent) : QObject(parent) {
}

LogWriter::~LogWriter() {
}

void LogWriter::writeLog(const QString &path, const QString &txt) {
    QFile outFile(path);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt;
    fprintf(stderr, "%s", txt.toLocal8Bit().constData());
}
