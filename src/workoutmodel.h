#ifndef WORKOUTMODEL_H
#define WORKOUTMODEL_H

#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QThread>
#include <QDate>
#include "fitdatabaseprocessor.h"

class WorkoutLoaderWorker;

class WorkoutModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingStatusChanged)
    Q_PROPERTY(bool isDatabaseProcessing READ isDatabaseProcessing NOTIFY databaseProcessingChanged)
    Q_PROPERTY(int currentStreak READ currentStreak NOTIFY streakChanged)
    Q_PROPERTY(int longestStreak READ longestStreak NOTIFY streakChanged)
    Q_PROPERTY(QString streakMessage READ streakMessage NOTIFY streakChanged)
    Q_PROPERTY(bool isDateFiltered READ isDateFiltered NOTIFY dateFilterChanged)
    Q_PROPERTY(QDate filteredDate READ filteredDate NOTIFY dateFilterChanged)

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
    Q_INVOKABLE void setDateFilter(const QDate& date);
    Q_INVOKABLE void clearDateFilter();
    Q_INVOKABLE QList<QDate> getWorkoutDates();

    bool isLoading() const;
    bool isDatabaseProcessing() const;
    int currentStreak() const;
    int longestStreak() const;
    QString streakMessage() const;
    bool isDateFiltered() const;
    QDate filteredDate() const;

  public slots:
    void setDatabaseProcessing(bool processing);

  signals:
    void loadWorkoutsRequested();
    void loadingStatusChanged();
    void databaseProcessingChanged();
    void streakChanged();
    void dateFilterChanged();

  private slots:
    void onWorkoutsLoaded(const QList<QVariantMap>& workouts);

  private:
    void calculateStreaks();
    QString getStreakMessage(int streak) const;
    void applyDateFilter();
    
    QList<QVariantMap> m_workouts;
    QList<QVariantMap> m_allWorkouts;
    QSqlDatabase m_db;
    QThread* m_workerThread;
    WorkoutLoaderWorker* m_worker;
    bool m_isLoading;
    bool m_isDatabaseProcessing;
    QString m_dbPath;
    int m_currentStreak;
    int m_longestStreak;
    QString m_streakMessage;
    bool m_isDateFiltered;
    QDate m_filteredDate;
};

#endif // WORKOUTMODEL_H
