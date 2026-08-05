#pragma once

#include <gtest/gtest.h>
#include <QByteArray>
#include <QString>
#include <type_traits>

#include "devices/ic15dbike/IC15DProtocol.h"
#include "devices/ic15dbike/IC15DUART.h"
#include "devices/ic15dbike/ic15dbike.h"

namespace {

// Independent bitwise CRC16-CCITT (poly 0x1021, init 0x0000) reference implementation, mirroring
// the style already used by keepbike.cpp, to cross-check IC15DProtocol::crc16's nibble-table
// implementation without hardcoding APK-derived constants that haven't been independently
// confirmed.
quint16 referenceCrc16(const QByteArray &data) {
    quint16 crc = 0;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data.at(i)) << 8;
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc & 0x8000) ? static_cast<quint16>((crc << 1) ^ 0x1021) : static_cast<quint16>(crc << 1);
    }
    return crc;
}

// Builds a well-formed wire frame (header, status, byte2, command, value, crc16, terminator),
// byte-stuffing any interior byte >= 253 as the protocol requires, for use as test fixtures.
QByteArray buildTestWireFrame(quint8 status, quint8 byte2, quint8 command, quint8 value) {
    QByteArray logical;
    logical.append(static_cast<char>(IC15DProtocol::FRAME_HEADER));
    logical.append(static_cast<char>(status));
    logical.append(static_cast<char>(byte2));
    logical.append(static_cast<char>(command));
    logical.append(static_cast<char>(value));

    const quint16 crc = IC15DProtocol::crc16(logical);
    logical.append(static_cast<char>((crc >> 8) & 0xFF));
    logical.append(static_cast<char>(crc & 0xFF));

    // Stuff every interior byte (everything but the header) that needs it.
    QByteArray wire;
    wire.append(static_cast<char>(IC15DProtocol::FRAME_HEADER));
    for (int i = 1; i < logical.size(); ++i) {
        quint8 b = static_cast<quint8>(logical.at(i));
        if (b >= IC15DProtocol::STUFF_OFFSET) {
            wire.append(static_cast<char>(IC15DProtocol::STUFF_ESCAPE));
            wire.append(static_cast<char>(b - IC15DProtocol::STUFF_OFFSET));
        } else {
            wire.append(static_cast<char>(b));
        }
    }
    wire.append(static_cast<char>(IC15DProtocol::FRAME_TERMINATOR));
    return wire;
}

} // namespace

TEST(IC15DProtocolUnstuffTest, PassesThroughUnescapedBytes) {
    const QByteArray in = QByteArray::fromHex("0102037f");
    EXPECT_EQ(IC15DProtocol::unstuff(in), in);
}

TEST(IC15DProtocolUnstuffTest, DecodesEscapedReservedBytes) {
    // 0xFD 0x00 -> 253 (0xFD), 0xFD 0x01 -> 254 (0xFE), 0xFD 0x02 -> 255 (0xFF)
    QByteArray in;
    in.append(static_cast<char>(0xFD)).append(static_cast<char>(0x00));
    in.append(static_cast<char>(0xFD)).append(static_cast<char>(0x01));
    in.append(static_cast<char>(0xFD)).append(static_cast<char>(0x02));

    const QByteArray out = IC15DProtocol::unstuff(in);
    ASSERT_EQ(out.size(), 3);
    EXPECT_EQ(static_cast<quint8>(out.at(0)), 0xFD);
    EXPECT_EQ(static_cast<quint8>(out.at(1)), 0xFE);
    EXPECT_EQ(static_cast<quint8>(out.at(2)), 0xFF);
}

TEST(IC15DProtocolUnstuffTest, DropsTruncatedTrailingEscape) {
    QByteArray in = QByteArray::fromHex("01");
    in.append(static_cast<char>(0xFD)); // escape byte with nothing following

    const QByteArray out = IC15DProtocol::unstuff(in);
    EXPECT_EQ(out, QByteArray::fromHex("01"));
}

TEST(IC15DProtocolCrc16Test, MatchesIndependentBitwiseCcittReferenceImplementation) {
    const QByteArray samples[] = {
        QByteArray::fromHex(""),
        QByteArray::fromHex("00"),
        QByteArray::fromHex("fe01032e10"),
        QByteArray::fromHex("fe0103014affeeddccbbaa998877665544332211"),
        QByteArray::fromHex("ffffffffffffffff"),
    };

    for (const QByteArray &sample : samples)
        EXPECT_EQ(IC15DProtocol::crc16(sample), referenceCrc16(sample)) << sample.toHex().toStdString();
}

TEST(IC15DProtocolParseWireFrameTest, DecodesRpmFrame) {
    const QByteArray wire = buildTestWireFrame(/*status=*/0x01, /*byte2=*/0x00, IC15DProtocol::CMD_RPM,
                                                /*value=*/0x4B);

    const IC15DProtocol::ParsedFrame parsed = IC15DProtocol::parseWireFrame(wire);

    EXPECT_TRUE(parsed.valid);
    EXPECT_EQ(parsed.status, 0x01);
    EXPECT_EQ(parsed.command, IC15DProtocol::CMD_RPM);
    EXPECT_EQ(parsed.value, 0x4B);
}

TEST(IC15DProtocolParseWireFrameTest, DecodesLevelFrame) {
    const QByteArray wire = buildTestWireFrame(/*status=*/0x02, /*byte2=*/0x00, IC15DProtocol::CMD_LEVEL,
                                                /*value=*/0x07);

    const IC15DProtocol::ParsedFrame parsed = IC15DProtocol::parseWireFrame(wire);

    EXPECT_TRUE(parsed.valid);
    EXPECT_EQ(parsed.command, IC15DProtocol::CMD_LEVEL);
    EXPECT_EQ(parsed.value, 0x07);
}

TEST(IC15DProtocolParseWireFrameTest, RoundTripsStuffedReservedValueByte) {
    // value == 0xFE forces byte-stuffing of the value byte itself.
    const QByteArray wire = buildTestWireFrame(/*status=*/0x01, /*byte2=*/0x00, IC15DProtocol::CMD_RPM,
                                                /*value=*/0xFE);

    ASSERT_TRUE(wire.contains(QByteArray::fromHex("fd01"))); // sanity: stuffing actually happened

    const IC15DProtocol::ParsedFrame parsed = IC15DProtocol::parseWireFrame(wire);
    EXPECT_TRUE(parsed.valid);
    EXPECT_EQ(parsed.value, 0xFE);
}

TEST(IC15DProtocolParseWireFrameTest, RejectsMissingHeader) {
    QByteArray wire = buildTestWireFrame(0x01, 0x00, IC15DProtocol::CMD_RPM, 0x4B);
    wire[0] = static_cast<char>(0x00);
    EXPECT_FALSE(IC15DProtocol::parseWireFrame(wire).valid);
}

TEST(IC15DProtocolParseWireFrameTest, RejectsMissingTerminator) {
    QByteArray wire = buildTestWireFrame(0x01, 0x00, IC15DProtocol::CMD_RPM, 0x4B);
    wire[wire.size() - 1] = static_cast<char>(0x00);
    EXPECT_FALSE(IC15DProtocol::parseWireFrame(wire).valid);
}

TEST(IC15DProtocolParseWireFrameTest, RejectsCorruptedCrc) {
    QByteArray wire = buildTestWireFrame(0x01, 0x00, IC15DProtocol::CMD_RPM, 0x4B);
    // Flip a bit in the byte just before the terminator (last CRC byte).
    wire[wire.size() - 2] = static_cast<char>(static_cast<quint8>(wire[wire.size() - 2]) ^ 0xFF);
    EXPECT_FALSE(IC15DProtocol::parseWireFrame(wire).valid);
}

TEST(IC15DProtocolParseWireFrameTest, RejectsTruncatedFrame) {
    const QByteArray wire = buildTestWireFrame(0x01, 0x00, IC15DProtocol::CMD_RPM, 0x4B);
    EXPECT_FALSE(IC15DProtocol::parseWireFrame(wire.left(4)).valid);
    EXPECT_FALSE(IC15DProtocol::parseWireFrame(QByteArray()).valid);
}

TEST(IC15DProtocolParseWireFrameTest, RejectsEmptyAndSingleByteInput) {
    EXPECT_FALSE(IC15DProtocol::parseWireFrame(QByteArray()).valid);
    EXPECT_FALSE(IC15DProtocol::parseWireFrame(QByteArray::fromHex("fe")).valid);
}

TEST(IC15DProtocolFindWireFrameTest, ReturnsZeroWhenFrameIncomplete) {
    const QByteArray wire = buildTestWireFrame(0x01, 0x00, IC15DProtocol::CMD_RPM, 0x4B);
    QByteArray out;
    EXPECT_EQ(IC15DProtocol::findWireFrame(wire.left(wire.size() - 1), out), 0);
}

TEST(IC15DProtocolFindWireFrameTest, ReturnsNegativeOneWhenNotStartingWithHeader) {
    QByteArray out;
    EXPECT_EQ(IC15DProtocol::findWireFrame(QByteArray::fromHex("00010203"), out), -1);
}

TEST(IC15DProtocolFindWireFrameTest, DoesNotMistakeStuffedTerminatorByteForRealTerminator) {
    // value == 0xFF forces the value byte to be stuffed (0xFD 0x02); findWireFrame must not stop
    // scanning at the escaped byte and must keep looking for the real, unescaped terminator.
    const QByteArray wire = buildTestWireFrame(0x01, 0x00, IC15DProtocol::CMD_RPM, 0xFF);
    QByteArray out;
    const int consumed = IC15DProtocol::findWireFrame(wire, out);

    EXPECT_EQ(consumed, wire.size());
    EXPECT_EQ(out, wire);
    EXPECT_TRUE(IC15DProtocol::parseWireFrame(out).valid);
}

TEST(IC15DProtocolFindWireFrameTest, ExtractsOnlyFirstFrameFromConcatenatedStream) {
    const QByteArray first = buildTestWireFrame(0x01, 0x00, IC15DProtocol::CMD_RPM, 0x4B);
    const QByteArray second = buildTestWireFrame(0x02, 0x00, IC15DProtocol::CMD_LEVEL, 0x07);
    const QByteArray stream = first + second;

    QByteArray out;
    const int consumed = IC15DProtocol::findWireFrame(stream, out);

    EXPECT_EQ(consumed, first.size());
    EXPECT_EQ(out, first);
}

// --- Receive-only API-surface checks -----------------------------------------------------
// The IC15D driver must never write/flush/configure the port (issue #4888 requires the stock
// console app's active session to be left untouched). These compile-time traits assert that no
// such public methods exist on IC15DUART/ic15dbike -- if someone adds one, this file fails to
// compile rather than silently allowing a write path back in.
namespace ic15d_write_api_traits {

template <typename T, typename = void> struct has_setResistance : std::false_type {};
template <typename T>
struct has_setResistance<T, std::void_t<decltype(std::declval<T>().setResistance(0))>> : std::true_type {};

template <typename T, typename = void> struct has_sendQuery : std::false_type {};
template <typename T>
struct has_sendQuery<T, std::void_t<decltype(std::declval<T>().sendQuery())>> : std::true_type {};

template <typename T, typename = void> struct has_sendReset : std::false_type {};
template <typename T>
struct has_sendReset<T, std::void_t<decltype(std::declval<T>().sendReset())>> : std::true_type {};

template <typename T, typename = void> struct has_write : std::false_type {};
template <typename T> struct has_write<T, std::void_t<decltype(std::declval<T>().write(nullptr, 0))>> : std::true_type {};

static_assert(!has_setResistance<IC15DUART>::value, "IC15DUART must stay receive-only: no setResistance()");
static_assert(!has_sendQuery<IC15DUART>::value, "IC15DUART must stay receive-only: no sendQuery()");
static_assert(!has_sendReset<IC15DUART>::value, "IC15DUART must stay receive-only: no sendReset()");
static_assert(!has_write<IC15DUART>::value, "IC15DUART must stay receive-only: no write()");

} // namespace ic15d_write_api_traits

TEST(IC15DReceiveOnlyApiTest, CompileTimeAssertionsRanAtBuildTime) {
    // The meaningful assertions for this test are the static_asserts above, evaluated at compile
    // time; this test body just gives them a visible place in the test report.
    SUCCEED();
}

TEST(IC15DConsoleDetectionTest, MatchesOnlyTheExactInspireIc15dModelString) {
    EXPECT_TRUE(ic15dbike::matchesConsoleModel(QStringLiteral("BX_RK3326_A11_INSPIRE_IC15D")));
    EXPECT_FALSE(ic15dbike::matchesConsoleModel(QStringLiteral("bx_rk3326_a11_inspire_ic15d")));
    EXPECT_FALSE(ic15dbike::matchesConsoleModel(QStringLiteral("BX_RK3326_A11_INSPIRE_IC15D_X")));
    EXPECT_FALSE(ic15dbike::matchesConsoleModel(QStringLiteral("")));
    EXPECT_FALSE(ic15dbike::matchesConsoleModel(QStringLiteral("SM-G960F")));
}
