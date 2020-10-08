#include "trainprogram.h"
#include <QFile>
#include <QtXml/QtXml>

extern volatile double requestSpeed;
extern volatile double requestIncline;
extern volatile int8_t requestStart;
extern volatile int8_t requestStop;

trainprogram::trainprogram(QList<trainrow> rows)
{
    this->rows = rows;
}

void trainprogram::scheduler(int tick)
{
    // entry point
    if(ticks == 0 && currentStep == 0)
    {
        if(rows[0].forcespeed && rows[0].speed)
        {
            requestSpeed = rows[0].speed;
        }
        requestIncline = rows[0].inclination;
    }

    ticks++;
    elapsed = ticks / (1000 / tick);
    ticksCurrentRow++;
    elapsedCurrentRow =  ticksCurrentRow / (1000 / tick);

    uint32_t currentRowLen = rows[currentStep].duration.second() +
            (rows[currentStep].duration.minute() * 60) +
            (rows[currentStep].duration.hour() * 3600);

    uint32_t nextRowLen = 0;

    if(rows.count() > currentStep + 1)
        nextRowLen = rows[currentStep + 1].duration.second() +
            (rows[currentStep + 1].duration.minute() * 60) +
            (rows[currentStep + 1].duration.hour() * 3600);

    if(elapsedCurrentRow >= currentRowLen && currentRowLen)
    {
        if(nextRowLen)
        {
            currentStep++;
            ticksCurrentRow = 0;
            elapsedCurrentRow = 0;
            if(rows[currentStep].forcespeed && rows[currentStep].speed)
            {
                requestSpeed = rows[currentStep].speed;
            }
            requestIncline = rows[currentStep].inclination;
        }
        else
        {
            requestStop = 1;
            restart();
        }
    }
}

void trainprogram::restart()
{
    ticks = 0;
    ticksCurrentRow = 0;
    elapsed = 0;
    elapsedCurrentRow = 0;
    currentStep = 0;
}

void trainprogram::save(QString filename)
{
    QFile output(filename);
    output.open(QIODevice::WriteOnly);
    QXmlStreamWriter stream(&output);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    foreach (trainrow row, rows) {
        stream.writeStartElement("row");
        stream.writeAttribute("duration", row.duration.toString());
        stream.writeAttribute("speed", QString::number(row.speed));
        stream.writeAttribute("inclination", QString::number(row.inclination));
        stream.writeAttribute("forcespeed", row.forcespeed?"1":"0");
        stream.writeEndElement(); // bookmark
    }
    stream.writeEndDocument();
}

trainprogram* trainprogram::load(QString filename)
{
    QList<trainrow> list;
    QFile input(filename);
    input.open(QIODevice::ReadOnly);
    QXmlStreamReader stream(&input);
    while(!stream.atEnd())
    {
        stream.readNext();
        trainrow row;
        QXmlStreamAttributes atts = stream.attributes();
        if(atts.length())
        {
            row.duration = QTime::fromString(atts.value("duration").toString(), "hh:mm:ss");
            row.speed = atts.value("speed").toDouble();
            row.inclination = atts.value("inclination").toDouble();
            row.forcespeed = atts.value("forcespeed").toInt()?true:false ;
            list.append(row);
        }
    }
    trainprogram *t = new trainprogram(list);
    return t;
}
