#include "workoutimport.h"
#include "qzsettings.h"
#include "fit-sdk/fit_decode.hpp"
#include "fit-sdk/fit_file_id_mesg.hpp"
#include "fit-sdk/fit_workout_mesg.hpp"
#include "fit-sdk/fit_workout_step_mesg.hpp"
#include <cmath>
#include <sstream>
#include <QDebug>
#include <QDir>
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
                                      QStringLiteral("XSR"), QStringLiteral("FIT")};
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
    if (kind == QStringLiteral("MRC") || kind == QStringLiteral("ERG") || kind == QStringLiteral("FIT"))
        return true;

    QFile input(filename);
    if (!input.open(QIODevice::ReadOnly))
        return false;
    return !exrField(exrData(input.readAll()), QStringLiteral("schedule")).toArray().isEmpty();
}

QList<trainrow> workoutimport::load(const QString &filename, const QString &extension, QString *description,
                                      BLUETOOTH_TYPE *fitDeviceType) {
    if (fitDeviceType)
        *fitDeviceType = UNKNOWN;
    const QString kind = fileKind(filename, extension);
    QFile input(filename);
    if (kind.isEmpty() || !input.open(QIODevice::ReadOnly)) {
        qDebug() << QStringLiteral("workoutimport: can't open") << filename << kind;
        return QList<trainrow>();
    }

    QSettings settings;
    const double ftp = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
    if (kind == QStringLiteral("FIT") && input.size() > 4 * 1024 * 1024)
        return {};
    const QByteArray content = input.readAll();
    if (kind == QStringLiteral("FIT")) {
        const double maxHr = settings.value(QZSettings::heart_max_override_enable,
                                             QZSettings::default_heart_max_override_enable).toBool()
            ? settings.value(QZSettings::heart_max_override_value, QZSettings::default_heart_max_override_value).toDouble()
            : 220.0 - settings.value(QZSettings::age, QZSettings::default_age).toDouble();
        const double runningFtp = settings.value(QZSettings::ftp_run, QZSettings::default_ftp_run).toDouble();
        return loadFit(content, ftp, runningFtp, maxHr, description, fitDeviceType);
    }
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

namespace {
constexpr int MaxFitRows = 10000;

class FitWorkoutListener : public fit::MesgListener {
public:
    bool workoutFile = false;
    bool seenFileId = false;
    bool seenWorkout = false;
    bool invalid = false;
    fit::WorkoutMesg workout;
    QMap<int, fit::WorkoutStepMesg> steps;

    void OnMesg(fit::Mesg &mesg) override {
        switch (mesg.GetNum()) {
        case FIT_MESG_NUM_FILE_ID: {
            fit::FileIdMesg id(mesg);
            if (seenFileId)
                invalid = true;
            seenFileId = true;
            workoutFile = id.IsTypeValid() && id.GetType() == FIT_FILE_WORKOUT;
            break;
        }
        case FIT_MESG_NUM_WORKOUT:
            if (seenWorkout)
                invalid = true;
            seenWorkout = true;
            workout = fit::WorkoutMesg(mesg);
            break;
        case FIT_MESG_NUM_WORKOUT_STEP: {
            fit::WorkoutStepMesg step(mesg);
            if (!step.IsMessageIndexValid() || steps.contains(step.GetMessageIndex()) ||
                steps.size() >= MaxFitRows) {
                invalid = true;
                break;
            }
            steps.insert(step.GetMessageIndex(), step);
            break;
        }
        default:
            break;
        }
    }
};

double fitPower(FIT_UINT32 value, double ftp) {
    return value > FIT_WORKOUT_POWER_WATTS_OFFSET
        ? double(value - FIT_WORKOUT_POWER_WATTS_OFFSET) : value * ftp / 100.0;
}

double fitHr(FIT_UINT32 value, double maxHr) {
    return value > FIT_WORKOUT_HR_BPM_OFFSET
        ? double(value - FIT_WORKOUT_HR_BPM_OFFSET) : value * maxHr / 100.0;
}

bool fitStepToRow(const fit::WorkoutStepMesg &step, double ftp, double maxHr, trainrow &row) {
    if (!step.IsDurationTypeValid() || !step.IsTargetTypeValid())
        return false;
    switch (step.GetDurationType()) {
    case FIT_WKT_STEP_DURATION_TIME:
    case FIT_WKT_STEP_DURATION_TIME_ONLY: {
        if (!step.IsDurationValueValid())
            return false;
        const double seconds = step.GetDurationValue() / 1000.0;
        if (seconds < 0.5 || seconds >= 86399.5)
            return false;
        row.duration = secondsToTime(qRound(seconds));
        break;
    }
    case FIT_WKT_STEP_DURATION_DISTANCE:
        if (!step.IsDurationDistanceValid() || step.GetDurationDistance() <= 0)
            return false;
        row.distance = step.GetDurationDistance() / 1000.0;
        break;
    case FIT_WKT_STEP_DURATION_OPEN:
        row.waitForLap = true;
        break;
    default:
        return false; // Do not silently change unsupported end conditions.
    }

    const auto target = step.GetTargetType();
    if (target == FIT_WKT_STEP_TARGET_OPEN)
        return true;

    const auto zone = step.IsTargetValueValid() ? step.GetTargetValue() : 0;
    if (zone != 0) {
        // QZ has configurable HR zones. FIT speed/power/cadence zone numbers do not
        // contain their boundaries, so don't invent a target for them.
        if (target == FIT_WKT_STEP_TARGET_HEART_RATE && zone <= 5) {
            row.zoneHR = int8_t(zone);
            return true;
        }
        return false;
    }

    if (!step.IsCustomTargetValueLowValid() || !step.IsCustomTargetValueHighValid())
        return false;
    const FIT_UINT32 low = step.GetCustomTargetValueLow();
    const FIT_UINT32 high = step.GetCustomTargetValueHigh();

    switch (target) {
    case FIT_WKT_STEP_TARGET_POWER: {
        const double a = fitPower(low, ftp), b = fitPower(high, ftp);
        if (!std::isfinite(a) || !std::isfinite(b) || a < 0 || b < a || b > 3000 ||
            ((low <= 1000 || high <= 1000) && (!std::isfinite(ftp) || ftp <= 0)))
            return false;
        row.power = qRound((a + b) / 2.0);
        break;
    }
    case FIT_WKT_STEP_TARGET_HEART_RATE: {
        const double a = fitHr(low, maxHr), b = fitHr(high, maxHr);
        if (!std::isfinite(a) || !std::isfinite(b) || a < 1 || b < a || b > 255 ||
            ((low <= 100 || high <= 100) && (!std::isfinite(maxHr) || maxHr <= 0 || maxHr > 255)))
            return false;
        row.HRmin = qRound(a);
        row.HRmax = qRound(b);
        break;
    }
    case FIT_WKT_STEP_TARGET_SPEED: {
        // FIT stores custom speed in millimetres per second.
        const double a = low * 0.0036, b = high * 0.0036;
        if (b < a || b > 100)
            return false;
        row.speed = (a + b) / 2.0;
        row.forcespeed = true;
        break;
    }
    case FIT_WKT_STEP_TARGET_CADENCE:
        if (high < low || high > 300)
            return false;
        row.lower_cadence = int16_t(low);
        row.upper_cadence = int16_t(high);
        row.cadence = qRound((double(low) + high) / 2.0);
        break;
    default:
        return false;
    }
    return true;
}
} // namespace

QList<trainrow> workoutimport::loadFit(const QByteArray &input, double ftp, double runningFtp, double maxHr,
                                         QString *description, BLUETOOTH_TYPE *fitDeviceType) {
    if (fitDeviceType)
        *fitDeviceType = UNKNOWN;
    if (description)
        description->clear();
    if (input.size() < 14 || input.size() > 4 * 1024 * 1024)
        return {};
    // Reject a truncated file before decoding. FIT's data size excludes header and trailing CRC.
    const auto *bytes = reinterpret_cast<const unsigned char *>(input.constData());
    const quint32 dataSize = quint32(bytes[4]) | (quint32(bytes[5]) << 8) |
                             (quint32(bytes[6]) << 16) | (quint32(bytes[7]) << 24);
    if (bytes[0] < 12 || quint64(bytes[0]) + dataSize + 2 != quint64(input.size()))
        return {};

    FitWorkoutListener listener;
    try {
        std::istringstream stream(std::string(input.constData(), size_t(input.size())),
                                  std::ios::in | std::ios::binary);
        fit::Decode decoder;
        if (!decoder.CheckIntegrity(stream))
            return {};
        stream.clear();
        stream.seekg(0);
        fit::Decode reader;
        if (!reader.Read(stream, listener))
            return {};
    } catch (const std::exception &e) {
        qWarning() << "workoutimport: invalid FIT" << e.what();
        return {};
    }

    if (listener.invalid || !listener.workoutFile || !listener.seenWorkout || listener.steps.isEmpty() ||
        (listener.workout.IsNumValidStepsValid() &&
         listener.workout.GetNumValidSteps() != listener.steps.size()))
        return {};

    BLUETOOTH_TYPE sportDevice = UNKNOWN;
    if (listener.workout.IsSportValid()) {
        switch (listener.workout.GetSport()) {
        case FIT_SPORT_RUNNING:
        case FIT_SPORT_WALKING:
            sportDevice = TREADMILL;
            break;
        case FIT_SPORT_CYCLING:
            sportDevice = BIKE;
            break;
        case FIT_SPORT_ROWING:
            sportDevice = ROWING;
            break;
        case FIT_SPORT_GENERIC:
            break;
        default:
            return {}; // A known unsupported sport must not control unrelated equipment.
        }
    }
    const double sportFtp = sportDevice == TREADMILL ? runningFtp : ftp;

    QList<trainrow> rows;
    QMap<int, int> rowOffsets;
    int expectedIndex = 0;
    for (auto it = listener.steps.constBegin(); it != listener.steps.constEnd(); ++it) {
        const auto &step = it.value();
        if (it.key() != expectedIndex++ || !step.IsDurationTypeValid())
            return {};
        rowOffsets.insert(it.key(), rows.size());
        if (step.GetDurationType() == FIT_WKT_STEP_DURATION_REPEAT_UNTIL_STEPS_CMPLT) {
            if (!step.IsDurationValueValid() || !step.IsTargetValueValid())
                return {};
            const auto first = step.GetDurationValue();
            const auto count = step.GetTargetValue();
            if (first >= quint32(it.key()) || !rowOffsets.contains(int(first)) || count < 1 || count > MaxFitRows)
                return {};
            const int start = rowOffsets.value(int(first));
            const int length = rows.size() - start;
            if (length <= 0 || quint64(rows.size()) + quint64(length) * (count - 1) > MaxFitRows)
                return {};
            const QList<trainrow> block = rows.mid(start, length);
            for (quint32 n = 1; n < count; ++n)
                rows.append(block);
        } else {
            trainrow row;
            if (rows.size() >= MaxFitRows || !fitStepToRow(step, sportFtp, maxHr, row))
                return {};
            QString message;
            if (step.IsWktStepNameValid()) {
                const auto name = step.GetWktStepName();
                message = QString::fromWCharArray(name.data(), int(name.size())).trimmed();
            }
            if (step.IsNotesValid()) {
                const auto notes = step.GetNotes();
                const QString note = QString::fromWCharArray(notes.data(), int(notes.size())).trimmed();
                if (!note.isEmpty() && note != message) {
                    if (!message.isEmpty())
                        message += QLatin1Char('\n');
                    message += note;
                }
            }
            if (!message.isEmpty()) {
                trainrow::TextEvent event;
                event.timeoffset = 0;
                event.message = message;
                row.textEvents.append(event);
            }
            rows.append(row);
        }
    }
    if (fitDeviceType)
        *fitDeviceType = sportDevice;
    if (description && listener.workout.IsWktNameValid()) {
        const auto name = listener.workout.GetWktName();
        *description = QString::fromWCharArray(name.data(), int(name.size()));
    }
    return rows;
}

bool workoutimport::isFitDeviceCompatible(BLUETOOTH_TYPE workoutDevice, BLUETOOTH_TYPE connectedDevice) {
    return workoutDevice == UNKNOWN || connectedDevice == UNKNOWN || workoutDevice == connectedDevice;
}

QString workoutimport::keepFitInTrainingFolder(const QString &sourcePath, const QString &trainingDirectory) {
    const QFileInfo source(sourcePath);
    if (!source.isFile() || source.suffix().compare(QStringLiteral("FIT"), Qt::CaseInsensitive) != 0 ||
        source.size() <= 0 || source.size() > 4 * 1024 * 1024 || trainingDirectory.isEmpty())
        return {};
    const QDir dir(trainingDirectory);
    if (!dir.exists() && !QDir().mkpath(dir.absolutePath()))
        return {};
    const QString root = dir.canonicalPath();
    const QString original = source.canonicalFilePath();
    if (root.isEmpty() || original.isEmpty())
        return {};
    if (original.startsWith(root + QStringLiteral("/")))
        return original;

    QFile input(sourcePath);
    if (!input.open(QIODevice::ReadOnly))
        return {};
    const QByteArray bytes = input.readAll();
    input.close();
    if (bytes.size() != source.size())
        return {};

    for (int suffix = 1; suffix <= 10000; ++suffix) {
        const QString name = suffix == 1 ? source.fileName() :
            source.completeBaseName() + QStringLiteral("_%1.fit").arg(suffix);
        const QString destination = dir.filePath(name);
        const QFileInfo existing(destination);
        if (existing.exists() || existing.isSymLink()) {
            QFile candidate(destination);
            if (!existing.isSymLink() && existing.isFile() && existing.size() == bytes.size() &&
                candidate.open(QIODevice::ReadOnly) && candidate.readAll() == bytes)
                return destination;
            continue;
        }
        if (QFile::copy(sourcePath, destination))
            return destination;
        if (!QFileInfo::exists(destination))
            return {};
    }
    return {};
}
