#include <gtest/gtest.h>

#include "devices/kettlerusbbike/KettlerUSB.h"

class KettlerUSBTestAccess {
public:
    static void start(KettlerUSB &bike) { bike.powerFilterClock.start(); }
    static void powerReply(KettlerUSB &bike, const QString &response) {
        bike.parseStatusResponse(response);
    }
    static void pollReply(KettlerUSB &bike, const QString &response) {
        bike.processNativePollResponse(response);
    }
};

static double currentPower(KettlerUSB &bike) {
    double power, heart, cadence, speed, distance;
    int status;
    bike.getTelemetry(power, heart, cadence, speed, distance, status);
    return power;
}

TEST(KettlerUSBProtocol, ProcessesDelayedStatusDuringPolling) {
    KettlerUSB bike(nullptr, QString(), 9600);
    KettlerUSBTestAccess::start(bike);
    KettlerUSBTestAccess::powerReply(bike, "000\t075\t266\t003\t105\t0011\t01:08\t105");
    EXPECT_DOUBLE_EQ(currentPower(bike), 105);
    KettlerUSBTestAccess::powerReply(bike, "000\t070\t248\t003\t175\t0013\t01:12\t010");
    EXPECT_DOUBLE_EQ(currentPower(bike), 105);
    // A delayed PW status was read while waiting for the CM response.
    KettlerUSBTestAccess::pollReply(bike, "000\t069\t244\t003\t170\t0013\t01:12\t170");
    EXPECT_DOUBLE_EQ(currentPower(bike), 170);
    KettlerUSBTestAccess::pollReply(bike, "ACK");
    KettlerUSBTestAccess::pollReply(bike, "AR1S");
    KettlerUSBTestAccess::pollReply(bike, "018");
    EXPECT_DOUBLE_EQ(currentPower(bike), 170);
}

TEST(KettlerUSBProtocol, PollRepliesRejectMalformedFramesAndPassStop) {
    KettlerUSB bike(nullptr, QString(), 9600);
    KettlerUSBTestAccess::start(bike);
    KettlerUSBTestAccess::powerReply(bike, "000\t069\t244\t003\t170\t0013\t01:12\t170");
    KettlerUSBTestAccess::pollReply(bike, "000\tinvalid\t244\t003\t170\t0013\t01:12\t050");
    EXPECT_DOUBLE_EQ(currentPower(bike), 170);
    KettlerUSBTestAccess::pollReply(bike, "000\t000\t000\t003\t025\t0013\t01:13\t000");
    EXPECT_DOUBLE_EQ(currentPower(bike), 0);
}

TEST(KettlerUSBProtocol, FilterPreservesBoundariesAndExpiresAcrossMixedLowValues) {
    KettlerPowerFilter filter;
    EXPECT_DOUBLE_EQ(filter.apply(10, 60, 0), 10);
    EXPECT_DOUBLE_EQ(filter.apply(100, 60, 100), 100);
    EXPECT_DOUBLE_EQ(filter.apply(5, 60, 200), 100);
    EXPECT_DOUBLE_EQ(filter.apply(10, 60, 4199), 100);
    EXPECT_DOUBLE_EQ(filter.apply(2, 60, 4200), 2);
    EXPECT_DOUBLE_EQ(filter.apply(100, 60, 4300), 100);
    EXPECT_DOUBLE_EQ(filter.apply(1, 60, 4400), 1);
    EXPECT_DOUBLE_EQ(filter.apply(100, 60, 4500), 100);
    EXPECT_DOUBLE_EQ(filter.apply(0, 60, 4600), 0);
    EXPECT_DOUBLE_EQ(filter.apply(100, 60, 4700), 100);
    EXPECT_DOUBLE_EQ(filter.apply(5, 0, 4800), 5);
}
