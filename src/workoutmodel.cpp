#include "workoutmodel.h"
#include "workoutloaderworker.h"
#include "homeform.h"
#include "trainprogram.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QSet>
#include <algorithm>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>

WorkoutModel::WorkoutModel(const QString& dbPath, QObject *parent)
    : QAbstractListModel(parent)
    , m_isLoading(false)
    , m_isDatabaseProcessing(true)
    , m_dbPath(dbPath)
    , m_currentStreak(0)
    , m_longestStreak(0)
    , m_streakMessage("")
    , m_isDateFiltered(false)
    , m_filteredDate()
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
    
    // Calculate initial streaks from existing data
    calculateStreaks();
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
    m_allWorkouts = workouts;
    
    // Apply current filter if active
    if (m_isDateFiltered) {
        applyDateFilter();
    } else {
        m_workouts = workouts;
    }
    
    endResetModel();

    // Calculate streaks after loading workouts (always use all workouts for streaks)
    calculateStreaks();

    m_isLoading = false;
    emit loadingStatusChanged();
}

bool WorkoutModel::isLoading() const {
    return m_isLoading;
}

bool WorkoutModel::isDatabaseProcessing() const {
    return m_isDatabaseProcessing;
}

void WorkoutModel::setDatabaseProcessing(bool processing) {
    if (m_isDatabaseProcessing != processing) {
        m_isDatabaseProcessing = processing;
        emit databaseProcessingChanged();
    }
}

int WorkoutModel::currentStreak() const {
    return m_currentStreak;
}

int WorkoutModel::longestStreak() const {
    return m_longestStreak;
}

QString WorkoutModel::streakMessage() const {
    return m_streakMessage;
}

bool WorkoutModel::deleteWorkout(int workoutId) {
    QSqlQuery query(m_db);

    // Get the file path before deleting
    query.prepare("SELECT file_path FROM workouts WHERE id = ?");
    query.addBindValue(workoutId);
    QString filePath;
    if (query.exec() && query.next()) {
        filePath = query.value("file_path").toString();
    }

    // Delete the workout record
    query.prepare("DELETE FROM workouts WHERE id = ?");
    query.addBindValue(workoutId);

    if (!query.exec()) {
        qDebug() << "Failed to delete workout:" << query.lastError().text();
        return false;
    }

    // Optionally, you could also delete the FIT file here if desired
    // if (!filePath.isEmpty()) {
    //     QFile::remove(filePath);
    // }

    // Refresh the model
    refresh();
    return true;
}

QVariantMap WorkoutModel::getWorkoutDetails(int workoutId) {
    QVariantMap details;

    // First get the summary data from database
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM workouts WHERE id = ?");
    query.addBindValue(workoutId);

    if (!query.exec() || !query.next()) {
        qDebug() << "Failed to get workout details:" << query.lastError().text();
        return details;
    }

    // Add file path to details
    details["filePath"] = query.value("file_path");  // Add this line

    // Fill in the summary data
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

    // Now load detailed data from the FIT file for charts
    QString filePath = query.value("file_path").toString();
    if (QFile::exists(filePath)) {
        QList<SessionLine> session;
        FIT_SPORT sport;
        qfit::open(filePath, &session, &sport);

        if (!session.isEmpty()) {
            QVariantList timestamps, heartRates, speeds, power, cadence;

            // Get first timestamp to calculate relative times
            qint64 startTime = session.first().time.toSecsSinceEpoch();

            for (const SessionLine& point : session) {
                // Convert elapsed time to minutes for x-axis
                double minutes = point.elapsedTime / 60.0;
                timestamps.append(minutes);
                heartRates.append(point.heart);
                speeds.append(point.speed);
                power.append(point.watt);
                cadence.append(point.cadence);
            }

            details["chartData"] = QVariantMap{
                {"timestamps", timestamps},
                {"heartRates", heartRates},
                {"speeds", speeds},
                {"power", power},
                {"cadence", cadence}
            };
        }
    } else {
        qDebug() << "FIT file not found:" << filePath;
        // Return empty chart data if file not found
        details["chartData"] = QVariantMap{
            {"timestamps", QVariantList()},
            {"heartRates", QVariantList()},
            {"speeds", QVariantList()},
            {"power", QVariantList()},
            {"cadence", QVariantList()}
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

bool WorkoutModel::isDateFiltered() const {
    return m_isDateFiltered;
}

QDate WorkoutModel::filteredDate() const {
    return m_filteredDate;
}

void WorkoutModel::setDateFilter(const QString& dateString) {
    QDate date = QDate::fromString(dateString, "yyyy-MM-dd");
    if (!date.isValid()) return;

    m_isDateFiltered = true;
    m_filteredDate = date;

    beginResetModel();
    applyDateFilter();
    endResetModel();

    emit dateFilterChanged();
}

void WorkoutModel::clearDateFilter() {
    if (!m_isDateFiltered) return;
    
    m_isDateFiltered = false;
    m_filteredDate = QDate();
    
    beginResetModel();
    m_workouts = m_allWorkouts;
    endResetModel();
    
    emit dateFilterChanged();
}

void WorkoutModel::applyDateFilter() {
    if (!m_isDateFiltered) {
        m_workouts = m_allWorkouts;
        return;
    }
    
    m_workouts.clear();
    QString targetDateStr = m_filteredDate.toString("yyyy-MM-dd");
    
    for (const QVariantMap& workout : m_allWorkouts) {
        QString workoutDateStr = workout["date"].toString();
        
        // Use the same logic as getWorkoutDates to extract the date
        QDateTime workoutDateTime = QDateTime::fromString(workoutDateStr, Qt::ISODate);
        QDate workoutDate;
        
        if (workoutDateTime.isValid()) {
            // Convert to local time to get the correct date
            workoutDate = workoutDateTime.toLocalTime().date();
        } else {
            // Try parsing as date only if datetime parsing fails
            workoutDate = QDate::fromString(workoutDateStr, "yyyy-MM-dd");
        }
        
        if (workoutDate.isValid() && workoutDate.toString("yyyy-MM-dd") == targetDateStr) {
            m_workouts.append(workout);
        }
    }
}

QStringList WorkoutModel::getWorkoutDates() {
    QStringList dates;
    QSet<QString> uniqueDates;
    
    for (const QVariantMap& workout : m_allWorkouts) {
        QString dateStr = workout["date"].toString();
        
        // Use QDateTime::toLocalTime() to avoid timezone issues
        QDateTime workoutDateTime = QDateTime::fromString(dateStr, Qt::ISODate);
        QDate workoutDate;
        
        if (workoutDateTime.isValid()) {
            // Convert to local time to get the correct date
            workoutDate = workoutDateTime.toLocalTime().date();
        } else {
            // Try parsing as date only if datetime parsing fails
            workoutDate = QDate::fromString(dateStr, "yyyy-MM-dd");
        }
        
        if (workoutDate.isValid()) {
            QString dateString = workoutDate.toString("yyyy-MM-dd");
            if (!uniqueDates.contains(dateString)) {
                uniqueDates.insert(dateString);
                dates.append(dateString);
            }
        }
    }
    
    std::sort(dates.begin(), dates.end());
    qDebug() << "getWorkoutDates returning:" << dates;
    return dates;
}

QString WorkoutModel::getWorkoutSource(int workoutId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT workout_source, training_program_file FROM workouts WHERE id = ?");
    query.addBindValue(workoutId);
    
    if (query.exec() && query.next()) {
        QString result = query.value("workout_source").toString();
        QString trainingProgramFile = query.value("training_program_file").toString();
        
        qDebug() << "WorkoutModel::getWorkoutSource for ID" << workoutId << "returned:" << result;
        qDebug() << "Training program file:" << trainingProgramFile;
        
        if (result.isEmpty()) {
            qDebug() << "Empty workout_source, using default QZ";
            result = "QZ";
        }
        
        // Check if this should be marked as ERG mode based on training program
        if (result == "QZ" && !trainingProgramFile.isEmpty() && trainprogram::hasTargetPower(trainingProgramFile)) {
            qDebug() << "Training program has target power > 0, returning ERG";
            return "ERG";
        }
        
        return result;
    }
    qDebug() << "WorkoutModel::getWorkoutSource query failed for ID" << workoutId << "- error:" << query.lastError().text();
    return "QZ"; // Default fallback
}

QString WorkoutModel::getPelotonUrl(int workoutId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT peloton_url FROM workouts WHERE id = ?");
    query.addBindValue(workoutId);
    
    if (query.exec() && query.next()) {
        return query.value("peloton_url").toString();
    }
    return "";
}

bool WorkoutModel::hasTrainingProgram(int workoutId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT training_program_file FROM workouts WHERE id = ?");
    query.addBindValue(workoutId);
    
    if (query.exec() && query.next()) {
        QString filename = query.value("training_program_file").toString();
        if (!filename.isEmpty()) {
            // Check if file actually exists
            return QFile::exists(filename);
        }
    }
    return false;
}

bool WorkoutModel::openPelotonUrl(int workoutId) {
    QString url = getPelotonUrl(workoutId);
    if (!url.isEmpty()) {
        return QDesktopServices::openUrl(QUrl(url));
    }
    return false;
}

bool WorkoutModel::loadTrainingProgram(int workoutId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT training_program_file FROM workouts WHERE id = ?");
    query.addBindValue(workoutId);
    
    if (query.exec() && query.next()) {
        QString filename = query.value("training_program_file").toString();
        if (!filename.isEmpty() && QFile::exists(filename)) {
            // Use homeform singleton to load the training program
            if (homeform::singleton()) {
                QUrl fileUrl = QUrl::fromLocalFile(filename);
                homeform::singleton()->trainprogram_open_clicked(fileUrl);
                qDebug() << "Training program opened:" << filename;
                return true;
            } else {
                qDebug() << "homeform singleton not available";
                return false;
            }
        }
    }
    return false;
}

void WorkoutModel::calculateStreaks() {
    QSqlQuery query(m_db);
    query.prepare("SELECT DISTINCT DATE(start_time) as workout_date FROM workouts "
                  "ORDER BY workout_date DESC");
    
    if (!query.exec()) {
        qDebug() << "Failed to calculate streaks:" << query.lastError().text();
        return;
    }

    QList<QDate> workoutDates;
    while (query.next()) {
        QDate date = query.value("workout_date").toDate();
        if (date.isValid()) {
            workoutDates.append(date);
        }
    }

    if (workoutDates.isEmpty()) {
        m_currentStreak = 0;
        m_longestStreak = 0;
        emit streakChanged();
        return;
    }

    // Calculate current streak (consecutive days from today backwards)
    QDate today = QDate::currentDate();
    int currentStreak = 0;
    
    // Check if today has a workout
    if (workoutDates.contains(today)) {
        currentStreak = 1;
        QDate checkDate = today.addDays(-1);
        
        while (workoutDates.contains(checkDate)) {
            currentStreak++;
            checkDate = checkDate.addDays(-1);
        }
    } else {
        // Check if yesterday has a workout (allow for timezone differences)
        QDate yesterday = today.addDays(-1);
        if (workoutDates.contains(yesterday)) {
            currentStreak = 1;
            QDate checkDate = yesterday.addDays(-1);
            
            while (workoutDates.contains(checkDate)) {
                currentStreak++;
                checkDate = checkDate.addDays(-1);
            }
        }
    }

    // Calculate longest streak ever
    int longestStreak = 0;
    int tempStreak = 1;
    
    if (!workoutDates.isEmpty()) {
        longestStreak = 1; // At least one workout exists
        
        for (int i = 1; i < workoutDates.size(); i++) {
            QDate currentDate = workoutDates[i];
            QDate previousDate = workoutDates[i-1];
            
            // Check if dates are consecutive (previous date should be exactly one day before current)
            if (currentDate.addDays(1) == previousDate) {
                tempStreak++;
                longestStreak = qMax(longestStreak, tempStreak);
            } else {
                tempStreak = 1;
            }
        }
    }

    // Update values and emit signal if changed
    QString newMessage = getStreakMessage(currentStreak);
    bool changed = (m_currentStreak != currentStreak || m_longestStreak != longestStreak || m_streakMessage != newMessage);
    m_currentStreak = currentStreak;
    m_longestStreak = longestStreak;
    m_streakMessage = newMessage;
    
    if (changed) {
        emit streakChanged();
    }
    
    qDebug() << "Streak calculation: Current=" << m_currentStreak << "Longest=" << m_longestStreak;
}

QString WorkoutModel::getStreakMessage(int streak) const {
    if (streak == 0) {
        return "Let's create a streak!";
    } else if (streak == 1) {
        return "Great start! 🚀";
    } else if (streak == 2) {
        return "Building momentum! 💪";
    } else if (streak == 3) {
        return "Three days strong! 🎯";
    } else if (streak >= 4 && streak <= 6) {
        return "Amazing dedication! ⭐";
    } else if (streak == 7) {
        return "One week warrior! 🏆";
    } else if (streak >= 8 && streak <= 13) {
        return "Unstoppable force! 🔥";
    } else if (streak == 14) {
        return "Two weeks champion! 👑";
    } else if (streak >= 15 && streak <= 20) {
        return "Legendary consistency! ⚡";
    } else if (streak >= 21 && streak <= 29) {
        return "Fitness machine! 🤖";
    } else if (streak == 30) {
        return "One month master! 🎊";
    } else if (streak >= 31 && streak <= 59) {
        return "Discipline incarnate! 💎";
    } else if (streak == 60) {
        return "Two months titan! 🏅";
    } else if (streak >= 61 && streak <= 89) {
        return "Workout legend! 🌟";
    } else if (streak == 90) {
        return "Three months beast! 🦁";
    } else if (streak >= 91 && streak <= 179) {
        return "Fitness deity! ✨";
    } else if (streak == 180) {
        return "Half-year hero! 🎖️";
    } else if (streak >= 181 && streak <= 364) {
        return "Unstoppable legend! 🔱";
    } else if (streak == 365) {
        return "FULL YEAR CHAMPION! 👑🔥";
    } else {
        return "Beyond legendary! 🌟✨";
    }
}
