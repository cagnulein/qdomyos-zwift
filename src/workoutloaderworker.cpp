// workoutloaderworker.cpp
#include "workoutloaderworker.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDebug>
#include <QDateTime>

WorkoutLoaderWorker::WorkoutLoaderWorker(const QString& dbPath, QObject *parent)
    : QObject(parent)
      , m_dbPath(dbPath)
{
}

void WorkoutLoaderWorker::loadWorkouts() {
    // Create database connection for this thread
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", FitDatabaseProcessor::DB_CONNECTION_NAME + "_worker");
        db.setDatabaseName(m_dbPath);
        if (!db.open()) {
            qDebug() << "Failed to open database in worker:" << db.lastError().text();
            emit loadingFinished(QList<QVariantMap>());
            return;
        }
    }

    QList<QVariantMap> workouts;
    QSqlDatabase db = QSqlDatabase::database(FitDatabaseProcessor::DB_CONNECTION_NAME + "_worker");
    QSqlQuery query(db);
    query.prepare("SELECT id, sport_type, start_time, total_time, "
                  "total_distance, total_calories FROM workouts "
                  "ORDER BY start_time DESC");

    if (!query.exec()) {
        qDebug() << "Failed to load workouts:" << query.lastError().text();
        emit loadingFinished(workouts);
        return;
    }

    while (query.next()) {
        QVariantMap workout;
        workout["id"] = query.value("id");
        workout["sport"] = query.value("sport_type");

        // Format the date
        QDateTime dateTime = query.value("start_time").toDateTime();
        workout["date"] = dateTime.toString("yyyy-MM-dd HH:mm");

        // Convert total_time (seconds) to formatted duration
        int totalSeconds = query.value("total_time").toInt();
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        workout["duration"] = QString("%1:%2:%3")
                                  .arg(hours, 2, 10, QLatin1Char('0'))
                                  .arg(minutes, 2, 10, QLatin1Char('0'))
                                  .arg(seconds, 2, 10, QLatin1Char('0'));

        workout["distance"] = query.value("total_distance").toDouble();
        workout["calories"] = query.value("total_calories");

               // Generate title based on sport type
        QString sportName;
        switch (query.value("sport_type").toInt()) {
        case 0: sportName = "Treadmill"; break;
        case 1: sportName = "Bike"; break;
        case 2: sportName = "Elliptical"; break;
        case 3: sportName = "Rowing"; break;
        default: sportName = "Workout"; break;
        }
        workout["title"] = QString("%1 Workout").arg(sportName);

        workouts.append(workout);
    }

    emit loadingFinished(workouts);
}

void WorkoutLoaderWorker::cleanup() {
    QSqlDatabase::removeDatabase(FitDatabaseProcessor::DB_CONNECTION_NAME + "_worker");
}
