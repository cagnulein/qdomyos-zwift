#include <gtest/gtest.h>

#include "devices/ftmsbike/ftmsbikepowerutils.h"

TEST(FtmsBikePowerFallbackTest, MerachS26cForcesZeroAtZeroInstantCadence) {
    EXPECT_TRUE(ftmsbikepowerutils::shouldForceZeroAveragePower(true, true, 0.0));
}

TEST(FtmsBikePowerFallbackTest, OtherFtmsBikesKeepExistingAveragePowerFallback) {
    EXPECT_FALSE(ftmsbikepowerutils::shouldForceZeroAveragePower(false, true, 0.0));
}

TEST(FtmsBikePowerFallbackTest, MerachS26cDoesNotForceZeroWhilePedaling) {
    EXPECT_FALSE(ftmsbikepowerutils::shouldForceZeroAveragePower(true, true, 80.0));
}

TEST(FtmsBikePowerFallbackTest, MerachS26cRequiresInstantCadenceInFrame) {
    EXPECT_FALSE(ftmsbikepowerutils::shouldForceZeroAveragePower(true, false, 0.0));
}
