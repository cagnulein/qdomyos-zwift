#pragma once

#include <gtest/gtest.h>
#include <QByteArray>

#include "devices/ftmsbike/soleb94profiledetector.h"

TEST(B94ResistanceProtocolTest, BuildsLegacySimulationParametersPacket) {
    EXPECT_EQ(soleb94profiledetector::buildResistanceCommand(
                  SoleB94Protocol::LegacySimulationParameters, 2, 1.0, 14.0),
              QByteArray::fromHex("110000a0002819"));
}

TEST(B94ResistanceProtocolTest, BuildsModernTargetResistancePacket) {
    EXPECT_EQ(soleb94profiledetector::buildResistanceCommand(
                  SoleB94Protocol::TargetResistanceTenths, 2, 1.0, 14.0),
              QByteArray::fromHex("0414"));
}

TEST(B94ResistanceProtocolTest, PreservesZeroResistanceInBothProtocols) {
    EXPECT_EQ(soleb94profiledetector::buildResistanceCommand(
                  SoleB94Protocol::LegacySimulationParameters, 0, 1.0, 14.0),
              QByteArray::fromHex("1100008c002819"));
    EXPECT_EQ(soleb94profiledetector::buildResistanceCommand(
                  SoleB94Protocol::TargetResistanceTenths, 0, 1.0, 14.0),
              QByteArray::fromHex("0400"));
}

TEST(B94ResistanceProtocolTest, ClampsModernResistanceToOneByteEncoding) {
    EXPECT_EQ(soleb94profiledetector::buildResistanceCommand(
                  SoleB94Protocol::TargetResistanceTenths, 30, 1.0, 14.0),
              QByteArray::fromHex("04ff"));
}

TEST(B94ResistanceProtocolTest, AppliesLegacyGainAndOffsetBeforeTenthsEncoding) {
    EXPECT_EQ(soleb94profiledetector::buildResistanceCommand(
                  SoleB94Protocol::LegacySimulationParameters, 3, 2.0, 1.0),
              QByteArray::fromHex("11000046002819"));
}

TEST(B94ResistanceProtocolTest, UnknownProtocolProducesNoCommand) {
    EXPECT_TRUE(soleb94profiledetector::buildResistanceCommand(
                    SoleB94Protocol::Unknown, 2, 1.0, 14.0)
                    .isEmpty());
}

TEST(B94ResistanceProtocolTest, PreservesLegacyTargetWhenTelemetryIsStale) {
    resistance_t target = 2;
    target = soleb94profiledetector::resistanceFromTelemetry(
        SoleB94Protocol::LegacySimulationParameters, true, target, 1);
    EXPECT_EQ(target, 2);

    target = 3;
    target = soleb94profiledetector::resistanceFromTelemetry(
        SoleB94Protocol::LegacySimulationParameters, true, target, 1);
    EXPECT_EQ(target, 3);

    target = 4;
    target = soleb94profiledetector::resistanceFromTelemetry(
        SoleB94Protocol::LegacySimulationParameters, true, target, 1);
    EXPECT_EQ(target, 4);
}

TEST(B94ResistanceProtocolTest, UsesReportedResistanceOutsideLegacyTargetMode) {
    EXPECT_EQ(soleb94profiledetector::resistanceFromTelemetry(
                  SoleB94Protocol::TargetResistanceTenths, true, 4, 1),
              1);
    EXPECT_EQ(soleb94profiledetector::resistanceFromTelemetry(
                  SoleB94Protocol::Unknown, false, -1, 1),
              1);
}
