#include "kettlerclassicprotocol.h"

#include <QCryptographicHash>

#include <cmath>

namespace kettlerclassicprotocol {
namespace {
constexpr quint8 STX = 0x02;
constexpr quint8 ETX = 0x03;
constexpr quint8 DLE = 0x10;
constexpr quint8 ESCAPE_XOR = 0x20;
constexpr quint16 CRC_POLYNOMIAL = 0x8408;
constexpr int MAX_DECODED_FRAME_SIZE = 4096;

bool isControlByte(quint8 value) {
    return value == STX || value == ETX || value == DLE;
}

void appendEscaped(QByteArray &destination, quint8 value) {
    if (isControlByte(value)) {
        destination.append(static_cast<char>(DLE));
        destination.append(static_cast<char>(value ^ ESCAPE_XOR));
    } else {
        destination.append(static_cast<char>(value));
    }
}

int consumeEscapedByte(const QByteArray &buffer, qsizetype &position, quint8 &value) {
    if (position >= buffer.size()) {
        return 0;
    }

    const quint8 encoded = static_cast<quint8>(buffer.at(position++));
    if (encoded != DLE) {
        value = encoded;
        return 1;
    }
    if (position >= buffer.size()) {
        --position;
        return 0;
    }

    value = static_cast<quint8>(buffer.at(position++)) ^ ESCAPE_XOR;
    return isControlByte(value) ? 1 : -1;
}

} // namespace

QByteArray coachSHandshakeForSalt(const QByteArray &salt) {
    if (salt.size() != 2) return {};
    // Recovered from Kettler S-FIT 1.6; validated against six independent
    // Coach S Kinomap handshake TX/RX payloads in three HCI captures.
    const QByteArray secret = QByteArray::fromHex("35d834d59da00bbb43c91743accd5603");
    return salt + QCryptographicHash::hash(salt + secret, QCryptographicHash::Md5);
}

bool isValidCoachSHandshake(const QByteArray &payload) {
    return payload.size() == 18 && coachSHandshakeForSalt(payload.left(2)) == payload;
}

quint16 coachSStrokeWatts(const QByteArray &telemetryPayload) {
    if (telemetryPayload.size() != 24 ||
        static_cast<quint8>(telemetryPayload.at(0)) != 0x00 ||
        static_cast<quint8>(telemetryPayload.at(1)) != 0x04) {
        return 0;
    }
    return (static_cast<quint16>(static_cast<quint8>(telemetryPayload.at(6))) << 8) |
           static_cast<quint8>(telemetryPayload.at(7));
}

quint16 coachSCandidateWatts(const QByteArray &telemetryPayload) {
    if (telemetryPayload.size() != 24 ||
        static_cast<quint8>(telemetryPayload.at(0)) != 0x00 ||
        static_cast<quint8>(telemetryPayload.at(1)) != 0x04) {
        return 0;
    }
    return (static_cast<quint16>(static_cast<quint8>(telemetryPayload.at(18))) << 8) |
           static_cast<quint8>(telemetryPayload.at(19));
}

quint8 coachSReportedResistance(const QByteArray &telemetryPayload) {
    if (telemetryPayload.size() != 24 ||
        static_cast<quint8>(telemetryPayload.at(0)) != 0x00 ||
        static_cast<quint8>(telemetryPayload.at(1)) != 0x04) {
        return 0;
    }
    const quint8 level = static_cast<quint8>(telemetryPayload.at(23));
    return level >= 1 && level <= 10 ? level : 0;
}

void CoachSStrokePowerSmoother::reset() {
    lastStrokeCounter = -1;
    lastCompletedStrokeMs = -1;
    strokePeriodMs = 0;
    completedWatts = 0;
}

int CoachSStrokePowerSmoother::inactivityTimeoutMs() const {
    // Preserve the already hardware-tested cadence-adaptive stop semantics.
    return strokePeriodMs > 0 ? qBound(3000, strokePeriodMs * 3 / 2, 6500) : 3000;
}

bool CoachSStrokePowerSmoother::active(qint64 nowMs) const {
    return lastCompletedStrokeMs >= 0 &&
           nowMs - lastCompletedStrokeMs <= inactivityTimeoutMs();
}

quint16 CoachSStrokePowerSmoother::value(qint64 nowMs) const {
    return active(nowMs) ? completedWatts : 0;
}

quint16 CoachSStrokePowerSmoother::addSample(quint16 strokeCounter,
                                            quint16 consoleStrokeWatts, qint64 nowMs) {
    if (lastStrokeCounter < 0 || strokeCounter < lastStrokeCounter) {
        // The console retains the previous completed watt value on reconnect.
        // Treat the first counter value (or a console reset) as baseline only.
        reset();
        lastStrokeCounter = strokeCounter;
        return 0;
    }

    if (strokeCounter > lastStrokeCounter) {
        const int increments = strokeCounter - lastStrokeCounter;
        if (lastCompletedStrokeMs >= 0) {
            const qint64 interval = (nowMs - lastCompletedStrokeMs) / increments;
            if (interval >= 500 && interval <= 6500)
                strokePeriodMs = static_cast<int>(interval);
        }
        // Byte offsets 6-7 carry the physical console's stroke average.
        // The candidate flywheel field (18-19) can stay positive for minutes.
        completedWatts = consoleStrokeWatts;
        lastStrokeCounter = strokeCounter;
        lastCompletedStrokeMs = nowMs;
    }
    return value(nowMs);
}

void CoachSPowerBasedPace::reset(double initialMeters) {
    sessionMeters = qMax(0.0, initialMeters);
    strokeSpeedKmh = 0.0;
}

void CoachSPowerBasedPace::completedStroke(quint16 consoleWatts, qint64 elapsedMs,
                                          bool previousStrokeActive) {
    // QZ and virtual PM5 use km/h. Concept2 rowing power is W = 2.8 * v^3,
    // with v in m/s; unlike physical flywheel distance it is independent of
    // the Coach S resistance level.
    const double speedMetersPerSecond = consoleWatts > 0
        ? std::cbrt(static_cast<double>(consoleWatts) / 2.8) : 0.0;
    strokeSpeedKmh = qMin(30.0, speedMetersPerSecond * 3.6);

    // No completed stroke => caller never invokes us. Only credit a stroke
    // interval with an active preceding stroke: no reconnect/nudge/idle
    // distance and no large jump after the adaptive inactivity timeout.
    if (previousStrokeActive && elapsedMs >= 300 && elapsedMs <= 6500)
        sessionMeters += (strokeSpeedKmh / 3.6) * (elapsedMs / 1000.0);
}

void CoachSSessionCounters::reset() {
    initialized = false;
    previousStrokes = 0;
    previousMeters = 0;
    distanceAnchorMeters = 0;
    clearWorkout();
}

void CoachSSessionCounters::clearWorkout() {
    sessionStrokes = 0;
    sessionMeters = 0;
    // Starting a fresh workout must not re-credit uncommitted coasting.
    if (initialized) distanceAnchorMeters = previousMeters;
}

bool CoachSSessionCounters::addSample(quint16 strokes, quint16 meters, bool strokeActive) {
    const bool rebase = !initialized || strokes < previousStrokes || meters < previousMeters;
    if (rebase) {
        // Keep already credited session totals across a physical console reset.
        // Never interpret an unsigned counter wrap as meters travelled.
        initialized = true;
        previousStrokes = strokes;
        previousMeters = meters;
        distanceAnchorMeters = meters;
        return true;
    }

    if (strokes > previousStrokes) {
        sessionStrokes += strokes - previousStrokes;
        // Commit only the distance up to a *completed* stroke. Following
        // flywheel motion without another stroke never changes QZ distance.
        if (strokeActive && meters >= distanceAnchorMeters)
            sessionMeters += meters - distanceAnchorMeters;
        distanceAnchorMeters = meters;
    } else if (!strokeActive) {
        // Ignore handle nudges before the first stroke, and rebase distance
        // accumulated by the freely spinning flywheel after an idle timeout.
        distanceAnchorMeters = meters;
    }
    previousStrokes = strokes;
    previousMeters = meters;
    return false;
}

quint16 crc16(const QByteArray &data) {
    quint16 crc = 0;
    for (const char byte : data) {
        crc ^= static_cast<quint8>(byte);
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 1) ? static_cast<quint16>((crc >> 1) ^ CRC_POLYNOMIAL)
                            : static_cast<quint16>(crc >> 1);
        }
    }
    return crc;
}

namespace {
QByteArray encodeFrameImpl(quint16 function, quint8 subFunction, const QByteArray &payload,
                           bool countEscapedPayloadBytes) {
    QByteArray raw;
    raw.reserve(5 + payload.size());
    raw.append(static_cast<char>((function >> 8) & 0xff));
    raw.append(static_cast<char>(function & 0xff));
    raw.append(static_cast<char>(subFunction));
    // Kinomap Coach S handshake: header length counts the encoded (escaped)
    // payload size, whereas regular Classic frames count decoded bytes.
    qsizetype advertisedLength = payload.size();
    if (countEscapedPayloadBytes) {
        for (const char byte : payload) {
            if (isControlByte(static_cast<quint8>(byte))) ++advertisedLength;
        }
    }
    raw.append(static_cast<char>((advertisedLength >> 8) & 0xff));
    raw.append(static_cast<char>(advertisedLength & 0xff));
    raw.append(payload);

    const quint16 checksum = crc16(raw);
    QByteArray encoded;
    encoded.reserve(raw.size() * 2 + 6);
    encoded.append(static_cast<char>(STX));
    for (const char byte : raw) {
        appendEscaped(encoded, static_cast<quint8>(byte));
    }
    encoded.append(static_cast<char>(ETX));
    appendEscaped(encoded, static_cast<quint8>((checksum >> 8) & 0xff));
    appendEscaped(encoded, static_cast<quint8>(checksum & 0xff));
    return encoded;
}
} // namespace

QByteArray encodeFrame(quint16 function, quint8 subFunction, const QByteArray &payload) {
    return encodeFrameImpl(function, subFunction, payload, false);
}

QByteArray encodeFrameWithEscapedPayloadLength(quint16 function, quint8 subFunction,
                                               const QByteArray &payload) {
    return encodeFrameImpl(function, subFunction, payload, true);
}

QVector<Frame> consumeFrames(QByteArray &buffer) {
    QVector<Frame> frames;

    while (true) {
        const qsizetype start = buffer.indexOf(static_cast<char>(STX));
        if (start < 0) {
            buffer.clear();
            break;
        }
        if (start > 0) {
            buffer.remove(0, start);
        }

        QByteArray raw;
        qsizetype position = 1;
        bool complete = false;
        bool invalid = false;
        while (position < buffer.size()) {
            const quint8 value = static_cast<quint8>(buffer.at(position));
            if (value == ETX) {
                ++position;
                complete = true;
                break;
            }
            if (value == STX) {
                invalid = true;
                break;
            }

            quint8 decoded = 0;
            const int decodedResult = consumeEscapedByte(buffer, position, decoded);
            if (decodedResult < 0) {
                invalid = true;
                break;
            }
            if (decodedResult == 0) {
                break;
            }
            raw.append(static_cast<char>(decoded));
            if (raw.size() > MAX_DECODED_FRAME_SIZE) {
                invalid = true;
                break;
            }
        }

        if (invalid) {
            buffer.remove(0, 1);
            continue;
        }
        if (!complete) {
            return frames;
        }

        quint8 crcHigh = 0;
        quint8 crcLow = 0;
        const int crcHighResult = consumeEscapedByte(buffer, position, crcHigh);
        if (crcHighResult == 0) {
            return frames;
        }
        if (crcHighResult < 0) {
            buffer.remove(0, 1);
            continue;
        }
        const int crcLowResult = consumeEscapedByte(buffer, position, crcLow);
        if (crcLowResult == 0) {
            return frames;
        }
        if (crcLowResult < 0) {
            buffer.remove(0, 1);
            continue;
        }

        buffer.remove(0, position);
        if (raw.size() < 5) {
            continue;
        }

        const quint16 payloadLength = (static_cast<quint16>(static_cast<quint8>(raw.at(3))) << 8) |
                                      static_cast<quint16>(static_cast<quint8>(raw.at(4)));
        const quint16 receivedCrc = (static_cast<quint16>(crcHigh) << 8) | static_cast<quint16>(crcLow);
        if (raw.size() != 5 + payloadLength || crc16(raw) != receivedCrc) {
            continue;
        }

        Frame frame;
        frame.function = (static_cast<quint16>(static_cast<quint8>(raw.at(0))) << 8) |
                         static_cast<quint16>(static_cast<quint8>(raw.at(1)));
        frame.subFunction = static_cast<quint8>(raw.at(2));
        frame.payload = raw.mid(5);
        frames.append(frame);
    }

    return frames;
}

} // namespace kettlerclassicprotocol
