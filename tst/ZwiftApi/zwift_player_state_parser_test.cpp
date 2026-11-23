#include "../../src/zwift-api/zwift_player_state_parser.h"

#include <cassert>
#include <cstring>
#include <limits>
#include <string>

namespace {
void appendAltitude(std::string &message, float altitude) {
    message += "\xd5\x01"; // field 26, wire type 5
    std::uint32_t bits = 0;
    std::memcpy(&bits, &altitude, sizeof(bits));
    for (int i = 0; i < 4; ++i)
        message.push_back(static_cast<char>((bits >> (i * 8)) & 0xff));
}

void check(const std::string &message, bool expected, float expectedDistance = 0, float expectedAltitude = 0) {
    float distance = -1, altitude = -1;
    assert(zwift_player_state::parse(message.data(), message.size(), distance, altitude) == expected);
    if (expected) {
        assert(distance == expectedDistance);
        assert(altitude == expectedAltitude);
    } else {
        assert(distance == -1 && altitude == -1); // A failed parse must not overwrite the caller's sample.
    }
}
} // namespace

int main() {
    std::string state = "\x08\x01\x18\xac\x02"; // id=1, distance=300
    appendAltitude(state, 12.5f);
    check(state, true, 300, 12.5f);

    // Ignore unrelated varint, 64-bit, length-delimited, and fixed32 fields.
    std::string withUnknown("\x41\x00\x01\x02\x03\x04\x05\x06\x07", 9);
    withUnknown += "\x12\x03" "abc"; // field 2, length 3
    withUnknown += state;
    withUnknown.append("\x2d\x00\x00\x80\x3f", 5); // field 5, fixed32
    check(withUnknown, true, 300, 12.5f);

    // Proto3 omits fields at their default values; an altitude of 0 is valid.
    check(std::string("\x18\x01", 2), true, 1, 0);
    check(std::string(), false);
    check(std::string("\x08\x01", 2), false); // unrelated-only payload
    check(std::string("\x18\x80", 2), false); // truncated distance varint
    check(std::string("\x18\x01\xd5\x01\x00", 5), false); // truncated altitude
    check(std::string("\x12\x05\x61", 3), false); // invalid length
    check(std::string("\x1d\x01\x02\x03\x04", 5), false); // wrong wire type for distance
    check(std::string("\x18\x01\xd0\x01\x01", 5), false); // wrong wire type for altitude
    check(std::string("\x00", 1), false); // invalid tag
    std::string nan = "\x18\x01";
    appendAltitude(nan, std::numeric_limits<float>::quiet_NaN());
    check(nan, false);
    return 0;
}
