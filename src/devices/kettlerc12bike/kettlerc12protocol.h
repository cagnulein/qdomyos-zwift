#ifndef KETTLERC12PROTOCOL_H
#define KETTLERC12PROTOCOL_H

#include <array>
#include <cstdint>
#include <cstddef>

namespace KettlerC12Protocol {

// ATT handle observed on the ERGO C12 in the Kinomap HCI capture.
// This is a firmware-specific fallback, not a portable characteristic UUID.
constexpr uint16_t targetPowerHandle = 0x0042;
constexpr int maxTargetPowerWatts = 250;
constexpr int minResistanceLevel = 1;
constexpr int maxResistanceLevel = 20;

// MyHomeFIT SecureData.txt is a sequential index-to-16-byte-response table:
// "0000:<32 hex digits>" through "FFFF:<32 hex digits>".
// Its runtime challenge conversion is unsigned big-endian, as verified in
// the attached independent protocol analysis. A full table is not included
// here: it must be supplied separately. This helper can decode 1-4 byte
// challenges but only accepts indices within the 16-bit table.
inline bool challengeIndex(const uint8_t *bytes, std::size_t length, uint16_t &index) {
    if (!bytes || length < 1 || length > 4)
        return false;

    uint32_t value = 0;
    for (std::size_t i = 0; i < length; ++i) {
        value = (value << 8) | bytes[i];
    }
    if (value > 0xffff)
        return false;

    index = static_cast<uint16_t>(value);
    return true;
}

inline int securityHexDigit(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

// Decode only the row for the requested challenge; fail closed if the row is
// missing, malformed, or associated with the wrong index. CRLF is supported.
inline bool parseSecurityRow(const char *row, std::size_t length, uint16_t expectedIndex,
                             std::array<uint8_t, 16> &response) {
    if (!row) return false;
    if (length == 38 && row[37] == '\r') --length;
    if (length != 37 || row[4] != ':') return false;

    uint16_t rowIndex = 0;
    for (unsigned i = 0; i < 4; ++i) {
        const int digit = securityHexDigit(row[i]);
        if (digit < 0) return false;
        rowIndex = static_cast<uint16_t>((rowIndex << 4) | digit);
    }
    if (rowIndex != expectedIndex) return false;

    for (unsigned i = 0; i < 16; ++i) {
        const int high = securityHexDigit(row[5 + i * 2]);
        const int low = securityHexDigit(row[6 + i * 2]);
        if (high < 0 || low < 0) return false;
        response[i] = static_cast<uint8_t>((high << 4) | low);
    }
    return true;
}

// The captured pair is a test vector and also a single-challenge fallback if
// the full lookup asset is unavailable. Never use this for a different index.
inline bool capturedHandshakeResponse(uint16_t index, std::array<uint8_t, 16> &response) {
    if (index != 0x3e7c) return false;
    response = {{0x4e, 0x0e, 0x07, 0x9d, 0xe0, 0xe7, 0xba, 0x94,
                 0x74, 0xaa, 0xab, 0x50, 0x3c, 0x5f, 0x4c, 0xc3}};
    return true;
}

inline uint8_t encodeResistanceLevel(int level) {
    return static_cast<uint8_t>(level < minResistanceLevel ? minResistanceLevel :
                                level > maxResistanceLevel ? maxResistanceLevel : level);
}

inline std::array<uint8_t, 2> encodeTargetPower(int watts) {
    if (watts < 0)
        watts = 0;
    else if (watts > maxTargetPowerWatts)
        watts = maxTargetPowerWatts;

    return {{static_cast<uint8_t>(watts & 0xff),
             static_cast<uint8_t>((watts >> 8) & 0xff)}};
}

} // namespace KettlerC12Protocol

#endif // KETTLERC12PROTOCOL_H
