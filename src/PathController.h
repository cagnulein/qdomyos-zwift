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

    double distance() const {
        return mDistance;
    }
    void setDistance(double distance) {
        if (qFuzzyCompare(distance, mDistance)) {
            return;
        }
        mDistance = distance;
        emit distanceChanged();
    }

    Q_PROPERTY(QGeoPath geopath READ geoPath WRITE setGeoPath NOTIFY geopathChanged)
    Q_PROPERTY(QGeoCoordinate center READ center WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(double distance READ distance WRITE setDistance NOTIFY distanceChanged)

signals:
    void geopathChanged();
    void centerChanged();
    void distanceChanged();

private:
    QGeoPath mGeoPath;
    QGeoCoordinate mCenter;
    double mDistance = 0.0;
};

#endif // APPLICATION_PATHCONTROLLER_H
