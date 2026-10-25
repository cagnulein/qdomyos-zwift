#pragma once

#include "gtest/gtest.h"
#include "ergtable.h"


class ErgTableTestSuite: public testing::Test {
protected:

    void test_wattageEstimation(const QList<ergDataPoint>& inputs, const QList<ergDataPoint>& expectedOutputs);
public:
    ErgTableTestSuite();

    // Sets up the test fixture.
    //void SetUp() override;

    // Tears down the test fixture.
    // void TearDown() override;

    /**
     * @brief Test the overall functionality of an ergtable object
     */
    void test_dynamicErgTable();
    void test_resistanceCadenceBanding();
    void test_pairedPowerTable();
    void test_legacyPowerTableLoading();
    void test_pairedPowerTableRejectsDistantResistance();
    void test_pairedPowerTableUsesPairedDeltaMedian();
    void test_pairedPowerTableWaitsForStableTarget();
    void test_pairedPowerTableRejectsRudyInstability();
    void test_pairedPowerTableRejectsExcessiveStableDelta();
    void test_pairedPowerTableIgnoresPersistedExcessiveDelta();

};

TEST_F(ErgTableTestSuite, TestDynamicErgTable) {
    this->test_dynamicErgTable();
}

TEST_F(ErgTableTestSuite, TestResistanceCadenceBanding) {
    this->test_resistanceCadenceBanding();
}

TEST_F(ErgTableTestSuite, TestPairedPowerTable) {
    this->test_pairedPowerTable();
}

TEST_F(ErgTableTestSuite, TestLegacyPowerTableLoading) {
    this->test_legacyPowerTableLoading();
}

TEST_F(ErgTableTestSuite, TestPairedPowerTableRejectsDistantResistance) {
    this->test_pairedPowerTableRejectsDistantResistance();
}

TEST_F(ErgTableTestSuite, TestPairedPowerTableUsesPairedDeltaMedian) {
    this->test_pairedPowerTableUsesPairedDeltaMedian();
}

TEST_F(ErgTableTestSuite, TestPairedPowerTableWaitsForStableTarget) {
    this->test_pairedPowerTableWaitsForStableTarget();
}

TEST_F(ErgTableTestSuite, TestPairedPowerTableRejectsRudyInstability) {
    this->test_pairedPowerTableRejectsRudyInstability();
}

TEST_F(ErgTableTestSuite, TestPairedPowerTableRejectsExcessiveStableDelta) {
    this->test_pairedPowerTableRejectsExcessiveStableDelta();
}

TEST_F(ErgTableTestSuite, TestPairedPowerTableIgnoresPersistedExcessiveDelta) {
    this->test_pairedPowerTableIgnoresPersistedExcessiveDelta();
}

