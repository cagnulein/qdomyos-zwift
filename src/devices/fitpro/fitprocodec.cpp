#include "fitprocodec.h"

#include <algorithm>
#include <cmath>

namespace fitpro {
namespace {

QByteArray u16le(quint16 value) {
    QByteArray result;
    result.append(static_cast<char>(value & 0xff));
    result.append(static_cast<char>((value >> 8) & 0xff));
    return result;
}

QList<FieldWrite> sortedWrites(const QList<FieldWrite> &writes) {
    QList<FieldWrite> result = writes;
    std::sort(result.begin(), result.end(), [](const FieldWrite &left, const FieldWrite &right) {
        return left.id < right.id;
    });
    return result;
}

QList<quint8> sortedReads(const QList<quint8> &reads) {
    QList<quint8> result = reads;
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

} // namespace

QByteArray Codec::fixedU16(double value, double scale) {
    const double scaled = std::trunc(value * scale);
    const quint16 raw = static_cast<quint16>(static_cast<qint64>(scaled) & 0xffff);
    return u16le(raw);
}

QByteArray Codec::speed(double kph) {
    return fixedU16(kph);
}

QByteArray Codec::grade(double percentage) {
    return fixedU16(percentage);
}

QByteArray Codec::resistance(double value) {
    return fixedU16(value);
}

QByteArray Codec::boolValue(bool value) {
    return QByteArray(1, value ? '\x01' : '\x00');
}

quint8 Codec::checksum(const QByteArray &data) {
    quint32 sum = 0;
    for (const char byte : data)
        sum += static_cast<quint8>(byte);
    return static_cast<quint8>(sum & 0xff);
}

QByteArray Codec::command(quint8 device, quint8 commandId, const QByteArray &payload) {
    const int length = payload.size() + 4; // device, length, command, checksum
    if (length > 0xff)
        return QByteArray();

    QByteArray result;
    result.reserve(length);
    result.append(static_cast<char>(device));
    result.append(static_cast<char>(length));
    result.append(static_cast<char>(commandId));
    result.append(payload);
    result.append(static_cast<char>(checksum(result)));
    return result;
}

QByteArray Codec::writeSection(const QList<FieldWrite> &writes) {
    const QList<FieldWrite> ordered = sortedWrites(writes);
    if (ordered.isEmpty())
        return QByteArray(1, '\x00');

    const int maskLength = (ordered.last().id / 8) + 1;
    QByteArray result(maskLength + 1, '\x00');
    result[0] = static_cast<char>(maskLength);
    for (const FieldWrite &field : ordered) {
        const int maskIndex = 1 + field.id / 8;
        result[maskIndex] = static_cast<char>(static_cast<quint8>(result.at(maskIndex)) |
                                               static_cast<quint8>(1u << (field.id % 8)));
        result.append(field.value);
    }
    return result;
}

QByteArray Codec::readSection(const QList<quint8> &reads) {
    const QList<quint8> ordered = sortedReads(reads);
    if (ordered.isEmpty())
        return QByteArray(1, '\x00');

    const int maskLength = (ordered.last() / 8) + 1;
    QByteArray result(maskLength + 1, '\x00');
    result[0] = static_cast<char>(maskLength);
    for (const quint8 field : ordered) {
        const int maskIndex = 1 + field / 8;
        result[maskIndex] = static_cast<char>(static_cast<quint8>(result.at(maskIndex)) |
                                               static_cast<quint8>(1u << (field % 8)));
    }
    return result;
}

QByteArray Codec::writeReadFrame(quint8 device, const QList<FieldWrite> &writes,
                                 const QList<quint8> &reads) {
    return command(device, WriteReadData, writeSection(writes) + readSection(reads));
}

QByteArray Codec::discoveryFrame(quint8 commandId, quint8 device) {
    if (commandId != GetSupportedDevices && commandId != GetInfo && commandId != GetSupportedCommands)
        return QByteArray();
    return command(device, commandId);
}

QByteArray Codec::wrap(const QByteArray &core) {
    if (core.size() > 0xff)
        return QByteArray();
    QByteArray result;
    result.reserve(core.size() + 4);
    result.append('\x02');
    result.append('\x04');
    result.append('\x02');
    result.append(static_cast<char>(core.size()));
    result.append(core);
    return result;
}

QList<QByteArray> Codec::packetize(const QByteArray &wrapped) {
    QList<QByteArray> result;
    if (wrapped.isEmpty() || wrapped.size() > 0xff)
        return result;

    const int fragmentCount = std::max(1, (wrapped.size() + 17) / 18);
    QByteArray header;
    header.append('\xfe');
    header.append('\x02');
    header.append(static_cast<char>(wrapped.size()));
    header.append(static_cast<char>(fragmentCount + 1));
    result.append(header);

    for (int index = 0; index < fragmentCount; ++index) {
        const QByteArray chunk = wrapped.mid(index * 18, 18);
        QByteArray fragment;
        fragment.reserve(20);
        fragment.append(static_cast<char>(index == fragmentCount - 1 ? 0xff : index));
        fragment.append(static_cast<char>(chunk.size()));
        fragment.append(chunk);
        fragment.append(QByteArray(20 - fragment.size(), '\x00'));
        result.append(fragment);
    }
    return result;
}

QList<QByteArray> Codec::writeRead(quint8 device, const QList<FieldWrite> &writes,
                                   const QList<quint8> &reads) {
    return packetize(wrap(writeReadFrame(device, writes, reads)));
}

QList<QByteArray> Codec::discovery(quint8 commandId, quint8 device) {
    return packetize(wrap(discoveryFrame(commandId, device)));
}

QList<QByteArray> Codec::speedCommand(double kph, quint8 device) {
    return writeRead(device, {{FieldSpeed, speed(kph)}});
}

QList<QByteArray> Codec::gradeCommand(double percentage, quint8 device) {
    return writeRead(device, {{FieldGrade, grade(percentage)}});
}

QList<QByteArray> Codec::resistanceCommand(double value, quint8 device) {
    return writeRead(device, {{FieldResistance, resistance(value)}});
}

QList<QByteArray> Codec::rawResistanceCommand(quint16 rawValue, quint8 device) {
    return writeRead(device, {{FieldResistance, u16le(rawValue)}});
}

QList<QByteArray> Codec::requiredStartCommand(bool requested, quint8 device) {
    return writeRead(device, {{FieldRequiredStartRequested, boolValue(requested)}});
}

QList<QByteArray> Codec::workoutModeRead(quint8 device) {
    return writeRead(device, {}, {FieldWorkoutMode});
}

QByteArray Codec::reassemble(const QList<QByteArray> &packets) {
    if (packets.size() < 2 || packets.first().size() != 4 ||
        static_cast<quint8>(packets.first().at(0)) != 0xfe ||
        static_cast<quint8>(packets.first().at(1)) != 0x02)
        return QByteArray();

    const int expectedLength = static_cast<quint8>(packets.first().at(2));
    const int expectedFragments = static_cast<quint8>(packets.first().at(3)) - 1;
    if (expectedFragments <= 0 || packets.size() != expectedFragments + 1)
        return QByteArray();

    QByteArray wrapped;
    for (int i = 0; i < expectedFragments; ++i) {
        const QByteArray &packet = packets.at(i + 1);
        if (packet.size() < 2)
            return QByteArray();
        const quint8 marker = static_cast<quint8>(packet.at(0));
        const quint8 length = static_cast<quint8>(packet.at(1));
        if (marker != static_cast<quint8>(i == expectedFragments - 1 ? 0xff : i) ||
            length > packet.size() - 2)
            return QByteArray();
        wrapped.append(packet.mid(2, length));
    }
    if (wrapped.size() != expectedLength)
        return QByteArray();
    return wrapped;
}

QByteArray Codec::unwrap(const QByteArray &wrapped) {
    if (wrapped.size() < 4 || static_cast<quint8>(wrapped.at(0)) != 0x02 ||
        static_cast<quint8>(wrapped.at(1)) != 0x04 || static_cast<quint8>(wrapped.at(2)) != 0x02 ||
        static_cast<quint8>(wrapped.at(3)) != wrapped.size() - 4)
        return QByteArray();
    return wrapped.mid(4);
}

bool Codec::decode(const QByteArray &core, Frame &frame) {
    if (core.size() < 4 || static_cast<quint8>(core.at(1)) != core.size() ||
        checksum(core.left(core.size() - 1)) != static_cast<quint8>(core.at(core.size() - 1)))
        return false;

    frame = {};
    frame.device = static_cast<quint8>(core.at(0));
    frame.command = static_cast<quint8>(core.at(2));
    if (frame.command != WriteReadData)
        return true;

    const QByteArray payload = core.mid(3, core.size() - 4);
    if (payload.isEmpty())
        return false;

    const int writeMaskLength = static_cast<quint8>(payload.at(0));
    if (payload.size() < writeMaskLength + 1)
        return false;
    const int writeMaskEnd = writeMaskLength + 1;
    int offset = writeMaskEnd;
    for (int byteIndex = 0; byteIndex < writeMaskLength; ++byteIndex) {
        const quint8 mask = static_cast<quint8>(payload.at(byteIndex + 1));
        for (int bit = 0; bit < 8; ++bit) {
            if ((mask & (1u << bit)) == 0)
                continue;
            const quint8 fieldId = static_cast<quint8>(byteIndex * 8 + bit);
            const int valueLength = fieldId == FieldRequiredStartRequested ? 1 : 2;
            if (offset + valueLength > payload.size())
                return false;
            frame.fields.append({fieldId, payload.mid(offset, valueLength)});
            offset += valueLength;
        }
    }
    return true;
}

} // namespace fitpro
