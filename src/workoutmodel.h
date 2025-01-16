#ifndef WORKOUTMODEL_H
#define WORKOUTMODEL_H

#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QThread>
#include "fitdatabaseprocessor.h"

class WorkoutLoaderWorker;

class WorkoutModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingStatusChanged)

  public:
    enum WorkoutRoles {
        SportRole = Qt::UserRole + 1,
        TitleRole,
        DateRole,
        DurationRole,
        DistanceRole,
        CaloriesRole,
        IdRole
    };

    explicit WorkoutModel(const QString& dbPath, QObject *parent = nullptr);
    ~WorkoutModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QVariantMap getWorkoutDetails(int workoutId);
    Q_INVOKABLE bool deleteWorkout(int workoutId);

    bool isLoading() const;

  signals:
    void loadWorkoutsRequested();
    void loadingStatusChanged();

  private slots:
    void onWorkoutsLoaded(const QList<QVariantMap>& workouts);

  private:
    QList<QVariantMap> m_workouts;
    QSqlDatabase m_db;
    QThread* m_workerThread;
    WorkoutLoaderWorker* m_worker;
    bool m_isLoading;
    QString m_dbPath;
};

#endif // WORKOUTMODEL_H
