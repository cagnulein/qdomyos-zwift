#pragma once

#include <gtest/gtest.h>

#include "devices/kettlerusbbike/KettlerUSB.h"

TEST(KettlerUSBProtocolTest, AcceptsStandardStatusResponse) {
    EXPECT_TRUE(KettlerUSB::isStatusResponse(QStringLiteral("190\t000\t053\t188\t007\t043\t0019\t02:50\t043")));
}

TEST(KettlerUSBProtocolTest, RejectsProtocolError) {
    EXPECT_FALSE(KettlerUSB::isStatusResponse(QStringLiteral("ERROR")));
}

TEST(KettlerUSBProtocolTest, RejectsMalformedStatusResponse) {
    EXPECT_FALSE(KettlerUSB::isStatusResponse(QStringLiteral("190\t000\t053\t188\t007\t043\t0019\t02:50")));
    EXPECT_FALSE(KettlerUSB::isStatusResponse(QStringLiteral("190\tRPM\t053\t188\t007\t043\t0019\t02:50\t043")));
    EXPECT_FALSE(KettlerUSB::isStatusResponse(QStringLiteral("190\t000\t053\t188\t007\t043\t0019\tinvalid\t043")));
    EXPECT_FALSE(KettlerUSB::isStatusResponse(QStringLiteral("190\t000\t053\t188\t007\t043\t0019\t02:50\tbad")));
}

TEST(KettlerUSBProtocolTest, PreservesFragmentedAndQueuedResponses) {
    QByteArray buffer("ER");
    QString response;

    EXPECT_FALSE(KettlerUSB::takeResponse(buffer, response));

    buffer.append("ROR\r\nACK\r\n");
    ASSERT_TRUE(KettlerUSB::takeResponse(buffer, response));
    EXPECT_EQ(QStringLiteral("ERROR"), response);
    ASSERT_TRUE(KettlerUSB::takeResponse(buffer, response));
    EXPECT_EQ(QStringLiteral("ACK"), response);
    EXPECT_TRUE(buffer.isEmpty());
}
