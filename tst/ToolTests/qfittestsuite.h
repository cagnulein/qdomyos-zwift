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
 * Tests backward compatibility for developer fields that were moved from
 * session message to custom developer data message (0xFF00)
 */
class QFitTestSuite: public testing::Test {

public:
    QFitTestSuite();
    ~QFitTestSuite() override;

    /**
     * @brief Test that FIT files with new format (developer fields in 0xFF00) can be read correctly
     */
    void test_newFormatDeveloperFields();

    /**
     * @brief Test that old FIT files with developer fields in session still work (backward compatibility)
     */
    void test_backwardCompatibilityOldFormat();

    /**
     * @brief Test that FIT files can be read by FIT database processor
     */
    void test_databaseReadability();

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
     * @brief Create a FIT file with developer fields in new format (0xFF00)
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

TEST_F(QFitTestSuite, TestDatabaseReadability) {
    this->test_databaseReadability();
}

#endif // QFITTESTSUITE_H
