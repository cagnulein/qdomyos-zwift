#include "qfittestsuite.h"
#include "../../src/qfit.h"
#include "../../src/fitdatabaseprocessor.h"
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

QFitTestSuite::QFitTestSuite() : tempDir(nullptr) {
}

QFitTestSuite::~QFitTestSuite() {
    if (tempDir) {
        delete tempDir;
        tempDir = nullptr;
    }
}

void QFitTestSuite::SetUp() {
    tempDir = new QTemporaryDir();
    ASSERT_TRUE(tempDir->isValid()) << "Failed to create temporary directory";
}

void QFitTestSuite::TearDown() {
    if (tempDir) {
        delete tempDir;
        tempDir = nullptr;
    }
}

QList<SessionLine> QFitTestSuite::createTestSession() {
    QList<SessionLine> session;
    QDateTime startTime = QDateTime::currentDateTime();

    // Create a simple 10-minute workout session
    for (int i = 0; i < 600; i += 5) {  // 5 second intervals for 10 minutes
        SessionLine line;
        line.time = startTime.addSecs(i);
        line.elapsedTime = i;
        line.distance = i * 0.05;  // 3 km/h = 0.05 km per 5 seconds
        line.speed = 3.0;  // 3 km/h
        line.cadence = 60;
        line.heart = 120 + (i % 30);  // Varying HR between 120-150
        line.calories = i / 10;
        line.watt = 100;

        session.append(line);
    }

    return session;
}

QString QFitTestSuite::createNewFormatFitFile() {
    QString filename = tempDir->filePath("test_new_format.fit");
    QList<SessionLine> session = createTestSession();

    // Create a FIT file with developer fields
    qfit::save(filename, session, BIKE, QFIT_PROCESS_NONE, FIT_SPORT_CYCLING,
               "Test Workout Title",
               "Test Device",
               "PELOTON",
               "test_workout_id_123",
               "https://peloton.com/workout/123",
               "/path/to/training.zwo");

    return filename;
}

bool QFitTestSuite::verifyDeveloperFields(const QString& workoutName, const QString& workoutSource,
                                          const QString& pelotonWorkoutId, const QString& pelotonUrl,
                                          const QString& trainingProgramFile) {
    bool allCorrect = true;

    if (workoutName != "Test Workout Title") {
        qDebug() << "Workout name mismatch. Expected: 'Test Workout Title', Got:" << workoutName;
        allCorrect = false;
    }

    if (workoutSource != "PELOTON") {
        qDebug() << "Workout source mismatch. Expected: 'PELOTON', Got:" << workoutSource;
        allCorrect = false;
    }

    if (pelotonWorkoutId != "test_workout_id_123") {
        qDebug() << "Peloton workout ID mismatch. Expected: 'test_workout_id_123', Got:" << pelotonWorkoutId;
        allCorrect = false;
    }

    if (pelotonUrl != "https://peloton.com/workout/123") {
        qDebug() << "Peloton URL mismatch. Expected: 'https://peloton.com/workout/123', Got:" << pelotonUrl;
        allCorrect = false;
    }

    if (trainingProgramFile != "/path/to/training.zwo") {
        qDebug() << "Training program file mismatch. Expected: '/path/to/training.zwo', Got:" << trainingProgramFile;
        allCorrect = false;
    }

    return allCorrect;
}

void QFitTestSuite::test_newFormatDeveloperFields() {
    // Create a FIT file with new format
    QString filename = createNewFormatFitFile();
    ASSERT_TRUE(QFile::exists(filename)) << "Failed to create FIT file";

    // Copy to test-artifacts directory for download
    QDir artifactsDir("test-artifacts");
    if (!artifactsDir.exists()) {
        artifactsDir.mkpath(".");
    }
    QString artifactPath = "test-artifacts/test_new_format.fit";
    QFile::remove(artifactPath);
    QFile::copy(filename, artifactPath);
    qDebug() << "FIT file saved to:" << artifactPath;

    // Read the file back
    QList<SessionLine> session;
    FIT_SPORT sport = FIT_SPORT_INVALID;
    QString workoutName;
    QString workoutSource;
    QString pelotonWorkoutId;
    QString pelotonUrl;
    QString trainingProgramFile;

    qfit::open(filename, &session, &sport, &workoutName, &workoutSource,
               &pelotonWorkoutId, &pelotonUrl, &trainingProgramFile);

    // Verify basic data was read
    EXPECT_FALSE(session.isEmpty()) << "Session should not be empty";
    EXPECT_EQ(sport, FIT_SPORT_CYCLING) << "Sport should be cycling";

    // Verify all developer fields were read correctly from WorkoutMesg
    EXPECT_TRUE(verifyDeveloperFields(workoutName, workoutSource, pelotonWorkoutId,
                                      pelotonUrl, trainingProgramFile))
        << "Developer fields should be read correctly from WorkoutMesg";

    qDebug() << "✓ New format developer fields test passed";
}

void QFitTestSuite::test_databaseReadability() {
    // Create a FIT file with new format
    QString filename = createNewFormatFitFile();
    ASSERT_TRUE(QFile::exists(filename)) << "Failed to create FIT file";

    // Copy to test-artifacts directory for download
    QDir artifactsDir("test-artifacts");
    if (!artifactsDir.exists()) {
        artifactsDir.mkpath(".");
    }
    QString artifactPath = "test-artifacts/test_database_readability.fit";
    QFile::remove(artifactPath);
    QFile::copy(filename, artifactPath);
    qDebug() << "FIT file saved to:" << artifactPath;

    // Create a temporary database path
    QString dbPath = tempDir->filePath("test_db.sqlite");

    // Create a FIT database processor with the database path
    FitDatabaseProcessor processor(dbPath);

    // Setup event loop to wait for async processing
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(5000);  // 5 second timeout

    // Process the FIT file
    bool processed = false;
    QObject::connect(&processor, &FitDatabaseProcessor::fileProcessed,
                     [&processed, &loop](const QString&) {
                         processed = true;
                         loop.quit();
                     });

    bool error = false;
    QString errorMsg;
    QObject::connect(&processor, &FitDatabaseProcessor::error,
                     [&error, &errorMsg, &loop](const QString& msg) {
                         qDebug() << "Database processor error:" << msg;
                         error = true;
                         errorMsg = msg;
                         loop.quit();
                     });

    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    processor.processFile(filename);
    timeout.start();

    // Wait for processing to complete or timeout
    loop.exec();
    timeout.stop();

    EXPECT_TRUE(processed) << "FIT file should be processed successfully by database";
    EXPECT_FALSE(error) << "No errors should occur during database processing. Error: "
                        << errorMsg.toStdString();

    // Copy database file to test-artifacts directory for download
    if (QFile::exists(dbPath)) {
        QString dbArtifactPath = "test-artifacts/test_database.sqlite";
        QFile::remove(dbArtifactPath);
        QFile::copy(dbPath, dbArtifactPath);
        qDebug() << "Database file saved to:" << dbArtifactPath;
    }

    qDebug() << "✓ Database readability test passed";
}
