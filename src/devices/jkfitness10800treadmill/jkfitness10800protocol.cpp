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

QByteArray inclineFrame(double incline) {
    const int value = qBound(0, qRound(incline), static_cast<int>(MAX_INCLINE));
    return makeFrame(0x11, QByteArray(1, static_cast<char>(value)));
}

QByteArray keepAliveFrame() {
    return makeFrame(0x0d, QByteArray(10, static_cast<char>(0xff)));
}

QVector<QByteArray> initializationFrames() {
    QVector<QByteArray> frames;
    frames.append(QByteArray::fromHex("55bb01ff"));
    frames.append(QByteArray::fromHex("550c01ff"));
    frames.append(QByteArray::fromHex("551f01ff"));
    frames.append(keepAliveFrame());
    frames.append(stopFrame());
    frames.append(QByteArray::fromHex("55010639013219a532"));
    frames.append(QByteArray::fromHex("55150100"));
    frames.append(QByteArray::fromHex("550f020100"));
    frames.append(QByteArray::fromHex("55110100"));
    frames.append(QByteArray::fromHex("55080101"));
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
    if (raw.size() != 13 || byteAt(raw, 0) != 0x55) {
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

    speed = static_cast<double>(byteAt(raw, 9)) + static_cast<double>(byteAt(raw, 10)) / 100.0;
    if (qFuzzyCompare(speed, 1.0)) {
        speed = 0.0;
    }
    incline = static_cast<double>(byteAt(raw, 11));
    return true;
}

bool isConsoleState(const QByteArray &raw, bool &running) {
    if (raw.size() != 4 || byteAt(raw, 0) != 0x55 || byteAt(raw, 1) != 0x09 ||
        byteAt(raw, 2) != 0x01) {
        return false;
    }

    const quint8 state = byteAt(raw, 3);
    if (state != 0x00 && state != 0x01) {
        return false;
    }
    running = state == 0x01;
    return true;
}

} // namespace jkfitness10800protocol
