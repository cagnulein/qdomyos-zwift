#include "logwriter.h"

LogWriter::LogWriter(QObject *parent) : QObject(parent) {
}

LogWriter::~LogWriter() {
}

qint64 LogWriter::maxFileSize = 0;

void LogWriter::writeLog(const QString &path, const QString &txt) {
    QFile outFile(path);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    if (maxFileSize <= 0 || outFile.size() < maxFileSize) {
        QTextStream ts(&outFile);
        ts << txt;
    }
    fprintf(stderr, "%s", txt.toLocal8Bit().constData());
}
