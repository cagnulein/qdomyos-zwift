#include <gtest/gtest.h>

#include "devices/bluetooth.h"

// The heart rate sensor bar offers a device advertising Heart Rate (0x180D), but not a fitness machine that
// advertises it next to its own service: QZ's virtual devices and most machines do. And it offers one only when
// the user asked with a tap or turned the background search on.

namespace {
const QBluetoothUuid heartRate(QBluetoothUuid::HeartRate);
const QBluetoothUuid ftms(quint16(0x1826));
const QBluetoothUuid runningSpeedAndCadence(QBluetoothUuid::RunningSpeedAndCadence);
} // namespace

TEST(HeartRateSensorOffer, HeartRateOnlyIsASensor) {
    EXPECT_FALSE(bluetooth::advertisesFitnessMachine({heartRate}));
    EXPECT_FALSE(bluetooth::advertisesFitnessMachine({}));
}

TEST(HeartRateSensorOffer, RunningSpeedAndCadenceIsStillASensor) {
    EXPECT_FALSE(bluetooth::advertisesFitnessMachine({heartRate, runningSpeedAndCadence}));
}

TEST(HeartRateSensorOffer, FitnessMachineServicesAreNotSensors) {
    // QZ's virtual bike, treadmill and rower with FTMS
    EXPECT_TRUE(bluetooth::advertisesFitnessMachine({ftms, heartRate}));
    // the virtual bike in power or cadence mode, and the Tacx mode
    EXPECT_TRUE(bluetooth::advertisesFitnessMachine({QBluetoothUuid(QBluetoothUuid::CyclingPower), heartRate}));
    EXPECT_TRUE(
        bluetooth::advertisesFitnessMachine({QBluetoothUuid(QBluetoothUuid::CyclingSpeedAndCadence), heartRate}));
    // the virtual rower in PM5 mode
    EXPECT_TRUE(bluetooth::advertisesFitnessMachine(
        {QBluetoothUuid(QStringLiteral("CE060000-43E5-11E4-916C-0800200C9A66")), heartRate}));
    // the virtual Echelon bike, treadmill and rower
    EXPECT_TRUE(bluetooth::advertisesFitnessMachine(
        {QBluetoothUuid(QStringLiteral("0bf669f0-45f2-11e7-9598-0800200c9a66")), heartRate}));
}

// heartRateSensorOffered(onTap, backgroundSearch, sensorChosen, heartRateComes, dismissed)

TEST(HeartRateSensorOffer, TapOffersWithTheBackgroundSearchOff) {
    // the background search is off by default: a tap on the heart rate tile or the Bluetooth icon still offers
    EXPECT_TRUE(bluetooth::heartRateSensorOffered(true, false, false, false, false));
    EXPECT_TRUE(bluetooth::heartRateSensorOffered(true, true, false, false, false));
}

TEST(HeartRateSensorOffer, BackgroundSearchOffersOnlyWhenOn) {
    EXPECT_FALSE(bluetooth::heartRateSensorOffered(false, false, false, false, false));
    EXPECT_TRUE(bluetooth::heartRateSensorOffered(false, true, false, false, false));
}

TEST(HeartRateSensorOffer, NothingToOffer) {
    for (bool onTap : {false, true}) {
        // a sensor chosen: Connect on the bar would replace that choice
        EXPECT_FALSE(bluetooth::heartRateSensorOffered(onTap, true, true, false, false));
        // the heart rate comes from elsewhere (the machine, a watch app)
        EXPECT_FALSE(bluetooth::heartRateSensorOffered(onTap, true, false, true, false));
        // hidden with Not now (a tap clears that list before its search)
        EXPECT_FALSE(bluetooth::heartRateSensorOffered(onTap, true, false, false, true));
    }
}

TEST(HeartRateSensorOffer, UuidCaseAndOrderDoNotMatter) {
    EXPECT_TRUE(bluetooth::advertisesFitnessMachine(
        {heartRate, runningSpeedAndCadence, QBluetoothUuid(QStringLiteral("{ce060000-43e5-11e4-916c-0800200c9a66}"))}));
}
