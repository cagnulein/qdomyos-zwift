#ifndef QFITTESTSUITE_H
#define QFITTESTSUITE_H

#include "gtest/gtest.h"
#include <QString>
#include <QTemporaryDir>
#include <QList>
#include "../../src/sessionline.h"
#include "../../src/devices/bluetoothdevice.h"

/**
 * @brief Test suite for qfit FIT file reading/writing
 *
 * Tests developer fields moved from Session to WorkoutMesg for better
 * Garmin Connect compatibility
 */
class QFitTestSuite: public testing::Test {

public:
    QFitTestSuite();
    ~QFitTestSuite() override;

    /**
     * @brief Test that FIT files with new format (developer fields in WorkoutMesg) can be read correctly
     */
    void test_newFormatDeveloperFields();

protected:
    void SetUp() override;
    void TearDown() override;

private:
    QTemporaryDir* tempDir;

    /**
     * @brief Create a sample session for testing
     */
    QList<SessionLine> createTestSession();

    /**
     * @brief Create a FIT file with developer fields in new format (WorkoutMesg)
     */
    QString createNewFormatFitFile();

    /**
     * @brief Verify developer fields were read correctly
     */
    bool verifyDeveloperFields(const QString& workoutName, const QString& workoutSource,
                               const QString& pelotonWorkoutId, const QString& pelotonUrl,
                               const QString& trainingProgramFile);
};

TEST_F(QFitTestSuite, TestNewFormatDeveloperFields) {
    this->test_newFormatDeveloperFields();
}

#endif // QFITTESTSUITE_H
