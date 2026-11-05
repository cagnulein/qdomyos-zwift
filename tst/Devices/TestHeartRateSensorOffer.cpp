#include <gtest/gtest.h>

#include "devices/bluetooth.h"

// The heart rate sensor bar offers a device advertising Heart Rate (0x180D), but not a fitness machine that
// advertises it next to its own service: QZ's virtual devices and most machines do.

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

TEST(HeartRateSensorOffer, UuidCaseAndOrderDoNotMatter) {
    EXPECT_TRUE(bluetooth::advertisesFitnessMachine(
        {heartRate, runningSpeedAndCadence, QBluetoothUuid(QStringLiteral("{ce060000-43e5-11e4-916c-0800200c9a66}"))}));
}
