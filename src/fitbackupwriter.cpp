#include "fitbackupwriter.h"
#include <QDebug>

FitBackupWriter::FitBackupWriter(QObject *parent) : QObject(parent) {
}

FitBackupWriter::~FitBackupWriter() {
}

void FitBackupWriter::writeFitBackup(const QString &filename, 
                                    const QList<SessionLine> &session,
                                    bluetoothdevice::BLUETOOTH_TYPE deviceType,
                                    uint32_t processType,
                                    FIT_SPORT workoutType,
                                    const QString &workoutName,
                                    const QString &deviceName) {
    qDebug() << QStringLiteral("Writing FIT backup file in background thread: ") << filename;
    
    // Remove existing file
    QFile::remove(filename);
    
    // Save FIT file using the same logic as the original backup() method
    qfit::save(filename, session, deviceType, processType, workoutType, 
               workoutName, deviceName, "", "", "", "");
    
    qDebug() << QStringLiteral("FIT backup file written successfully: ") << filename;
}
