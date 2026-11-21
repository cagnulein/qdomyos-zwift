#ifndef BKOOLCADENCEPARSER_H
#define BKOOLCADENCEPARSER_H

#include <QByteArray>
#include <QtGlobal>

namespace BkoolCadence {

// The vendor's 5-byte cadence sample is still scaled by 36 (as in PR #4541).
// Values above 200 rpm were seen repeatedly alongside implausible 200+ rpm
// telemetry in a Bkool Smart Bike v1 trace; reject them rather than forwarding
// unrealistic speed/distance. Do not clamp them to 200.
constexpr double kMaxCadenceRpm = 200.0;
constexpr double kCustomCadenceScale = 36.0;

inline quint16 readLe16(const QByteArray &data, int index) {
    return static_cast<quint16>(static_cast<quint8>(data.at(index))) |
           (static_cast<quint16>(static_cast<quint8>(data.at(index + 1))) << 8);
}

inline quint32 readLe32(const QByteArray &data, int index) {
    return static_cast<quint32>(readLe16(data, index)) |
           (static_cast<quint32>(readLe16(data, index + 2)) << 16);
}

inline double parseCustomCadence(const QByteArray &data, bool fitnessBike) {
    if (data.size() != 5) {
        return -1.0;
    }
    const quint16 raw = readLe16(data, 2);
    if (raw == 0) {
        return 0.0;
    }
    const double cadence = raw / kCustomCadenceScale;
    // Keep the old <255 rpm acceptance for BKOOLSMARTPRO; the stricter
    // ceiling is specific to the Smart Bike v1 data observed in the log.
    const bool withinRange = fitnessBike ? cadence <= kMaxCadenceRpm : cadence < 255.0;
    return cadence > 0.0 && withinRange ? cadence : -1.0;
}

struct Counter {
    quint32 revolutions = 0;
    quint16 eventTime = 0;
    bool isWheel = false;
};

struct PowerSample {
    bool hasPublicationCounter = false;
    bool hasCadenceCounter = false;
    bool emptyCrankFields = false;
    Counter publicationCounter;
    Counter cadenceCounter;
};

// The Cycling Power Measurement payload contains [flags:2][power:2],
// optional balance/torque, then wheel revolution data (6 bytes) followed by
// crank revolution data (4 bytes). Bkool v1 sets both flags (0x30), but its
// crank fields are zero; wheel revolutions are not pedal revolutions.
// Preserve the wheel counter for legacy virtual-device publication, but do
// not infer *cadence* from it when crank data was advertised but empty.
inline PowerSample parsePowerSample(const QByteArray &data, quint16 flags, int index, bool fitnessBike) {
    PowerSample sample;
    if (index < 0 || index > data.size()) {
        return sample;
    }

    const bool hasWheel = (flags & 0x10u) != 0u;
    const bool hasCrank = (flags & 0x20u) != 0u;

    if (hasWheel) {
        if (data.size() - index < 6) {
            return sample;
        }
        Counter wheel;
        wheel.revolutions = readLe32(data, index);
        wheel.eventTime = readLe16(data, index + 4);
        wheel.isWheel = true;
        sample.publicationCounter = wheel;
        sample.hasPublicationCounter = true;
        index += 6;
        if (!hasCrank || !fitnessBike) {
            // Legacy BKOOLSMARTPRO behavior: prefer the wheel counter even
            // if crank data is advertised. The v1 fix is opt-in by model.
            sample.cadenceCounter = wheel;
            sample.hasCadenceCounter = true;
        }
    }

    if (hasCrank) {
        if (data.size() - index < 4) {
            return sample;
        }
        Counter crank;
        crank.revolutions = readLe16(data, index);
        crank.eventTime = readLe16(data, index + 2);
        if (hasWheel && crank.revolutions == 0 && crank.eventTime == 0) {
            sample.emptyCrankFields = fitnessBike;
        } else if (fitnessBike || !hasWheel) {
            // On v1 prefer the actual crank block. Non-v1 wheel+crank
            // packets retain the original wheel-first behavior.
            sample.publicationCounter = crank;
            sample.hasPublicationCounter = true;
            sample.cadenceCounter = crank;
            sample.hasCadenceCounter = true;
        }
    }
    return sample;
}

// Returns -1 when there is no meaningful new cadence; caller keeps the last
// valid sample or applies its existing 2-second stop timeout.
inline double cadenceFromCounters(const Counter &previous, const Counter &current,
                                  double maxRpm = kMaxCadenceRpm) {
    if (previous.isWheel != current.isWheel) {
        return -1.0;
    }
    const quint32 deltaRevolutions = current.isWheel
                                        ? current.revolutions - previous.revolutions
                                        : static_cast<quint16>(current.revolutions - previous.revolutions);
    const quint16 deltaTime = static_cast<quint16>(current.eventTime - previous.eventTime);
    if (deltaTime == 0 || deltaRevolutions == 0) {
        return -1.0;
    }
    const double tickRate = current.isWheel ? 2048.0 : 1024.0;
    const double cadence = static_cast<double>(deltaRevolutions) * tickRate * 60.0 /
                           static_cast<double>(deltaTime);
    return cadence > 0.0 && (maxRpm <= 0.0 || cadence <= maxRpm) ? cadence : -1.0;
}

} // namespace BkoolCadence

#endif // BKOOLCADENCEPARSER_H
