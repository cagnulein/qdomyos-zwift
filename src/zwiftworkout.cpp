#include "zwiftworkout.h"
#include <QXmlStreamReader>

QList<trainrow> zwiftworkout::load(const QString &filename) {
    QSettings settings;
    QList<trainrow> list;
    QFile input(filename);
    input.open(QIODevice::ReadOnly);
    QXmlStreamReader stream(&input);
    while (!stream.atEnd()) {
        stream.readNext();
        QXmlStreamAttributes atts = stream.attributes();
        if (!atts.isEmpty()) {
            if (stream.name().contains(QStringLiteral("IntervalsT"))) {
                uint32_t repeat = 1;
                uint32_t OnDuration = 1;
                uint32_t OffDuration = 1;
                double OnPower = 1;
                double OffPower = 1;
                if (atts.hasAttribute(QStringLiteral("Repeat"))) {
                    repeat = atts.value(QStringLiteral("Repeat")).toUInt();
                }
                if (atts.hasAttribute(QStringLiteral("OnDuration"))) {
                    OnDuration = atts.value(QStringLiteral("OnDuration")).toUInt();
                }
                if (atts.hasAttribute(QStringLiteral("OffDuration"))) {
                    OffDuration = atts.value(QStringLiteral("OffDuration")).toUInt();
                }
                if (atts.hasAttribute(QStringLiteral("OnPower"))) {
                    OnPower = atts.value(QStringLiteral("OnPower")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("OffPower"))) {
                    OffPower = atts.value(QStringLiteral("OffPower")).toDouble();
                }

                for (uint32_t i = 0; i < repeat; i++) {
                    trainrow row;
                    row.duration = QTime(OnDuration / 3600, OnDuration / 60, OnDuration % 60, 0);
                    row.power = OnPower * settings.value(QStringLiteral("ftp"), 200.0).toDouble();
                    list.append(row);
                    row.duration = QTime(OffDuration / 3600, OffDuration / 60, OffDuration % 60, 0);
                    row.power = OffPower * settings.value(QStringLiteral("ftp"), 200.0).toDouble();
                    list.append(row);
                }
            } else if (stream.name().contains(QStringLiteral("FreeRide"))) {
                uint32_t Duration = 1;
                // double FlatRoad = 1;
                if (atts.hasAttribute(QStringLiteral("Duration"))) {
                    Duration = atts.value(QStringLiteral("Duration")).toUInt();
                }
                //                if (atts.hasAttribute(QStringLiteral("FlatRoad"))) {
                //                    // FlatRoad = atts.value(QStringLiteral("FlatRoad")).toDouble(); //NOTE:
                //                    // clang-analyzer-deadcode.DeadStores
                //                }

                trainrow row;
                row.duration = QTime(Duration / 3600, Duration / 60, Duration % 60, 0);
                list.append(row);
            } else if (stream.name().contains(QStringLiteral("Ramp"))) {
                uint32_t Duration = 1;
                double PowerLow = 1;
                double PowerHigh = 1;
                if (atts.hasAttribute(QStringLiteral("Duration"))) {
                    Duration = atts.value(QStringLiteral("Duration")).toUInt();
                }
                if (atts.hasAttribute(QStringLiteral("PowerLow"))) {
                    PowerLow = atts.value(QStringLiteral("PowerLow")).toDouble();
                }
                if (atts.hasAttribute(QStringLiteral("PowerHigh"))) {
                    PowerHigh = atts.value(QStringLiteral("PowerHigh")).toDouble();
                }

                for (uint32_t i = 0; i < Duration; i++) {
                    trainrow row;
                    row.duration = QTime(0, 0, 1, 0);
                    if (PowerHigh > PowerLow) {
                        row.power = (PowerLow + (((PowerHigh - PowerLow) / Duration) * i)) *
                                    settings.value(QStringLiteral("ftp"), 200.0).toDouble();
                    } else {
                        row.power = (PowerLow - (((PowerLow - PowerHigh) / Duration) * i)) *
                                    settings.value(QStringLiteral("ftp"), 200.0).toDouble();
                    }
                    list.append(row);
                }
            } else if (stream.name().contains(QStringLiteral("SteadyState"))) {
                uint32_t Duration = 1;
                double Power = 1;
                if (atts.hasAttribute(QStringLiteral("Duration"))) {
                    Duration = atts.value(QStringLiteral("Duration")).toUInt();
                }
                if (atts.hasAttribute(QStringLiteral("Power"))) {
                    Power = atts.value(QStringLiteral("Power")).toDouble();
                }

                trainrow row;
                row.duration = QTime(Duration / 3600, Duration / 60, Duration % 60, 0);
                row.power = Power * settings.value(QStringLiteral("ftp"), 200.0).toDouble();
                list.append(row);
            }
        }
    }
    return list;
}
