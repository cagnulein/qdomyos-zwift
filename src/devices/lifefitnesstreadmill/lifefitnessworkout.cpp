#include "lifefitnessworkout.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamWriter>

#include <cmath>

namespace lifefitnessworkout {
namespace {

void setError(QString *error, const QString &message) {
    if (error)
        *error = message;
}

int durationSeconds(const trainrow &row) {
    return row.duration.hour() * 3600 + row.duration.minute() * 60 + row.duration.second();
}

QByteArray wrappedBase64(const QByteArray &input) {
    const QByteArray encoded = input.toBase64();
    QByteArray wrapped;
    for (int offset = 0; offset < encoded.size(); offset += 76)
        wrapped += encoded.mid(offset, 76) + QByteArrayLiteral("\n");
    return wrapped;
}

QString number(double value) {
    return QString::number(value, 'f', 1);
}

} // namespace

QList<QByteArray> frameBody(const QByteArray &body, QString *error) {
    constexpr int chunkSize = 15;
    constexpr int maxChunks = 255;
    QList<QByteArray> frames;

    if (body.isEmpty()) {
        setError(error, QStringLiteral("Life Fitness workout body is empty"));
        return frames;
    }

    const int chunkCount = (body.size() + chunkSize - 1) / chunkSize;
    if (chunkCount > maxChunks) {
        setError(error, QStringLiteral("Life Fitness workout body exceeds 255 transport chunks"));
        return frames;
    }

    frames.reserve(chunkCount);
    for (int index = 0; index < chunkCount; ++index) {
        const QByteArray chunk = body.mid(index * chunkSize, chunkSize);
        quint8 checksum = 0;
        for (const char byte : chunk)
            checksum = static_cast<quint8>(checksum + static_cast<quint8>(byte));

        QByteArray frame;
        frame.reserve(5 + chunk.size());
        frame.append(char(0));
        frame.append(static_cast<char>(index));
        frame.append(char(0));
        frame.append(static_cast<char>(chunkCount));
        frame.append(static_cast<char>(checksum));
        frame.append(chunk);
        frames.append(frame);
    }
    return frames;
}

bool parseSelectionEvent(const QByteArray &event, int *index) {
    if (!index || event.size() < 3 || static_cast<quint8>(event.at(0)) != 0x15)
        return false;
    *index = static_cast<quint8>(event.at(2));
    return true;
}

bool buildProgram(const QList<trainrow> &rows, const QString &name, const Profile &profile, Program &program,
                  QString *error, const QDateTime &timestamp) {
    program = Program{};
    if (rows.isEmpty()) {
        setError(error, QStringLiteral("Life Fitness workout has no intervals"));
        return false;
    }
    if (rows.size() > 100) {
        setError(error, QStringLiteral("Life Fitness workout supports at most 100 intervals"));
        return false;
    }

    int totalSeconds = 0;
    double initialSpeed = -1.0;
    QStringList speedTargets;
    QStringList inclineTargets;
    QStringList intervalTimes;
    speedTargets.reserve(rows.size());
    inclineTargets.reserve(rows.size());
    intervalTimes.reserve(rows.size());

    for (int i = 0; i < rows.size(); ++i) {
        const trainrow &row = rows.at(i);
        const int seconds = durationSeconds(row);
        if (seconds <= 0) {
            setError(error, QStringLiteral("Life Fitness interval %1 has no positive duration").arg(i));
            return false;
        }
        totalSeconds += seconds;

        if (row.speed != -1.0 && (!std::isfinite(row.speed) || row.speed < 0.0 || row.speed > 22.5)) {
            setError(error, QStringLiteral("Life Fitness interval %1 has an unsupported speed").arg(i));
            return false;
        }
        if (row.speed > 0.0 && initialSpeed < 0.0)
            initialSpeed = row.speed;
        speedTargets.append(QString::number(row.speed < 0.0 ? 0.0 : row.speed * 10.0, 'f', 0));

        const bool missingIncline = row.inclination == -200.0;
        if (!missingIncline && (!std::isfinite(row.inclination) || row.inclination < 0.0 || row.inclination > 15.0)) {
            setError(error, QStringLiteral("Life Fitness interval %1 has an unsupported incline").arg(i));
            return false;
        }
        const double incline = missingIncline ? 0.0 : row.inclination;
        inclineTargets.append(QString::number(incline * 10.0, 'f', 0));
        intervalTimes.append(QString::number(seconds));
    }

    if (initialSpeed <= 0.0) {
        setError(error, QStringLiteral("Life Fitness workout needs a positive initial speed"));
        return false;
    }

    const QString workoutName = name.trimmed().isEmpty() ? QStringLiteral("QZ Workout") : name.trimmed();
    const QDateTime created = timestamp.isValid() ? timestamp.toUTC() : QDateTime::currentDateTimeUtc();
    const int cyoMinutes = (totalSeconds + 59) / 60;

    QByteArray xmlBytes;
    QXmlStreamWriter xml(&xmlBytes);
    xml.setAutoFormatting(false);
    xml.writeStartDocument(QStringLiteral("1.0"), true);
    xml.writeStartElement(QStringLiteral("workout"));
    xml.writeTextElement(QStringLiteral("version"), QStringLiteral("2.0"));
    xml.writeTextElement(QStringLiteral("create-date"), created.toString(Qt::ISODate));
    xml.writeTextElement(QStringLiteral("modify-date"), created.toString(Qt::ISODate));

    xml.writeStartElement(QStringLiteral("device-info"));
    xml.writeTextElement(QStringLiteral("device-type"), QStringLiteral("1"));
    xml.writeEndElement();

    xml.writeStartElement(QStringLiteral("user-profile"));
    xml.writeTextElement(QStringLiteral("user-id"), profile.userId.isEmpty() ? QStringLiteral("qz-user@lfconnect.local")
                                                                               : profile.userId);
    xml.writeTextElement(QStringLiteral("gender"), QString::number(qBound(0, profile.gender, 1)));
    xml.writeTextElement(QStringLiteral("age"), QString::number(qBound(0, profile.age, 255)));
    xml.writeStartElement(QStringLiteral("weight"));
    xml.writeAttribute(QStringLiteral("units"), QStringLiteral("1"));
    xml.writeCharacters(number(profile.weightKg));
    xml.writeEndElement();
    xml.writeStartElement(QStringLiteral("height"));
    xml.writeAttribute(QStringLiteral("units"), QStringLiteral("1"));
    xml.writeCharacters(number(profile.heightCm));
    xml.writeEndElement();
    xml.writeTextElement(QStringLiteral("units"), QStringLiteral("1"));
    xml.writeEndElement();

    xml.writeStartElement(QStringLiteral("workout-preset"));
    xml.writeTextElement(QStringLiteral("program-type"), QStringLiteral("57"));
    xml.writeTextElement(QStringLiteral("goal-type"), QStringLiteral("1"));
    xml.writeTextElement(QStringLiteral("goal-value"), number(totalSeconds));
    xml.writeTextElement(QStringLiteral("goal-unit"), QStringLiteral("1"));
    xml.writeStartElement(QStringLiteral("speed"));
    xml.writeAttribute(QStringLiteral("units"), QStringLiteral("1"));
    xml.writeCharacters(number(initialSpeed));
    xml.writeEndElement();
    xml.writeTextElement(QStringLiteral("cyo-segment-type"), QStringLiteral("1"));
    xml.writeTextElement(QStringLiteral("cyo-time"), QString::number(cyoMinutes));
    xml.writeTextElement(QStringLiteral("cyo-is-metric"), QStringLiteral("1"));
    xml.writeTextElement(QStringLiteral("cyo-segment-data1"), speedTargets.join(','));
    xml.writeTextElement(QStringLiteral("cyo-segment-data2"), inclineTargets.join(','));
    xml.writeTextElement(QStringLiteral("cyo-segment-time"), intervalTimes.join(','));
    const double initialIncline = rows.first().inclination == -200.0 ? 0.0 : rows.first().inclination;
    xml.writeTextElement(QStringLiteral("incline"), number(initialIncline));
    xml.writeEndElement();

    xml.writeStartElement(QStringLiteral("vivo-block"));
    xml.writeTextElement(QStringLiteral("user-id"), QStringLiteral("0"));
    xml.writeTextElement(QStringLiteral("assigned-prog-id"), QStringLiteral("0"));
    xml.writeTextElement(QStringLiteral("activity-id"), QStringLiteral("0"));
    xml.writeTextElement(QStringLiteral("activity-def-id"), QStringLiteral("0"));
    xml.writeTextElement(QStringLiteral("workout-id"), QStringLiteral("0"));
    xml.writeTextElement(QStringLiteral("workout-sess-id"), QStringLiteral("0"));
    xml.writeTextElement(QStringLiteral("location-id"), QStringLiteral("0"));
    xml.writeTextElement(QStringLiteral("source"), QStringLiteral("0"));
    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndDocument();

    QJsonObject summaryObject;
    summaryObject.insert(QStringLiteral("workoutId"), 900001);
    summaryObject.insert(QStringLiteral("workoutName"), workoutName);
    summaryObject.insert(QStringLiteral("fileName"), workoutName);
    summaryObject.insert(QStringLiteral("index"), 0);
    summaryObject.insert(QStringLiteral("workoutType"), QStringLiteral("WORKOUT_LIBRARY"));
    QJsonArray summaryArray;
    summaryArray.append(summaryObject);

    program.summary = QJsonDocument(summaryArray).toJson(QJsonDocument::Compact);
    program.xml = xmlBytes;
    program.summaryFrames = frameBody(program.summary, error);
    if (program.summaryFrames.isEmpty())
        return false;
    program.detailFrames = frameBody(wrappedBase64(program.xml), error);
    if (program.detailFrames.isEmpty()) {
        program = Program{};
        return false;
    }
    return true;
}

} // namespace lifefitnessworkout
