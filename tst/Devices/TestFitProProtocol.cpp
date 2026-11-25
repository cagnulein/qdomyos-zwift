// SPDX-License-Identifier: GPL-3.0-or-later
#include "gtest/gtest.h"
#include "devices/proformtreadmill/fitproprotocol.h"
#include <string>

namespace {
using namespace FitProProtocol;
Bytes hex(const std::string &s) {
    Bytes result;
    for (std::size_t i = 0; i + 1 < s.size(); i += 2)
        result.push_back(static_cast<uint8_t>(std::stoul(s.substr(i, 2), nullptr, 16)));
    return result;
}

TEST(FitProProtocolTest, OpenFitT65MatchesQzLegacyUnlock) {
    const Bytes d = hex("041d810251010300000000000fdbfefbdbfcfbfb0ce71f38c0d31818b6");
    const Bytes s = hex("04218202640001edae0a00d6350600770033e8030024f400f40100000102000069");
    const Bytes v = hex("041c840251d200302e312e30363132323031372e30393038012a0346");
    const Bytes expected = hex("0701cec4b0aaa2a8949696aca8a2bad0dccefe14003a52786486a6fc18324aa088020000");
    EXPECT_EQ(expected, computeUnlock(d, s, v));
    // Compare all BLE writes against the existing T6.5S 81 miles profile.
    EXPECT_EQ((Chunks{hex("fe022c04"),
                      hex("0012020402280428900701cec4b0aaa2a8949696"),
                      hex("0112aca8a2bad0dccefe14003a52786486a6fc18"),
                      hex("ff08324aa0880200004400000000000000000000")}),
              encode(command(0x04, 0x90, expected)));
}

TEST(FitProProtocolTest, CapturedTSeries8MatchesQzAndAcceptedReply) {
    // QZ capture 2026-11-12. Notification padding is deliberately nonzero.
    const Bytes d = decodeReply({hex("fe02210302ffff10800100ffdf08006fdf8a007f"),
                                hex("00120104021d041d810254010100000000000fd3"),
                                hex("ff0ffffbdbfcfb3b0ce71f00c0d71818bc000fd3")});
    const Bytes s = decodeReply({hex("fe022504"),
                                hex("00120104022104218202640001b928000056ef06"),
                                hex("01120042002be8030024f400f401000001020000"),
                                hex("ff01a242002be8030024f400f401000001020000")});
    const Bytes v = decodeReply({hex("fe022003"),
                                hex("00120104021c041c840254f900302e312e303631"),
                                hex("ff0e32323031372e30393038012a03702e303631")});
    ASSERT_FALSE(d.empty());
    ASSERT_FALSE(s.empty());
    ASSERT_FALSE(v.empty());
    const Bytes expected = hex("077632f4b4723e08c09e6a3c0cdaa690482602e4c4a28e98704e5a2c3c0a1620a0020000");
    EXPECT_EQ(expected, computeUnlock(d, s, v));
    EXPECT_EQ((Chunks{hex("fe022c04"),
                      hex("001202040228042890077632f4b4723e08c09e6a"),
                      hex("01123c0cdaa690482602e4c4a28e98704e5a2c3c"),
                      hex("ff080a1620a00200002d00000000000000000000")}),
              encode(command(0x04, 0x90, expected)));
    const Bytes ack = decodeReply({hex("fe020a02"), hex("ff0a010402060406900208a4")});
    EXPECT_TRUE(successfulReply(ack, 0x90, 6));
}

TEST(FitProProtocolTest, OtherCapturedConsolesDoNotMatchLegacySelection) {
    // QZ captures 2026-10-30 and 2026-10-08 sent the same device-07
    // hardcoded payload, but neither capture establishes successful unlock.
    const Bytes legacy = hex("070110cc7a3ef4b8663af8b472461cf0be92403ceacea488764a2804e2f6cce098020000");
    const Bytes a = computeUnlock(hex("041981024c010300000000000bdbfffbdbfcfbfb0ce71f38e7"),
                                  hex("04218202640001b75b010026ad0500890029e8030024f400f401010001020000a7"),
                                  hex("041c84024c5c00302e312e31323133323031352e32313535012a03c3"));
    const Bytes b = computeUnlock(hex("041d810254010100000000000fdbfffbdbfcfbfb0ce71f00c0d51358bd"),
                                  hex("042182026400026fca0a00d5dc05009d00527d00006cdc02640001010102000027"),
                                  hex("041c840254d700302e312e30313036323032312e31313335012a0340"));
    EXPECT_EQ(hex("040028eca26e2cf8be8a4014eab684702602f8dcb29e7c482e3a10e4fac6d4e060020000"), a);
    EXPECT_EQ(hex("04e250cc3abe34b826ba38b432c65cf07e12803caa4ee48836ca6804a2760ce0a0020000"), b);
    EXPECT_NE(legacy, a);
    EXPECT_NE(legacy, b);
}

TEST(FitProProtocolTest, RejectsCorruptOrUnsuccessfulDiscovery) {
    Bytes d = command(0x04, 0x81, {2, 81, 1, 3, 0, 0, 0});
    const Bytes s = command(0x04, 0x82, {2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0});
    const Bytes v = command(0x04, 0x84, {2, 81});
    ASSERT_EQ(36u, computeUnlock(d, s, v).size());
    EXPECT_TRUE(computeUnlock({}, s, v).empty());
    d.back() ^= 1;
    EXPECT_TRUE(computeUnlock(d, s, v).empty());
    EXPECT_TRUE(computeUnlock(command(0x04, 0x81, {4, 81, 1, 3, 0, 0, 0}), s, v).empty());
    EXPECT_TRUE(computeUnlock(command(0x04, 0x82, {2, 81, 1, 3, 0, 0, 0}), s, v).empty());
    EXPECT_TRUE(computeUnlock(command(0x04, 0x81, {2, 81}), s, v).empty());
}

TEST(FitProProtocolTest, RejectsMalformedNotificationGroups) {
    const Chunks good{hex("fe020a02"), hex("ff0a010402060406900208a4")};
    ASSERT_FALSE(decodeReply(good).empty());
    EXPECT_TRUE(decodeReply({good.front()}).empty());
    for (const auto &bad : {hex("000a010402060406900208a4"),
                            hex("ff0b010402060406900208a4"),
                            hex("ff0a010402060406900208a5"),
                            hex("ff0a020402060406900208a4"),
                            hex("ff0a010402070406900208a4")})
        EXPECT_TRUE(decodeReply({good.front(), bad}).empty());
    EXPECT_TRUE(decodeReply({hex("fe020a03"), good.back()}).empty());
    EXPECT_TRUE(decodeReply({hex("fe020b02"), good.back()}).empty());
}

TEST(FitProProtocolTest, FramingAndChecksumForStandardSpeedCommand) {
    const Bytes frame = command(4, 2, hex("0101bc0200"));
    EXPECT_EQ(hex("0409020101bc0200cf"), frame);
    // 7.00 km/h, two chunks. Compare the additive checksum used by
    // most QZ profiles; legacy profiles with exceptions remain untouched.
    EXPECT_EQ((Chunks{hex("fe020d02"), hex("ff0d020402090409020101bc0200cf0000000000")}), encode(frame));
    EXPECT_TRUE(encode({4, 4, 2, 0}).empty());
    EXPECT_TRUE(command(4, 2, Bytes(252, 0)).empty());
    EXPECT_TRUE(encode(command(4, 2, Bytes(248, 0))).empty());
}
TEST(FitProProtocolTest, CapturedTelemetryAndArmingFields) {
    ReplyAssembler assembler;
    EXPECT_TRUE(assembler.append(hex("fe0235040000005802000000000000000000c12c")).empty());
    EXPECT_TRUE(assembler.append(hex("0012010402310431020200000000000000000000")).empty());
    EXPECT_TRUE(assembler.append(hex("011200000000010000000000000000000000002c")).empty());
    const Bytes frame = assembler.append(hex("ff1101000000005802000000000000000000c12c"));
    Fields values;
    ASSERT_TRUE(decodeFields(frame, telemetryFields(), values));
    EXPECT_EQ(0u, values.at(0));
    EXPECT_EQ(0u, values.at(16));
    EXPECT_EQ(1u, values.at(12));
    EXPECT_FALSE(decodeFields(command(4, 2, {2, 0, 0}), telemetryFields(), values));
    EXPECT_TRUE(values.empty());
    EXPECT_EQ(hex("0415020e000000000000000000000000001001003a"), command(4, 2, writeFields({{108, 1}})));
    EXPECT_EQ(hex("0413020c0000000000000000000000800000a5"), command(4, 2, writeFields({{95, 0}})));
    EXPECT_EQ(hex("020310a000ceff0200"), writeFields({{12, 2}, {1, wireHundredths(-0.5)}, {0, 160}}));
    EXPECT_DOUBLE_EQ(-0.5, signedHundredths(wireHundredths(-0.5)));
    EXPECT_FALSE(within(22.1, 0.5, 22.0));
    EXPECT_FALSE(within(0.1, 0.5, 22.0));
    EXPECT_TRUE(within(1.0, 0.5, 22.0));
}

TEST(FitProProtocolTest, RunningCaptureAndSignedIncline) {
    const Bytes running = hex("04310202a00096007f000700000000000000021100000011000000106101002c014f007400b400580200005802000000e3");
    Fields fields;
    ASSERT_TRUE(decodeFields(running, telemetryFields(), fields));
    EXPECT_EQ(160u, fields.at(0));
    EXPECT_EQ(150u, fields.at(1));
    EXPECT_EQ(2u, fields.at(12));
    EXPECT_EQ(0u, fields.at(16)); // actual speed can be zero despite target
    EXPECT_EQ(hex("040902020010040025"), command(4, 2, writeFields({{12, 4}})));
}
} // namespace
