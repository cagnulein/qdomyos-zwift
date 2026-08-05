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

#ifndef IC15DPROTOCOL_H
#define IC15DPROTOCOL_H

#include <QByteArray>
#include <QtGlobal>

// Pure, I/O-free decoder for the Inspire IC15D internal-UART ("CSX") protocol observed on
// /dev/ttyS2 (19200 8N1). Reverse engineered from the stock console APK for GitHub issue #4888.
//
// Wire frame layout (before byte-unstuffing):
//   [0]      0xFE                 frame header
//   [1]      status byte
//   [2]      (unidentified)
//   [3]      command byte
//   [4..]    command-specific data
//   [-3..-2] CRC16, big-endian, computed over bytes [0 .. size-3)
//   [-1]     0xFF                 frame terminator
//
// Any byte with value >= 253 (0xFD, 0xFE, 0xFF) that occurs between the header and the
// terminator is escaped as 0xFD followed by (value - 253), so the header/terminator markers
// stay unambiguous on the wire.
class IC15DProtocol {
  public:
    static const quint8 FRAME_HEADER = 0xFE;
    static const quint8 FRAME_TERMINATOR = 0xFF;
    static const quint8 STUFF_ESCAPE = 0xFD;
    static const int STUFF_OFFSET = 253;

    // Known command bytes (frame byte index 3).
    static const quint8 CMD_RPM = 0x4A;   // value at byte index 4: RPM
    static const quint8 CMD_LEVEL = 0x2E; // value at byte index 4: resistance/gear level

    static const int INDEX_STATUS = 1;
    static const int INDEX_COMMAND = 3;
    static const int INDEX_VALUE = 4;

    // Minimum logical (post-unstuffing, header included, terminator excluded) frame size:
    // header + status + byte2 + command + value + crcHi + crcLo.
    static const int MIN_LOGICAL_FRAME_SIZE = 7;

    struct ParsedFrame {
        bool valid = false;
        quint8 status = 0;
        quint8 command = 0;
        quint8 value = 0;
    };

    // Reverses byte-stuffing on bytes strictly between the header and the terminator. 0xFD is
    // the escape byte; 0xFD followed by byte b decodes to (b + STUFF_OFFSET). A truncated escape
    // sequence at the end of the input (0xFD with nothing following) is dropped.
    static QByteArray unstuff(const QByteArray &stuffedInterior);

    // Computes the CSX CRC16 (CCITT, polynomial 0x1021, initial value 0x0000) via a nibble
    // (half-byte) lookup table, mathematically equivalent to the bitwise CRC16-CCITT already
    // used elsewhere in this codebase (see keepbike.cpp).
    static quint16 crc16(const QByteArray &data);

    // Scans buffer for one complete wire frame starting at offset 0, honoring escape sequences
    // so an escaped 0xFF byte is never mistaken for the terminator.
    // Returns:
    //   > 0  number of bytes consumed from buffer (a full frame, header through terminator,
    //        written to outWireFrame)
    //   0    no complete frame yet (need more data)
    //   -1   buffer[0] is not FRAME_HEADER (caller should discard the byte and resync)
    static int findWireFrame(const QByteArray &buffer, QByteArray &outWireFrame);

    // Unstuffs and validates a single complete wire frame (header ... terminator, inclusive):
    // checks minimum length and CRC, then extracts status/command/value. Returns
    // ParsedFrame::valid == false for any malformed, truncated, or CRC-mismatched input.
    static ParsedFrame parseWireFrame(const QByteArray &wireFrame);
};

#endif // IC15DPROTOCOL_H
