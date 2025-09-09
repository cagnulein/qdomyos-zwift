// fitbackupwriter.h
#ifndef FITBACKUPWRITER_H
#define FITBACKUPWRITER_H

#include <QObject>
#include <QFile>
#include "sessionline.h"
#include "fit_profile.hpp"
#include "qfit.h"
#include "bluetoothdevice.h"

class FitBackupWriter : public QObject {
    Q_OBJECT
public:
    explicit FitBackupWriter(QObject *parent = nullptr);
    virtual ~FitBackupWriter();

public slots:
    void writeFitBackup(const QString &filename, 
                       const QList<SessionLine> &session,
                       bluetoothdevice::BLUETOOTH_TYPE deviceType,
                       QFIT_PROCESS_TYPE processType,
                       FIT_SPORT workoutType,
                       const QString &workoutName,
                       const QString &deviceName);
};

#endif // FITBACKUPWRITER_H