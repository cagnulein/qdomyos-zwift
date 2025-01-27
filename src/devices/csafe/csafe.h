#ifndef CSAFE_H
#define CSAFE_H

#include <QVariant>

#include <QByteArray>

#include <QList>
#include <QObject>
#include <QString>
#include <stdexcept>

#include <QBuffer>
#include <QMap>

class csafe {
  private:
    QVector<quint8> int2bytes(int numbytes, int integer);
    int bytes2int(const QVector<quint8> &raw_bytes);
    QString bytes2ascii(const QVector<quint8> &raw_bytes);

    // Unique Frame Flags
    const int Extended_Frame_Start_Flag = 0xF0;
    const int Standard_Frame_Start_Flag = 0xF1;
    const int Stop_Frame_Flag = 0xF2;
    const int Byte_Stuffing_Flag = 0xF3;

    // cmds['COMMAND_NAME'] = [0xCmd_Id, [Bytes, ...]]
    QMap<QString, QList<QList<int>>> cmds;
    QMap<int, QPair<QString, QList<int>>> resp;

    QList<QList<int>> populateCmd(int First, QList<int> Second, int Third);
    QList<QList<int>> populateCmd(int First, QList<int> Second);

  public:
    csafe();
    QByteArray write(const QStringList &arguments , bool surround_msg = false); //surround_msg is for wrapping the communication in CSAFE non-standard way for some devices like PM3
    QVector<quint8> check_message(QVector<quint8> message);
    QVariantMap read(const QVector<quint8> &transmission);
};

#endif // CSAFE_H
