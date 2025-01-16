#include "workoutmodel.h"
#include "workoutloaderworker.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

WorkoutModel::WorkoutModel(const QString& dbPath, QObject *parent)
    : QAbstractListModel(parent)
      , m_isLoading(false)
      , m_dbPath(dbPath)
{
    // Create main database connection
    {
        m_db = QSqlDatabase::addDatabase("QSQLITE", FitDatabaseProcessor::DB_CONNECTION_NAME + "_main");
        m_db.setDatabaseName(dbPath);
        if (!m_db.open()) {
            qDebug() << "Failed to open database in main thread:" << m_db.lastError().text();
            return;
        }
    }
    m_db = QSqlDatabase::database(FitDatabaseProcessor::DB_CONNECTION_NAME + "_main");

           // Create worker and move to thread
    m_workerThread = new QThread(this);
    m_worker = new WorkoutLoaderWorker(dbPath);

           // Connect signals/slots
    connect(m_workerThread, &QThread::finished, m_worker, &WorkoutLoaderWorker::cleanup);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &WorkoutModel::loadWorkoutsRequested, m_worker, &WorkoutLoaderWorker::loadWorkouts);
    connect(m_worker, &WorkoutLoaderWorker::loadingFinished, this, &WorkoutModel::onWorkoutsLoaded,
            Qt::QueuedConnection);

    m_worker->moveToThread(m_workerThread);

    m_workerThread->start();

    // Initial load
    refresh();
}

WorkoutModel::~WorkoutModel() {
    m_workerThread->quit();
    m_workerThread->wait();
    QSqlDatabase::removeDatabase(FitDatabaseProcessor::DB_CONNECTION_NAME + "_main");
}

void WorkoutModel::refresh() {
    if (m_isLoading) return;

    m_isLoading = true;
    emit loadingStatusChanged();
    emit loadWorkoutsRequested();
}

void WorkoutModel::onWorkoutsLoaded(const QList<QVariantMap>& workouts) {
    beginResetModel();
    m_workouts = workouts;
    endResetModel();

    m_isLoading = false;
    emit loadingStatusChanged();
}

bool WorkoutModel::isLoading() const {
    return m_isLoading;
}

bool WorkoutModel::deleteWorkout(int workoutId) {
    QSqlQuery query(m_db);

    // Start transaction
    m_db.transaction();

    // First delete workout details
    query.prepare("DELETE FROM workout_details WHERE workout_id = ?");
    query.addBindValue(workoutId);

    if (!query.exec()) {
        m_db.rollback();
        qDebug() << "Failed to delete workout details:" << query.lastError().text();
        return false;
    }

    // Then delete the workout
    query.prepare("DELETE FROM workouts WHERE id = ?");
    query.addBindValue(workoutId);

    if (!query.exec()) {
        m_db.rollback();
        qDebug() << "Failed to delete workout:" << query.lastError().text();
        return false;
    }

    if (!m_db.commit()) {
        m_db.rollback();
        qDebug() << "Failed to commit transaction:" << m_db.lastError().text();
        return false;
    }

    // Refresh the model
    refresh();
    return true;
}

QVariantMap WorkoutModel::getWorkoutDetails(int workoutId) {
    QVariantMap details;

    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM workouts WHERE id = ?");
    query.addBindValue(workoutId);

    if (!query.exec() || !query.next()) {
        qDebug() << "Failed to get workout details:" << query.lastError().text();
        return details;
    }

    details["id"] = query.value("id");
    details["sport"] = query.value("sport_type");
    details["startTime"] = query.value("start_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
    details["endTime"] = query.value("end_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
    details["duration"] = query.value("total_time");
    details["distance"] = query.value("total_distance");
    details["calories"] = query.value("total_calories");
    details["avgHeartRate"] = query.value("avg_heart_rate");
    details["maxHeartRate"] = query.value("max_heart_rate");
    details["avgCadence"] = query.value("avg_cadence");
    details["maxCadence"] = query.value("max_cadence");
    details["avgSpeed"] = query.value("avg_speed");
    details["maxSpeed"] = query.value("max_speed");
    details["avgPower"] = query.value("avg_power");
    details["maxPower"] = query.value("max_power");
    details["totalAscent"] = query.value("total_ascent");
    details["totalDescent"] = query.value("total_descent");

           // Get workout details for charts
    QSqlQuery detailsQuery(m_db);
    detailsQuery.prepare("SELECT timestamp, elapsed_time, heart_rate, speed, power, cadence "
                         "FROM workout_details WHERE workout_id = ? "
                         "ORDER BY timestamp");
    detailsQuery.addBindValue(workoutId);

    if (detailsQuery.exec()) {
        QVariantList timestamps, heartRates, speeds, power, cadence;

        while (detailsQuery.next()) {
            // Convert elapsed time to minutes for x-axis
            double minutes = detailsQuery.value("elapsed_time").toDouble() / 60.0;
            timestamps.append(minutes);
            heartRates.append(detailsQuery.value("heart_rate"));
            speeds.append(detailsQuery.value("speed"));
            power.append(detailsQuery.value("power"));
            cadence.append(detailsQuery.value("cadence"));
        }

        details["chartData"] = QVariantMap{
            {"timestamps", timestamps},
            {"heartRates", heartRates},
            {"speeds", speeds},
            {"power", power},
            {"cadence", cadence}
        };
    }

    return details;
}

int WorkoutModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return m_workouts.count();
}

QVariant WorkoutModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_workouts.count())
        return QVariant();

    const QVariantMap &workout = m_workouts[index.row()];

    switch (role) {
    case SportRole:
        return workout["sport"];
    case TitleRole:
        return workout["title"];
    case DateRole:
        return workout["date"];
    case DurationRole:
        return workout["duration"];
    case DistanceRole:
        return workout["distance"];
    case CaloriesRole:
        return workout["calories"];
    case IdRole:
        return workout["id"];
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> WorkoutModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[SportRole] = "sport";
    roles[TitleRole] = "title";
    roles[DateRole] = "date";
    roles[DurationRole] = "duration";
    roles[DistanceRole] = "distance";
    roles[CaloriesRole] = "calories";
    roles[IdRole] = "id";
    return roles;
}
