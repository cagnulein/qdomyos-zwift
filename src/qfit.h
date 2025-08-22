#ifndef QFIT_H
#define QFIT_H

#include "devices/bluetoothdevice.h"
#include "fit_profile.hpp"
#include "sessionline.h"
#include <QFile>
#include <QGeoCoordinate>
#include <QObject>
#include <QTime>

#define QFIT_PROCESS_NONE 0
#define QFIT_PROCESS_DISTANCENOISE 1

class qfit : public QObject {
    Q_OBJECT
  public:
    explicit qfit(QObject *parent = nullptr);
    static void save(const QString &filename, QList<SessionLine> session, bluetoothdevice::BLUETOOTH_TYPE type,
                     uint32_t processFlag = QFIT_PROCESS_NONE, FIT_SPORT overrideSport = FIT_SPORT_INVALID, QString workoutName = "", QString bluetooth_device_name = "",
                     QString workoutSource = "", QString pelotonWorkoutId = "", QString pelotonUrl = "", QString trainingProgramFile = "");
    static void open(const QString &filename, QList<SessionLine>* output, FIT_SPORT *sport);
    static void open(const QString &filename, QList<SessionLine>* output, FIT_SPORT *sport, QString *workoutName);
    static void open(const QString &filename, QList<SessionLine>* output, FIT_SPORT *sport, QString *workoutName, 
                     QString *workoutSource, QString *pelotonWorkoutId, QString *pelotonUrl, QString *trainingProgramFile);
    
  signals:
};

#endif // QFIT_H
