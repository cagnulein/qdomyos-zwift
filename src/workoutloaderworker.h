// workoutloaderworker.h
#ifndef WORKOUTLOADERWORKER_H
#define WORKOUTLOADERWORKER_H

#include <QObject>
#include <QVariantMap>
#include "fitdatabaseprocessor.h"

class WorkoutLoaderWorker : public QObject {
    Q_OBJECT
  public:
    explicit WorkoutLoaderWorker(const QString& dbPath, QObject *parent = nullptr);

  public slots:
    void loadWorkouts();
    void cleanup();

  signals:
    void loadingFinished(const QList<QVariantMap>& workouts);

  private:
    QString m_dbPath;
};

#endif // WORKOUTLOADERWORKER_H
