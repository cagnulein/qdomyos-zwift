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

// Only completed Coach S strokes publish power; raw flywheel samples can
// remain positive long after the handle stops.
class CoachSStrokePowerSmoother {
public:
    void reset();
    quint16 addSample(quint16 strokeCounter, quint16 rawWatts, qint64 nowMs);
    quint16 value(qint64 nowMs) const;
    bool active(qint64 nowMs) const;
private:
    int inactivityTimeoutMs() const;
    struct Sample { qint64 timeMs; quint16 watts; };
    QVector<Sample> samples;
    int lastStrokeCounter = -1;
    qint64 lastCompletedStrokeMs = -1;
    int strokePeriodMs = 0;
    quint16 completedWatts = 0;
};

// The console retains raw counters across app sessions. Only accumulate
// deltas after the initial frame; rebase safely on a console counter reset.
class CoachSSessionCounters {
public:
    void reset();
    void clearWorkout();
    bool addSample(quint16 strokes, quint16 meters);
    bool hasSample() const { return initialized; }
    quint16 rawStrokes() const { return previousStrokes; }
    quint16 rawMeters() const { return previousMeters; }
    quint64 strokes() const { return sessionStrokes; }
    quint64 meters() const { return sessionMeters; }
private:
    bool initialized = false;
    quint16 previousStrokes = 0;
    quint16 previousMeters = 0;
    quint64 sessionStrokes = 0;
    quint64 sessionMeters = 0;
};

QVector<Frame> consumeFrames(QByteArray &buffer);

} // namespace kettlerclassicprotocol

#endif // KETTLERCLASSICPROTOCOL_H
