#ifndef LIFEFITNESSWORKOUT_H
#define LIFEFITNESSWORKOUT_H

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QString>

#include "trainprogram.h"

namespace lifefitnessworkout {

struct Profile {
    QString userId = QStringLiteral("qz-user@lfconnect.local");
    int gender = 0;
    int age = 60;
    double weightKg = 75.0;
    double heightCm = 170.0;
};

struct Program {
    QByteArray summary;
    QByteArray xml;
    QList<QByteArray> summaryFrames;
    QList<QByteArray> detailFrames;
};

QList<QByteArray> frameBody(const QByteArray &body, QString *error = nullptr);
bool parseSelectionEvent(const QByteArray &event, int *index);
bool buildProgram(const QList<trainrow> &rows, const QString &name, const Profile &profile, Program &program,
                  QString *error = nullptr, const QDateTime &timestamp = QDateTime());

} // namespace lifefitnessworkout

#endif // LIFEFITNESSWORKOUT_H
