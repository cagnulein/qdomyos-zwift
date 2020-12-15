#include "gpx.h"
#include <QDomDocument>
#include <QDebug>
#include <QXmlStreamWriter>
#include "math.h"

gpx::gpx(QObject *parent) : QObject(parent)
{

}

QList<gpx_altitude_point_for_treadmill> gpx::open(QString gpx)
{
    QFile input(gpx);
    input.open(QIODevice::ReadOnly);
    QDomDocument doc;
    doc.setContent(&input);
    QDomNodeList points = doc.elementsByTagName("trkpt");
    for (int i = 0; i < points.size(); i++)
    {
       QDomNode point = points.item(i);
       QDomNamedNodeMap att = point.attributes();
       QString lat = att.namedItem("lat").nodeValue();
       QString lon = att.namedItem("lon").nodeValue();
       QDomElement ele = point.firstChildElement("ele");
       QDomElement time = point.firstChildElement("time");
       gpx_point g;
       //2020-10-10T10:54:45
       g.time = QDateTime::fromString(time.text(), Qt::ISODate);
       g.p.setAltitude(ele.text().toFloat());
       g.p.setLatitude(lat.toFloat());
       g.p.setLongitude(lon.toFloat());
       this->points.append(g);
    }

    const uint8_t secondsInclination = 60;
    QList<gpx_altitude_point_for_treadmill> inclinationList;
    for(int32_t i=secondsInclination; i<this->points.count(); i+=secondsInclination)
    {
        double distance = this->points[i].p.distanceTo(this->points[i-secondsInclination].p);
        double elevation = this->points[i].p.altitude() - this->points[i-secondsInclination].p.altitude();

        gpx_altitude_point_for_treadmill g;
        g.seconds = secondsInclination;
        g.speed = (distance / 1000.0) * (3600 / secondsInclination);
        g.inclination = (elevation / distance) * 100;
        inclinationList.append(g);
    }
    return inclinationList;
}

void gpx::save(QString filename, QList<SessionLine> session)
{
    if(!session.length()) return;

    QFile output(filename);
    output.open(QIODevice::WriteOnly);
    QXmlStreamWriter stream(&output);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    // header
    stream.writeStartElement("gpx");
    stream.writeAttribute("creator", "qdomyos-zwift");
    stream.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    stream.writeAttribute("xsi:schemaLocation","http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd http://www.garmin.com/xmlschemas/GpxExtensions/v3 http://www.garmin.com/xmlschemas/GpxExtensionsv3.xsd http://www.garmin.com/xmlschemas/TrackPointExtension/v1 http://www.garmin.com/xmlschemas/TrackPointExtensionv1.xsd");
    stream.writeAttribute("version","1.1");
    stream.writeAttribute("xmlns", "http://www.topografix.com/GPX/1/1");
    stream.writeAttribute("xmlns:gpxtpx", "http://www.garmin.com/xmlschemas/TrackPointExtension/v1");
    stream.writeAttribute("xmlns:gpxx", "http://www.garmin.com/xmlschemas/GpxExtensions/v3");
    stream.writeEndElement();

    stream.writeStartElement("metadata");
       stream.writeTextElement("time", session.at(0).time.toString("yyyy-MM-ddTHH:mm:ssZ"));
    stream.writeEndElement();

    stream.writeStartElement("trk");
    stream.writeTextElement("name", session.at(0).time.toString("yyyy-MM-dd HH:mm:ss"));
    stream.writeTextElement("type", "53"); // replace with the current device type
       stream.writeStartElement("trkseg");
       foreach(SessionLine s, session)
       {
           if(s.speed > 0)
           {
               stream.writeStartElement("trkpt");
               stream.writeTextElement("ele", "0"); // replace with the cumulative inclination
               stream.writeTextElement("time", s.time.toString("yyyy-MM-ddTHH:mm:ssZ"));
               stream.writeStartElement("extensions");
               stream.writeStartElement("gpxtpx:TrackPointExtension");
               stream.writeTextElement("gpxtpx:hr", QString::number(s.heart));
               stream.writeTextElement("gpxtpx:cad", QString::number(s.cadence));
               stream.writeTextElement("gpxtpx:power", QString::number(s.watt));
               stream.writeTextElement("gpxtpx:distance", QString::number(s.distance));
               stream.writeEndElement(); //gpxtpx:TrackPointExtension
               stream.writeEndElement(); //extensions
               stream.writeEndElement(); //trkpt
           }
       }
       stream.writeEndElement(); //trkseg
    stream.writeEndElement(); //trk

    stream.writeEndDocument();
}
