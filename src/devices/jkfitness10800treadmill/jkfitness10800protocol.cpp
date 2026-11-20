#include "jkfitness10800protocol.h"

#include <QtMath>

namespace jkfitness10800protocol {

namespace {
constexpr int MAX_PAYLOAD_LENGTH = 64;
constexpr double MAX_SPEED = 22.0;
constexpr double MAX_INCLINE = 15.0;

quint8 byteAt(const QByteArray &data, int index) {
    return static_cast<quint8>(data.at(index));
}

} // namespace

QByteArray makeFrame(quint8 opcode, const QByteArray &payload) {
    QByteArray frame;
    frame.append(static_cast<char>(0x55));
    frame.append(static_cast<char>(opcode));
    frame.append(static_cast<char>(payload.size()));
    frame.append(payload);
    return frame;
}

QByteArray startFrame() {
    return makeFrame(0x0a, QByteArray(1, static_cast<char>(0x01)));
}

QByteArray stopFrame() {
    return makeFrame(0x0a, QByteArray(1, static_cast<char>(0x02)));
}

QByteArray pauseFrame() {
    return makeFrame(0x0a, QByteArray(1, static_cast<char>(0x00)));
}

QByteArray speedFrame(double speed) {
    const double clamped = qBound(0.0, speed, MAX_SPEED);
    int whole = qFloor(clamped);
    int fraction = qRound((clamped - whole) * 100.0);
    if (fraction >= 100) {
        ++whole;
        fraction = 0;
    }

    QByteArray payload;
    payload.append(static_cast<char>(whole));
    payload.append(static_cast<char>(fraction));
    return makeFrame(0x0f, payload);
}

QByteArray speedFrame(double speed, int units) {
    return speedFrame(units == 0 ? speed : speed * 0.62137);
}

QByteArray inclineFrame(double incline) {
    const int value = qBound(0, qRound(incline), static_cast<int>(MAX_INCLINE));
    return makeFrame(0x11, QByteArray(1, static_cast<char>(value)));
}

QByteArray keepAliveFrame() {
    return makeFrame(0x0d, QByteArray(10, static_cast<char>(0xff)));
}

QByteArray heartbeatFrame(bool active) {
    QByteArray payload(1, static_cast<char>(active ? 0x01 : 0x00));
    return makeFrame(0x17, payload);
}

QByteArray telemetryQueryFrame() {
    return QByteArray::fromHex("550701ff");
}

QByteArray profileFrame(int age, bool male, double heightCm, double weightKg) {
    return profileFrame(age, male, heightCm, weightKg, 0);
}

QByteArray profileFrame(int age, bool male, double heightCm, double weightKg, int units) {
    const auto appendDecimal = [](QByteArray &payload, double value) {
        int whole = qFloor(qMax(0.0, value));
        int fraction = qRound((qMax(0.0, value) - whole) * 100.0);
        if (fraction >= 100) {
            ++whole;
            fraction = 0;
        }
        payload.append(static_cast<char>(whole));
        payload.append(static_cast<char>(fraction));
    };
    QByteArray payload;
    payload.append(static_cast<char>(qBound(0, age, 255)));
    payload.append(static_cast<char>(male ? 0x00 : 0x01));
    // The native profile frame places weight before height after conversion to
    // the app's selected units. QZ stores both values in metric units.
    appendDecimal(payload, units == 0 ? weightKg : weightKg * 2.20462);
    appendDecimal(payload, units == 0 ? heightCm : heightCm * 0.3937);
    return makeFrame(0x01, payload);
}

QVector<QByteArray> initializationFrames() {
    QVector<QByteArray> frames;
    const quint8 opcodes[] = {0x0c, 0xbb, 0x24, 0x25, 0x26, 0x27, 0x02, 0x03, 0x04,
                              0x06, 0x1f, 0xa0, 0xb0, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
                              0xb7, 0xb8, 0xb9, 0xba, 0x0b, 0x18, 0x19, 0x1a, 0x1b};
    for (quint8 opcode : opcodes) {
        frames.append(makeFrame(opcode, QByteArray::fromHex("ff")));
    }
    return frames;
}

QVector<Frame> consumeFrames(QByteArray &buffer) {
    QVector<Frame> frames;

    while (!buffer.isEmpty()) {
        const int prefix = buffer.indexOf(static_cast<char>(0x55));
        if (prefix < 0) {
            buffer.clear();
            break;
        }
        if (prefix > 0) {
            buffer.remove(0, prefix);
        }
        if (buffer.size() < 3) {
            break;
        }

        const int payloadLength = byteAt(buffer, 2);
        if (payloadLength > MAX_PAYLOAD_LENGTH) {
            buffer.remove(0, 1);
            continue;
        }

        const int frameLength = 3 + payloadLength;
        if (buffer.size() < frameLength) {
            break;
        }

        Frame frame;
        frame.raw = buffer.left(frameLength);
        frame.opcode = byteAt(frame.raw, 1);
        frame.payload = frame.raw.mid(3, payloadLength);
        frames.append(frame);
        buffer.remove(0, frameLength);
    }

    return frames;
}

bool decodeTelemetry(const QByteArray &raw, double &speed, double &incline) {
    return decodeTelemetry(raw, 2, 0, speed, incline);
}

bool decodeTelemetry(const QByteArray &raw, int deviceType, int units, double &speed, double &incline) {
    if (deviceType < 0 || deviceType > 2 || raw.size() < 3 || byteAt(raw, 0) != 0x55) return false;
    const quint8 opcode = byteAt(raw, 1);
    int expectedSize = 0;
    int speedIndex = 9;
    int inclineIndex = 11;
    if (opcode == 0x0d) {
        expectedSize = deviceType == 2 ? 13 : 16;
    } else if (opcode == 0x1d) {
        expectedSize = deviceType == 2 ? 13 : 10;
        if (deviceType != 2) {
            speedIndex = 3;
            inclineIndex = 5;
        }
    } else {
        return false;
    }
    if (raw.size() != expectedSize || byteAt(raw, 2) != expectedSize - 3) {
        return false;
    }

    // The driver sends the same 13-byte all-FF frame as a poll/keep-alive.
    // It is not telemetry and must not become 255 km/h / 255% incline.
    bool allPayloadBytesAreFF = true;
    for (int i = 3; i < raw.size(); ++i) {
        if (byteAt(raw, i) != 0xff) {
            allPayloadBytesAreFF = false;
            break;
        }
    }
    if (allPayloadBytesAreFF) {
        return false;
    }

    speed = static_cast<double>(byteAt(raw, speedIndex)) + static_cast<double>(byteAt(raw, speedIndex + 1)) / 100.0;
    incline = static_cast<double>(byteAt(raw, inclineIndex));
    if (units != 0) {
        speed /= 0.62137;
    }
    return true;
}

bool isConsoleState(const QByteArray &raw, bool &running, bool &paused) {
    paused = false;
    if (raw.size() != 4 || byteAt(raw, 0) != 0x55 || byteAt(raw, 1) != 0x09 ||
        byteAt(raw, 2) != 0x01) {
        return false;
    }

    const quint8 state = byteAt(raw, 3);
    if (state == 0x02) {
        running = false;
        return true;
    }
    if (state != 0x00 && state != 0x01) {
        return false;
    }
    running = state == 0x01;
    paused = state == 0x00;
    return true;
}

bool isConsoleState(const QByteArray &raw, bool &running) {
    bool paused = false;
    return isConsoleState(raw, running, paused);
}

bool decodeDeviceType(const QByteArray &raw, int &deviceType) {
    if (raw.size() != 4 || byteAt(raw, 0) != 0x55 || byteAt(raw, 1) != 0xbb ||
        byteAt(raw, 2) != 0x01 || byteAt(raw, 3) > 2) return false;
    deviceType = byteAt(raw, 3);
    return true;
}

bool decodeUnits(const QByteArray &raw, int &units) {
    if (raw.size() != 4 || byteAt(raw, 0) != 0x55 || byteAt(raw, 1) != 0x0c || byteAt(raw, 2) != 0x01)
        return false;
    units = byteAt(raw, 3);
    return true;
}

bool isRetryDeviceTypeQuery(const QByteArray &raw) {
    return raw == QByteArray::fromHex("55bb00");
}

} // namespace jkfitness10800protocol
