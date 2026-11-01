#ifndef JKFITNESS10800PROTOCOL_H
#define JKFITNESS10800PROTOCOL_H

#include <QByteArray>
#include <QVector>

namespace jkfitness10800protocol {

struct Frame {
    QByteArray raw;
    quint8 opcode = 0;
    QByteArray payload;
};

QByteArray makeFrame(quint8 opcode, const QByteArray &payload);
QByteArray startFrame();
QByteArray stopFrame();
QByteArray pauseFrame();
QByteArray speedFrame(double speed);
QByteArray speedFrame(double speed, int units);
QByteArray inclineFrame(double incline);
QByteArray keepAliveFrame();
QByteArray heartbeatFrame(bool active = true);
QByteArray telemetryQueryFrame();
QByteArray profileFrame(int age, bool male, double heightCm, double weightKg);
QByteArray profileFrame(int age, bool male, double heightCm, double weightKg, int units);
QVector<QByteArray> initializationFrames();

QVector<Frame> consumeFrames(QByteArray &buffer);
bool decodeTelemetry(const QByteArray &raw, double &speed, double &incline);
bool decodeTelemetry(const QByteArray &raw, int deviceType, int units, double &speed, double &incline);
bool isConsoleState(const QByteArray &raw, bool &running, bool &paused);
bool isConsoleState(const QByteArray &raw, bool &running);
bool decodeDeviceType(const QByteArray &raw, int &deviceType);
bool decodeUnits(const QByteArray &raw, int &units);
bool isRetryDeviceTypeQuery(const QByteArray &raw);

} // namespace jkfitness10800protocol

#endif // JKFITNESS10800PROTOCOL_H
