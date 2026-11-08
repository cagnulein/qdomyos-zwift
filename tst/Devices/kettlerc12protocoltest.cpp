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
}
