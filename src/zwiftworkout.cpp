#include "zwiftworkout.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamReader>
#include <cstdarg>

QList<trainrow> zwiftworkout::load(const QString &filename, QString *description, QString *tags) {
    QSettings settings;
    // QList<trainrow> list; //NOTE: clazy-unuzed-non-trivial-variable
    QFile input(filename);
    input.open(QIODevice::ReadOnly);
    return load(input.readAll(), description, tags);
}

bool zwiftworkout::durationAsDistance(QString sportType, QString durationType) {
    // watch out to the compatibility with HomeFitnessBuddy
    if (!sportType.toLower().contains(QStringLiteral("run")))
        return false;
    else if (sportType.toLower().contains(QStringLiteral("run")) &&
             durationType.toLower().contains(QStringLiteral("time")))
        return false;
    else
        return true;
}

double zwiftworkout::speedFromPace(int Pace) {
    QSettings settings;
    double speed = 0;
    QString pace_default = settings.value(QZSettings::pace_default, QZSettings::default_pace_default).toString();
    if (Pace == 0) {
        speed = settings.value(QZSettings::pacef_1mile, QZSettings::default_pacef_1mile).toDouble();
    } else if (Pace == 1) {
        speed = settings.value(QZSettings::pacef_5km, QZSettings::default_pacef_5km).toDouble();
    } else if (Pace == 2) {
        speed = settings.value(QZSettings::pacef_10km, QZSettings::default_pacef_10km).toDouble();
    } else if (Pace == 3) {
        speed = settings.value(QZSettings::pacef_halfmarathon, QZSettings::default_pacef_halfmarathon).toDouble();
    } else if (Pace == 4) {
        speed = settings.value(QZSettings::pacef_marathon, QZSettings::default_pacef_marathon).toDouble();
    } else {
        if (!pace_default.compare(QStringLiteral("1 mile")))
            speed = settings.value(QZSettings::pacef_1mile, QZSettings::default_pacef_1mile).toDouble();
        else if (!pace_default.compare(QStringLiteral("5 km")))
            speed = settings.value(QZSettings::pacef_5km, QZSettings::default_pacef_5km).toDouble();
        else if (!pace_default.compare(QStringLiteral("10 km")))
            speed = settings.value(QZSettings::pacef_10km, QZSettings::default_pacef_10km).toDouble();
        else if (!pace_default.compare(QStringLiteral("Half Marathon")))
            speed = settings.value(QZSettings::pacef_halfmarathon, QZSettings::default_pacef_halfmarathon).toDouble();
        else
            speed = settings.value(QZSettings::pacef_marathon, QZSettings::default_pacef_marathon).toDouble();
    }

    return speed;
}

void zwiftworkout::convertTag(double thresholdSecPerKm, const QString &sportType, const QString &durationType,
                              QList<trainrow> &list, const char *tag, ...) {
    va_list args;
    va_start(args, tag);
    QSettings settings;
    qDebug() << QStringLiteral("Tag is") << tag;
    if (!qstricmp(tag, "IntervalsT")) {
        uint32_t repeat = 1;
        uint32_t OnDuration = 1;
        uint32_t OffDuration = 1;
        double OnPower = 1;
        double OffPower = 1;
        double Incline = -100;
        int Pace = -1;
        int Cadence = -1;
        repeat = va_arg(args, uint32_t);
        if (repeat <= 0)
            repeat = 1;
        OnDuration = va_arg(args, uint32_t);
        OffDuration = va_arg(args, uint32_t);
        OnPower = va_arg(args, double);
        OffPower = va_arg(args, double);
        Pace = va_arg(args, int);
        Cadence = va_arg(args, int);
        Incline = va_arg(args, double);
        for (uint32_t i = 0; i < repeat; i++) {
            trainrow row;
            if (!durationAsDistance(sportType, durationType))
                row.duration = QTime(OnDuration / 3600, OnDuration / 60, OnDuration % 60, 0);
            else
                row.distance = OnDuration / 1000.0;
            if (sportType.toLower().contains(QStringLiteral("run"))) {
                row.forcespeed = 1;
                double speed = speedFromPace(Pace);
                row.speed = ((60.0 / speed) * 60.0) * OnPower;
                if(Incline != -100)
                    row.inclination = Incline * 100;
            } else {
                row.power = OnPower * settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
            }
            list.append(row);
            if (!durationAsDistance(sportType, durationType))
                row.duration = QTime(OffDuration / 3600, OffDuration / 60, OffDuration % 60, 0);
            else
                row.distance = OffDuration / 1000.0;
            if (sportType.toLower().contains(QStringLiteral("run"))) {
                row.forcespeed = 1;
                double speed = speedFromPace(Pace);
                row.speed = ((60.0 / speed) * 60.0) * OffPower;
                if(Incline != -100)
                    row.inclination = Incline * 100;
            } else {
                row.power = OffPower * settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
            }
            if(Cadence != -1)
                row.cadence = Cadence;
            qDebug() << "TrainRow" << row.toString();
            list.append(row);
        }
    } else if (!qstricmp(tag, "Ramp") || !qstricmp(tag, "Warmup") || !qstricmp(tag, "Cooldown")) {
        uint32_t Duration = 1;
        double PowerLow = 1;
        double PowerHigh = 1;
        double Incline = -100;
        int Pace = -1;
        int Cadence = -1;
        Duration = va_arg(args, uint32_t);
        PowerLow = va_arg(args, double);
        PowerHigh = va_arg(args, double);
        Pace = va_arg(args, int);
        Cadence = va_arg(args, int);
        Incline = va_arg(args, double);
        if (sportType.toLower().contains(QStringLiteral("run")) && !durationAsDistance(sportType, durationType)) {
            double speed = speedFromPace(Pace);
            int speedDelta = qAbs(qCeil((((60.0 / speed) * 60.0) * (PowerHigh - PowerLow)) * 10)) + 1;

            // Ensure we don't divide by zero
            speedDelta = qMax(1, speedDelta);

            // Calculate base duration step and remaining seconds
            int durationStep = Duration / speedDelta;
            int remainingSeconds = Duration % speedDelta;

            int totalElapsed = 0;

            for (int i = 0; i < speedDelta; i++) {
                trainrow row;

                // Distribute remaining seconds evenly across iterations
                int extraSecond = (i < remainingSeconds) ? 1 : 0;
                int currentStepDuration = durationStep + extraSecond;

                // Set row duration
                row.duration = QTime(0, 0, 0, 0).addSecs(currentStepDuration);
                row.rampElapsed = QTime(0, 0, 0, 0).addSecs(totalElapsed);
                row.rampDuration = QTime(0, 0, 0, 0).addSecs(Duration - totalElapsed);

                // Update total elapsed time
                totalElapsed += currentStepDuration;

                row.forcespeed = 1;

                // Calculate speed based on tag type
                if (!qstricmp(tag, "Ramp") || !qstricmp(tag, "Warmup")) {
                    row.speed = (double)qFloor(((((60.0 / speed) * 60.0) * (PowerLow)) + (0.1 * i)) * 10.0) / 10.0;
                } else {
                    row.speed = (double)qFloor(((((60.0 / speed) * 60.0) * (PowerLow)) - (0.1 * i)) * 10.0) / 10.0;
                }

                // Set inclination if provided
                if (Incline != -100) {
                    row.inclination = Incline * 100;
                }

                qDebug() << "TrainRow" << row.toString();
                list.append(row);
            }

        } else {
            for (uint32_t i = 0; i < Duration; i++) {
                trainrow row;
                if (!durationAsDistance(sportType, durationType)) {
                    row.duration = QTime(0, 0, 1, 0);
                    row.rampDuration = QTime((Duration - i) / 3600, (Duration - i) / 60, (Duration - i) % 60, 0);
                    row.rampElapsed = QTime(i / 3600, i / 60, i % 60, 0);
                } else {
                    row.distance = 0.001;
                }
                if(Cadence != -1)
                    row.cadence = Cadence;
                if (PowerHigh > PowerLow) {
                    if (!sportType.toLower().contains(QStringLiteral("run"))) {
                        row.power = (PowerLow + (((PowerHigh - PowerLow) / Duration) * i)) *
                                    settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                    } else {
                        double speed = speedFromPace(Pace);
                        row.speed = (double)qFloor(((((60.0 / speed) * 60.0) * (PowerLow + (((PowerHigh - PowerLow) / Duration) * i))) * 10.0)) / 10.0;
                        row.forcespeed = 1;
                    }
                } else {
                    if (!sportType.toLower().contains(QStringLiteral("run"))) {
                        row.power = (PowerLow - (((PowerLow - PowerHigh) / Duration) * i)) *
                                    settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                    } else {
                        double speed = speedFromPace(Pace);
                        row.speed = (double)qFloor(((((60.0 / speed) * 60.0) * ((PowerLow - (((PowerLow - PowerHigh) / Duration) * i)))) * 10.0)) / 10.0;
                        row.forcespeed = 1;
                    }
                }
                if(Incline != -100)
                    row.inclination = Incline * 100;
                qDebug() << "TrainRow" << row.toString();
                list.append(row);
            }
        }
    } else if (!qstricmp(tag, "FreeRide")) {
        uint32_t Duration = 1;
        // double FlatRoad = 1;
        Duration = va_arg(args, uint32_t);

        trainrow row;
        if (!durationAsDistance(sportType, durationType))
            row.duration = QTime(Duration / 3600, Duration / 60, Duration % 60, 0);
        else
            row.distance = ((double)Duration) / 1000.0;
        qDebug() << "TrainRow" << row.toString();
        list.append(row);
    } else if (!qstricmp(tag, "Steadystate")) {
        uint32_t Duration = 1;
        double Power = 1;
        double Incline = -100;
        int Pace = -1;
        int Cadence = -1;
        trainrow row;

        Duration = va_arg(args, uint32_t);
        Power = va_arg(args, double);
        Pace = va_arg(args, int);
        Incline = va_arg(args, double);
        Cadence = va_arg(args, int);

        if (sportType.toLower().contains(QStringLiteral("run")) && Duration != 1) {
            if (thresholdSecPerKm != 0) {
                row.forcespeed = 1;
                row.speed = (60.0 / (thresholdSecPerKm / Power)) * 60.0;
            } else {
                double speed = speedFromPace(Pace);
                row.forcespeed = 1;
                row.speed = ((60.0 / speed) * 60.0) * Power;
            }
            Power = 1;
        } else {
            row.power = Power * settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
        }

        if (!durationAsDistance(sportType, durationType))
            row.duration = QTime(Duration / 3600, Duration / 60, Duration % 60, 0);
        else
            row.distance = Duration / 1000.0;

        if(Incline != -100) {
            row.inclination = Incline * 100;
        }

        if(Cadence != -1)
            row.cadence = Cadence;

        qDebug() << "TrainRow" << row.toString();
        list.append(row);
    }
    va_end(args);
}

QList<trainrow> zwiftworkout::loadJSON(const QString &input, QString *description, QString *tags) {
    QList<trainrow> list;
    QJsonDocument doc = QJsonDocument::fromJson(input.toUtf8());
    if (doc.isObject()) {
        QSettings settings;
        QJsonObject obj = doc.object();
        QJsonArray arr;
        QString sportType = QStringLiteral("");
        QString durationType = QStringLiteral("");
        if (obj.contains(QStringLiteral("durationType"))) {
            durationType = obj[QStringLiteral("durationType")].toString();
        }
        if (obj.contains(QStringLiteral("sportType"))) {
            sportType = obj[QStringLiteral("sportType")].toString();
        }
        if (description != nullptr && obj.contains(QStringLiteral("description"))) {
            *description = obj[QStringLiteral("description")].toString();
        }

        if (tags) {
            tags->clear();
            arr = obj[QStringLiteral("tags")].toArray();
            for (int k = 0; k < arr.count(); k++) {
                QJsonObject element = arr.at(k).toObject();
                if (element.contains(QStringLiteral("name"))) {
                    QString tag = element[QStringLiteral("name")].toString();
                    tags->append("#" + tag + " ");
                }
            }
        }
        arr = obj[QStringLiteral("workout")].toArray();
        for (int k = 0; k < arr.count(); k++) {
            QJsonObject element = arr.at(k).toObject();
            if (element.contains(QStringLiteral("type"))) {
                QString type = element[QStringLiteral("type")].toString();
                if (type == QStringLiteral("IntervalsT")) {
                    uint32_t repeat = 1;
                    uint32_t OnDuration = 1;
                    uint32_t OffDuration = 1;
                    double OnPower = 1;
                    double OffPower = 1;
                    int Pace = -1;
                    int Cadence = -1;
                    repeat = element[QStringLiteral("Repeat")].toInt();
                    if (!repeat)
                        repeat = 1;
                    OnDuration = element[QStringLiteral("OnDuration")].toDouble();
                    OffDuration = element[QStringLiteral("OffDuration")].toDouble();
                    OnPower = element[QStringLiteral("OnPower")].toDouble();
                    OffPower = element[QStringLiteral("OffPower")].toDouble();
                    if (element.contains(QStringLiteral("pace"))) {
                        Pace = element[QStringLiteral("pace")].toInt();
                    }
                    if (element.contains(QStringLiteral("Cadence"))) {
                        Cadence = element[QStringLiteral("Cadence")].toInt();
                    }
                    convertTag(0.0, sportType, durationType, list, type.toUtf8().constData(), repeat, OnDuration,
                               OffDuration, OnPower, OffPower, Pace, Cadence);
                } else if (type == QStringLiteral("FreeRide")) {
                    uint32_t Duration = 1;
                    // double FlatRoad = 1;
                    Duration = element[QStringLiteral("Duration")].toDouble();

                    convertTag(0.0, sportType, durationType, list, type.toUtf8().constData(), Duration);
                } else if (type == QStringLiteral("Ramp") || type == QStringLiteral("Warmup") ||
                           type == QStringLiteral("Cooldown")) {
                    uint32_t Duration = 1;
                    double PowerLow = 1;
                    double PowerHigh = 1;
                    int Pace = -1;
                    int Cadence = -1;
                    Duration = element[QStringLiteral("Duration")].toDouble();
                    PowerLow = element[QStringLiteral("PowerLow")].toDouble();
                    PowerHigh = element[QStringLiteral("PowerHigh")].toDouble();
                    if (element.contains(QStringLiteral("pace"))) {
                        Pace = element[QStringLiteral("pace")].toInt();
                    }
                    if (element.contains(QStringLiteral("Cadence"))) {
                        Cadence = element[QStringLiteral("Cadence")].toInt();
                    }
                    convertTag(0.0, sportType, durationType, list, type.toUtf8().constData(), Duration, PowerLow,
                               PowerHigh, Pace, Cadence);
                } else if (type == QStringLiteral("SteadyState")) {
                    uint32_t Duration = 1;
                    double Power = 1;
                    int Pace = -1;
                    double Incline = -100;
                    int Cadence = -1;

                    Duration = element[QStringLiteral("Duration")].toDouble();
                    if (element.contains(QStringLiteral("pace"))) {
                        Pace = element[QStringLiteral("pace")].toInt();
                    }
                    Power = element[QStringLiteral("Power")].toDouble();
                    if (Power == 1) {
                        Power = element[QStringLiteral("PowerLow")].toDouble();
                    }
                    if (element.contains(QStringLiteral("Incline"))) {
                        Incline = element[QStringLiteral("Incline")].toDouble();
                    }
                    if (element.contains(QStringLiteral("Cadence"))) {
                        Cadence = element[QStringLiteral("Cadence")].toInt();
                    }
                    convertTag(0.0, sportType, durationType, list, type.toUtf8().constData(), Duration, Power, Pace, Incline, Cadence);
                }
            }
        }
    }
    return list;
}

QList<trainrow> zwiftworkout::load(const QByteArray &input, QString *description, QString *tags) {
    QSettings settings;
    QList<trainrow> list;
    QXmlStreamReader stream(input);
    double thresholdSecPerKm = 0;
    QString sportType = QStringLiteral("");
    QString durationType = QStringLiteral("");
    if (description != nullptr)
        description->clear();
    if (tags != nullptr)
        tags->clear();

    while (!stream.atEnd()) {
        stream.readNext();
        QString name = stream.name().toString();
        QString text = stream.text().toString();
        QXmlStreamAttributes atts = stream.attributes();
        if (name.toLower().contains(QStringLiteral("thresholdsecperkm")) && thresholdSecPerKm == 0) {
            stream.readNext();
            thresholdSecPerKm = stream.text().toDouble();
        } else if (name.toLower().contains(QStringLiteral("sporttype")) && sportType.length() == 0) {
            stream.readNext();
            sportType = stream.text().toString();
        } else if (description != nullptr && name.toLower().contains(QStringLiteral("description")) &&
                   description->length() == 0) {
            stream.readNext();
            *description = stream.text().toString();
        } else if (tags != nullptr && name.toLower().contains(QStringLiteral("tag")) && name.length() == 3) {
            if (atts.hasAttribute(QStringLiteral("name"))) {
                tags->append("#" + atts.value(QStringLiteral("name")).toString() + " ");
            }
        } else if (name.toLower().contains(QStringLiteral("durationtype")) && durationType.length() == 0) {
            stream.readNext();
            durationType = stream.text().toString();
        } else if (!atts.isEmpty()) {
            if (name.contains(QStringLiteral("IntervalsT"))) {
                uint32_t repeat = 1;
                uint32_t OnDuration = 1;
                uint32_t OffDuration = 1;
                double OnPower = 1;
                double OffPower = 1;
                int Pace = -1;
                int Cadence = -1;
                double Incline = -100;

                if (atts.hasAttribute(QStringLiteral("Repeat"))) {
                    repeat = atts.value(QStringLiteral("Repeat")).toUInt();
                }
                if (atts.hasAttribute(QStringLiteral("OnDuration"))) {
                    OnDuration = atts.value(QStringLiteral("OnDuration")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("OffDuration"))) {
                    OffDuration = atts.value(QStringLiteral("OffDuration")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("OnPower"))) {
                    OnPower = atts.value(QStringLiteral("OnPower")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("OffPower"))) {
                    OffPower = atts.value(QStringLiteral("OffPower")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("pace"))) {
                    Pace = atts.value(QStringLiteral("pace")).toUInt();
                }
                if (atts.hasAttribute(QStringLiteral("Cadence"))) {
                    Cadence = atts.value(QStringLiteral("Cadence")).toUInt();
                }
                if (atts.hasAttribute(QStringLiteral("Incline"))) {
                    Incline = atts.value(QStringLiteral("Incline")).toDouble();
                }

                convertTag(thresholdSecPerKm, sportType, durationType, list, name.toUtf8().constData(), repeat,
                           OnDuration, OffDuration, OnPower, OffPower, Pace, Cadence, Incline);
            } else if (name.contains(QStringLiteral("FreeRide"))) {
                uint32_t Duration = 1;
                // double FlatRoad = 1;
                if (atts.hasAttribute(QStringLiteral("Duration"))) {
                    Duration = atts.value(QStringLiteral("Duration")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("FlatRoad"))) {
                    // NOTE: Value stored to FlatRoad is never read clang-analyzer-deadcode.DeadStores
                    // FlatRoad = atts.value(QStringLiteral("FlatRoad")).toDouble();
                }

                convertTag(thresholdSecPerKm, sportType, durationType, list, name.toUtf8().constData(), Duration);
            } else if (name.contains(QStringLiteral("Ramp")) ||
                       name.contains(QStringLiteral("Warmup"), Qt::CaseInsensitive) ||
                       name.contains(QStringLiteral("Cooldown"))) {
                uint32_t Duration = 1;
                double PowerLow = 1;
                double PowerHigh = 1;
                int Pace = -1;
                int Cadence = -1;
                double Incline = -100;

                if (atts.hasAttribute(QStringLiteral("Duration"))) {
                    Duration = atts.value(QStringLiteral("Duration")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("PowerLow"))) {
                    PowerLow = atts.value(QStringLiteral("PowerLow")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("PowerHigh"))) {
                    PowerHigh = atts.value(QStringLiteral("PowerHigh")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("pace"))) {
                    Pace = atts.value(QStringLiteral("pace")).toUInt();
                }
                if (atts.hasAttribute(QStringLiteral("Cadence"))) {
                    Cadence = atts.value(QStringLiteral("Cadence")).toUInt();
                }
                if (atts.hasAttribute(QStringLiteral("Incline"))) {
                    Incline = atts.value(QStringLiteral("Incline")).toDouble();
                }

                convertTag(thresholdSecPerKm, sportType, durationType, list, name.toUtf8().constData(), Duration,
                           PowerLow, PowerHigh, Pace, Cadence, Incline);
            } else if (name.contains(QStringLiteral("SteadyState"))) {
                uint32_t Duration = 1;
                double Power = 1;
                int Pace = -1;
                double Incline = -100;
                int Cadence = -1;

                if (atts.hasAttribute(QStringLiteral("Duration"))) {
                    Duration = atts.value(QStringLiteral("Duration")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("pace"))) {
                    Pace = atts.value(QStringLiteral("pace")).toUInt();
                }
                if (atts.hasAttribute(QStringLiteral("Power"))) {
                    Power = atts.value(QStringLiteral("Power")).toDouble();
                }
                if (Power == 1 && atts.hasAttribute(QStringLiteral("PowerLow"))) {
                    Power = atts.value(QStringLiteral("PowerLow")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("Incline"))) {
                    Incline = atts.value(QStringLiteral("Incline")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("Cadence"))) {
                    Cadence = atts.value(QStringLiteral("Cadence")).toUInt();
                }
                convertTag(thresholdSecPerKm, sportType, durationType, list, name.toUtf8().constData(), Duration, Power,
                           Pace, Incline, Cadence);
            }
        }
    }
    return list;
}
