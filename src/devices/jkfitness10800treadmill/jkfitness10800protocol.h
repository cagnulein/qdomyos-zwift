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
QByteArray inclineFrame(double incline);
QByteArray keepAliveFrame();
QVector<QByteArray> initializationFrames();

QVector<Frame> consumeFrames(QByteArray &buffer);
bool decodeTelemetry(const QByteArray &raw, double &speed, double &incline);
bool isConsoleState(const QByteArray &raw, bool &running);

} // namespace jkfitness10800protocol

#endif // JKFITNESS10800PROTOCOL_H
