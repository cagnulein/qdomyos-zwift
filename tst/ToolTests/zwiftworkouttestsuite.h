#ifndef ZWIFTWORKOUTTESTSUITE_H
#define ZWIFTWORKOUTTESTSUITE_H

#include "gtest/gtest.h"

class ZwiftWorkoutTestSuite: public testing::Test {

public:
    void test_xmlWorkoutFileParsesPercentIncline();
    void test_xmlWorkoutFileParsesNegativeIncline();
    void test_xmlExtensionWorkoutFileUsesZwiftParser();
};

TEST_F(ZwiftWorkoutTestSuite, XmlWorkoutFileParsesPercentIncline) {
    this->test_xmlWorkoutFileParsesPercentIncline();
}

TEST_F(ZwiftWorkoutTestSuite, XmlWorkoutFileParsesNegativeIncline) {
    this->test_xmlWorkoutFileParsesNegativeIncline();
}

TEST_F(ZwiftWorkoutTestSuite, XmlExtensionWorkoutFileUsesZwiftParser) {
    this->test_xmlExtensionWorkoutFileUsesZwiftParser();
}

#endif // ZWIFTWORKOUTTESTSUITE_H
