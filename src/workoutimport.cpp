#include "workoutimport.h"
#include "qzsettings.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QRegularExpression>
#include <QSettings>
#include <limits>

// EXR web profile exports use camelCase keys ("data", "schedule", "length"), the workouts installed with the game
// (.xsr) use PascalCase ("TrainingData", "Schedule", "Length")
static QJsonValue exrField(const QJsonObject &object, const QString &name) {
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (it.key().compare(name, Qt::CaseInsensitive) == 0)
            return it.value();
    }
    return QJsonValue();
}

static QJsonObject exrData(const QByteArray &input) {
    QJsonDocument doc = QJsonDocument::fromJson(input);
    if (!doc.isObject()) {
        // some installed workouts have a trailing comma before ] or }
        QString text = QString::fromUtf8(input);
        text.replace(QRegularExpression(QStringLiteral(",(\\s*[\\]}])")), QStringLiteral("\\1"));
        doc = QJsonDocument::fromJson(text.toUtf8());
        if (!doc.isObject())
            return QJsonObject();
    }
    const QJsonObject root = doc.object();
    const QJsonValue data = exrField(root, QStringLiteral("data"));
    return data.isObject() ? data.toObject() : exrField(root, QStringLiteral("TrainingData")).toObject();
}

static QTime secondsToTime(int seconds) { return QTime(0, 0, 0, 0).addSecs(seconds); }

QString workoutimport::fileKind(const QString &filename, const QString &extension) {
    static const QStringList kinds = {QStringLiteral("MRC"), QStringLiteral("ERG"), QStringLiteral("JSON"),
                                      QStringLiteral("XSR")};
    QString ext = QFileInfo(filename).suffix().toUpper();
    if (kinds.contains(ext))
        return ext;
    ext = extension.toUpper();
    // homeform passes the last three characters of the file name
    if (ext == QStringLiteral("SON"))
        ext = QStringLiteral("JSON");
    return kinds.contains(ext) ? ext : QString();
}

bool workoutimport::isSupportedFile(const QString &filename, const QString &extension) {
    const QString kind = fileKind(filename, extension);
    if (kind.isEmpty())
        return false;
    if (kind == QStringLiteral("MRC") || kind == QStringLiteral("ERG"))
        return true;

    QFile input(filename);
    if (!input.open(QIODevice::ReadOnly))
        return false;
    return !exrField(exrData(input.readAll()), QStringLiteral("schedule")).toArray().isEmpty();
}

QList<trainrow> workoutimport::load(const QString &filename, const QString &extension, QString *description) {
    const QString kind = fileKind(filename, extension);
    QFile input(filename);
    if (kind.isEmpty() || !input.open(QIODevice::ReadOnly)) {
        qDebug() << QStringLiteral("workoutimport: can't open") << filename << kind;
        return QList<trainrow>();
    }

    QSettings settings;
    const double ftp = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
    const QByteArray content = input.readAll();
    const bool courseFile = kind == QStringLiteral("MRC") || kind == QStringLiteral("ERG");
    const QList<trainrow> rows = courseFile ? loadErgMrc(content, kind == QStringLiteral("MRC"), ftp, description)
                                            : loadExr(content, ftp, description);
    qDebug() << QStringLiteral("workoutimport:") << filename << kind << rows.length() << QStringLiteral("rows");
    return rows;
}

QList<trainrow> workoutimport::loadErgMrc(const QByteArray &input, bool percentByDefault, double ftp,
                                          QString *description) {
    enum Section { None, Header, Data, Text };
    Section section = None;
    bool percent = percentByDefault;
    QString fileDescription;
    QList<QPair<double, double>> points;   // minutes, %FTP or watts
    QList<QPair<double, QString>> messages; // seconds from the start, text

    const QStringList lines = QString::fromUtf8(input).split(QRegularExpression(QStringLiteral("[\r\n]+")),
                                                             Qt::SkipEmptyParts);
    for (const QString &rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line.isEmpty())
            continue;
        const QString upper = line.toUpper();
        if (line.startsWith(QLatin1Char('['))) {
            if (upper.contains(QStringLiteral("END")))
                section = None;
            else if (upper.contains(QStringLiteral("COURSE HEADER")))
                section = Header;
            else if (upper.contains(QStringLiteral("COURSE DATA")))
                section = Data;
            else if (upper.contains(QStringLiteral("COURSE TEXT")))
                section = Text;
            else
                section = None;
            continue;
        }

        if (section == Header) {
            const int eq = line.indexOf(QLatin1Char('='));
            if (eq > 0) {
                if (line.left(eq).trimmed().toUpper() == QStringLiteral("DESCRIPTION"))
                    fileDescription = line.mid(eq + 1).trimmed();
            } else if (upper.contains(QStringLiteral("PERCENT"))) {
                percent = true;
            } else if (upper.contains(QStringLiteral("WATTS"))) {
                percent = false;
            }
        } else if (section == Data) {
            const QStringList fields = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
            if (fields.length() < 2)
                continue;
            bool okTime = false;
            bool okValue = false;
            const double minutes = fields.at(0).toDouble(&okTime);
            const double value = fields.at(1).toDouble(&okValue);
            if (okTime && okValue)
                points.append(qMakePair(minutes, value));
        } else if (section == Text) {
            // TrainerRoad: seconds<TAB>message<TAB>duration
            QStringList fields = line.split(QLatin1Char('\t'));
            if (fields.length() < 2)
                fields = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
            bool ok = false;
            const double seconds = fields.at(0).toDouble(&ok);
            if (ok && fields.length() >= 2 && !fields.at(1).trimmed().isEmpty())
                messages.append(qMakePair(seconds, fields.at(1).trimmed()));
        }
    }

    // .erg targets are absolute watts and stay so whatever FTP is set in QZ
    const double scale = percent ? ftp / 100.0 : 1.0;
    QList<trainrow> list;
    QList<int> rowStart; // second of the workout at which each row starts
    for (int i = 0; i + 1 < points.length(); i++) {
        const int start = qRound(points.at(i).first * 60.0);
        const int end = qRound(points.at(i + 1).first * 60.0);
        if (end <= start)
            continue;
        const int duration = end - start;
        const double from = points.at(i).second * scale;
        const double to = points.at(i + 1).second * scale;
        if (qAbs(from - to) < 1e-9) {
            trainrow row;
            row.duration = secondsToTime(duration);
            row.power = qRound(from);
            list.append(row);
            rowStart.append(start);
        } else {
            for (int s = 0; s < duration; s++) {
                trainrow row;
                row.duration = secondsToTime(1);
                row.rampDuration = secondsToTime(duration - s);
                row.rampElapsed = secondsToTime(s);
                row.power = qRound(from + ((to - from) / duration) * s);
                list.append(row);
                rowStart.append(start + s);
            }
        }
    }
    if (list.isEmpty())
        return list;

    for (const auto &message : qAsConst(messages)) {
        const int second = qRound(message.first);
        int index = 0;
        while (index + 1 < rowStart.length() && rowStart.at(index + 1) <= second)
            index++;
        if (second < rowStart.at(index))
            continue;
        const uint32_t offset = second - rowStart.at(index);
        // trainprogram shows one event per row and offset: join messages for the same second
        QList<trainrow::TextEvent> &events = list[index].textEvents;
        if (!events.isEmpty() && events.last().timeoffset == offset) {
            events.last().message += QStringLiteral("\n") + message.second;
            continue;
        }
        trainrow::TextEvent event;
        event.timeoffset = offset;
        event.message = message.second;
        events.append(event);
    }

    if (description != nullptr)
        *description = fileDescription;
    return list;
}

QList<trainrow> workoutimport::loadExr(const QByteArray &input, double ftp, QString *description) {
    const QJsonObject data = exrData(input);
    // unitType 1: length in seconds, 0: length in meters
    const bool byDistance = exrField(data, QStringLiteral("unitType")).toInt(1) == 0;

    // rest pauses, at a position of the workout in the same unit as length (always between two steps in the
    // installed workouts)
    QMap<double, int> rests;
    for (const QJsonValue &value : exrField(data, QStringLiteral("events")).toArray()) {
        const QJsonObject event = value.toObject();
        const QString type = exrField(event, QStringLiteral("_dataTypeString")).toString();
        if (!type.startsWith(QStringLiteral("RowingTrainingRestData")))
            continue;
        const QJsonObject payload =
            QJsonDocument::fromJson(exrField(event, QStringLiteral("DataString")).toString().toUtf8()).object();
        const int seconds = qRound(exrField(payload, QStringLiteral("_restTime")).toDouble());
        if (seconds > 0)
            rests[exrField(event, QStringLiteral("_timeStamp")).toDouble()] += seconds;
    }

    QList<trainrow> list;
    auto rest = rests.constBegin();
    auto appendRestsUpTo = [&](double position) {
        for (; rest != rests.constEnd() && rest.key() <= position; ++rest) {
            trainrow row;
            row.duration = secondsToTime(rest.value());
            list.append(row);
        }
    };

    double position = 0;
    for (const QJsonValue &value : exrField(data, QStringLiteral("schedule")).toArray()) {
        const QJsonObject step = value.toObject();
        const double length = exrField(step, QStringLiteral("length")).toDouble();
        if (length <= 0)
            continue;
        appendRestsUpTo(position);
        const double target = exrField(step, QStringLiteral("FTPTarget")).toDouble(-1);
        const int strokes = qRound(exrField(step, QStringLiteral("strokesPerMin")).toDouble());
        trainrow row;
        if (byDistance)
            row.distance = length / 1000.0;
        else
            row.duration = secondsToTime(qRound(length));
        // FTPTarget -1: free rowing (rest), no target
        if (target > 0)
            row.power = qRound(target * ftp);
        if (strokes > 0)
            row.cadence = strokes;
        list.append(row);
        position += length;
    }
    if (list.isEmpty())
        return list;
    appendRestsUpTo(std::numeric_limits<double>::max());

    if (description != nullptr)
        *description = exrField(data, QStringLiteral("description")).toString().trimmed();
    return list;
}
