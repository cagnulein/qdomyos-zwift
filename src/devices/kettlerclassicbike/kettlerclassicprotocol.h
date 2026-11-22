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
// Console-confirmed completed-stroke watts: E0 payload bytes 6-7, U16 BE.
// Read only on a physical stroke-counter transition: this field retains the
// previous workout's last watts while the Coach S console is idle.
quint16 coachSStrokeWatts(const QByteArray &telemetryPayload);
// Raw flywheel effort candidate at bytes 18-19; diagnostics only, not watts.
quint16 coachSCandidateWatts(const QByteArray &telemetryPayload);
// Coach S 0x00E0 telemetry: byte 23 reports the active resistance level.
// Returns 0 when the frame is invalid or the reported level is outside 1..10.
quint8 coachSReportedResistance(const QByteArray &telemetryPayload);

// Publish the console's exact completed-stroke watts on counter increments.
// Ignore any retained console watts before the first fresh stroke.
class CoachSStrokePowerSmoother {
public:
    void reset();
    quint16 addSample(quint16 strokeCounter, quint16 consoleStrokeWatts, qint64 nowMs);
    quint16 value(qint64 nowMs) const;
    bool active(qint64 nowMs) const;
private:
    int inactivityTimeoutMs() const;
    int lastStrokeCounter = -1;
    qint64 lastCompletedStrokeMs = -1;
    int strokePeriodMs = 0;
    quint16 completedWatts = 0;
};

// Concept2-equivalent pace/distance for Coach S when speed_power_based is enabled.
// Distance is committed at completed-stroke boundaries, never on flywheel coast.
// The optional initial distance allows a seamless settings-mode transition.
class CoachSPowerBasedPace {
public:
    void reset(double initialMeters = 0.0);
    void completedStroke(quint16 consoleWatts, qint64 elapsedMs, bool previousStrokeActive);
    double speedKmh() const { return strokeSpeedKmh; }
    double meters() const { return sessionMeters; }
private:
    double sessionMeters = 0.0;
    double strokeSpeedKmh = 0.0;
};

// The console retains raw counters across app sessions. Credit distance only
// at completed stroke boundaries, dropping handle nudges and idle coasting.
class CoachSSessionCounters {
public:
    void reset();
    void clearWorkout();
    bool addSample(quint16 strokes, quint16 meters, bool strokeActive);
    bool hasSample() const { return initialized; }
    quint16 rawStrokes() const { return previousStrokes; }
    quint16 rawMeters() const { return previousMeters; }
    quint64 strokes() const { return sessionStrokes; }
    quint64 meters() const { return sessionMeters; }
private:
    bool initialized = false;
    quint16 previousStrokes = 0;
    quint16 previousMeters = 0;
    quint16 distanceAnchorMeters = 0;
    quint64 sessionStrokes = 0;
    quint64 sessionMeters = 0;
};

QVector<Frame> consumeFrames(QByteArray &buffer);

} // namespace kettlerclassicprotocol

#endif // KETTLERCLASSICPROTOCOL_H
