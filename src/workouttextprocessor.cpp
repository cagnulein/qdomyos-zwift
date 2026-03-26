#include "workouttextprocessor.h"
#include "qzsettings.h"
#include "zwiftworkout.h"
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSettings>
#include <QtMath>

namespace {
struct ParsedDuration {
    int seconds = 0;
    bool valid = false;
};

static QString normalizeSpace(QString text) {
    text.replace(QRegularExpression(QStringLiteral("[\\t\\r\\n]+")), QStringLiteral(" "));
    text.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    return text.trimmed();
}

static QString segmentLabel(const QString &segment) {
    const QString lower = segment.toLower();
    if (lower.contains(QStringLiteral("warmup")) || lower.contains(QStringLiteral("riscald"))) {
        return QStringLiteral("Warmup");
    }
    if (lower.contains(QStringLiteral("cooldown")) || lower.contains(QStringLiteral("defatic")) ||
        lower.contains(QStringLiteral("cool down"))) {
        return QStringLiteral("Cooldown");
    }
    if (lower.contains(QStringLiteral("recover")) || lower.contains(QStringLiteral("easy")) ||
        lower.contains(QStringLiteral("rest")) || lower.contains(QStringLiteral("facile")) ||
        lower.contains(QStringLiteral("recuper"))) {
        return QStringLiteral("Recovery");
    }
    if (lower.contains(QStringLiteral("sprint")) || lower.contains(QStringLiteral("vo2")) ||
        lower.contains(QStringLiteral("anaerob"))) {
        return QStringLiteral("Hard");
    }
    if (lower.contains(QStringLiteral("threshold")) || lower.contains(QStringLiteral("soglia")) ||
        lower.contains(QStringLiteral("tempo"))) {
        return QStringLiteral("Tempo");
    }
    return QStringLiteral("Interval");
}

static bool promptMentionsWarmup(const QString &text) {
    const QString lower = text.toLower();
    return lower.contains(QStringLiteral("warmup")) || lower.contains(QStringLiteral("warm up")) ||
           lower.contains(QStringLiteral("riscald"));
}

static bool promptMentionsCooldown(const QString &text) {
    const QString lower = text.toLower();
    return lower.contains(QStringLiteral("cooldown")) || lower.contains(QStringLiteral("cool down")) ||
           lower.contains(QStringLiteral("defatic"));
}

static ParsedDuration parseDurationToken(const QString &numberToken, const QString &unitToken) {
    ParsedDuration out;
    const QString number = numberToken.trimmed();
    const QString unit = unitToken.trimmed().toLower();

    if (number.isEmpty()) {
        return out;
    }

    if (number.contains(QLatin1Char(':'))) {
        const QStringList parts = number.split(QLatin1Char(':'));
        if (parts.size() == 2) {
            out.seconds = (parts.at(0).toInt() * 60) + parts.at(1).toInt();
            out.valid = out.seconds > 0;
            return out;
        }
        if (parts.size() == 3) {
            out.seconds = (parts.at(0).toInt() * 3600) + (parts.at(1).toInt() * 60) + parts.at(2).toInt();
            out.valid = out.seconds > 0;
            return out;
        }
    }

    bool ok = false;
    const double value = number.toDouble(&ok);
    if (!ok || value <= 0.0) {
        return out;
    }

    if (unit.startsWith(QStringLiteral("s")) || unit.startsWith(QStringLiteral("sec"))) {
        out.seconds = qRound(value);
    } else if (unit.startsWith(QStringLiteral("h")) || unit.startsWith(QStringLiteral("hr")) ||
               unit.startsWith(QStringLiteral("ore"))) {
        out.seconds = qRound(value * 3600.0);
    } else {
        out.seconds = qRound(value * 60.0);
    }
    out.valid = out.seconds > 0;
    return out;
}

static ParsedDuration parseFirstDuration(const QString &segment) {
    static const QRegularExpression durationRe(
        QStringLiteral("(\\d+(?::\\d{1,2}(?::\\d{1,2})?)?|\\d+(?:[\\.,]\\d+)?)\\s*(ore|hours?|hr|h|minuti|minuto|mins?|minutes?|m|secondi|secondo|secs?|seconds?|s)?"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = durationRe.match(segment);
    if (!match.hasMatch()) {
        return {};
    }
    return parseDurationToken(match.captured(1).replace(',', '.'), match.captured(2));
}

static int intensityScore(const QString &text) {
    const QString lower = text.toLower();
    if (lower.contains(QStringLiteral("sprint")) || lower.contains(QStringLiteral("max")) ||
        lower.contains(QStringLiteral("all out")) || lower.contains(QStringLiteral("massim"))) {
        return 4;
    }
    if (lower.contains(QStringLiteral("hard")) || lower.contains(QStringLiteral("forte")) ||
        lower.contains(QStringLiteral("threshold")) || lower.contains(QStringLiteral("soglia")) ||
        lower.contains(QStringLiteral("vo2"))) {
        return 3;
    }
    if (lower.contains(QStringLiteral("tempo")) || lower.contains(QStringLiteral("moderate")) ||
        lower.contains(QStringLiteral("medio")) || lower.contains(QStringLiteral("steady"))) {
        return 2;
    }
    if (lower.contains(QStringLiteral("warmup")) || lower.contains(QStringLiteral("cooldown")) ||
        lower.contains(QStringLiteral("cool down")) || lower.contains(QStringLiteral("easy")) ||
        lower.contains(QStringLiteral("recovery")) || lower.contains(QStringLiteral("recover")) ||
        lower.contains(QStringLiteral("rest")) || lower.contains(QStringLiteral("facile")) ||
        lower.contains(QStringLiteral("recuper")) || lower.contains(QStringLiteral("riscald")) ||
        lower.contains(QStringLiteral("defatic"))) {
        return 1;
    }
    return 2;
}

static void applyDefaultTargets(trainrow &row, const QString &deviceKey, const QString &context) {
    const int score = intensityScore(context);
    QSettings settings;
    const double ftp = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();

    if (deviceKey == QStringLiteral("treadmill")) {
        row.forcespeed = true;
        switch (score) {
        case 1:
            row.speed = 6.0;
            row.inclination = 0.5;
            break;
        case 2:
            row.speed = 8.0;
            row.inclination = 1.0;
            break;
        case 3:
            row.speed = 10.5;
            row.inclination = 1.5;
            break;
        default:
            row.speed = 13.0;
            row.inclination = 1.0;
            break;
        }
        return;
    }

    if (deviceKey == QStringLiteral("elliptical")) {
        switch (score) {
        case 1:
            row.resistance = 12;
            row.inclination = 5;
            row.cadence = 70;
            break;
        case 2:
            row.resistance = 18;
            row.inclination = 8;
            row.cadence = 80;
            break;
        case 3:
            row.resistance = 24;
            row.inclination = 10;
            row.cadence = 88;
            break;
        default:
            row.resistance = 30;
            row.inclination = 12;
            row.cadence = 95;
            break;
        }
        return;
    }

    if (deviceKey == QStringLiteral("rower")) {
        switch (score) {
        case 1:
            row.power = qRound(ftp * 0.55);
            row.cadence = 20;
            break;
        case 2:
            row.power = qRound(ftp * 0.75);
            row.cadence = 24;
            break;
        case 3:
            row.power = qRound(ftp * 0.95);
            row.cadence = 28;
            break;
        default:
            row.power = qRound(ftp * 1.15);
            row.cadence = 32;
            break;
        }
        return;
    }

    switch (score) {
    case 1:
        row.power = qRound(ftp * 0.6);
        break;
    case 2:
        row.power = qRound(ftp * 0.75);
        break;
    case 3:
        row.power = qRound(ftp * 0.92);
        break;
    default:
        row.power = qRound(ftp * 1.15);
        break;
    }
}

static void applyExplicitTargets(trainrow &row, const QString &segment, const QString &deviceKey) {
    QRegularExpressionMatch match;

    static const QRegularExpression wattRe(QStringLiteral("(\\d{2,4})\\s*(?:w|watt)"),
                                           QRegularExpression::CaseInsensitiveOption);
    match = wattRe.match(segment);
    if (match.hasMatch()) {
        row.power = match.captured(1).toInt();
    }

    static const QRegularExpression cadenceRe(QStringLiteral("(\\d{2,3})\\s*(?:rpm|spm)"),
                                              QRegularExpression::CaseInsensitiveOption);
    match = cadenceRe.match(segment);
    if (match.hasMatch()) {
        row.cadence = match.captured(1).toInt();
    }

    static const QRegularExpression resistanceRe(
        QStringLiteral("(?:res(?:istance)?|livello|level)\\s*(\\d{1,3})|(\\d{1,3})\\s*(?:res(?:istance)?|level)"),
        QRegularExpression::CaseInsensitiveOption);
    match = resistanceRe.match(segment);
    if (match.hasMatch()) {
        const QString value = match.captured(1).isEmpty() ? match.captured(2) : match.captured(1);
        row.resistance = value.toInt();
    }

    static const QRegularExpression speedRe(QStringLiteral("(\\d+(?:[\\.,]\\d+)?)\\s*(km/h|kph|mph)"),
                                            QRegularExpression::CaseInsensitiveOption);
    match = speedRe.match(segment);
    if (match.hasMatch()) {
        double speed = match.captured(1).replace(',', '.').toDouble();
        if (match.captured(2).toLower() == QStringLiteral("mph")) {
            speed *= 1.60934;
        }
        row.speed = speed;
        row.forcespeed = true;
    }

    static const QRegularExpression inclineRe(QStringLiteral("(-?\\d+(?:[\\.,]\\d+)?)\\s*%"),
                                              QRegularExpression::CaseInsensitiveOption);
    match = inclineRe.match(segment);
    if (match.hasMatch() && deviceKey != QStringLiteral("bike") && deviceKey != QStringLiteral("rower")) {
        row.inclination = match.captured(1).replace(',', '.').toDouble();
    }
}

static trainrow makeRow(const QString &segment, const QString &deviceKey, int durationSeconds) {
    trainrow row;
    row.duration = QTime(0, 0, 0).addSecs(qMax(1, durationSeconds));
    applyDefaultTargets(row, deviceKey, segment);
    applyExplicitTargets(row, segment, deviceKey);
    return row;
}

static bool parseRepeatBlock(const QString &segment, const QString &deviceKey, QList<trainrow> &rows) {
    static const QRegularExpression repeatRe(
        QStringLiteral("^\\s*(\\d+)\\s*[xX]\\s*(\\d+(?::\\d{1,2}(?::\\d{1,2})?)?|\\d+(?:[\\.,]\\d+)?)\\s*(ore|hours?|hr|h|minuti|minuto|mins?|minutes?|m|secondi|secondo|secs?|seconds?|s)?\\s*([^/,+]*)\\s*(?:/|\\+|,\\s*)\\s*(\\d+(?::\\d{1,2}(?::\\d{1,2})?)?|\\d+(?:[\\.,]\\d+)?)\\s*(ore|hours?|hr|h|minuti|minuto|mins?|minutes?|m|secondi|secondo|secs?|seconds?|s)?\\s*(.*)$"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = repeatRe.match(segment);
    if (!match.hasMatch()) {
        return false;
    }

    const int repeat = match.captured(1).toInt();
    const ParsedDuration onDuration = parseDurationToken(match.captured(2).replace(',', '.'), match.captured(3));
    const ParsedDuration offDuration = parseDurationToken(match.captured(5).replace(',', '.'), match.captured(6));
    if (!onDuration.valid || !offDuration.valid || repeat <= 0) {
        return false;
    }

    QString onSegment = normalizeSpace(match.captured(4));
    QString offSegment = normalizeSpace(match.captured(7));
    if (onSegment.isEmpty()) {
        onSegment = QStringLiteral("hard");
    }
    if (offSegment.isEmpty()) {
        offSegment = QStringLiteral("easy");
    }

    for (int i = 0; i < repeat; ++i) {
        rows.append(makeRow(onSegment, deviceKey, onDuration.seconds));
        rows.append(makeRow(offSegment, deviceKey, offDuration.seconds));
    }
    return true;
}

static QList<trainrow> parsePlainTextPrompt(const QString &prompt, const QString &deviceKey) {
    QList<trainrow> rows;
    const QStringList rawSegments =
        prompt.split(QRegularExpression(QStringLiteral("[\\n;]+")), Qt::SkipEmptyParts);

    for (const QString &rawSegment : rawSegments) {
        const QString segment = normalizeSpace(rawSegment);
        if (segment.isEmpty()) {
            continue;
        }

        if (parseRepeatBlock(segment, deviceKey, rows)) {
            continue;
        }

        const ParsedDuration duration = parseFirstDuration(segment);
        if (!duration.valid) {
            continue;
        }

        rows.append(makeRow(segment, deviceKey, duration.seconds));
    }

    if (!rows.isEmpty()) {
        if (rows.size() == 1 && promptMentionsWarmup(prompt) && promptMentionsCooldown(prompt)) {
            const int totalSeconds = QTime(0, 0, 0).secsTo(rows.first().duration);
            if (totalSeconds >= 15 * 60) {
                const int totalMinutes = qMax(15, totalSeconds / 60);
                const int warmupMinutes = qMax(5, qMin(10, totalMinutes / 6));
                const int cooldownMinutes = qMax(5, qMin(10, totalMinutes / 6));
                const int mainMinutes = qMax(5, totalMinutes - warmupMinutes - cooldownMinutes);

                QList<trainrow> structuredRows;
                structuredRows.append(makeRow(QStringLiteral("warmup easy"), deviceKey, warmupMinutes * 60));
                structuredRows.append(makeRow(prompt, deviceKey, mainMinutes * 60));
                structuredRows.append(makeRow(QStringLiteral("cooldown easy"), deviceKey, cooldownMinutes * 60));
                return structuredRows;
            }
        }
        return rows;
    }

    const QString normalized = normalizeSpace(prompt);
    rows.append(makeRow(QStringLiteral("warmup ") + normalized, deviceKey, 600));
    rows.append(makeRow(normalized, deviceKey, 1200));
    rows.append(makeRow(QStringLiteral("cooldown ") + normalized, deviceKey, 300));
    return rows;
}
} // namespace

WorkoutTextProcessor::Result WorkoutTextProcessor::fromCanonicalJson(const QString &canonicalJson,
                                                                     const QString &defaultDeviceKey) {
    Result result;
    const QString trimmed = canonicalJson.trimmed();
    if (trimmed.isEmpty()) {
        result.warning = QStringLiteral("Empty canonical workout JSON");
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(trimmed.toUtf8(), &parseError);
    if (!doc.isObject()) {
        result.warning = QStringLiteral("Canonical workout JSON is invalid: %1").arg(parseError.errorString());
        return result;
    }

    const QJsonObject root = doc.object();
    const QString deviceKey = root.value(QStringLiteral("device")).toString(defaultDeviceKey).trimmed();
    result.title = root.value(QStringLiteral("title")).toString(QStringLiteral("AI Workout")).trimmed();
    const QJsonArray steps = root.value(QStringLiteral("steps")).toArray();
    if (steps.isEmpty()) {
        result.warning = QStringLiteral("Canonical workout JSON has no steps");
        return result;
    }

    std::function<void(const QJsonObject &, int)> appendStepObject = [&](const QJsonObject &step, int repeatMultiplier) {
        const int repeat = qMax(1, step.value(QStringLiteral("repeat")).toInt(1));
        const QJsonArray nestedSteps = step.value(QStringLiteral("steps")).toArray();

        if (!nestedSteps.isEmpty()) {
            for (int outer = 0; outer < repeatMultiplier; ++outer) {
                for (int i = 0; i < repeat; ++i) {
                    for (const QJsonValue &nestedValue : nestedSteps) {
                        if (nestedValue.isObject()) {
                            appendStepObject(nestedValue.toObject(), 1);
                        }
                    }
                }
            }
            return;
        }

        const int durationSeconds = step.value(QStringLiteral("durationSeconds")).toInt();
        if (durationSeconds <= 0) {
            return;
        }

        for (int outer = 0; outer < repeatMultiplier; ++outer) {
            for (int i = 0; i < repeat; ++i) {
                trainrow row;
                row.duration = QTime(0, 0, 0).addSecs(durationSeconds);

                if (step.contains(QStringLiteral("speedKph"))) {
                    row.speed = step.value(QStringLiteral("speedKph")).toDouble();
                }
                if (step.contains(QStringLiteral("inclinePercent"))) {
                    row.inclination = step.value(QStringLiteral("inclinePercent")).toDouble();
                }
                if (deviceKey != QStringLiteral("bike") && step.contains(QStringLiteral("resistance"))) {
                    row.resistance = step.value(QStringLiteral("resistance")).toInt();
                }
                if (deviceKey != QStringLiteral("bike") && step.contains(QStringLiteral("cadenceRpm"))) {
                    row.cadence = step.value(QStringLiteral("cadenceRpm")).toInt();
                }
                if (step.contains(QStringLiteral("powerWatts"))) {
                    row.power = step.value(QStringLiteral("powerWatts")).toInt();
                }
                if (step.contains(QStringLiteral("fanSpeed"))) {
                    row.fanspeed = step.value(QStringLiteral("fanSpeed")).toInt();
                }
                if (deviceKey != QStringLiteral("bike") && step.contains(QStringLiteral("pelotonResistance"))) {
                    row.requested_peloton_resistance = step.value(QStringLiteral("pelotonResistance")).toInt();
                }
                if (step.contains(QStringLiteral("heartRateZone"))) {
                    row.zoneHR = step.value(QStringLiteral("heartRateZone")).toInt();
                }
                if (step.contains(QStringLiteral("heartRateMin"))) {
                    row.HRmin = step.value(QStringLiteral("heartRateMin")).toInt();
                }
                if (step.contains(QStringLiteral("heartRateMax"))) {
                    row.HRmax = step.value(QStringLiteral("heartRateMax")).toInt();
                }
                if (step.contains(QStringLiteral("mets"))) {
                    row.mets = step.value(QStringLiteral("mets")).toInt();
                }
                if (step.contains(QStringLiteral("forceSpeed"))) {
                    row.forcespeed = step.value(QStringLiteral("forceSpeed")).toBool();
                }

                if (row.power < 0 && row.resistance < 0 && row.speed < 0 && row.cadence < 0 &&
                    row.inclination <= -200) {
                    applyDefaultTargets(row, deviceKey,
                                        step.value(QStringLiteral("name")).toString(QStringLiteral("steady")));
                }

                result.rows.append(row);
            }
        }
    };

    for (const QJsonValue &stepValue : steps) {
        if (stepValue.isObject()) {
            appendStepObject(stepValue.toObject(), 1);
        }
    }

    if (result.rows.isEmpty()) {
        result.warning = QStringLiteral("Canonical workout JSON did not yield any usable steps");
    }
    return result;
}

WorkoutTextProcessor::Result WorkoutTextProcessor::fromPromptFallback(const QString &prompt, const QString &deviceKey) {
    Result result;
    const QString trimmed = prompt.trimmed();
    if (trimmed.isEmpty()) {
        result.warning = QStringLiteral("Empty prompt");
        return result;
    }

    result.rows = parsePlainTextPrompt(trimmed, deviceKey);
    result.title = segmentLabel(trimmed) + QStringLiteral(" Workout");
    result.warning = QStringLiteral("Parsed locally from free text");
    return result;
}

WorkoutTextProcessor::Result WorkoutTextProcessor::generate(const QString &prompt, const QString &deviceKey) {
    const Result canonical = fromCanonicalJson(prompt, deviceKey);
    if (!canonical.rows.isEmpty()) {
        return canonical;
    }
    return fromPromptFallback(prompt, deviceKey);
}
