#ifndef APPLICATION_PATHCONTROLLER_H
#define APPLICATION_PATHCONTROLLER_H
#include <QGeoPath>
#include <QGeoPositionInfoSource>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

    class PathController : public QObject {
    Q_OBJECT
  public:
    PathController(QObject *parent = 0);

    QGeoPath geoPath() const { return mGeoPath; }
    void setGeoPath(const QGeoPath &geoPath) {
        if (geoPath == mGeoPath) {
            return;
        }
        mGeoPath = geoPath;
        emit geopathChanged();
    }

    QGeoCoordinate center() const {
        return mCenter;
    }
    void setCenter(const QGeoCoordinate &center) {
        if (center == mCenter) {
            return;
        }
        mCenter = center;
        emit centerChanged();
    }

    Q_PROPERTY(QGeoPath geopath READ geoPath WRITE setGeoPath NOTIFY geopathChanged)
    Q_PROPERTY(QGeoCoordinate center READ center WRITE setCenter NOTIFY centerChanged)

  signals:
    void geopathChanged();
    void centerChanged();

  private:
    QGeoPath mGeoPath;
    QGeoCoordinate mCenter;
};
#endif // APPLICATION_PATHCONTROLLER_H
