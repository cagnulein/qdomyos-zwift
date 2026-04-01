// logwriter.h
#ifndef LOGWRITER_H
#define LOGWRITER_H

#include <QObject>
#include <QFile>
#include <QTextStream>

class LogWriter : public QObject {
    Q_OBJECT
public:
    explicit LogWriter(QObject *parent = nullptr);
    virtual ~LogWriter();

public slots:
    void writeLog(const QString &path, const QString &txt);
};

#endif // LOGWRITER_H
