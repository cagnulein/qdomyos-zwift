#pragma once

#include <gtest/gtest.h>
#include <QBluetoothUuid>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QRegularExpression>
#include <QString>

#include "devices/ftmsbike/soleb94profiledetector.h"

namespace {
struct SoleB94CapturedCommand {
    int rawResistance;
    QByteArray bytes;
};

struct SoleB94LogReplay {
    QList<QBluetoothUuid> characteristicUuids;
    QList<SoleB94CapturedCommand> commands;
};

QString soleB94LogRoot() {
    return QString::fromLocal8Bit(qgetenv("QZ_B94_LOG_ROOT"));
}

QString soleB94LogPath(const QString &relativePath) {
    return soleB94LogRoot() + QStringLiteral("/") + relativePath;
}

SoleB94LogReplay replaySoleB94Log(const QString &path) {
    SoleB94LogReplay replay;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return replay;

    const QString log = QString::fromUtf8(file.readAll());
    const QRegularExpression uuidPattern(
        QStringLiteral("\\{([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})\\}"));
    QRegularExpressionMatchIterator uuidMatches = uuidPattern.globalMatch(log);
    while (uuidMatches.hasNext())
        replay.characteristicUuids.append(QBluetoothUuid(uuidMatches.next().captured(1)));

    const QRegularExpression rawResistancePattern(QStringLiteral("writing resistance \\\"\\s*(-?\\d+)"));
    const QRegularExpression commandPattern(
        QStringLiteral(">>\\s*([0-9a-fA-F ]+?)\\s*//\\s*forceResistance\\s+(-?\\d+)"));
    int rawResistance = -1;
    const QStringList lines = log.split(QChar('\n'));
    for (const QString &line : lines) {
        const QRegularExpressionMatch rawMatch = rawResistancePattern.match(line);
        if (rawMatch.hasMatch())
            rawResistance = rawMatch.captured(1).toInt();

        const QRegularExpressionMatch commandMatch = commandPattern.match(line);
        if (!commandMatch.hasMatch())
            continue;

        replay.commands.append({rawResistance, QByteArray::fromHex(commandMatch.captured(1).toLatin1())});
    }

    return replay;
}

QList<SoleB94CapturedCommand> commandsForRawResistance(const SoleB94LogReplay &replay, int rawResistance) {
    QList<SoleB94CapturedCommand> result;
    for (const SoleB94CapturedCommand &command : replay.commands) {
        if (command.rawResistance == rawResistance)
            result.append(command);
    }
    return result;
}

bool soleB94LogIsAvailable(const QString &path) {
    return !soleB94LogRoot().isEmpty() && QFileInfo::exists(path);
}
} // namespace

TEST(SoleB94LogReplayTest, Legacy21LogRegeneratesCapturedLegacyBytes) {
    const QString path = soleB94LogPath(
        QStringLiteral("old-2.21.6/debug-Sa__Okt__24_14_44_32_2026.log"));
    if (!soleB94LogIsAvailable(path))
        GTEST_SKIP() << "Set QZ_B94_LOG_ROOT to the directory containing the captured B94 logs";

    const SoleB94LogReplay replay = replaySoleB94Log(path);
    EXPECT_EQ(soleb94profiledetector::detect(replay.characteristicUuids),
              SoleB94Protocol::LegacySimulationParameters);

    const QList<SoleB94CapturedCommand> commands = commandsForRawResistance(replay, 2);
    ASSERT_EQ(commands.size(), 26);
    const QByteArray expected = soleb94profiledetector::buildResistanceCommand(
        SoleB94Protocol::LegacySimulationParameters, 2, 1.0, 14.0);
    EXPECT_EQ(expected, QByteArray::fromHex("110000a0002819"));
    for (const SoleB94CapturedCommand &command : commands)
        EXPECT_EQ(command.bytes, expected);
}

TEST(SoleB94LogReplayTest, Legacy22LogIsRecognizedAsLegacyNotModern) {
    const QString path = soleB94LogPath(QStringLiteral("debug-Sa__Okt__24_15_39_18_2026.log"));
    if (!soleB94LogIsAvailable(path))
        GTEST_SKIP() << "Set QZ_B94_LOG_ROOT to the directory containing the captured B94 logs";

    const SoleB94LogReplay replay = replaySoleB94Log(path);
    EXPECT_EQ(soleb94profiledetector::detect(replay.characteristicUuids),
              SoleB94Protocol::LegacySimulationParameters);

    const QList<SoleB94CapturedCommand> commands = commandsForRawResistance(replay, 2);
    ASSERT_EQ(commands.size(), 33);
    const QByteArray expected = soleb94profiledetector::buildResistanceCommand(
        SoleB94Protocol::LegacySimulationParameters, 2, 1.0, 1.0);
    EXPECT_EQ(expected, QByteArray::fromHex("1100001e002819"));
    for (const SoleB94CapturedCommand &command : commands) {
        EXPECT_EQ(command.bytes, QByteArray::fromHex("0414"));
        EXPECT_NE(command.bytes, expected);
    }
}

TEST(SoleB94LogReplayTest, Modern5380LogUsesTenthsForCapturedResistanceLevels) {
    const QString path = soleB94LogPath(QStringLiteral("issue-5380/qz-debug-B94-vGitHub.log"));
    if (!soleB94LogIsAvailable(path))
        GTEST_SKIP() << "Set QZ_B94_LOG_ROOT to the directory containing the captured B94 logs";

    const SoleB94LogReplay replay = replaySoleB94Log(path);
    EXPECT_EQ(soleb94profiledetector::detect(replay.characteristicUuids),
              SoleB94Protocol::TargetResistanceTenths);

    for (const int rawResistance : {2, 5, 7}) {
        const QList<SoleB94CapturedCommand> commands = commandsForRawResistance(replay, rawResistance);
        ASSERT_FALSE(commands.isEmpty());
        const QByteArray expected = soleb94profiledetector::buildResistanceCommand(
            SoleB94Protocol::TargetResistanceTenths, rawResistance, 1.0, 18.0);
        EXPECT_EQ(expected, QByteArray::fromHex(
                               QStringLiteral("04%1").arg(rawResistance * 10, 2, 16, QChar('0')).toLatin1()));
        for (const SoleB94CapturedCommand &command : commands)
            EXPECT_NE(command.bytes, expected);
    }
}
