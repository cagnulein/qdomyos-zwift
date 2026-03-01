#include "PathController.h"
#include <QtDebug>
PathController::PathController(QObject *parent)
    : QObject(parent)
{
    QGeoPositionInfoSource *source = QGeoPositionInfoSource::createDefaultSource(this);
    if (source) {
        auto sourceName = source->sourceName();
        auto objectName = source->objectName();
        qDebug() << "Found QGeoPositionInfoSource: sourceName:" << sourceName
                 << ", objectName:"<<objectName;
        QGeoCoordinate const coordinate = source->lastKnownPosition().coordinate();
        if(coordinate.isValid()) {
            this->setCenter(coordinate);
        } else {
            // This has been known to happen.
            // The Trixter X-Dream V1 bike using a Prolific PL2303HXA Serial to USB converter
            // is identified by QGeoPositionInfoSource in Qt 5.15.2 as a source, but it delivers
            // invalid data.
            qDebug() << "Last known coordinate was not valid. Ignoring device.";
            delete source;
        }
    }
}
