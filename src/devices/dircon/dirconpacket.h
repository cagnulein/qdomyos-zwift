#ifndef DIRCONPACKET_H
#define DIRCONPACKET_H
#include <QByteArray>
#include <QList>
#include <QObject>

#define ZWIFT_PLAY_ENUM_VALUE 0x0001
#define ZWIFT_PLAY_UUID_STRING "00000001-19CA-4651-86E5-FA29DCDD09D1"
#define ZWIFT_PLAY_CHAR1_ENUM_VALUE 0x0002
#define ZWIFT_PLAY_CHAR2_ENUM_VALUE 0x0003
#define ZWIFT_PLAY_CHAR3_ENUM_VALUE 0x0004
#define ZWIFT_PLAY_CHAR1_UUID_STRING "00000002-19CA-4651-86E5-FA29DCDD09D1"
#define ZWIFT_PLAY_CHAR2_UUID_STRING "00000003-19CA-4651-86E5-FA29DCDD09D1"
#define ZWIFT_PLAY_CHAR3_UUID_STRING "00000004-19CA-4651-86E5-FA29DCDD09D1"

#define DPKT_MESSAGE_HEADER_LENGTH 6
#define DPKT_CHAR_PROP_FLAG_READ 0x01
#define DPKT_CHAR_PROP_FLAG_WRITE 0x02
#define DPKT_CHAR_PROP_FLAG_NOTIFY 0x04
#define DPKT_CHAR_PROP_FLAG_INDICATE 0x08
#define DPKT_MSGID_ERROR 0xFF
#define DPKT_MSGID_DISCOVER_SERVICES 0x01
#define DPKT_MSGID_DISCOVER_CHARACTERISTICS 0x02
#define DPKT_MSGID_READ_CHARACTERISTIC 0x03
#define DPKT_MSGID_WRITE_CHARACTERISTIC 0x04
#define DPKT_MSGID_ENABLE_CHARACTERISTIC_NOTIFICATIONS 0x05
#define DPKT_MSGID_UNSOLICITED_CHARACTERISTIC_NOTIFICATION 0x06
#define DPKT_MSGID_UNKNOWN_0x07 0x07
#define DPKT_RESPCODE_SUCCESS_REQUEST 0x00
#define DPKT_RESPCODE_UNKNOWN_MESSAGE_TYPE 0x01
#define DPKT_RESPCODE_UNEXPECTED_ERROR 0x02
#define DPKT_RESPCODE_SERVICE_NOT_FOUND 0x03
#define DPKT_RESPCODE_CHARACTERISTIC_NOT_FOUND 0x04
#define DPKT_RESPCODE_CHARACTERISTIC_OPERATION_NOT_SUPPORTED 0x05
#define DPKT_RESPCODE_CHARACTERISTIC_WRITE_FAILED 0x06
#define DPKT_RESPCODE_UNKNOWN_PROTOCOL 0x07

#define DPKT_PARSE_ERROR -20
#define DPKT_PARSE_WAIT -3

#define DPKT_POS_SH8 2
#define DPKT_POS_SH0 3

class DirconPacket {
  public:
    DirconPacket();
    quint8 MessageVersion = 1;
    quint8 Identifier = DPKT_MSGID_ERROR;
    quint8 SequenceNumber = 0;
    quint8 ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
    quint16 Length = 0;
    quint16 uuid = 0;
    QList<quint16> uuids;
    QByteArray additional_data;
    bool isRequest = false;
    DirconPacket(const DirconPacket &cp);
    DirconPacket &operator=(const DirconPacket &cp);
    QByteArray encode(int last_seq_number, bool rouvy_compatibility = false);
    int parse(const QByteArray &buf, int last_seq_number, bool rouvy_compatibility = false);
    operator QString() const;

  private:
    quint8 uuid_bytes[16] = {0x00, 0x00, 0x18, 0x26, 0x00, 0x00, 0x10, 0x00,
                             0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
    quint8 uuid_bytes_zwift_play[16] = {0x00, 0x00, 0x00, 0x04, 0x19, 0xCA, 0x46, 0x51,
                             0x86, 0xE5, 0xFA, 0x29, 0xDC, 0xDD, 0x09, 0xD1};
    bool checkIsRequest(int last_seq_number);
};

#endif // DIRCONPACKET_H
