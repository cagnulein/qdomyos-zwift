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
QVector<Frame> consumeFrames(QByteArray &buffer);

} // namespace kettlerclassicprotocol

#endif // KETTLERCLASSICPROTOCOL_H
