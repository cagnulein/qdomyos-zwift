#include "trainprogram.h"
#include <QFile>
#include <QtXml/QtXml>

trainprogram::trainprogram(QList<trainrow> rows)
{
    this->rows = rows;
}

void trainprogram::scheduler(int tick)
{
    Q_ASSERT(tick);

    ticks++;
    elapsed = ticks / (1000 / tick);
    ticksCurrentRow++;
    elapsedCurrentRow =  ticksCurrentRow / (1000 / tick);

    if(rows.count() == 0 || started == false || enabled == false) return;

    // entry point
    if(ticks == 1 && currentStep == 0)
    {
        if(rows[0].forcespeed && rows[0].speed)
        {
            qDebug() << "trainprogram change speed" + QString::number(rows[0].speed);
            emit changeSpeedAndInclination(rows[0].speed, rows[0].inclination);
        }
        else
        {
            qDebug() << "trainprogram change inclination" + QString::number(rows[0].inclination);
            emit changeInclination(rows[0].inclination);
        }
    }

    uint32_t currentRowLen = rows[currentStep].duration.second() +
            (rows[currentStep].duration.minute() * 60) +
            (rows[currentStep].duration.hour() * 3600);

    uint32_t nextRowLen = 0;

    if(rows.count() > currentStep + 1)
        nextRowLen = rows[currentStep + 1].duration.second() +
            (rows[currentStep + 1].duration.minute() * 60) +
            (rows[currentStep + 1].duration.hour() * 3600);

    qDebug() << "trainprogram elapsed current row" + QString::number(elapsedCurrentRow) + "current row len" + QString::number(currentRowLen);

    if(elapsedCurrentRow >= currentRowLen && currentRowLen)
    {
        if(nextRowLen)
        {
            currentStep++;
            ticksCurrentRow = 0;
            elapsedCurrentRow = 0;
            if(rows[currentStep].forcespeed && rows[currentStep].speed)
            {
                qDebug() << "trainprogram change speed" + QString::number(rows[currentStep].speed);
                emit changeSpeedAndInclination(rows[currentStep].speed, rows[currentStep].inclination);
            }
            qDebug() << "trainprogram change inclination" + QString::number(rows[currentStep].inclination);
            emit changeInclination(rows[currentStep].inclination);
        }
        else
        {
            qDebug() << "trainprogram ends!";
            started = false;
            emit stop();
        }
    }
}

void trainprogram::onTapeStarted()
{
    started = true;
}

void trainprogram::restart()
{
    ticks = 0;
    ticksCurrentRow = 0;
    elapsed = 0;
    elapsedCurrentRow = 0;
    currentStep = 0;
    started = true;
}

void trainprogram::save(QString filename)
{
    QFile output(filename);
    output.open(QIODevice::WriteOnly);
    QXmlStreamWriter stream(&output);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("rows");
    foreach (trainrow row, rows) {
        stream.writeStartElement("row");
        stream.writeAttribute("duration", row.duration.toString());
        stream.writeAttribute("speed", QString::number(row.speed));
        stream.writeAttribute("inclination", QString::number(row.inclination));
        stream.writeAttribute("forcespeed", row.forcespeed?"1":"0");
        stream.writeEndElement();
    }
    stream.writeEndElement();
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
    trainprogram *tr = new trainprogram(list);
    return tr;
}

QTime trainprogram::totalElapsedTime()
{
    return QTime(0,0,elapsed);
}

QTime trainprogram::currentRowElapsedTime()
{
    return QTime(0,0,elapsedCurrentRow);
}

QTime trainprogram::duration()
{
    QTime total(0,0,0,0);
    foreach (trainrow row, rows) {
        total = total.addSecs((row.duration.hour() * 3600) + (row.duration.minute() * 60) + row.duration.second());
    }
    return total;
}
