#include "fitdatabaseprocessor.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDirIterator>
#include <QSqlDatabase>
#include <QDateTime>

const QString FitDatabaseProcessor::DB_CONNECTION_NAME = "FitProcessor";

FitDatabaseProcessor::FitDatabaseProcessor(const QString& dbPath, QObject* parent)
    : QObject(parent)
      , dbPath(dbPath)
      , stopRequested(0)
{
    moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, this, &QObject::deleteLater);
}

FitDatabaseProcessor::~FitDatabaseProcessor() {
    stopProcessing();
    workerThread.wait();

    QSqlDatabase::removeDatabase(DB_CONNECTION_NAME);
}

bool FitDatabaseProcessor::initializeDatabase() {
    QMutexLocker locker(&mutex);

    if (QSqlDatabase::contains(DB_CONNECTION_NAME)) {
        db = QSqlDatabase::database(DB_CONNECTION_NAME);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", DB_CONNECTION_NAME);
        db.setDatabaseName(dbPath);
    }

    if (!db.open()) {
        emit error("Failed to open database: " + db.lastError().text());
        return false;
    }

           // Start transaction for table creation
    db.transaction();

    QSqlQuery query(db);

    // Create workouts table
    if (!query.exec("CREATE TABLE IF NOT EXISTS workouts ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "file_hash TEXT UNIQUE,"
                    "file_path TEXT,"
                    "sport_type INTEGER,"
                    "start_time DATETIME,"
                    "end_time DATETIME,"
                    "total_time INTEGER,"  // in seconds
                    "total_distance REAL,"  // in km
                    "total_calories INTEGER,"
                    "avg_heart_rate INTEGER,"
                    "max_heart_rate INTEGER,"
                    "avg_cadence INTEGER,"
                    "max_cadence INTEGER,"
                    "avg_speed REAL,"
                    "max_speed REAL,"
                    "avg_power INTEGER,"
                    "max_power INTEGER,"
                    "total_ascent REAL,"
                    "total_descent REAL,"
                    "avg_stride_length REAL,"
                    "total_strides INTEGER,"
                    "processed_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                    ")")) {
        db.rollback();
        emit error("Failed to create workouts table: " + query.lastError().text());
        return false;
    }

           // Create workout_details table for point-by-point data
    if (!query.exec("CREATE TABLE IF NOT EXISTS workout_details ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "workout_id INTEGER,"
                    "timestamp DATETIME,"
                    "elapsed_time INTEGER,"  // in seconds
                    "distance REAL,"  // in km
                    "heart_rate INTEGER,"
                    "cadence INTEGER,"
                    "speed REAL,"
                    "power INTEGER,"
                    "resistance INTEGER,"
                    "calories INTEGER,"
                    "stride_length REAL,"
                    "vertical_oscillation REAL,"
                    "ground_contact_time INTEGER,"
                    "elevation REAL,"
                    "latitude REAL,"
                    "longitude REAL,"
                    "elevation_gain REAL,"
                    "temperature INTEGER,"
                    "target_cadence REAL,"
                    "target_power REAL,"
                    "target_resistance REAL,"
                    "FOREIGN KEY(workout_id) REFERENCES workouts(id)"
                    ")")) {
        db.rollback();
        emit error("Failed to create workout_details table: " + query.lastError().text());
        return false;
    }

           // Create indexes for better performance
    query.exec("CREATE INDEX IF NOT EXISTS idx_workout_start_time ON workouts(start_time)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_workout_details_workout_id ON workout_details(workout_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_workout_details_timestamp ON workout_details(timestamp)");

    return db.commit();
}

void FitDatabaseProcessor::processDirectory(const QString& dirPath) {
    currentDirPath = dirPath;
    stopRequested.storeRelease(0);

    if (!workerThread.isRunning()) {
        connect(&workerThread, &QThread::started, this, &FitDatabaseProcessor::doWork);
        workerThread.start();
    }
}

void FitDatabaseProcessor::stopProcessing() {
    stopRequested.storeRelease(1);
    workerThread.quit();
}

QString FitDatabaseProcessor::getFileHash(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        return QString();
    }

    return hash.result().toHex();
}

bool FitDatabaseProcessor::isFileProcessed(const QString& filePath) {
    QString fileHash = getFileHash(filePath);
    if (fileHash.isEmpty()) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM workouts WHERE file_hash = ?");
    query.addBindValue(fileHash);

    if (!query.exec() || !query.next()) {
        return false;
    }

    return query.value(0).toInt() > 0;
}

bool FitDatabaseProcessor::saveWorkout(const QString& filePath,
                                       const QList<SessionLine>& session,
                                       FIT_SPORT sport,
                                       qint64& workoutId) {
    if (session.isEmpty()) {
        return false;
    }

    QString fileHash = getFileHash(filePath);
    if (fileHash.isEmpty()) {
        return false;
    }

           // Calculate aggregate values
    double totalDistance = session.last().distance - session.first().distance;
    int maxHr = 0, totalHr = 0, hrCount = 0;
    int maxCadence = 0, totalCadence = 0, cadenceCount = 0;
    double maxSpeed = 0, totalSpeed = 0, speedCount = 0;
    int maxPower = 0, totalPower = 0, powerCount = 0;
    double totalAscent = 0, totalDescent = 0;
    double lastElevation = session.first().coordinate.altitude();

    for (const SessionLine& point : session) {
        // Heart rate
        if (point.heart > 0) {
            maxHr = qMax(maxHr, static_cast<int>(point.heart));
            totalHr += point.heart;
            hrCount++;
        }

               // Cadence
        if (point.cadence > 0) {
            maxCadence = qMax(maxCadence, static_cast<int>(point.cadence));
            totalCadence += point.cadence;
            cadenceCount++;
        }

               // Speed
        if (point.speed > 0) {
            maxSpeed = qMax(maxSpeed, point.speed);
            totalSpeed += point.speed;
            speedCount++;
        }

               // Power
        if (point.watt > 0) {
            maxPower = qMax(maxPower, static_cast<int>(point.watt));
            totalPower += point.watt;
            powerCount++;
        }

               // Elevation changes
        if (point.coordinate.isValid()) {
            double currentElevation = point.coordinate.altitude();
            if (lastElevation > 0) {
                double diff = currentElevation - lastElevation;
                if (diff > 0) totalAscent += diff;
                else totalDescent += qAbs(diff);
            }
            lastElevation = currentElevation;
        }
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO workouts ("
                  "file_hash, file_path, sport_type, start_time, end_time, "
                  "total_time, total_distance, total_calories, "
                  "avg_heart_rate, max_heart_rate, avg_cadence, max_cadence, "
                  "avg_speed, max_speed, avg_power, max_power, "
                  "total_ascent, total_descent, avg_stride_length, total_strides"
                  ") VALUES ("
                  "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?"
                  ")");

    query.addBindValue(fileHash);
    query.addBindValue(filePath);
    query.addBindValue(static_cast<int>(sport));
    query.addBindValue(session.first().time);
    query.addBindValue(session.last().time);
    query.addBindValue(session.last().elapsedTime);
    query.addBindValue(totalDistance);
    query.addBindValue(session.last().calories);
    query.addBindValue(hrCount > 0 ? totalHr / hrCount : 0);
    query.addBindValue(maxHr);
    query.addBindValue(cadenceCount > 0 ? totalCadence / cadenceCount : 0);
    query.addBindValue(maxCadence);
    query.addBindValue(speedCount > 0 ? totalSpeed / speedCount : 0);
    query.addBindValue(maxSpeed);
    query.addBindValue(powerCount > 0 ? totalPower / powerCount : 0);
    query.addBindValue(maxPower);
    query.addBindValue(totalAscent);
    query.addBindValue(totalDescent);
    query.addBindValue(session.last().instantaneousStrideLengthCM);
    query.addBindValue(session.last().stepCount);

    if (!query.exec()) {
        emit error("Failed to save workout: " + query.lastError().text());
        return false;
    }

    workoutId = query.lastInsertId().toLongLong();
    return true;
}


bool FitDatabaseProcessor::saveWorkoutDetails(qint64 workoutId, const QList<SessionLine>& session) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO workout_details ("
                  "workout_id, timestamp, elapsed_time, distance, heart_rate, "
                  "cadence, speed, power, resistance, calories, stride_length, "
                  "vertical_oscillation, ground_contact_time, elevation, "
                  "latitude, longitude, elevation_gain, target_cadence, "
                  "target_power, target_resistance"
                  ") VALUES ("
                  "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?"
                  ")");

    for (const SessionLine& point : session) {
        query.addBindValue(workoutId);
        query.addBindValue(point.time);
        query.addBindValue(point.elapsedTime);
        query.addBindValue(point.distance);
        query.addBindValue(point.heart);
        query.addBindValue(point.cadence);
        query.addBindValue(point.speed);
        query.addBindValue(point.watt);
        query.addBindValue(point.resistance);
        query.addBindValue(point.calories);
        query.addBindValue(point.instantaneousStrideLengthCM);
        query.addBindValue(point.verticalOscillationMM);
        query.addBindValue(point.groundContactMS);
        query.addBindValue(point.coordinate.isValid() ? point.coordinate.altitude() : 0);
        query.addBindValue(point.coordinate.isValid() ? point.coordinate.latitude() : 0);
        query.addBindValue(point.coordinate.isValid() ? point.coordinate.longitude() : 0);
        query.addBindValue(point.elevationGain);
        query.addBindValue(point.target_cadence);
        query.addBindValue(point.target_watt);
        query.addBindValue(point.target_resistance);

        if (!query.exec()) {
            emit error("Failed to save workout detail: " + query.lastError().text());
            return false;
        }
    }

    return true;
}

bool FitDatabaseProcessor::processFitFile(const QString& filePath) {
    if (isFileProcessed(filePath)) {
        return true;
    }

    QList<SessionLine> session;
    FIT_SPORT sport = FIT_SPORT_INVALID;

    try {
        qfit::open(filePath, &session, &sport);

        if (session.isEmpty()) {
            emit error("No data found in file: " + filePath);
            return false;
        }

        db.transaction();

        qint64 workoutId;
        if (!saveWorkout(filePath, session, sport, workoutId)) {
            db.rollback();
            return false;
        }

        if (!saveWorkoutDetails(workoutId, session)) {
            db.rollback();
            return false;
        }

        return db.commit();

    } catch (const std::exception& e) {
        emit error(QString("Error processing file %1: %2").arg(filePath, e.what()));
        db.rollback();
        return false;
    }
}

void FitDatabaseProcessor::doWork() {
    if (!initializeDatabase()) {
        return;
    }

    QDir dir(currentDirPath);
    QStringList fitFiles = dir.entryList(QStringList() << "*.fit" << "*.FIT", QDir::Files);
    int totalFiles = fitFiles.size();
    int processedFiles = 0;

    for (const QString& fileName : fitFiles) {
        if (stopRequested.loadAcquire()) {
            break;
        }

        QString filePath = dir.absoluteFilePath(fileName);

        if (processFitFile(filePath)) {
            emit fileProcessed(fileName);
        }

        processedFiles++;
        emit progress(processedFiles, totalFiles);
    }

    emit processingStopped();
}
