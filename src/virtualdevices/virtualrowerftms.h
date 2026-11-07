#pragma once

#include <QByteArray>
#include <QtGlobal>
#include <cstdint>

// FTMS Rower Data (0x2AD1) and resistance control (0x2AD9).
namespace virtualrowerftms {

inline void appendLittleEndian16(QByteArray &data, uint16_t value) {
    data.append(static_cast<char>(value & 0xff));
    data.append(static_cast<char>((value >> 8) & 0xff));
}

// Keep the notification within the 20-byte ATT payload at the default MTU.
// Flags 0x03AC include resistance; 0x032C omits it when a device has no telemetry.
inline QByteArray buildRowerData(uint8_t strokeRate, uint16_t strokeCount, uint32_t distanceMeters,
                                uint16_t paceSeconds, int powerWatts, int resistanceLevel,
                                uint16_t calories, uint8_t heartRate, bool hasResistanceTelemetry = true) {
    QByteArray data;
    data.reserve(20);
    appendLittleEndian16(data, hasResistanceTelemetry ? 0x03ac : 0x032c);
    data.append(static_cast<char>(strokeRate));
    appendLittleEndian16(data, strokeCount);
    data.append(static_cast<char>(distanceMeters & 0xff));
    data.append(static_cast<char>((distanceMeters >> 8) & 0xff));
    data.append(static_cast<char>((distanceMeters >> 16) & 0xff));
    appendLittleEndian16(data, paceSeconds);
    appendLittleEndian16(data, static_cast<uint16_t>(static_cast<int16_t>(qBound(-32768, powerWatts, 32767))));
    // Unlike 0x2AD9, Rower Data represents resistance as SINT16 with resolution 1.
    if (hasResistanceTelemetry) {
        appendLittleEndian16(data, static_cast<uint16_t>(static_cast<int16_t>(qBound(-32768, resistanceLevel, 32767))));
    }
    appendLittleEndian16(data, calories);
    appendLittleEndian16(data, calories); // Energy per hour: preserve the existing QZ value
    data.append(static_cast<char>(calories & 0xff)); // Energy per minute: preserve the existing QZ value
    data.append(static_cast<char>(heartRate));
    return data;
}

// Advertised 0x2AD6 values use 0.1 resistance-level units.
inline QByteArray supportedResistanceRange(int minLevel, int maxLevel, int stepLevel) {
    QByteArray data;
    data.reserve(6);
    appendLittleEndian16(data, static_cast<uint16_t>(minLevel * 10));
    appendLittleEndian16(data, static_cast<uint16_t>(maxLevel * 10));
    appendLittleEndian16(data, static_cast<uint16_t>(stepLevel * 10));
    return data;
}

// The FTMS 0x04 command uses tenths of a level. Older clients send one
// UINT8 byte; current FTMS revisions use a little-endian SINT16 parameter.
// Validation uses the individual rower's supported discrete level range.
inline bool decodeTargetResistance(const QByteArray &command, int minLevel, int maxLevel,
                                   int stepLevel, int *level) {
    if (!level || stepLevel <= 0 || minLevel > maxLevel ||
        (command.size() != 2 && command.size() != 3) ||
        static_cast<uint8_t>(command.at(0)) != 0x04)
        return false;

    int raw = static_cast<uint8_t>(command.at(1));
    if (command.size() == 3) {
        raw |= static_cast<uint8_t>(command.at(2)) << 8;
        raw = static_cast<int16_t>(raw);
    }

    // Hardware levels are discrete integers; never silently round a request.
    if (raw % 10 != 0 || raw < minLevel * 10 || raw > maxLevel * 10)
        return false;
    const int candidate = raw / 10;
    if ((candidate - minLevel) % stepLevel != 0)
        return false;
    *level = candidate;
    return true;
}

// Keep the prior helper signature available for existing callers and tests.
inline bool decodeTargetResistance(const QByteArray &command, int maxLevel, int *level) {
    return decodeTargetResistance(command, 1, maxLevel, 1, level);
}

inline QByteArray targetResistanceChangedStatus(int level) {
    QByteArray status;
    status.append(static_cast<char>(0x07)); // Fitness Machine Status: target resistance changed
    appendLittleEndian16(status, static_cast<uint16_t>(level * 10));
    return status;
}
} // namespace virtualrowerftms
