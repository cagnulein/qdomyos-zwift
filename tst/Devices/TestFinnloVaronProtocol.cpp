#include <gtest/gtest.h>
#include "devices/trxappgateusbbike/finnlovaronprotocol.h"

using Protocol = FinnloVaronProtocol;
using Cmd = Protocol::Command;
namespace {
const uint8_t identity[] = {0x5b,0x81,0x0c,0x74,0x01,0x14,0x90,0x01,0x01,0x28,0xd0,0x5d};
const uint8_t initAck[] = {0x5b,0xc2,0x06,0x31,0xf9,0x5d};
const uint8_t startAck[] = {0x5b,0xc3,0x06,0x31,0xfa,0x5d};
}
TEST(FinnloVaronProtocolTest, ReplaysCapturedHandshakeAndDuplicateIdentity) {
    Protocol p;
    EXPECT_EQ(Cmd::None, p.tick(0));
    EXPECT_EQ(Cmd::IdentityAck, p.receive(identity, sizeof(identity), 0));
    EXPECT_EQ((std::array<uint8_t,6>{{0x5b,0xc1,0x06,0x31,0xf8,0x5d}}), Protocol::encode(Cmd::IdentityAck));
    EXPECT_EQ(Cmd::IdentityAck, p.receive(identity, sizeof(identity), 100));
    EXPECT_EQ(Cmd::None, p.tick(199));
    EXPECT_EQ(Cmd::Initialize, p.tick(200));
    EXPECT_EQ((std::array<uint8_t,6>{{0x5b,0x82,0x06,0x31,0xb9,0x5d}}), Protocol::encode(Cmd::Initialize));
    EXPECT_EQ(Cmd::None, p.receive(initAck, sizeof(initAck), 280));
    EXPECT_EQ(Cmd::IdentityAck, p.receive(identity, sizeof(identity), 500));
    EXPECT_EQ(Cmd::None, p.tick(1179));
    EXPECT_EQ(Cmd::Start, p.tick(1180));
    EXPECT_EQ((std::array<uint8_t,6>{{0x5b,0x83,0x06,0x30,0xb9,0x5d}}), Protocol::encode(Cmd::Start));
    EXPECT_FALSE(p.streaming());
    p.receive(startAck, sizeof(startAck), 1275);
    EXPECT_TRUE(p.streaming());
    EXPECT_EQ(Cmd::Stop, p.stop());
    EXPECT_EQ((std::array<uint8_t,6>{{0x5b,0x83,0x06,0x31,0xba,0x5d}}), Protocol::encode(Cmd::Stop));
    EXPECT_FALSE(p.streaming());
    EXPECT_EQ(Cmd::Start, p.resume(2000));
    p.receive(startAck, sizeof(startAck), 2090);
    EXPECT_TRUE(p.streaming());
    p.reset();
    EXPECT_FALSE(p.streaming());
    EXPECT_EQ(Cmd::None, p.tick(9999));
}

TEST(FinnloVaronProtocolTest, RejectsCorruptedAndPaddedTelemetry) {
    const uint8_t moving[] = {0x5b,0x85,0x0a,0x86,0,0,0x0d,0,0x22,0x5d};
    const uint8_t stopped[] = {0x5b,0x85,0x0a,0,0,0,0,0,0x8f,0x5d};
    const uint8_t padded[] = {0x5b,0x85,0x0a,0xbc,0x03,0,0x64,0,0x86,0x5d,0,0};
    const uint8_t corrupt[] = {0x5b,0x85,0x0a,0x86,0,0,0x0d,0,0x23,0x5d};
    Protocol::Telemetry sample{};
    ASSERT_TRUE(Protocol::telemetry(moving, sizeof(moving), sample));
    EXPECT_EQ(134, sample.first);
    EXPECT_EQ(13, sample.second);
    EXPECT_EQ(0, sample.middle);
    ASSERT_TRUE(Protocol::telemetry(stopped, sizeof(stopped), sample));
    EXPECT_EQ(0, sample.first);
    EXPECT_EQ(0, sample.second);
    EXPECT_FALSE(Protocol::telemetry(padded, sizeof(padded), sample));
    EXPECT_FALSE(Protocol::telemetry(corrupt, sizeof(corrupt), sample));
    EXPECT_FALSE(Protocol::telemetry(moving, sizeof(moving)-1, sample));
    EXPECT_FALSE(Protocol::telemetry(identity, sizeof(identity), sample));
}
TEST(FinnloVaronProtocolTest, RetryIsBoundedAndNoUnsolicitedStart) {
    Protocol p;
    p.receive(startAck, sizeof(startAck), 0);
    EXPECT_FALSE(p.streaming());
    p.receive(identity, sizeof(identity), 10);
    EXPECT_EQ(Cmd::Initialize, p.tick(210));
    EXPECT_EQ(Cmd::Initialize, p.tick(3210));
    EXPECT_EQ(Cmd::Initialize, p.tick(6210));
    EXPECT_EQ(Cmd::None, p.tick(9210));
    EXPECT_EQ(Cmd::None, p.tick(12210));
    EXPECT_EQ(Cmd::IdentityAck, p.receive(identity, sizeof(identity), 12300));
    EXPECT_EQ(Cmd::Initialize, p.tick(12500));
}
