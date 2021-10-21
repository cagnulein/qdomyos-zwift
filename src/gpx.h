#ifndef GPX_H
#define GPX_H

#include "bluetoothdevice.h"
#include "sessionline.h"
#include <QFile>
#include <QGeoCoordinate>
#include <QObject>
#include <QTime>

class gpx_altitude_point_for_treadmill {
  public:
    uint32_t seconds;
    float inclination;
    float speed;
    double latitude;
    double longitude;
};

class gpx_point {
  public:
    QDateTime time;
    QGeoCoordinate p;
};

class gpx : public QObject {
    Q_OBJECT
  public:
    explicit gpx(QObject *parent = nullptr);
    QList<gpx_altitude_point_for_treadmill> open(const QString &gpx);
    static void save(const QString &filename, QList<SessionLine> session, bluetoothdevice::BLUETOOTH_TYPE type);

  private:
    QList<gpx_point> points;

  signals:
};

#endif // GPX_H
