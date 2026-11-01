
#include <wobjectimpl.h>

#include "PathController.h"

#include <QtDebug>
#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

W_OBJECT_IMPL(PathController)

PathController::PathController(QObject *parent)
: QObject(parent)
{
#ifdef Q_OS_ANDROID
    // QtPositioning may synchronously request missing Location permission and process
    // pending BLE events before homeform has connected the deviceConnected signal.
    if (QtAndroid::checkPermission(QStringLiteral("android.permission.ACCESS_FINE_LOCATION")) !=
        QtAndroid::PermissionResult::Granted) {
        qDebug() << "PathController: skip lastKnownPosition without Location permission";
        return;
    }
#endif

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
            // The Trixter X-Dream bike using a Prolific PL2303HXA Serial to USB converter
            // is identified by QGeoPositionInfoSource in Qt 5.15.2 as a source, but it delivers
            // invalid data.

            qDebug() << "Last known coordinate was not valid. Ignoring device.";
            delete source;
        }
    }
}



