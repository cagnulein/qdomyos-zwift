#include "TestXcxBikeParser.h"

TEST_P(XcxBikeParserTest, DecodesCapturedFff6Telemetry) {
  XcxTelemetry actual;
  ASSERT_TRUE(xcxbike::parseTelemetry(std::get<0>(GetParam()), &actual));
  const XcxTelemetry &expected = std::get<1>(GetParam());
  EXPECT_EQ(expected.state1, actual.state1);
  EXPECT_EQ(expected.state2, actual.state2);
  EXPECT_EQ(expected.timer, actual.timer);
  EXPECT_DOUBLE_EQ(expected.speedKph, actual.speedKph);
  EXPECT_EQ(expected.distanceRaw, actual.distanceRaw);
  EXPECT_EQ(expected.energy, actual.energy);
  EXPECT_EQ(expected.cadence, actual.cadence);
  EXPECT_EQ(expected.unknown15, actual.unknown15);
  EXPECT_EQ(expected.power, actual.power);
  EXPECT_EQ(expected.resistance, actual.resistance);
}

INSTANTIATE_TEST_SUITE_P(
    CapturedPackets, XcxBikeParserTest,
    testing::Values(
        std::make_tuple(
            QByteArray::fromHex("fa0504003b00780000000001002e0000006e3c00"),
            XcxTelemetry{4, 0, 59, 12.0, 0, 1, 46, 0, 110, 60}),
        std::make_tuple(
            QByteArray::fromHex("fa05040021009300010000200038000100823c00"),
            XcxTelemetry{4, 0, 33, 14.7, 1, 32, 56, 1, 130, 60}),
        std::make_tuple(
            QByteArray::fromHex("fa0504000100d6000200004f0051000200b43c00"),
            XcxTelemetry{4, 0, 1, 21.4, 2, 79, 81, 2, 180, 60}),
        std::make_tuple(
            QByteArray::fromHex("fa05000100002800050000a50010000500323c00"),
            XcxTelemetry{0, 1, 0, 4.0, 5, 165, 16, 5, 50, 60})));

TEST(XcxBikeParserRejectionTest, RejectsWrongLengthAndHeaders) {
  XcxTelemetry telemetry;
  EXPECT_FALSE(
      xcxbike::parseTelemetry(QByteArray::fromHex("fa05"), &telemetry));
  EXPECT_FALSE(xcxbike::parseTelemetry(
      QByteArray::fromHex("fb0504003b00780000000001002e0000006e3c00"),
      &telemetry));
  EXPECT_FALSE(xcxbike::parseTelemetry(
      QByteArray::fromHex("fa0604003b00780000000001002e0000006e3c00"),
      &telemetry));
}
