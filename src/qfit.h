#ifndef QFIT_H
#define QFIT_H

#include <QObject>
#include <QFile>
#include <QTime>
#include <QGeoCoordinate>
#include "sessionline.h"
#include "bluetoothdevice.h"

class qfit : public QObject
{
    Q_OBJECT
public:
    explicit qfit(QObject *parent = nullptr);
    void save(QString filename, QList<SessionLine> session, bluetoothdevice::BLUETOOTH_TYPE type);

signals:

};

#endif // QFIT_H
