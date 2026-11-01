#pragma once

#include <gtest/gtest.h>
#include <QByteArray>

#include "devices/trxappgateusbtreadmill/trxappgateusbtreadmill.h"
#include "trainprogram.h"

TEST(TrxAppGateUsTreadmillParserTest, BuildsCapturedFlowFitnessStartCommand) {
    EXPECT_EQ(trxappgateusbtreadmill::flowFitnessStartPacket(),
              QByteArray::fromHex("f0a52ed30298"));
}

TEST(TrxAppGateUsTreadmillParserTest, BuildsCapturedFlowFitnessStopCommand) {
    EXPECT_EQ(trxappgateusbtreadmill::flowFitnessStopPacket(),
              QByteArray::fromHex("f0a52ed3049a"));
}

TEST(TrxAppGateUsTreadmillParserTest, ParsesFlowFitnessMetrics) {
    const QByteArray packet = QByteArray::fromHex("f0b02ed3020304330319024c01180404030101");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    ASSERT_TRUE(metrics.valid);
    EXPECT_DOUBLE_EQ(metrics.speed, 2.3);
    EXPECT_DOUBLE_EQ(metrics.inclination, 3.0);
    EXPECT_DOUBLE_EQ(metrics.kcal, 536.0);
    EXPECT_DOUBLE_EQ(metrics.distance, 3.5);
    EXPECT_EQ(metrics.heart, 175);
    EXPECT_EQ(metrics.elapsed, 121);
}

TEST(TrxAppGateUsTreadmillParserTest, ParsesRuntimeFlowFitnessTelemetryFrame) {
    // Captured from the Android debug log: telemetry is f0 b2, not f0 b0.
    const QByteArray packet = QByteArray::fromHex("f0b22ed30101010101010101010101010102b2");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    ASSERT_TRUE(metrics.valid);
    EXPECT_DOUBLE_EQ(metrics.speed, 0.0);
    EXPECT_DOUBLE_EQ(metrics.inclination, 0.0);
    EXPECT_DOUBLE_EQ(metrics.kcal, 0.0);
    EXPECT_DOUBLE_EQ(metrics.distance, 0.0);
    EXPECT_EQ(metrics.heart, 0);
    EXPECT_EQ(metrics.elapsed, 0);
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsFlowFitnessAcknowledgement) {
    const QByteArray packet = QByteArray::fromHex("f0b02ed3a1");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    EXPECT_FALSE(metrics.valid);
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsShortFlowFitnessPacket) {
    const QByteArray packet = QByteArray::fromHex("f0b02ed3020304330319024c011804040301");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    EXPECT_FALSE(metrics.valid);
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsDifferentProfileSelector) {
    const QByteArray packet = QByteArray::fromHex("f0b023d3020304330319024c01180404030101");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    EXPECT_FALSE(metrics.valid);
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsUnsupportedFlowFitnessFrameType) {
    const QByteArray packet = QByteArray::fromHex("f0b52ed3020304330319024c01180404030101");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    EXPECT_FALSE(metrics.valid);
}

TEST(TrxAppGateUsTreadmillParserTest, ParsesZeroMetricValuesWithoutUnsignedUnderflow) {
    const QByteArray packet = QByteArray::fromHex("f0b02ed3010101010101010101010101010101");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    ASSERT_TRUE(metrics.valid);
    EXPECT_DOUBLE_EQ(metrics.speed, 0.0);
    EXPECT_DOUBLE_EQ(metrics.inclination, 0.0);
    EXPECT_DOUBLE_EQ(metrics.kcal, 0.0);
    EXPECT_DOUBLE_EQ(metrics.distance, 0.0);
    EXPECT_EQ(metrics.heart, 0);
    EXPECT_EQ(metrics.elapsed, 0);
}

TEST(TrxAppGateUsTreadmillParserTest, ParsesHighBitMetricBytesAsUnsigned) {
    // These decoders are shared by the legacy AppGate treadmill profiles too.
    // Bytes >= 0x80 must not be sign-extended on platforms where char is signed.
    const QByteArray packet = QByteArray::fromHex("f0b22ed3828382838283024c82838204040301");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    ASSERT_TRUE(metrics.valid);
    EXPECT_DOUBLE_EQ(metrics.speed, 1303.0);
    EXPECT_DOUBLE_EQ(metrics.inclination, 129.0);
    EXPECT_DOUBLE_EQ(metrics.kcal, 33154.0);
    EXPECT_DOUBLE_EQ(metrics.distance, 130.3);
    EXPECT_EQ(metrics.heart, 175);
    EXPECT_EQ(metrics.elapsed, 7929);
}

TEST(TrxAppGateUsTreadmillParserTest, MatchesCapturedFlowFitnessDeviceName) {
    EXPECT_TRUE(trxappgateusbtreadmill::isFlowFitnessDeviceName(QStringLiteral("TREADMILL00028")));
    EXPECT_TRUE(trxappgateusbtreadmill::isFlowFitnessDeviceName(QStringLiteral("treadmill00028")));
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsGenericTreadmillDeviceNames) {
    EXPECT_FALSE(trxappgateusbtreadmill::isFlowFitnessDeviceName(QStringLiteral("TREADMILL")));
    EXPECT_FALSE(trxappgateusbtreadmill::isFlowFitnessDeviceName(QStringLiteral("TREADMILLABC")));
    EXPECT_FALSE(trxappgateusbtreadmill::isFlowFitnessDeviceName(QStringLiteral("OTHER00028")));
}

TEST(TrxAppGateUsTreadmillParserTest, BuildsCapturedIconsoleSpeedProgramFrames) {
    const QList<double> speeds = {4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 12.0, 10.0, 8.0, 6.0};

    EXPECT_EQ(trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa6, speeds),
              QByteArray::fromHex("f0a601d30129013d0151020102150229021502010151013d13"));
    EXPECT_EQ(trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa7, speeds),
              QByteArray::fromHex("f0a701d30129013d0151020102150229021502010151013d14"));
}

TEST(TrxAppGateUsTreadmillParserTest, BuildsCapturedIconsoleInitializationSequence) {
    EXPECT_EQ(trxappgateusbtreadmill::iconsoleInitializationFrames(),
              (QList<QByteArray>{QByteArray::fromHex("f0a0000090"), QByteArray::fromHex("f0a001d364"),
                                 QByteArray::fromHex("f0a101d365")}));
}

TEST(TrxAppGateUsTreadmillParserTest, BuildsCapturedIconsoleKeepAlivePoll) {
    EXPECT_EQ(trxappgateusbtreadmill::iconsoleKeepAliveFrame(), QByteArray::fromHex("f0a201d366"));
}

TEST(TrxAppGateUsTreadmillParserTest, PreventsTelemetryAutoStartForIConsole) {
    EXPECT_FALSE(trxappgateusbtreadmill::shouldAutoStartFromTelemetry(true, true, false));
    EXPECT_TRUE(trxappgateusbtreadmill::shouldAutoStartFromTelemetry(false, true, false));
    EXPECT_FALSE(trxappgateusbtreadmill::shouldAutoStartFromTelemetry(false, true, true));
    EXPECT_FALSE(trxappgateusbtreadmill::shouldAutoStartFromTelemetry(false, false, false));
}

TEST(TrxAppGateUsTreadmillParserTest, BoundsIConsoleInitializationRetries) {
    EXPECT_TRUE(trxappgateusbtreadmill::shouldRetryIconsoleInitialization(0));
    EXPECT_TRUE(trxappgateusbtreadmill::shouldRetryIconsoleInitialization(1));
    EXPECT_TRUE(trxappgateusbtreadmill::shouldRetryIconsoleInitialization(2));
    EXPECT_FALSE(trxappgateusbtreadmill::shouldRetryIconsoleInitialization(3));
    EXPECT_FALSE(trxappgateusbtreadmill::shouldRetryIconsoleInitialization(-1));
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsOldUploadCompletionAfterInvalidWorkoutRequest) {
    EXPECT_FALSE(trxappgateusbtreadmill::shouldAcceptIconsoleUploadCompletion(true, true));
    EXPECT_TRUE(trxappgateusbtreadmill::shouldAcceptIconsoleUploadCompletion(true, false));
    EXPECT_FALSE(trxappgateusbtreadmill::shouldAcceptIconsoleUploadCompletion(false, false));
}

TEST(TrxAppGateUsTreadmillParserTest, BlocksStartWhileLatestWorkoutIsRejected) {
    EXPECT_TRUE(trxappgateusbtreadmill::shouldBlockIconsoleStart(true, false, false, true));
    EXPECT_TRUE(trxappgateusbtreadmill::shouldBlockIconsoleStart(true, true, true, false));
    EXPECT_TRUE(trxappgateusbtreadmill::shouldBlockIconsoleStart(true, false, false, false));
    EXPECT_FALSE(trxappgateusbtreadmill::shouldBlockIconsoleStart(true, false, true, false));
    EXPECT_FALSE(trxappgateusbtreadmill::shouldBlockIconsoleStart(false, false, false, true));
}

TEST(TrxAppGateUsTreadmillParserTest, StopsUploadAfterNewerWorkoutRequest) {
    EXPECT_TRUE(trxappgateusbtreadmill::shouldContinueIconsoleUpload(true, 7, 7));
    EXPECT_FALSE(trxappgateusbtreadmill::shouldContinueIconsoleUpload(true, 7, 8));
    EXPECT_FALSE(trxappgateusbtreadmill::shouldContinueIconsoleUpload(false, 7, 7));
}

TEST(TrxAppGateUsTreadmillParserTest, RetainsNewerPendingWorkoutAfterOlderAckFailure) {
    EXPECT_TRUE(trxappgateusbtreadmill::hasNewerPendingIconsoleWorkout(true, 7, 8));
    EXPECT_FALSE(trxappgateusbtreadmill::hasNewerPendingIconsoleWorkout(false, 7, 8));
    EXPECT_FALSE(trxappgateusbtreadmill::hasNewerPendingIconsoleWorkout(true, 7, 7));
}

TEST(TrxAppGateUsTreadmillParserTest, SerializesNestedIconsoleTransactions) {
    EXPECT_FALSE(trxappgateusbtreadmill::shouldSkipIconsoleTransaction(false, false));
    EXPECT_FALSE(trxappgateusbtreadmill::shouldSkipIconsoleTransaction(true, false));
    EXPECT_TRUE(trxappgateusbtreadmill::shouldSkipIconsoleTransaction(true, true));
    EXPECT_FALSE(trxappgateusbtreadmill::shouldSkipIconsoleTransaction(false, true));

    bool pollingTransaction = false;
    {
        trxappgateusbtreadmill::IconsoleTransactionScope pollingScope(pollingTransaction, true);
        EXPECT_TRUE(trxappgateusbtreadmill::shouldSkipIconsoleTransaction(true, pollingTransaction));
    }
    EXPECT_FALSE(pollingTransaction);

    bool transactionInProgress = false;
    {
        trxappgateusbtreadmill::IconsoleTransactionScope uploadScope(transactionInProgress, true);
        EXPECT_TRUE(trxappgateusbtreadmill::shouldSkipIconsoleTransaction(true, transactionInProgress));
        {
            trxappgateusbtreadmill::IconsoleTransactionScope responseWaitScope(transactionInProgress, true);
            EXPECT_TRUE(transactionInProgress);
        }
        EXPECT_TRUE(transactionInProgress);
    }
    EXPECT_FALSE(transactionInProgress);
}

TEST(TrxAppGateUsTreadmillParserTest, MatchesCompleteCapturedIconsoleResponses) {
    EXPECT_TRUE(trxappgateusbtreadmill::iconsoleResponseMatches(0xa0, QByteArray::fromHex("f0b001d374")));
    EXPECT_TRUE(trxappgateusbtreadmill::iconsoleResponseMatches(0xa1, QByteArray::fromHex("f0b101d301150201109e")));
    EXPECT_TRUE(trxappgateusbtreadmill::iconsoleResponseMatches(0xa3, QByteArray::fromHex("f0b301d3020101010101017f")));
    EXPECT_TRUE(trxappgateusbtreadmill::iconsoleResponseMatches(0xa5, QByteArray::fromHex("f0b501d3027b")));
    EXPECT_TRUE(trxappgateusbtreadmill::iconsoleResponseMatches(
        0xa6, QByteArray::fromHex("f0b601d30129013d0151020102150229021502010151013d23")));
    EXPECT_TRUE(trxappgateusbtreadmill::iconsoleResponseMatches(0xaf, QByteArray::fromHex("f0bf01d30285")));
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsWrongOrIncompleteIconsoleResponses) {
    EXPECT_FALSE(trxappgateusbtreadmill::iconsoleResponseMatches(0xa3, QByteArray::fromHex("f0b401d3150101010101010101010197")));
    EXPECT_FALSE(trxappgateusbtreadmill::iconsoleResponseMatches(0xa6, QByteArray::fromHex("f0b601d30129013d015102010215022902150201")));
    EXPECT_FALSE(trxappgateusbtreadmill::iconsoleResponseMatches(0xaf, QByteArray::fromHex("f0bf01d30284")));
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsIconsoleWorkoutWithNoRows) {
    QList<trainrow> rows;
    QList<QByteArray> frames;
    QString error;

    EXPECT_FALSE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames(rows, &frames, &error));
    EXPECT_TRUE(error.contains(QStringLiteral("at least one row")));
    EXPECT_TRUE(frames.isEmpty());
}

TEST(TrxAppGateUsTreadmillParserTest, BuildsConstantTwentyMinuteWorkout) {
    trainrow row;
    row.duration = QTime(0, 20, 0);
    row.speed = 6.0;
    row.inclination = 3.0;

    QList<QByteArray> frames;
    QString error;
    ASSERT_TRUE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames({row}, &frames, &error)) << error.toStdString();
    ASSERT_EQ(frames.size(), 8);

    const QList<double> speeds = {6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0};
    EXPECT_EQ(frames.at(2), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa6, speeds));
    EXPECT_EQ(frames.at(3), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa7, speeds));
    EXPECT_EQ(frames.at(4), QByteArray::fromHex("f0a801d30404040404040404040404040404040404040404bc"));
    EXPECT_EQ(frames.at(5), QByteArray::fromHex("f0a901d302010201020102010201020102010201020102018b"));
    EXPECT_EQ(frames.at(6), QByteArray::fromHex("f0aa01d302010201020102010201020102010201020102018c"));
}

TEST(TrxAppGateUsTreadmillParserTest, PreservesTwoTenMinuteIntervals) {
    trainrow first;
    first.duration = QTime(0, 10, 0);
    first.speed = 4.0;
    trainrow second;
    second.duration = QTime(0, 10, 0);
    second.speed = 8.0;

    QList<QByteArray> frames;
    QString error;
    ASSERT_TRUE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames({first, second}, &frames, &error))
        << error.toStdString();
    ASSERT_EQ(frames.size(), 8);

    const QList<double> firstHalf = {4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0};
    const QList<double> secondHalf = {8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0};
    EXPECT_EQ(frames.at(2), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa6, firstHalf));
    EXPECT_EQ(frames.at(3), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa7, secondHalf));
}

TEST(TrxAppGateUsTreadmillParserTest, BuildsIconsoleWorkoutFromTenTwoMinuteRows) {
    QList<trainrow> rows;
    for (int i = 0; i < 10; ++i) {
        trainrow row;
        row.duration = QTime(0, 2, 0);
        row.speed = 6.0;
        rows.append(row);
    }

    QList<QByteArray> frames;
    QString error;
    ASSERT_TRUE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames(rows, &frames, &error)) << error.toStdString();
    ASSERT_EQ(frames.size(), 8);

    const QList<double> speeds = {6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0};
    EXPECT_EQ(frames.at(2), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa6, speeds));
    EXPECT_EQ(frames.at(3), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa7, speeds));
}

TEST(TrxAppGateUsTreadmillParserTest, MapsTenTwoMinuteRowsToTwentyMinuteSlots) {
    QList<trainrow> rows;
    for (int i = 0; i < 10; ++i) {
        trainrow row;
        row.duration = QTime(0, 2, 0);
        row.speed = i < 5 ? 4.0 : 8.0;
        rows.append(row);
    }

    QList<QByteArray> frames;
    QString error;
    ASSERT_TRUE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames(rows, &frames, &error)) << error.toStdString();
    ASSERT_EQ(frames.size(), 8);

    const QList<double> fourSpeedSlots = {4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0};
    const QList<double> eightSpeedSlots = {8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0};
    EXPECT_EQ(frames.at(2), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa6, fourSpeedSlots));
    EXPECT_EQ(frames.at(3), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa7, eightSpeedSlots));
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsIconsoleWorkoutWithInvalidRows) {
    QList<trainrow> rows;
    for (int i = 0; i < 10; ++i) {
        trainrow row;
        row.duration = QTime(0, 2, 0);
        row.speed = 6.0;
        rows.append(row);
    }
    rows[0].duration = QTime(0, 0, 0);

    QList<QByteArray> frames;
    QString error;
    EXPECT_FALSE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames(rows, &frames, &error));
    EXPECT_TRUE(error.contains(QStringLiteral("positive-duration")));
    EXPECT_TRUE(frames.isEmpty());
}

TEST(TrxAppGateUsTreadmillParserTest, BuildsCompleteIconsoleWorkoutUploadSequence) {
    const QList<double> speeds = {4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 12.0, 10.0, 8.0, 6.0};
    QList<trainrow> rows;
    for (int i = 0; i < speeds.size(); ++i) {
        trainrow row;
        row.duration = QTime(0, 2, 0);
        row.speed = speeds.at(i);
        row.inclination = i;
        rows.append(row);
    }

    QList<QByteArray> frames;
    QString error;
    ASSERT_TRUE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames(rows, &frames, &error)) << error.toStdString();
    ASSERT_EQ(frames.size(), 8);

    EXPECT_EQ(frames.at(0), QByteArray::fromHex("f0a301d3020101010101016f"));
    EXPECT_EQ(frames.at(1), QByteArray::fromHex("f0a401d3150101010101010101010187"));
    EXPECT_EQ(frames.at(2), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(
                                0xa6, {4.0, 4.0, 6.0, 6.0, 8.0, 8.0, 10.0, 10.0, 12.0, 12.0}));
    EXPECT_EQ(frames.at(3), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(
                                0xa7, {14.0, 14.0, 12.0, 12.0, 10.0, 10.0, 8.0, 8.0, 6.0, 6.0}));
    EXPECT_EQ(frames.at(4), QByteArray::fromHex("f0a801d30101020203030404050506060707080809090a0ada"));
    EXPECT_EQ(frames.at(5), QByteArray::fromHex("f0a901d302010201020102010201020102010201020102018b"));
    EXPECT_EQ(frames.at(6), QByteArray::fromHex("f0aa01d302010201020102010201020102010201020102018c"));
    EXPECT_EQ(frames.at(7), QByteArray::fromHex("f0af01d30275"));
}

// Actual TX90 reply from the 2026-10-31 22:00:49 Android session.
TEST(TrxAppGateUsTreadmillParserTest, AcceptsSixteenByteTx90ParameterReply) {
    const QByteArray ack = QByteArray::fromHex("f0b401d3150101010101010101010197");
    ASSERT_EQ(ack.size(), 16);
    EXPECT_TRUE(trxappgateusbtreadmill::iconsoleResponseMatches(0xa4, ack));
    EXPECT_FALSE(trxappgateusbtreadmill::iconsoleResponseMatches(0xa4, ack.left(15)));
    EXPECT_FALSE(trxappgateusbtreadmill::iconsoleResponseMatches(0xa4, ack + char(0)));
    QByteArray corrupt = ack;
    corrupt[15] = char(0x96);
    EXPECT_FALSE(trxappgateusbtreadmill::iconsoleResponseMatches(0xa4, corrupt));
}

TEST(TrxAppGateUsTreadmillParserTest, ReassemblesEveryTx90AckAndTelemetrySplit) {
    const QList<QByteArray> packets = {
        QByteArray::fromHex("f0b401d3150101010101010101010197"),
        QByteArray::fromHex("f0b601d30129013d0151020102150229021502010151013d23"),
        QByteArray::fromHex("f0b201d301010101010101010101010404028b")};
    for (const QByteArray &packet : packets) {
        for (int split = 1; split < packet.size(); ++split) {
            QByteArray buffer = packet.left(split);
            QByteArray frame;
            ASSERT_FALSE(trxappgateusbtreadmill::takeIconsoleResponse(buffer, frame));
            buffer.append(packet.mid(split));
            ASSERT_TRUE(trxappgateusbtreadmill::takeIconsoleResponse(buffer, frame));
            EXPECT_EQ(frame, packet);
            EXPECT_TRUE(buffer.isEmpty());
        }
    }
}

TEST(TrxAppGateUsTreadmillParserTest, SeparatesAckFromFollowingTelemetry) {
    const QByteArray ack = QByteArray::fromHex("f0b401d3150101010101010101010197");
    const QByteArray telemetry = QByteArray::fromHex("f0b201d301010101010101010101010404028b");
    QByteArray buffer = ack + telemetry;
    QByteArray frame;
    ASSERT_TRUE(trxappgateusbtreadmill::takeIconsoleResponse(buffer, frame));
    EXPECT_EQ(frame, ack);
    ASSERT_TRUE(trxappgateusbtreadmill::takeIconsoleResponse(buffer, frame));
    EXPECT_EQ(frame, telemetry);
    EXPECT_TRUE(buffer.isEmpty());
}

TEST(TrxAppGateUsTreadmillParserTest, ResynchronizesAfterCorruptAckAndNoise) {
    const QByteArray telemetry = QByteArray::fromHex("f0b201d301010101010101010101010404028b");
    QByteArray buffer = QByteArray::fromHex("00fff0b401d3150101010101010101010196") + telemetry;
    QByteArray frame;
    ASSERT_TRUE(trxappgateusbtreadmill::takeIconsoleResponse(buffer, frame));
    EXPECT_EQ(frame, telemetry);
    EXPECT_TRUE(buffer.isEmpty());
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsDurationRescalingAndUnrepresentableIntervals) {
    trainrow row;
    row.speed = 6;
    row.duration = QTime(0, 2, 0);
    QList<QByteArray> frames;
    QString error;
    EXPECT_FALSE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames({row}, &frames, &error));
    EXPECT_TRUE(error.contains(QStringLiteral("exactly twenty minutes")));
    EXPECT_TRUE(frames.isEmpty());
    row.duration = QTime(0, 5, 0);
    trainrow second = row;
    row.duration = QTime(0, 4, 30);
    second.duration = QTime(0, 5, 30);
    EXPECT_FALSE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames({row, second}, &frames, &error));
    EXPECT_TRUE(error.contains(QStringLiteral("whole-minute")));
    row.duration = QTime(0, 20, 0);
    row.speed = -1;
    EXPECT_FALSE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames({row}, &frames, &error));
    EXPECT_TRUE(frames.isEmpty());
}

TEST(TrxAppGateUsTreadmillParserTest, ReproducesEntireSuccessfulTx90Profile) {
    const QList<double> capturedSpeeds = {4, 6, 8, 10, 12, 14, 12, 10, 8, 6,
                                         4, 6, 8, 10, 12, 14, 12, 10, 8, 6};
    const QList<double> capturedInclines = {2, 2, 9, 9, 9, 9, 12, 12, 12, 12,
                                           12, 5, 5, 5, 5, 5, 5, 0, 0, 0};
    QList<trainrow> rows;
    for (int i = 0; i < 20; ++i) {
        trainrow row;
        row.duration = QTime(0, 1, 0);
        row.speed = capturedSpeeds[i];
        row.inclination = capturedInclines[i];
        rows.append(row);
    }
    QList<QByteArray> frames;
    ASSERT_TRUE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames(rows, &frames));
    const QList<QByteArray> capturedFrames = {
        QByteArray::fromHex("f0a301d3020101010101016f"),
        QByteArray::fromHex("f0a401d3150101010101010101010187"),
        QByteArray::fromHex("f0a601d30129013d0151020102150229021502010151013d13"),
        QByteArray::fromHex("f0a701d30129013d0151020102150229021502010151013d14"),
        QByteArray::fromHex("f0a801d303030a0a0a0a0d0d0d0d0d06060606060601010102"),
        QByteArray::fromHex("f0a901d302010201020102010201020102010201020102018b"),
        QByteArray::fromHex("f0aa01d302010201020102010201020102010201020102018c"),
        QByteArray::fromHex("f0af01d30275")};
    EXPECT_EQ(frames, capturedFrames);
    for (const auto &request : capturedFrames) {
        QByteArray reply = request;
        reply[1] = char(static_cast<quint8>(reply[1]) + 0x10);
        reply[reply.size()-1] = char(static_cast<quint8>(reply.back()) + 0x10);
        EXPECT_TRUE(trxappgateusbtreadmill::iconsoleResponseMatches(static_cast<quint8>(request[1]), reply));
    }
}

TEST(TrxAppGateUsTreadmillParserTest, TracksCapturedConsoleReadyRunningPauseAndResumeStates) {
    using State = trxappgateusbtreadmill::IconsoleRunState;
    EXPECT_EQ(trxappgateusbtreadmill::iconsoleRunStateFromPacket(
                  QByteArray::fromHex("f0b201d301010101010101010101010404028b")), State::Unknown);
    EXPECT_EQ(trxappgateusbtreadmill::iconsoleRunStateFromPacket(
                  QByteArray::fromHex("f0b201d3010101010101010101010103020288")), State::Ready);
    EXPECT_EQ(trxappgateusbtreadmill::iconsoleRunStateFromPacket(
                  QByteArray::fromHex("f0b201d33c1401010101010101290303040303")), State::Running);
    EXPECT_EQ(trxappgateusbtreadmill::iconsoleRunStateFromPacket(
                  QByteArray::fromHex("f0b201d3240f014d0155010101010a04040366")), State::Paused);
    EXPECT_EQ(trxappgateusbtreadmill::iconsoleRunStateFromPacket(
                  QByteArray::fromHex("f0b201d3240f014d0155010101010a03020262")), State::Ready);
    EXPECT_EQ(trxappgateusbtreadmill::iconsoleRunStateFromPacket(
                  QByteArray::fromHex("f0b201d3230f014e0155010102290a0304038e")), State::Running);
    // A B5 ACK alone is not telemetry and cannot establish belt movement.
    EXPECT_EQ(trxappgateusbtreadmill::iconsoleRunStateFromPacket(
                  QByteArray::fromHex("f0b501d3027b")), State::Unknown);
}

TEST(TrxAppGateUsTreadmillParserTest, LimitsExperimentalSessionToCapturedTx90Name) {
    EXPECT_TRUE(trxappgateusbtreadmill::isIconsoleTx90DeviceName(QStringLiteral("iConsole+0014")));
    EXPECT_TRUE(trxappgateusbtreadmill::isIconsoleTx90DeviceName(QStringLiteral("I-CONSOLE+0014")));
    EXPECT_FALSE(trxappgateusbtreadmill::isIconsoleTx90DeviceName(QStringLiteral("iConsole+0011")));
    EXPECT_FALSE(trxappgateusbtreadmill::isIconsoleTx90DeviceName(QStringLiteral("iConsole+0014-other")));
    EXPECT_FALSE(trxappgateusbtreadmill::isIconsoleTx90DeviceName(QStringLiteral("iConsole+")));
}
