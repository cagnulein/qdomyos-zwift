#include <gtest/gtest.h>

#include "devices/bike.h"

// Issue 5316: the T.Power(W) tile keeps re-sending its target every second while an app
// (Zwift over FTMS BLE or DIRCON) is driving the same bike in slope mode. QZ keeps that
// override, but warns the rider once.

TEST(TargetPowerOverride, WarnsWhenAppDrivesSlope) {
    const qint64 now = 100000;
    EXPECT_TRUE(bike::targetPowerOverridesExternalApp(now - 500, now, false, true));
    EXPECT_TRUE(bike::targetPowerOverridesExternalApp(now - 2000, now, false, true));
}

TEST(TargetPowerOverride, SilentWhenAppSendsItsOwnTargetPower) {
    const qint64 now = 100000;
    EXPECT_FALSE(bike::targetPowerOverridesExternalApp(now - 500, now, true, true));
}

TEST(TargetPowerOverride, SilentWithoutApp) {
    const qint64 now = 100000;
    EXPECT_FALSE(bike::targetPowerOverridesExternalApp(0, now, false, true));
    EXPECT_FALSE(bike::targetPowerOverridesExternalApp(now - 2001, now, false, true));
}

TEST(TargetPowerOverride, SilentWithAutoResistanceOff) {
    // changePower() and the app's slope are both ignored, so nothing overrides anything
    const qint64 now = 100000;
    EXPECT_FALSE(bike::targetPowerOverridesExternalApp(now - 500, now, false, false));
}
