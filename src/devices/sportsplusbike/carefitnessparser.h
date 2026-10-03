#ifndef CAREFITNESSPARSER_H
#define CAREFITNESSPARSER_H

#include <QByteArray>

#include <cstdint>

namespace carefitness {

enum class Protocol {
    Unknown,
    Cv351,
    Cv385,
};

struct Telemetry {
    bool hasSpeed = false;
    double speed = 0;
    bool hasCadence = false;
    double cadence = 0;
};

inline Protocol detectProtocol(Protocol current, const QByteArray &packet) {
    if (current != Protocol::Unknown || packet.size() < 2) {
        return current;
    }

    switch (static_cast<uint8_t>(packet.at(1))) {
    case 0x20:
    case 0x30:
        return Protocol::Cv351;
    case 0x00:
    case 0x10:
        return Protocol::Cv385;
    default:
        return Protocol::Unknown;
    }
}

inline Telemetry parsePacket(Protocol protocol, const QByteArray &packet) {
    Telemetry result;
    if (packet.size() != 12) {
        return result;
    }

    const uint8_t type = static_cast<uint8_t>(packet.at(1));
    const uint16_t value = static_cast<uint16_t>((static_cast<uint8_t>(packet.at(2)) << 8) |
                                                 static_cast<uint8_t>(packet.at(3)));

    if (protocol == Protocol::Cv351 && type == 0x30) {
        const uint8_t bcd = static_cast<uint8_t>(packet.at(3));
        result.hasCadence = true;
        result.cadence = static_cast<double>(((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F));
    } else if (protocol == Protocol::Cv385 && type == 0x00) {
        result.hasSpeed = true;
        result.speed = static_cast<double>(value) / 100.0;
    } else if (protocol == Protocol::Cv385 && type == 0x10) {
        result.hasCadence = true;
        result.cadence = static_cast<double>(value);
    }

    return result;
}

} // namespace carefitness

#endif // CAREFITNESSPARSER_H
