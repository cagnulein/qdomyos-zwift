/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "IC15DProtocol.h"

// Out-of-line definitions for the static const members declared in the header. Required so they
// have a linkable address when odr-used (e.g. passed by const-reference, as gtest's EXPECT_EQ
// does in the test suite) rather than only ever appearing as compile-time constants.
const quint8 IC15DProtocol::FRAME_HEADER;
const quint8 IC15DProtocol::FRAME_TERMINATOR;
const quint8 IC15DProtocol::STUFF_ESCAPE;
const int IC15DProtocol::STUFF_OFFSET;
const quint8 IC15DProtocol::CMD_RPM;
const quint8 IC15DProtocol::CMD_LEVEL;
const int IC15DProtocol::INDEX_STATUS;
const int IC15DProtocol::INDEX_COMMAND;
const int IC15DProtocol::INDEX_VALUE;
const int IC15DProtocol::MIN_LOGICAL_FRAME_SIZE;

namespace {
// Standard CRC16-CCITT (poly 0x1021) half-byte lookup table.
const quint16 kCrc16NibbleTable[16] = {0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
                                        0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF};
} // namespace

QByteArray IC15DProtocol::unstuff(const QByteArray &stuffedInterior) {
    QByteArray out;
    out.reserve(stuffedInterior.size());
    for (int i = 0; i < stuffedInterior.size(); ++i) {
        quint8 b = static_cast<quint8>(stuffedInterior.at(i));
        if (b == STUFF_ESCAPE) {
            if (i + 1 >= stuffedInterior.size())
                break; // truncated escape sequence at end of buffer; drop it
            quint8 next = static_cast<quint8>(stuffedInterior.at(++i));
            out.append(static_cast<char>(static_cast<quint8>(next + STUFF_OFFSET)));
        } else {
            out.append(static_cast<char>(b));
        }
    }
    return out;
}

quint16 IC15DProtocol::crc16(const QByteArray &data) {
    quint16 crc = 0x0000;
    for (int i = 0; i < data.size(); ++i) {
        quint8 b = static_cast<quint8>(data.at(i));
        crc = static_cast<quint16>((crc << 4) ^ kCrc16NibbleTable[((crc >> 12) ^ (b >> 4)) & 0x0F]);
        crc = static_cast<quint16>((crc << 4) ^ kCrc16NibbleTable[((crc >> 12) ^ (b & 0x0F)) & 0x0F]);
    }
    return crc;
}

int IC15DProtocol::findWireFrame(const QByteArray &buffer, QByteArray &outWireFrame) {
    if (buffer.isEmpty())
        return 0;

    if (static_cast<quint8>(buffer.at(0)) != FRAME_HEADER)
        return -1;

    for (int i = 1; i < buffer.size(); ++i) {
        quint8 b = static_cast<quint8>(buffer.at(i));
        if (b == STUFF_ESCAPE) {
            ++i;                 // skip the escaped byte
            if (i >= buffer.size())
                return 0;         // incomplete escape sequence, need more data
            continue;
        }
        if (b == FRAME_TERMINATOR) {
            outWireFrame = buffer.left(i + 1);
            return i + 1;
        }
    }

    return 0; // no terminator found yet
}

IC15DProtocol::ParsedFrame IC15DProtocol::parseWireFrame(const QByteArray &wireFrame) {
    ParsedFrame result;

    if (wireFrame.size() < 2)
        return result;
    if (static_cast<quint8>(wireFrame.front()) != FRAME_HEADER)
        return result;
    if (static_cast<quint8>(wireFrame.back()) != FRAME_TERMINATOR)
        return result;

    const QByteArray stuffedInterior = wireFrame.mid(1, wireFrame.size() - 2);
    const QByteArray unstuffedInterior = unstuff(stuffedInterior);

    QByteArray logical;
    logical.reserve(unstuffedInterior.size() + 1);
    logical.append(static_cast<char>(FRAME_HEADER));
    logical.append(unstuffedInterior);

    if (logical.size() < MIN_LOGICAL_FRAME_SIZE)
        return result;

    const QByteArray crcData = logical.left(logical.size() - 2);
    const quint16 computedCrc = crc16(crcData);
    const quint16 frameCrc = static_cast<quint16>((static_cast<quint8>(logical.at(logical.size() - 2)) << 8) |
                                                   static_cast<quint8>(logical.at(logical.size() - 1)));
    if (computedCrc != frameCrc)
        return result;

    result.status = static_cast<quint8>(logical.at(INDEX_STATUS));
    result.command = static_cast<quint8>(logical.at(INDEX_COMMAND));
    result.value = static_cast<quint8>(logical.at(INDEX_VALUE));
    result.valid = true;
    return result;
}
