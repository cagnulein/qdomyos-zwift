#include "urevotreadmill/urevoprotocol.h"
#include <gtest/gtest.h>
#include <limits>

// ATT values copied from the 2026-11-06 URTM047 iPhone HCI snoop.
// TX handle 0x0020 = FFF2; TX handle 0x0026 = FEE2.
// RX handle 0x001d = FFF1; RX handle 0x0023 = FEE1.
// The test replays characteristic payloads, not HCI/L2CAP framing.
namespace {
QByteArray bytes(const char *hex) { return QByteArray::fromHex(hex); }
const char *idle = "025100020903";
const char *infoAck = "0250035a005b5203";
const char *auxAck = "5aa503010001100c00220701060000000000000000908f";
const char *queryAck = "0240020800000000000000000000000000001003";
const char *startAck = "025301030d03";
const char *countdown3 = "025102030c03";
const char *running06 = "02510306000000000000000000000000000b00000000003f03";
const char *running20Incline4 = "0251031428610003002f000100000000000b00000000007503";

void expectSingle(UrevoProtocol &protocol, UrevoProtocol::Channel channel, const char *hex) {
    const auto cmds = protocol.takeCommands();
    ASSERT_EQ(cmds.size(), 1);
    EXPECT_EQ(cmds[0].channel, channel);
    EXPECT_EQ(cmds[0].payload, bytes(hex));
}
void captureInit(UrevoProtocol &p, qint64 at) {
    p.subscribed(at);
    expectSingle(p, UrevoProtocol::Channel::Main, "02510b03");
    p.receiveMain(bytes(idle), at + 100);
    expectSingle(p, UrevoProtocol::Channel::Main, "0250030903");
    p.receiveMain(bytes(infoAck), at + 200);
    expectSingle(p, UrevoProtocol::Channel::Auxiliary, "5aa501030001100000cc76");
    p.receiveAuxiliary(bytes(auxAck), at + 300);
    EXPECT_TRUE(p.ready());
    EXPECT_TRUE(p.takeCommands().isEmpty());
}
} // namespace

TEST(UrevoHci, CapturedDiscoveryAndInitNeverStartMotor) {
    UrevoProtocol p;
    EXPECT_EQ(p.state(), UrevoProtocol::State::Disconnected);
    captureInit(p, 1000);
    p.receiveMain(bytes(idle), 2400); // periodic idle notification
    p.tick(2500);
    EXPECT_TRUE(p.takeCommands().isEmpty());
    EXPECT_FALSE(p.running());
}

TEST(UrevoHci, ReplayObservedSecondHandshakeAndDoubleStartWithResponses) {
    UrevoProtocol p;
    captureInit(p, 1000);

    p.receiveMain(bytes(idle), 2000);
    ASSERT_TRUE(p.requestStart(2100));
    expectSingle(p, UrevoProtocol::Channel::Main, "02510b03");
    p.receiveMain(bytes(idle), 2250);
    expectSingle(p, UrevoProtocol::Channel::Main, "0250030903");
    p.receiveMain(bytes(infoAck), 2350);
    expectSingle(p, UrevoProtocol::Channel::Auxiliary, "5aa501030001100000cc76");
    p.receiveAuxiliary(bytes(auxAck), 2450);
    EXPECT_TRUE(p.takeCommands().isEmpty());

    p.tick(3249);
    EXPECT_TRUE(p.takeCommands().isEmpty());
    p.tick(3250);
    expectSingle(p, UrevoProtocol::Channel::Main, "02530100000000000000000e03"); // first start
    p.tick(3430);
    expectSingle(p, UrevoProtocol::Channel::Main, "0240021803"); // status query
    p.receiveMain(bytes(queryAck), 3540);
    p.tick(3989);
    EXPECT_TRUE(p.takeCommands().isEmpty());
    p.tick(3990);
    expectSingle(p, UrevoProtocol::Channel::Main, "02530100000000000000000e03"); // second start
    p.receiveMain(bytes(startAck), 4100);
    expectSingle(p, UrevoProtocol::Channel::Main, "0250020803"); // post-start info
    p.receiveMain(bytes(countdown3), 4200);
    EXPECT_FALSE(p.running()); // countdown does not prove movement
    p.receiveMain(bytes(running06), 4400);
    EXPECT_TRUE(p.running());
    EXPECT_NEAR(p.speedKmh(), 0.9656064, 1e-7);
    EXPECT_DOUBLE_EQ(p.inclination(), 0.0);
}

TEST(UrevoHci, CapturedTargetsUseMphTenthsAndCombinedInclination) {
    UrevoProtocol p;
    captureInit(p, 1000);
    double speed = 0, incline = 0;
    ASSERT_TRUE(UrevoProtocol::decodeFull(bytes(running20Incline4), speed, incline));
    EXPECT_NEAR(speed, 3.218688, 1e-7);
    EXPECT_DOUBLE_EQ(incline, 4.0);
    EXPECT_EQ(UrevoProtocol::targetFrame(speed, incline), bytes("0253021428cb03"));
    EXPECT_EQ(UrevoProtocol::targetFrame(speed, 0), bytes("02530214003303"));
    EXPECT_EQ(UrevoProtocol::targetFrame(9 * 0.1609344, 0), bytes("02530209000403"));
    EXPECT_EQ(UrevoProtocol::targetFrame(speed, 3.0), bytes("025302141edd03"));

    // A physical running notification (after an explicitly acknowledged Start)
    // permits a single combined target. No blind follow-up before new status.
    p.receiveMain(bytes(idle), 2000);
    ASSERT_TRUE(p.requestStart(2100));
    p.takeCommands();
    p.receiveMain(bytes(idle), 2200); p.takeCommands();
    p.receiveMain(bytes(infoAck), 2300); p.takeCommands();
    p.receiveAuxiliary(bytes(auxAck), 2400);
    p.tick(3200); p.takeCommands();
    p.tick(3380); p.takeCommands();
    p.receiveMain(bytes(queryAck), 3490);
    p.tick(3940); p.takeCommands();
    p.receiveMain(bytes(startAck), 4000); p.takeCommands();
    p.receiveMain(bytes(running06), 4500);
    ASSERT_TRUE(p.requestTarget(speed, 4, 4600));
    expectSingle(p, UrevoProtocol::Channel::Main, "0253021428cb03");
    EXPECT_FALSE(p.requestTarget(speed, 0, 4700));
    p.receiveMain(bytes(running20Incline4), 4800);
    EXPECT_TRUE(p.requestTarget(speed, 0, 4900));
    expectSingle(p, UrevoProtocol::Channel::Main, "02530214003303");
}

TEST(UrevoHci, StopPreemptsPendingStartAndNeverAutoRestarts) {
    UrevoProtocol p;
    captureInit(p, 1000);
    p.receiveMain(bytes(idle), 2000);
    ASSERT_TRUE(p.requestStart(2100));
    p.requestStop(false, 2150);
    expectSingle(p, UrevoProtocol::Channel::Main, "0253030c03");
    EXPECT_EQ(p.state(), UrevoProtocol::State::Stopping);
    EXPECT_FALSE(p.requestStart(2200));
    p.receiveMain(bytes(idle), 2300); // potentially delayed pre-stop idle
    EXPECT_EQ(p.state(), UrevoProtocol::State::Stopping);
    p.receiveMain(bytes(idle), 3300);
    EXPECT_TRUE(p.ready()); // local gate only; physical stop not proven
    EXPECT_TRUE(p.takeCommands().isEmpty());
    p.requestStop(true, 3400);
    expectSingle(p, UrevoProtocol::Channel::Main, "02530a0703");
}

TEST(UrevoHci, RejectMalformedUnprovenOrOutOfRangeFrames) {
    UrevoProtocol p;
    EXPECT_FALSE(p.requestStart(1000));
    EXPECT_TRUE(UrevoProtocol::targetFrame(-1, 1).isEmpty());
    EXPECT_TRUE(UrevoProtocol::targetFrame(3, 17).isEmpty());
    EXPECT_TRUE(UrevoProtocol::targetFrame(std::numeric_limits<double>::quiet_NaN(), 3).isEmpty());
    double speed = -1, incline = -1;
    auto full = bytes(running20Incline4);
    ASSERT_TRUE(UrevoProtocol::decodeFull(full, speed, incline));
    full[23] = char(quint8(full[23]) ^ 0x01);
    EXPECT_FALSE(UrevoProtocol::decodeFull(full, speed, incline));
    EXPECT_FALSE(UrevoProtocol::decodeFull(bytes(countdown3), speed, incline));
    EXPECT_FALSE(UrevoProtocol::decodeFull(bytes(idle), speed, incline));
    p.subscribed(1000);
    p.takeCommands();
    p.tick(6000);
    EXPECT_EQ(p.state(), UrevoProtocol::State::Failed); // no speculative retry
    EXPECT_TRUE(p.takeCommands().isEmpty());
    p.reset();
    EXPECT_EQ(p.state(), UrevoProtocol::State::Disconnected);
}

TEST(UrevoHci, InitialStatusDoesNotAuthorizeMotionOrTarget) {
    UrevoProtocol p;
    captureInit(p, 1000);
    p.receiveMain(bytes(running06), 1400);
    EXPECT_FALSE(p.requestTarget(3, 0, 1500)); // not in Running
    EXPECT_FALSE(p.requestStart(1500));        // device is already moving
    EXPECT_TRUE(p.takeCommands().isEmpty());
}
