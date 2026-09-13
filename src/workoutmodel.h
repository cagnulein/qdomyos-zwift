#ifndef WORKOUTMODEL_H
#define WORKOUTMODEL_H

#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QThread>
#include <QDate>
#include <QDateTime>
#include "fitdatabaseprocessor.h"
#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

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
    Q_INVOKABLE void setDateFilter(const QString& dateString);
    Q_INVOKABLE void clearDateFilter();
    Q_INVOKABLE QStringList getWorkoutDates();
    Q_INVOKABLE QString getWorkoutSource(int workoutId);
    Q_INVOKABLE QString getPelotonUrl(int workoutId);
    Q_INVOKABLE bool hasTrainingProgram(int workoutId);
    Q_INVOKABLE bool openPelotonUrl(int workoutId);
    Q_INVOKABLE bool loadTrainingProgram(int workoutId);

    Q_INVOKABLE bool canWriteAppleHealth(int workoutId) const {
#ifdef Q_OS_IOS
        QSqlQuery query(m_db);
        query.prepare("SELECT sport_type FROM workouts WHERE id = ?");
        query.addBindValue(workoutId);
        if (!query.exec() || !query.next()) {
            return false;
        }

        lockscreen ls;
        return ls.canWriteHistoricalWorkoutToHealthKit(static_cast<unsigned short>(query.value(0).toUInt()));
#else
        Q_UNUSED(workoutId);
        return false;
#endif
    }

    Q_INVOKABLE bool uploadWorkoutToAppleHealth(int workoutId) {
#ifdef Q_OS_IOS
        QSqlQuery query(m_db);
        query.prepare("SELECT sport_type, start_time, end_time, total_distance, total_calories FROM workouts WHERE id = ?");
        query.addBindValue(workoutId);
        if (!query.exec() || !query.next()) {
            return false;
        }

        const QDateTime start = query.value(1).toDateTime();
        const QDateTime end = query.value(2).toDateTime();
        if (!start.isValid() || !end.isValid() || end <= start) {
            return false;
        }

        const unsigned short sport = static_cast<unsigned short>(query.value(0).toUInt());
        lockscreen ls;
        if (!ls.canWriteHistoricalWorkoutToHealthKit(sport)) {
            return false;
        }

        return ls.saveHistoricalWorkoutToHealthKit(sport,
                                                    start.toMSecsSinceEpoch() / 1000.0,
                                                    end.toMSecsSinceEpoch() / 1000.0,
                                                    query.value(3).toDouble() * 1000.0,
                                                    query.value(4).toDouble());
#else
        Q_UNUSED(workoutId);
        return false;
#endif
    }

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
    QString makeAbsolutePath(const QString& relativePath) const;

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
