#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <gtest/gtest.h>

#include "devices/lifefitnesstreadmill/lifefitnessworkout.h"

namespace {
trainrow makeLifeFitnessWorkoutRow(int seconds, double speed, double incline) {
    trainrow row;
    row.duration = QTime(0, 0, 0).addSecs(seconds);
    row.speed = speed;
    row.inclination = incline;
    return row;
}
}

TEST(LifeFitnessWorkoutTest, BuildsCommercialIntervalXmlAndSummary) {
    const QList<trainrow> rows = {makeLifeFitnessWorkoutRow(60, 3.0, 0.0),
                                  makeLifeFitnessWorkoutRow(60, 3.5, 0.5)};
    lifefitnessworkout::Profile profile;
    profile.userId = QStringLiteral("test-user@example.invalid");
    profile.gender = 0;
    profile.age = 42;
    profile.weightKg = 70.0;
    profile.heightCm = 180.0;

    lifefitnessworkout::Program program;
    QString error;
    ASSERT_TRUE(lifefitnessworkout::buildProgram(rows, QStringLiteral("QZ two minute example"), profile, program, &error,
                                                  QDateTime(QDate(2026, 10, 13), QTime(0, 0), Qt::UTC)))
        << error.toStdString();

    const QJsonDocument summaryDocument = QJsonDocument::fromJson(program.summary);
    ASSERT_TRUE(summaryDocument.isArray());
    ASSERT_EQ(summaryDocument.array().size(), 1);
    const QJsonObject summary = summaryDocument.array().at(0).toObject();
    EXPECT_EQ(summary.value(QStringLiteral("workoutId")).toInt(), 900001);
    EXPECT_EQ(summary.value(QStringLiteral("workoutName")).toString(), QStringLiteral("QZ two minute example"));
    EXPECT_EQ(summary.value(QStringLiteral("fileName")).toString(), QStringLiteral("QZ two minute example"));
    EXPECT_EQ(summary.value(QStringLiteral("index")).toInt(), 0);
    EXPECT_EQ(summary.value(QStringLiteral("workoutType")).toString(), QStringLiteral("WORKOUT_LIBRARY"));

    const QString xml = QString::fromUtf8(program.xml);
    EXPECT_TRUE(xml.contains(QStringLiteral("<program-type>57</program-type>")));
    EXPECT_TRUE(xml.contains(QStringLiteral("<goal-value>120.0</goal-value>")));
    EXPECT_TRUE(xml.contains(QStringLiteral("<speed units=\"1\">3.0</speed>")));
    EXPECT_TRUE(xml.contains(QStringLiteral("<cyo-segment-data1>30,35</cyo-segment-data1>")));
    EXPECT_TRUE(xml.contains(QStringLiteral("<cyo-segment-data2>0,5</cyo-segment-data2>")));
    EXPECT_TRUE(xml.contains(QStringLiteral("<cyo-segment-time>60,60</cyo-segment-time>")));
    EXPECT_EQ(program.summaryFrames.size(), 10);
    EXPECT_FALSE(program.detailFrames.isEmpty());
}

TEST(LifeFitnessWorkoutTest, FramesBodyWithChecksumAndFifteenByteChunks) {
    const QByteArray body = QByteArrayLiteral("abcdefghijklmnop");
    QString error;
    const QList<QByteArray> frames = lifefitnessworkout::frameBody(body, &error);

    ASSERT_EQ(frames.size(), 2);
    EXPECT_TRUE(error.isEmpty());
    EXPECT_EQ(frames.at(0), QByteArray::fromHex("00000002186162636465666768696a6b6c6d6e6f"));
    EXPECT_EQ(frames.at(1), QByteArray::fromHex("000100027070"));
}

TEST(LifeFitnessWorkoutTest, RejectsMissingInitialSpeed) {
    const QList<trainrow> rows = {makeLifeFitnessWorkoutRow(60, -1.0, 0.0)};
    lifefitnessworkout::Program program;
    QString error;

    EXPECT_FALSE(lifefitnessworkout::buildProgram(rows, QStringLiteral("invalid"), lifefitnessworkout::Profile{},
                                                   program, &error));
    EXPECT_FALSE(error.isEmpty());
}

TEST(LifeFitnessWorkoutTest, RejectsMoreThanOneHundredIntervals) {
    QList<trainrow> rows;
    for (int i = 0; i < 101; ++i)
        rows.append(makeLifeFitnessWorkoutRow(10, 3.0, 0.0));

    lifefitnessworkout::Program program;
    QString error;
    EXPECT_FALSE(lifefitnessworkout::buildProgram(rows, QStringLiteral("too many"), lifefitnessworkout::Profile{},
                                                   program, &error));
    EXPECT_FALSE(error.isEmpty());
}

TEST(LifeFitnessWorkoutTest, ParsesConsoleSelectionIndex) {
    int index = -1;
    ASSERT_TRUE(lifefitnessworkout::parseSelectionEvent(QByteArray::fromHex("150000"), &index));
    EXPECT_EQ(index, 0);
    EXPECT_FALSE(lifefitnessworkout::parseSelectionEvent(QByteArray::fromHex("0401"), &index));
    EXPECT_FALSE(lifefitnessworkout::parseSelectionEvent(QByteArray::fromHex("15"), &index));
}
