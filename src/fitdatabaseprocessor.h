#ifndef FITDATABASEPROCESSOR_H
#define FITDATABASEPROCESSOR_H

#include <QObject>
#include <QThread>
#include <QSqlDatabase>
#include <QString>
#include <QDir>
#include <QMutex>
#include <QAtomicInt>
#include "qfit.h"

class FitDatabaseProcessor : public QObject {
    Q_OBJECT

  public:
    explicit FitDatabaseProcessor(const QString& dbPath, QObject* parent = nullptr);
    ~FitDatabaseProcessor();

    void processDirectory(const QString& dirPath);
    void processFile(const QString& filePath);
    void stopProcessing();

    static const QString DB_CONNECTION_NAME;

  signals:
    void processingStopped();
    void fileProcessed(const QString& filename);
    void progress(int processedFiles, int totalFiles);
    void error(const QString& errorMessage);

  private slots:
    void doWork();

  private:
    bool initializeDatabase();
    bool processFitFile(const QString& filePath);
    bool isFileProcessed(const QString& filePath);
    QString getFileHash(const QString& filePath);

    // Method for handling workout summary data
    bool saveWorkout(const QString& filePath,
                     const QList<SessionLine>& session,
                     FIT_SPORT sport,
                     const QString& workoutName,
                     int elapsedSeconds,
                     qint64& workoutId,
                     const QString& workoutSource = "QZ",
                     const QString& pelotonWorkoutId = "",
                     const QString& pelotonUrl = "",
                     const QString& trainingProgramFile = "");

    // Helper methods for path handling
    QString makeRelativePath(const QString& absolutePath);
    QString makeAbsolutePath(const QString& relativePath);
    bool migrateOldPaths();

    QThread workerThread;
    QString dbPath;
    QString currentDirPath;
    QString basePath;  // Base directory for relative paths
    QAtomicInt stopRequested;
    QMutex mutex;
    QSqlDatabase db;
};

#endif // FITDATABASEPROCESSOR_H
