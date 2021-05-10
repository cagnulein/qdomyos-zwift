#ifndef QFIT_H
#define QFIT_H

#include <QObject>
#include <QFile>
#include <QTime>
#include <QGeoCoordinate>
#include "sessionline.h"
#include "bluetoothdevice.h"

#define QFIT_PROCESS_NONE 0
#define QFIT_PROCESS_DISTANCENOISE 1

class qfit : public QObject
{
    Q_OBJECT
public:
    explicit qfit(QObject *parent = nullptr);
    static void save(QString filename, QList<SessionLine> session, bluetoothdevice::BLUETOOTH_TYPE type, uint32_t processFlag = QFIT_PROCESS_NONE);

signals:

};

#endif // QFIT_H
