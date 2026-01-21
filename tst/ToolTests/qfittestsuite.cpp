#include "qfittestsuite.h"
#include "../../src/qfit.h"
#include "../../src/fitdatabaseprocessor.h"
#include <QDateTime>
#include <QFile>
#include <QDebug>

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

    // Verify all developer fields were read correctly from the new format (message 0xFF00)
    EXPECT_TRUE(verifyDeveloperFields(workoutName, workoutSource, pelotonWorkoutId,
                                      pelotonUrl, trainingProgramFile))
        << "Developer fields should be read correctly from new format (0xFF00)";

    qDebug() << "✓ New format developer fields test passed";
}

void QFitTestSuite::test_databaseReadability() {
    // Create a FIT file with new format
    QString filename = createNewFormatFitFile();
    ASSERT_TRUE(QFile::exists(filename)) << "Failed to create FIT file";

    // Create a FIT database processor
    FITDatabaseProcessor processor;

    // Create a temporary database
    QString dbPath = tempDir->filePath("test_db.sqlite");

    // Initialize the database
    bool dbInitialized = processor.initializeDatabase(dbPath);
    ASSERT_TRUE(dbInitialized) << "Failed to initialize test database";

    // Process the FIT file
    bool processed = false;
    QObject::connect(&processor, &FITDatabaseProcessor::fileProcessed,
                     [&processed](const QString&) { processed = true; });

    bool error = false;
    QObject::connect(&processor, &FITDatabaseProcessor::error,
                     [&error](const QString& msg) {
                         qDebug() << "Database processor error:" << msg;
                         error = true;
                     });

    processor.processFile(filename);

    // Wait briefly for async processing
    QCoreApplication::processEvents();

    EXPECT_TRUE(processed) << "FIT file should be processed successfully by database";
    EXPECT_FALSE(error) << "No errors should occur during database processing";

    qDebug() << "✓ Database readability test passed";
}
