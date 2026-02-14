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
    // Extract base path from database path (remove the database filename)
    QFileInfo dbInfo(dbPath);
    basePath = dbInfo.absolutePath();
    if (!basePath.endsWith('/')) {
        basePath += '/';
    }

    moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, this, &QObject::deleteLater);
}

FitDatabaseProcessor::~FitDatabaseProcessor() {
    stopProcessing();
    workerThread.wait();

    QSqlDatabase::removeDatabase(DB_CONNECTION_NAME);
}

bool FitDatabaseProcessor::initializeDatabase() {
    // Limit the scope of the mutex locker to avoid deadlock with migrateOldPaths()
    {
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

        // Create workouts table - Only storing summary data
        if (!query.exec("CREATE TABLE IF NOT EXISTS workouts ("
                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "file_hash TEXT UNIQUE,"
                        "file_path TEXT,"
                        "workout_name TEXT,"
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
                        "workout_source TEXT DEFAULT 'QZ',"
                        "peloton_workout_id TEXT,"
                        "peloton_url TEXT,"
                        "training_program_file TEXT,"
                        "processed_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                        ")")) {
            db.rollback();
            emit error("Failed to create workouts table: " + query.lastError().text());
            return false;
        }

        // Create index for better performance
        query.exec("CREATE INDEX IF NOT EXISTS idx_workout_start_time ON workouts(start_time)");

        // Add workout_name column if it doesn't exist (for existing databases)
        query.exec("ALTER TABLE workouts ADD COLUMN workout_name TEXT");

        // Add new Peloton-related columns if they don't exist (for existing databases)
        query.exec("ALTER TABLE workouts ADD COLUMN workout_source TEXT DEFAULT 'QZ'");
        query.exec("ALTER TABLE workouts ADD COLUMN peloton_workout_id TEXT");
        query.exec("ALTER TABLE workouts ADD COLUMN peloton_url TEXT");
        query.exec("ALTER TABLE workouts ADD COLUMN training_program_file TEXT");

        if (!db.commit()) {
            emit error("Failed to commit database initialization: " + db.lastError().text());
            return false;
        }
    } // Mutex is released here

    // Migrate old absolute paths to relative paths (after mutex is released)
    qDebug() << "Checking for old absolute paths to migrate...";
    migrateOldPaths();

    return true;
}

void FitDatabaseProcessor::processDirectory(const QString& dirPath) {
    currentDirPath = dirPath;
    stopRequested.storeRelease(0);

    if (!workerThread.isRunning()) {
        connect(&workerThread, &QThread::started, this, &FitDatabaseProcessor::doWork);
        workerThread.start();
    }
}

void FitDatabaseProcessor::processFile(const QString& filePath) {
    if (!db.isOpen()) {
        if (!initializeDatabase()) {
            emit error("Failed to initialize database for single file processing");
            return;
        }
    }

    if (!processFitFile(filePath)) {
        emit error(QString("Failed to process file: %1").arg(filePath));
        return;
    }

    emit fileProcessed(filePath);
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
                                       const QString& workoutName,
                                       int elapsedSeconds,
                                       qint64& workoutId,
                                       const QString& workoutSource,
                                       const QString& pelotonWorkoutId,
                                       const QString& pelotonUrl,
                                       const QString& trainingProgramFile) {
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

    // Convert absolute path to relative path for storage
    QString relativeFilePath = makeRelativePath(filePath);

    QSqlQuery query(db);
    query.prepare("INSERT INTO workouts ("
                  "file_hash, file_path, workout_name, sport_type, start_time, end_time, "
                  "total_time, total_distance, total_calories, "
                  "avg_heart_rate, max_heart_rate, avg_cadence, max_cadence, "
                  "avg_speed, max_speed, avg_power, max_power, "
                  "total_ascent, total_descent, avg_stride_length, total_strides, "
                  "workout_source, peloton_workout_id, peloton_url, training_program_file"
                  ") VALUES ("
                  "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?"
                  ")");

    query.addBindValue(fileHash);
    query.addBindValue(relativeFilePath);  // Store relative path instead of absolute
    query.addBindValue(workoutName);
    query.addBindValue(static_cast<int>(sport));
    query.addBindValue(session.first().time);
    query.addBindValue(session.last().time);
    query.addBindValue(elapsedSeconds);
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
    query.addBindValue(workoutSource);
    query.addBindValue(pelotonWorkoutId.isEmpty() ? QVariant() : pelotonWorkoutId);
    query.addBindValue(pelotonUrl.isEmpty() ? QVariant() : pelotonUrl);
    query.addBindValue(trainingProgramFile.isEmpty() ? QVariant() : trainingProgramFile);

    if (!query.exec()) {
        emit error("Failed to save workout: " + query.lastError().text());
        return false;
    }

    workoutId = query.lastInsertId().toLongLong();
    return true;
}

bool FitDatabaseProcessor::processFitFile(const QString& filePath) {
    if (isFileProcessed(filePath)) {
        return true;
    }

    QList<SessionLine> session;
    FIT_SPORT sport = FIT_SPORT_INVALID;
    QString workoutName = "";  // Initialize to empty string
    QString workoutSource = "";
    QString pelotonWorkoutId = "";
    QString pelotonUrl = "";
    QString trainingProgramFile = "";

    try {
        qfit::open(filePath, &session, &sport, &workoutName, &workoutSource, &pelotonWorkoutId, &pelotonUrl, &trainingProgramFile);

        if (session.isEmpty()) {
            emit error("No data found in file: " + filePath);
            return false;
        }

        // Debug logging
        qDebug() << "Processing FIT file:" << filePath;
        qDebug() << "Sport type detected:" << static_cast<int>(sport);
        qDebug() << "Session duration (elapsedTime):" << session.last().elapsedTime;
        qDebug() << "Workout name from FIT:" << workoutName;

        // Validate elapsed time (should be reasonable, between 1 minute and 24 hours)
        int elapsedSeconds = session.last().elapsedTime;
        if (elapsedSeconds < 60 || elapsedSeconds > 86400) {
            qDebug() << "Warning: Unusual elapsed time detected:" << elapsedSeconds << "seconds. Using session duration calculation.";
            // Calculate duration from first to last record
            elapsedSeconds = session.first().time.secsTo(session.last().time);
            if (elapsedSeconds < 60 || elapsedSeconds > 86400) {
                qDebug() << "Warning: Still unusual duration. Setting to 1 minute minimum.";
                elapsedSeconds = qMax(60, qMin(86400, elapsedSeconds));
            }
        }

        // Generate fallback workout name based on sport and duration if not found in FIT file
        if (workoutName.isEmpty()) {
            QString sportName;
            switch (sport) {
            case FIT_SPORT_RUNNING:
            case FIT_SPORT_WALKING:
                sportName = "Run";
                break;
            case FIT_SPORT_CYCLING:
                sportName = "Ride";
                break;
            case FIT_SPORT_FITNESS_EQUIPMENT:
                sportName = "Elliptical";
                break;
            case FIT_SPORT_ROWING:
                sportName = "Row";
                break;
            default:
                sportName = "Workout";
                qDebug() << "Unknown sport type, using default. Sport value:" << static_cast<int>(sport);
                break;
            }
            
            int totalMinutes = elapsedSeconds / 60;
            workoutName = QString("%1 minutes %2").arg(totalMinutes).arg(sportName);
            qDebug() << "Generated fallback workout name:" << workoutName;
        }

        db.transaction();

        qint64 workoutId;
        if (!saveWorkout(filePath, session, sport, workoutName, elapsedSeconds, workoutId, workoutSource, pelotonWorkoutId, pelotonUrl, trainingProgramFile)) {
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

QString FitDatabaseProcessor::makeRelativePath(const QString& absolutePath) {
    // Convert absolute path to relative path
    // Remove the base path from the absolute path to get a relative path
    if (absolutePath.startsWith(basePath)) {
        return absolutePath.mid(basePath.length());
    }

    // If the path doesn't start with basePath, try to extract just the fit/filename part
    // This handles cases where the Application Container ID has changed on iOS
    int fitIndex = absolutePath.indexOf("/fit/");
    if (fitIndex != -1) {
        // Extract everything from "fit/" onwards
        return absolutePath.mid(fitIndex + 1); // +1 to skip the leading '/'
    }

    // Fallback: return the original path if we can't make it relative
    return absolutePath;
}

QString FitDatabaseProcessor::makeAbsolutePath(const QString& relativePath) {
    // Convert relative path to absolute path
    // If already absolute, return as-is
    if (QFileInfo(relativePath).isAbsolute()) {
        return relativePath;
    }

    // Construct absolute path by prepending basePath
    return basePath + relativePath;
}

bool FitDatabaseProcessor::migrateOldPaths() {
    // Migrate old absolute paths to new relative paths
    QMutexLocker locker(&mutex);

    if (!db.isOpen()) {
        qDebug() << "Database not open for path migration";
        return false;
    }

    // Start a transaction for better performance
    db.transaction();

    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT id, file_path FROM workouts");

    if (!selectQuery.exec()) {
        qDebug() << "Failed to query workouts for migration:" << selectQuery.lastError().text();
        db.rollback();
        return false;
    }

    QSqlQuery updateQuery(db);
    updateQuery.prepare("UPDATE workouts SET file_path = ? WHERE id = ?");

    int migratedCount = 0;
    while (selectQuery.next()) {
        qint64 id = selectQuery.value(0).toLongLong();
        QString oldPath = selectQuery.value(1).toString();

        // Check if path is already relative (doesn't start with /)
        if (!oldPath.startsWith('/')) {
            continue; // Already relative, skip
        }

        // Convert to relative path
        QString newPath = makeRelativePath(oldPath);

        // Only update if path changed
        if (newPath != oldPath) {
            updateQuery.addBindValue(newPath);
            updateQuery.addBindValue(id);

            if (!updateQuery.exec()) {
                qDebug() << "Failed to update path for workout ID" << id << ":" << updateQuery.lastError().text();
                db.rollback();
                return false;
            }

            migratedCount++;
        }
    }

    if (!db.commit()) {
        qDebug() << "Failed to commit path migration transaction:" << db.lastError().text();
        db.rollback();
        return false;
    }

    qDebug() << "Successfully migrated" << migratedCount << "workout paths to relative format";
    return true;
}
