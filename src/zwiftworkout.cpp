#include "zwiftworkout.h"
#include <QXmlStreamReader>

QList<trainrow> zwiftworkout::load(QString filename)
{
    QSettings settings;
    QList<trainrow> list;
    QFile input(filename);
    input.open(QIODevice::ReadOnly);
    QXmlStreamReader stream(&input);
    while(!stream.atEnd())
    {
        stream.readNext();
        QXmlStreamAttributes atts = stream.attributes();
        if(atts.length())
        {
            if(stream.name().contains("IntervalsT"))
            {
                uint32_t repeat = 1;
                uint32_t OnDuration = 1;
                uint32_t OffDuration = 1;
                double OnPower = 1;
                double OffPower = 1;
                if(atts.hasAttribute("Repeat"))
                {
                    repeat = atts.value("Repeat").toUInt();
                }
                if(atts.hasAttribute("OnDuration"))
                {
                    OnDuration = atts.value("OnDuration").toUInt();
                }
                if(atts.hasAttribute("OffDuration"))
                {
                    OffDuration = atts.value("OffDuration").toUInt();
                }
                if(atts.hasAttribute("OnPower"))
                {
                    OnPower = atts.value("OnPower").toDouble();
                }
                if(atts.hasAttribute("OffPower"))
                {
                    OffPower = atts.value("OffPower").toDouble();
                }

                for(uint32_t i=0; i<repeat; i++)
                {
                    trainrow row;
                    row.duration = QTime(OnDuration / 3600, OnDuration / 60, OnDuration % 60, 0);
                    row.power = OnPower * settings.value("ftp", 200.0).toDouble();
                    list.append(row);
                    row.duration = QTime(OffDuration / 3600, OffDuration / 60, OffDuration % 60, 0);
                    row.power = OffPower * settings.value("ftp", 200.0).toDouble();
                    list.append(row);
                }
            }
            else if(stream.name().contains("FreeRide"))
            {
                uint32_t Duration = 1;
                double FlatRoad = 1;
                if(atts.hasAttribute("Duration"))
                {
                    Duration = atts.value("Duration").toUInt();
                }
                if(atts.hasAttribute("FlatRoad"))
                {
                    FlatRoad = atts.value("FlatRoad").toDouble();
                }

                trainrow row;
                row.duration = QTime(Duration / 3600, Duration / 60, Duration % 60, 0);
                list.append(row);
            }
            else if(stream.name().contains("Ramp"))
            {
                uint32_t Duration = 1;
                double PowerLow = 1;
                double PowerHigh = 1;
                if(atts.hasAttribute("Duration"))
                {
                    Duration = atts.value("Duration").toUInt();
                }
                if(atts.hasAttribute("PowerLow"))
                {
                    PowerLow = atts.value("PowerLow").toDouble();
                }
                if(atts.hasAttribute("PowerHigh"))
                {
                    PowerHigh = atts.value("PowerHigh").toDouble();
                }

                for(uint32_t i=0; i<Duration; i++)
                {
                    trainrow row;
                    row.duration = QTime(0, 0, 1, 0);
                    if(PowerHigh > PowerLow)
                        row.power = (PowerLow + (((PowerHigh - PowerLow) / Duration) * i)) * settings.value("ftp", 200.0).toDouble();
                    else
                        row.power = (PowerLow - (((PowerLow - PowerHigh) / Duration) * i)) * settings.value("ftp", 200.0).toDouble();
                    list.append(row);
                }
            }
            else if(stream.name().contains("SteadyState"))
            {
                uint32_t Duration = 1;
                double Power = 1;
                if(atts.hasAttribute("Duration"))
                {
                    Duration = atts.value("Duration").toUInt();
                }
                if(atts.hasAttribute("Power"))
                {
                    Power = atts.value("Power").toDouble();
                }

                trainrow row;
                row.duration = QTime(Duration / 3600, Duration / 60, Duration % 60, 0);
                row.power = Power * settings.value("ftp", 200.0).toDouble();
                list.append(row);
            }
        }
    }
    return list;
}
