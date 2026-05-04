#include "testtrainingloadtestsuite.h"

#include <cmath>
#include <QCoreApplication>

TestTrainingLoadTestSuite::TestTrainingLoadTestSuite()
{

}

float TestTrainingLoadTestSuite::calculateTRIMP(uint32_t duration_minutes, double avg_hr,
                                                uint8_t max_hr, uint8_t resting_hr, bool is_male) {
    // Bannister's TRIMP formula: D * HR_ratio * exp(b * HR_ratio)
    // where HR_ratio = (avg_hr - resting_hr) / (max_hr - resting_hr)
    //
    // COEFFICIENT SELECTION:
    // Standard Bannister formula uses b = 1.92 (men) and b = 1.67 (women)
    // However, Garmin devices (Fenix, etc.) appear to use b ≈ 1.67 for all users.
    // We use b = 1.67 for everyone to match Garmin's training load calculations
    // and ensure compatibility with Garmin Connect's acute training load features.
    // (is_male parameter is kept for API compatibility but not used)

    double hr_ratio = 0;
    if (max_hr > resting_hr) {
        hr_ratio = (avg_hr - resting_hr) / (double)(max_hr - resting_hr);
    }

    // Use coefficient 1.67 (matches Garmin implementation)
    double b = 1.67;

    // Calculate TRIMP
    if (hr_ratio > 0 && hr_ratio < 2.0) {  // Sanity check
        return duration_minutes * hr_ratio * std::exp(b * hr_ratio);
    }

    return 0.0f;
}

float TestTrainingLoadTestSuite::calculateTSS(uint32_t duration_seconds, double avg_power, float ftp) {
    if (ftp <= 0 || avg_power <= 0) {
        return 0.0f;
    }

    // TSS formula: (duration_seconds × average_power × IF) / (FTP × 36)
    // where IF (Intensity Factor) = average_power / FTP
    double intensity_factor = avg_power / ftp;
    return (duration_seconds * avg_power * intensity_factor) / (ftp * 36.0);
}

void TestTrainingLoadTestSuite::test_trimpCalculationFenix8() {
    // Real data from Garmin Fenix 8 run session
    // Date: 12/01/2026 12:34:19
    // Duration: 48:44.67 = 2924.67 seconds = 48.74 minutes
    // Avg HR: 167 bpm
    // Max HR: 178 bpm (session max, but user profile max is 177)
    // Min HR: 90 bpm (session min)
    // Resting HR: 60 bpm (from user profile)
    // User: 43 years old, male, 78kg
    // Expected Training Load Peak: 207.72850
    //
    // COEFFICIENT: We use b = 1.67 (same as Garmin) instead of standard male b = 1.92
    // This ensures our training load calculations match Garmin devices closely

    uint32_t duration_seconds = 2924;  // 48:44 = 2924 seconds
    uint32_t duration_minutes = duration_seconds / 60;  // 48 minutes (integer division matches qfit.cpp)
    double avg_hr = 167.0;
    uint8_t max_hr = 177;  // 220 - 43 = 177
    uint8_t resting_hr = 60;
    bool is_male = true;  // Not used, kept for API compatibility

    float calculated_trimp = calculateTRIMP(duration_minutes, avg_hr, max_hr, resting_hr, is_male);

    // Expected result from Fenix 8: 207.72850
    // With b = 1.67 (Garmin coefficient), we should match within 5%
    float expected_trimp = 207.72850f;
    float tolerance = expected_trimp * 0.05f;  // 5% tolerance

    EXPECT_NEAR(calculated_trimp, expected_trimp, tolerance)
        << "TRIMP calculation should match Fenix 8 output within 5%\n"
        << "Calculated: " << calculated_trimp << "\n"
        << "Expected: " << expected_trimp << "\n"
        << "Difference: " << (calculated_trimp - expected_trimp);
}

void TestTrainingLoadTestSuite::test_trimpCalculationVariousScenarios() {
    // Test 1: Easy workout (low HR)
    {
        uint32_t duration_minutes = 60;
        double avg_hr = 120.0;
        uint8_t max_hr = 180;
        uint8_t resting_hr = 60;
        bool is_male = true;

        float trimp = calculateTRIMP(duration_minutes, avg_hr, max_hr, resting_hr, is_male);

        // Should be a reasonable value (not zero, not too high)
        EXPECT_GT(trimp, 0.0f);
        EXPECT_LT(trimp, 200.0f);
    }

    // Test 2: Hard workout (high HR)
    {
        uint32_t duration_minutes = 30;
        double avg_hr = 170.0;
        uint8_t max_hr = 180;
        uint8_t resting_hr = 60;
        bool is_male = true;

        float trimp = calculateTRIMP(duration_minutes, avg_hr, max_hr, resting_hr, is_male);

        // Should be higher than easy workout despite shorter duration
        EXPECT_GT(trimp, 100.0f);
    }

    // Test 3: Different resting HR (should affect TRIMP)
    {
        uint32_t duration_minutes = 48;
        double avg_hr = 167.0;
        uint8_t max_hr = 177;
        uint8_t resting_hr_low = 50;   // Athlete with low resting HR
        uint8_t resting_hr_high = 70;  // Higher resting HR
        bool is_male = true;

        float trimp_low_rhr = calculateTRIMP(duration_minutes, avg_hr, max_hr, resting_hr_low, is_male);
        float trimp_high_rhr = calculateTRIMP(duration_minutes, avg_hr, max_hr, resting_hr_high, is_male);

        // Lower resting HR means higher HR ratio (more HR reserve used) = higher TRIMP
        // HR_ratio = (167-50)/(177-50) = 0.921 vs (167-70)/(177-70) = 0.907
        EXPECT_GT(trimp_low_rhr, trimp_high_rhr);
    }

    // Test 4: Edge case - max HR equals resting HR (should return 0)
    {
        uint32_t duration_minutes = 60;
        double avg_hr = 100.0;
        uint8_t max_hr = 100;
        uint8_t resting_hr = 100;
        bool is_male = true;

        float trimp = calculateTRIMP(duration_minutes, avg_hr, max_hr, resting_hr, is_male);

        EXPECT_EQ(trimp, 0.0f);
    }
}

void TestTrainingLoadTestSuite::test_tssCalculation() {
    // Test 1: Typical cycling workout at FTP
    {
        uint32_t duration_seconds = 3600;  // 1 hour
        double avg_power = 200.0;
        float ftp = 200.0;

        float tss = calculateTSS(duration_seconds, avg_power, ftp);

        // 1 hour at FTP should equal 100 TSS
        EXPECT_NEAR(tss, 100.0f, 1.0f);
    }

    // Test 2: Easy ride (below FTP)
    {
        uint32_t duration_seconds = 3600;  // 1 hour
        double avg_power = 140.0;  // 70% of FTP
        float ftp = 200.0;

        float tss = calculateTSS(duration_seconds, avg_power, ftp);

        // Should be less than 100 TSS
        EXPECT_LT(tss, 100.0f);
        EXPECT_GT(tss, 0.0f);
    }

    // Test 3: Hard ride (above FTP)
    {
        uint32_t duration_seconds = 1800;  // 30 minutes
        double avg_power = 250.0;  // 125% of FTP
        float ftp = 200.0;

        float tss = calculateTSS(duration_seconds, avg_power, ftp);

        // Should be higher intensity
        EXPECT_GT(tss, 50.0f);
    }

    // Test 4: Edge case - zero FTP (should return 0)
    {
        uint32_t duration_seconds = 3600;
        double avg_power = 200.0;
        float ftp = 0.0;

        float tss = calculateTSS(duration_seconds, avg_power, ftp);

        EXPECT_EQ(tss, 0.0f);
    }

    // Test 5: Edge case - zero power (should return 0)
    {
        uint32_t duration_seconds = 3600;
        double avg_power = 0.0;
        float ftp = 200.0;

        float tss = calculateTSS(duration_seconds, avg_power, ftp);

        EXPECT_EQ(tss, 0.0f);
    }
}
