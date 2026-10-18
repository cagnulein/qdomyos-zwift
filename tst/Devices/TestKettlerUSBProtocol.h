#pragma once

#include <gtest/gtest.h>

#include "devices/kettlerusbbike/KettlerUSB.h"

TEST(KettlerUSBProtocolTest, AcceptsStandardStatusResponse) {
    EXPECT_TRUE(KettlerUSB::isStatusResponse(QStringLiteral("101\t047\t074\t002\t025\t0312\t01:12\t025")));
}

TEST(KettlerUSBProtocolTest, RejectsProtocolError) {
    EXPECT_FALSE(KettlerUSB::isStatusResponse(QStringLiteral("ERROR")));
}

TEST(KettlerUSBProtocolTest, RejectsMalformedStatusResponse) {
    EXPECT_FALSE(KettlerUSB::isStatusResponse(QStringLiteral("101\t047\t074")));
    EXPECT_FALSE(KettlerUSB::isStatusResponse(QStringLiteral("101\tRPM\t074\t002\t025\t0312\t01:12\t025")));
    EXPECT_FALSE(KettlerUSB::isStatusResponse(QStringLiteral("101\t047\t074\t002\t025\t0312\tinvalid\t025")));
    EXPECT_FALSE(KettlerUSB::isStatusResponse(QStringLiteral("101\t047\t074\t002\t025\t0312\t01:12\t025\textra")));
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
