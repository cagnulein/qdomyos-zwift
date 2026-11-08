// Standalone protocol regression test, including byte sequences captured from
// Kinomap controlling an ERGO C12. Run from the repository root with:
// g++ -std=c++11 -Wall -Wextra -Werror tst/Devices/kettlerc12protocoltest.cpp -o /tmp/kettlerc12protocoltest && /tmp/kettlerc12protocoltest

#include "../../src/devices/kettlerc12bike/kettlerc12protocol.h"

#include <cassert>
#include <cstdint>

static void check(int watts, uint8_t low, uint8_t high) {
    const auto value = KettlerC12Protocol::encodeTargetPower(watts);
    assert(value.size() == 2);
    assert(value[0] == low);
    assert(value[1] == high);
}

int main() {
    assert(KettlerC12Protocol::targetPowerHandle == 0x0042);
    check(25, 0x19, 0x00);
    check(50, 0x32, 0x00);
    check(100, 0x64, 0x00);
    check(105, 0x69, 0x00);
    check(250, 0xfa, 0x00);
    check(300, 0xfa, 0x00); // existing driver limit
    check(0, 0x00, 0x00);
    check(-1, 0x00, 0x00);

    // Challenge is interpreted as a big-endian integer; the MyHomeFIT asset
    // maps every 16-bit index to one 16-byte response.
    const uint8_t capturedChallenge[] = {0x3e, 0x7c};
    const uint8_t differentChallenge[] = {0x3e, 0x7d};
    const uint8_t shortestChallenge[] = {0x7c};
    const uint8_t longestValidChallenge[] = {0x00, 0x00, 0x3e, 0x7c};
    const uint8_t tooLargeChallenge[] = {0x01, 0x00, 0x00};
    uint16_t index = 0;

    assert(KettlerC12Protocol::challengeIndex(capturedChallenge, 2, index) && index == 0x3e7c);
    assert(KettlerC12Protocol::challengeIndex(shortestChallenge, 1, index) && index == 0x007c);
    assert(KettlerC12Protocol::challengeIndex(longestValidChallenge, 4, index) && index == 0x3e7c);
    assert(KettlerC12Protocol::challengeIndex(differentChallenge, 2, index) && index == 0x3e7d);
    assert(!KettlerC12Protocol::challengeIndex(tooLargeChallenge, 3, index));
    assert(!KettlerC12Protocol::challengeIndex(capturedChallenge, 0, index));
    assert(!KettlerC12Protocol::challengeIndex(capturedChallenge, 5, index));
    assert(!KettlerC12Protocol::challengeIndex(nullptr, 2, index));

    std::array<uint8_t, 16> handshake{};
    const char correctRow[] = "3E7C:4E0E079DE0E7BA9474AAAB503C5F4CC3";
    assert(KettlerC12Protocol::parseSecurityRow(correctRow, sizeof(correctRow) - 1,
                                                 0x3e7c, handshake));
    assert(handshake[0] == 0x4e && handshake[1] == 0x0e);
    assert(handshake[14] == 0x4c && handshake[15] == 0xc3);

    const char crlfRow[] = "3e7c:4e0e079de0e7ba9474aaab503c5f4cc3\r";
    assert(KettlerC12Protocol::parseSecurityRow(crlfRow, sizeof(crlfRow) - 1, 0x3e7c, handshake));
    assert(!KettlerC12Protocol::parseSecurityRow(correctRow, sizeof(correctRow) - 1,
                                                  0x3e7d, handshake));
    assert(!KettlerC12Protocol::parseSecurityRow("3E7C:DEADBEEF", 13, 0x3e7c, handshake));
    const char invalidHex[] = "3E7C:4E0E079DE0E7BA9474AAAB503C5F4CCZ";
    assert(!KettlerC12Protocol::parseSecurityRow(invalidHex, sizeof(invalidHex) - 1,
                                                  0x3e7c, handshake));

    assert(KettlerC12Protocol::capturedHandshakeResponse(0x3e7c, handshake));
    assert(handshake[0] == 0x4e && handshake[15] == 0xc3);
    assert(!KettlerC12Protocol::capturedHandshakeResponse(0x3e7d, handshake));

    assert(KettlerC12Protocol::encodeResistanceLevel(-1) == 1);
    assert(KettlerC12Protocol::encodeResistanceLevel(1) == 1);
    assert(KettlerC12Protocol::encodeResistanceLevel(11) == 11);
    assert(KettlerC12Protocol::encodeResistanceLevel(20) == 20);
    assert(KettlerC12Protocol::encodeResistanceLevel(90) == 20);
}
