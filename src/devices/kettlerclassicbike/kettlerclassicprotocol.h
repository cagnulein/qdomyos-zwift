#ifndef KETTLERCLASSICPROTOCOL_H
#define KETTLERCLASSICPROTOCOL_H

#include <QByteArray>
#include <QVector>
#include <QtGlobal>

namespace kettlerclassicprotocol {

struct Frame {
    quint16 function = 0;
    quint8 subFunction = 0;
    QByteArray payload;
};

quint16 crc16(const QByteArray &data);
QByteArray encodeFrame(quint16 function, quint8 subFunction, const QByteArray &payload = QByteArray());
// Coach S Kinomap handshake counts escaped payload bytes in its length field.
// Keep the standard encoder unchanged for other devices and regular commands.
QByteArray encodeFrameWithEscapedPayloadLength(quint16 function, quint8 subFunction, const QByteArray &payload);
// Coach S handshake: 2-byte salt || MD5(salt || 16-byte S-FIT key).
// The Coach S response uses an independently generated salt.
QByteArray coachSHandshakeForSalt(const QByteArray &salt);
bool isValidCoachSHandshake(const QByteArray &payload);
// Experimental Coach S power candidate: E0 payload bytes 18-19, U16 BE.
// Hardware logs show zero at rest; not yet compared with console watts.
quint16 coachSCandidateWatts(const QByteArray &telemetryPayload);
// Coach S resistance levels 1..10 mapped to proposed Kinomap target watts.
// Levels 1..8 are user-reported; 9..10 are unverified extrapolations.
quint16 coachSTargetWattsForLevel(int level);
QVector<Frame> consumeFrames(QByteArray &buffer);

} // namespace kettlerclassicprotocol

#endif // KETTLERCLASSICPROTOCOL_H
