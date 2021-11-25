#ifndef DIRCONMANAGER_H
#define DIRCONMANAGER_H

#include "bluetoothdevice.h"
#include "characteristicnotifier2a37.h"
#include "characteristicnotifier2a63.h"
#include "characteristicnotifier2ad2.h"
#include "characteristicwriteprocessor2ad9.h"
#include "dirconpacket.h"
#include "dirconprocessor.h"
#include <QObject>

#define DM_SERV_OP(OP, P1, P2, P3)                                                                                     \
    OP(FITNESS_MACHINE, 0x1826, P1, P2, P3)                                                                            \
    OP(CYCLING_POWER, 0x1818, P1, P2, P3)                                                                              \
    OP(HEART_RATE, 0x180D, P1, P2, P3)

#define DP_PROCESS_WRITE_2AD9() writeP2AD9
#define DP_PROCESS_WRITE_NULL() 0

#define DM_CHAR_OP(OP, P1, P2, P3)                                                                                     \
    OP(FITNESS_MACHINE, 0x2ACC, DPKT_CHAR_PROP_FLAG_READ, "\x83\x14\x00\x00\x0C\xE0\x00\x00", DP_PROCESS_WRITE_NULL,   \
       P1, P2, P3)                                                                                                     \
    OP(FITNESS_MACHINE, 0x2AD6, DPKT_CHAR_PROP_FLAG_READ, "\x0A\x00\x96\x00\x0A\x00", DP_PROCESS_WRITE_NULL, P1, P2,   \
       P3)                                                                                                             \
    OP(FITNESS_MACHINE, 0x2AD9, DPKT_CHAR_PROP_FLAG_WRITE, "\x00", DP_PROCESS_WRITE_2AD9, P1, P2, P3)                  \
    OP(FITNESS_MACHINE, 0x2AD2, DPKT_CHAR_PROP_FLAG_NOTIFY, "\x00", DP_PROCESS_WRITE_NULL, P1, P2, P3)                 \
    OP(FITNESS_MACHINE, 0x2AD3, DPKT_CHAR_PROP_FLAG_READ, "\x00\x01", DP_PROCESS_WRITE_NULL, P1, P2, P3)               \
    OP(CYCLING_POWER, 0x2A65, DPKT_CHAR_PROP_FLAG_READ, "\x08\x00\x00\x00", DP_PROCESS_WRITE_NULL, P1, P2, P3)         \
    OP(CYCLING_POWER, 0x2A5D, DPKT_CHAR_PROP_FLAG_READ, "\x0d", DP_PROCESS_WRITE_NULL, P1, P2, P3)                     \
    OP(CYCLING_POWER, 0x2A63, DPKT_CHAR_PROP_FLAG_NOTIFY, "\x00", DP_PROCESS_WRITE_NULL, P1, P2, P3)                   \
    OP(HEART_RATE, 0x2A37, DPKT_CHAR_PROP_FLAG_NOTIFY, "\x00", DP_PROCESS_WRITE_NULL, P1, P2, P3)

#define DM_SERV_ENUMU_OP(DESC, UUID, P1, P2, P3) DM_SERV_U_##DESC = UUID,

enum { DM_SERV_OP(DM_SERV_ENUMU_OP, 0, 0, 0) };

#define DM_SERV_ENUMI_OP(DESC, UUID, P1, P2, P3) DM_SERV_I_##DESC,

enum { DM_SERV_OP(DM_SERV_ENUMI_OP, 0, 0, 0) DM_SERV_I_NUM };

#define DM_CHAR_INIT_OP(SDESC, UUID, TYPE, READV, WRITEP, P1, P2, P3)                                                  \
    if (P1.size() <= DM_SERV_I_##SDESC) {                                                                              \
        P2 = new DirconProcessorService(                                                                               \
            QStringLiteral(#SDESC),                                                                                    \
            server_base_name + QString(QStringLiteral("%1")).arg(DM_SERV_U_##SDESC, 4, 16, QLatin1Char('0')),          \
            server_base_port + DM_SERV_I_##SDESC, QString(QStringLiteral("%1")).arg(DM_SERV_U_##SDESC),                \
            DM_SERV_U_##SDESC, this);                                                                                  \
        P1.append(P2);                                                                                                 \
    } else                                                                                                             \
        P2 = P1.at(DM_SERV_I_##SDESC);                                                                                 \
    P2->chars.append(new DirconProcessorCharacteristic(UUID, TYPE, QByteArrayLiteral(READV), WRITEP(), P2));

class DirconManager : public QObject {
    Q_OBJECT
    QTimer bikeTimer;
    CharacteristicWriteProcessor2AD9 *writeP2AD9 = 0;
    CharacteristicNotifier2AD2 *notif2AD2 = 0;
    CharacteristicNotifier2A63 *notif2A63 = 0;
    CharacteristicNotifier2A37 *notif2A37 = 0;
    QList<DirconProcessor *> processors;
    static QString getMacAddress();

  public:
    explicit DirconManager(bluetoothdevice *t, uint8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0,
                           QObject *parent = nullptr);
  private slots:
    void bikeProvider();
  signals:
    void changeInclination(double grade, double percentage);
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // DIRCOMMANAGER_H
