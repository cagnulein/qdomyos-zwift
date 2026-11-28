#include "PathController.h"
#include <QtDebug>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

PathController::PathController(QObject *parent)
    : QObject(parent)
{
#ifdef Q_OS_ANDROID
    // QtPositioning may synchronously request missing Location permission and process
    // pending BLE events before homeform has connected the deviceConnected signal.
    const QJniObject context = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "getContext", "()Landroid/content/Context;");
    const QJniObject permission = QJniObject::fromString(QStringLiteral("android.permission.ACCESS_FINE_LOCATION"));
    const jint granted = QJniObject::callStaticMethod<jint>("androidx/core/content/ContextCompat", "checkSelfPermission",
        "(Landroid/content/Context;Ljava/lang/String;)I", context.object(), permission.object<jstring>());
    if (granted != 0) {
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
