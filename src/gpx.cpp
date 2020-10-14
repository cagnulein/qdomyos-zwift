#include "gpx.h"
#include <QDomDocument>
#include <QDebug>
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
