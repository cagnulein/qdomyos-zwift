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
// Coach S 0x00E0 telemetry: byte 23 reports the active resistance level.
// Returns 0 when the frame is invalid or the reported level is outside 1..10.
quint8 coachSReportedResistance(const QByteArray &telemetryPayload);

// Coach S console-style power: mean of nonzero drive samples, published
// after a stroke ends and held through recovery. Times are monotonic ms.
class CoachSStrokePowerSmoother {
public:
    void reset();
    quint16 addSample(quint16 strokeCounter, quint16 rawWatts, qint64 nowMs);
    quint16 value(qint64 nowMs) const;
private:
    void completeDrive();
    int inactivityTimeoutMs() const;

    int lastStrokeCounter = -1;
    qint64 lastStrokeMs = -1;
    qint64 lastPositiveMs = -1;
    int strokePeriodMs = 0;
    quint64 driveSum = 0;
    quint32 driveSamples = 0;
    quint16 completedWatts = 0;
};

QVector<Frame> consumeFrames(QByteArray &buffer);

} // namespace kettlerclassicprotocol

#endif // KETTLERCLASSICPROTOCOL_H
