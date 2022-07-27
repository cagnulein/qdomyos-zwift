
#include <wobjectimpl.h>

#include "PathController.h"

#include <QtDebug>

W_OBJECT_IMPL(PathController)

PathController::PathController(QObject *parent)
: QObject(parent)
{
    QGeoPositionInfoSource *source = QGeoPositionInfoSource::createDefaultSource(this);
    if (source) {
        QGeoCoordinate const coordinate = source->lastKnownPosition().coordinate();
        this->setCenter(coordinate);
    }
}
