#pragma once

#include <gtest/gtest.h>
#include <QBluetoothUuid>
#include <QList>

#include "devices/ftmsbike/soleb94profiledetector.h"

namespace {
QBluetoothUuid soleB94Uuid(quint16 uuid) {
    return QBluetoothUuid(uuid);
}

QList<QBluetoothUuid> soleB94Uuids(std::initializer_list<quint16> uuids) {
    QList<QBluetoothUuid> result;
    for (quint16 uuid : uuids) {
        result.append(soleB94Uuid(uuid));
    }
    return result;
}
} // namespace

TEST(SoleB94ProfileDetectorTest, SelectsModernProfileWhenModernMarkersArePresent) {
    EXPECT_EQ(soleb94profiledetector::detect(soleB94Uuids({0x2ACC, 0x2AD6, 0x2AD2, 0x2AD3, 0x2AD9, 0x2ADA})),
              SoleB94Protocol::TargetResistanceTenths);
}

TEST(SoleB94ProfileDetectorTest, SelectsLegacyProfileWhenLegacyMarkersArePresent) {
    EXPECT_EQ(soleb94profiledetector::detect(soleB94Uuids({0x2ACD, 0x2ACE, 0x2AD2, 0x2AD9, 0x2ADA})),
              SoleB94Protocol::LegacySimulationParameters);
}

TEST(SoleB94ProfileDetectorTest, RejectsEachIncompleteModernMarkerPair) {
    EXPECT_EQ(soleb94profiledetector::detect(soleB94Uuids({0x2ACC})), SoleB94Protocol::Unknown);
    EXPECT_EQ(soleb94profiledetector::detect(soleB94Uuids({0x2AD6})), SoleB94Protocol::Unknown);
}

TEST(SoleB94ProfileDetectorTest, RejectsEachIncompleteLegacyMarkerPair) {
    EXPECT_EQ(soleb94profiledetector::detect(soleB94Uuids({0x2ACD})), SoleB94Protocol::Unknown);
    EXPECT_EQ(soleb94profiledetector::detect(soleB94Uuids({0x2ACE})), SoleB94Protocol::Unknown);
}

TEST(SoleB94ProfileDetectorTest, RejectsMixedOrAmbiguousMarkerSets) {
    EXPECT_EQ(soleb94profiledetector::detect(soleB94Uuids({0x2ACC, 0x2AD6, 0x2ACD, 0x2ACE})),
              SoleB94Protocol::Unknown);
    EXPECT_EQ(soleb94profiledetector::detect(soleB94Uuids({0x2ACC, 0x2ACE})), SoleB94Protocol::Unknown);
    EXPECT_EQ(soleb94profiledetector::detect(soleB94Uuids({0x2AD6, 0x2ACD})), SoleB94Protocol::Unknown);
}

TEST(SoleB94ProfileDetectorTest, IgnoresUnrelatedCharacteristicsOnlyWhenACompletePairIsPresent) {
    EXPECT_EQ(soleb94profiledetector::detect(soleB94Uuids({0x180D, 0x2A37, 0xFFF1})), SoleB94Protocol::Unknown);
    EXPECT_EQ(soleb94profiledetector::detect(soleB94Uuids({0x2ACC, 0x2AD6, 0x180D})),
              SoleB94Protocol::TargetResistanceTenths);
}
