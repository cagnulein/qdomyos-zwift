#include "trainprogram.h"
#include <QFile>
#include <QtXml/QtXml>

trainprogram::trainprogram(QList<trainrow> rows)
{
    this->rows = rows;
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
