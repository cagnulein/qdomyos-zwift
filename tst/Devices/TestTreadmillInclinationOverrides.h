#pragma once

#include <gtest/gtest.h>

#include "Tools/testsettings.h"
#include "devices/treadmill.h"

class TreadmillInclinationOverrideTest : public testing::Test {
  protected:
    class TestTreadmill : public treadmill {
      public:
        double feedRawInclination(double rawValue) {
            Inclination = treadmillInclinationOverrideForRawFeedback(rawValue);
            return Inclination.value();
        }
        double rawInclinationValue() const { return rawInclination.value(); }
    };

    TestSettings testSettings{QStringLiteral("qz-treadmill-inclination-test"),
                              QStringLiteral("qz-treadmill-inclination-test")};
    TestTreadmill treadmill;

    void SetUp() override;
    void TearDown() override;
};
