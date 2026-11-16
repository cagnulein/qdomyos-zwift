#include "kettlerclassicprotocol.h"

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

QByteArray encodeFrame(quint16 function, quint8 subFunction, const QByteArray &payload) {
    QByteArray raw;
    raw.reserve(5 + payload.size());
    raw.append(static_cast<char>((function >> 8) & 0xff));
    raw.append(static_cast<char>(function & 0xff));
    raw.append(static_cast<char>(subFunction));
    raw.append(static_cast<char>((payload.size() >> 8) & 0xff));
    raw.append(static_cast<char>(payload.size() & 0xff));
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
