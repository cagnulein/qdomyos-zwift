#include "dirconpacket.h"

DirconPacket::DirconPacket() {}

DirconPacket::operator QString() const {
    QString us = QString();
    foreach (quint16 u, uuids) { us += QString(QStringLiteral("%1,")).arg(u, 4, 16, QLatin1Char('0')); }
    return QString(QStringLiteral("vers=%1 Id=%2 sn=%3 resp=%4 len=%5 req?=%8 uuid=%6 dat=%7 uuids=[%9]"))
        .arg(MessageVersion)
        .arg(Identifier)
        .arg(SequenceNumber)
        .arg(ResponseCode)
        .arg(Length)
        .arg(uuid, 4, 16, QLatin1Char('0'))
        .arg(additional_data.toHex().constData())
        .arg(isRequest)
        .arg(us);
}

int DirconPacket::parse(const QByteArray &buf, int last_seq_number) {
    if (buf.size() >= DPKT_MESSAGE_HEADER_LENGTH) {
        this->MessageVersion = ((quint8)buf.at(0));
        this->Identifier = ((quint8)buf.at(1));
        this->SequenceNumber = ((quint8)buf.at(2));
        this->ResponseCode = ((quint8)buf.at(3));
        this->Length = (((quint8)buf.at(4)) << 8) | ((quint8)buf.at(5));
        this->isRequest = false;
        int difflen = buf.size() - DPKT_MESSAGE_HEADER_LENGTH;
        int rembuf = DPKT_MESSAGE_HEADER_LENGTH + this->Length;
        if (difflen < this->Length)
            return DPKT_PARSE_WAIT;
        else if (this->ResponseCode != DPKT_RESPCODE_SUCCESS_REQUEST)
            return rembuf;
        else if (this->Identifier == DPKT_MSGID_DISCOVER_SERVICES) {
            if (!this->Length) {
                this->isRequest = this->checkIsRequest(last_seq_number);
                return DPKT_MESSAGE_HEADER_LENGTH;
            } else if (this->Length % 16 == 0) {
                int idx = 0;
                this->uuids.clear();
                while (this->Length >= idx + 16) {
                    quint16 uuid = (((quint16)buf.at(idx + DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH8)) << 8);
                    uuid |= ((quint16)buf.at(idx + DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH0)) & 0x00FF;
                    this->uuids.append(uuid);
                    idx += 16;
                }
                return rembuf;
            } else
                return DPKT_PARSE_ERROR - rembuf;
        } else if (this->Identifier == DPKT_MSGID_DISCOVER_CHARACTERISTICS) {
            if (this->Length >= 16) {
                quint16 uuid = ((quint16)buf.at(DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH8)) << 8;
                uuid |= ((quint16)buf.at(DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH0)) & 0x00FF;
                this->uuid = uuid;
                if (this->Length == 16) {
                    this->isRequest = this->checkIsRequest(last_seq_number);
                    return rembuf;
                } else if ((this->Length - 16) % 17 == 0) {
                    this->uuids.clear();
                    this->additional_data.clear();
                    int idx = 16;
                    while (this->Length >= idx + 17) {
                        quint16 uuid = (((quint16)buf.at(idx + DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH8)) << 8);
                        uuid |= ((quint16)buf.at(idx + DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH0)) & 0x00FF;
                        this->uuids.append(uuid);
                        this->additional_data.append(((quint8)buf.at(idx + DPKT_MESSAGE_HEADER_LENGTH + 16)));
                        idx += 17;
                    }
                    return rembuf;
                } else
                    return DPKT_PARSE_ERROR - rembuf;
            } else
                return DPKT_PARSE_ERROR - rembuf;
        } else if (this->Identifier == DPKT_MSGID_READ_CHARACTERISTIC) {
            if (this->Length >= 16) {
                quint16 uuid = ((quint16)buf.at(DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH8)) << 8;
                uuid |= ((quint16)buf.at(DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH0)) & 0x00FF;
                this->uuid = uuid;
                if (this->Length == 16)
                    this->isRequest = this->checkIsRequest(last_seq_number);
                else
                    this->additional_data =
                        buf.mid(DPKT_MESSAGE_HEADER_LENGTH + 16, rembuf - (DPKT_MESSAGE_HEADER_LENGTH + 16));
                return rembuf;
            } else
                return DPKT_PARSE_ERROR - rembuf;
        } else if (this->Identifier == DPKT_MSGID_WRITE_CHARACTERISTIC) {
            if (this->Length > 16) {
                quint16 uuid = ((quint16)buf.at(DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH8)) << 8;
                uuid |= ((quint16)buf.at(DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH0)) & 0x00FF;
                this->uuid = uuid;
                this->additional_data =
                    buf.mid(DPKT_MESSAGE_HEADER_LENGTH + 16, rembuf - (DPKT_MESSAGE_HEADER_LENGTH + 16));
                this->isRequest = this->checkIsRequest(last_seq_number);
                return rembuf;
            } else
                return DPKT_PARSE_ERROR - rembuf;
        } else if (this->Identifier == DPKT_MSGID_ENABLE_CHARACTERISTIC_NOTIFICATIONS) {
            if (this->Length >= 16) {
                quint16 uuid = ((quint16)buf.at(DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH8)) << 8;
                uuid |= ((quint16)buf.at(DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH0)) & 0x00FF;
                this->uuid = uuid;
                if (this->Length >= 17) {
                    this->isRequest = true;
                    this->additional_data = buf.mid(DPKT_MESSAGE_HEADER_LENGTH + 16, 1);
                }
                return rembuf;
            } else
                return DPKT_PARSE_ERROR - rembuf;
        } else if (this->Identifier == DPKT_MSGID_UNSOLICITED_CHARACTERISTIC_NOTIFICATION) {
            if (this->Length > 16) {
                quint16 uuid = ((quint16)buf.at(DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH8)) << 8;
                uuid |= ((quint16)buf.at(DPKT_MESSAGE_HEADER_LENGTH + DPKT_POS_SH0)) & 0x00FF;
                this->uuid = uuid;
                this->additional_data =
                    buf.mid(DPKT_MESSAGE_HEADER_LENGTH + 16, rembuf - (DPKT_MESSAGE_HEADER_LENGTH + 16));
                return rembuf;
            } else
                return DPKT_PARSE_ERROR - rembuf;
        } else if (this->Identifier == DPKT_MSGID_UNKNOWN_0x07) {
            if (this->Length == 0) {
                this->isRequest = this->checkIsRequest(last_seq_number);
                return DPKT_MESSAGE_HEADER_LENGTH;
            } else
                return DPKT_PARSE_ERROR - rembuf;
        } else
            return DPKT_PARSE_ERROR - rembuf;
    } else
        return DPKT_PARSE_WAIT;
}

bool DirconPacket::checkIsRequest(int last_seq_number) {
    return this->ResponseCode == DPKT_RESPCODE_SUCCESS_REQUEST &&
           (last_seq_number <= 0 || last_seq_number != this->SequenceNumber);
}

DirconPacket::DirconPacket(const DirconPacket &cp) { this->operator=(cp); }

DirconPacket &DirconPacket::operator=(const DirconPacket &cp) {
    MessageVersion = cp.MessageVersion;
    Identifier = cp.Identifier;
    SequenceNumber = cp.SequenceNumber;
    ResponseCode = cp.ResponseCode;
    Length = cp.Length;
    uuid = cp.uuid;
    uuids = QList<quint16>(cp.uuids);
    additional_data = cp.additional_data;
    isRequest = cp.isRequest;
    return *this;
}

QByteArray DirconPacket::encode(int last_seq_number) {
    quint16 u;
    int i = 0;
    if (this->Identifier == DPKT_MSGID_ERROR)
        return QByteArray();
    else if (this->isRequest)
        this->SequenceNumber = last_seq_number & 0xFF;
    else if (this->Identifier == DPKT_MSGID_UNSOLICITED_CHARACTERISTIC_NOTIFICATION)
        this->SequenceNumber = 0;
    else
        this->SequenceNumber = last_seq_number;
    this->MessageVersion = 1;
    QByteArray byteout;
    byteout.append((char)this->MessageVersion);
    byteout.append((char)this->Identifier);
    byteout.append((char)this->SequenceNumber);
    byteout.append((char)this->ResponseCode);
    if (!this->isRequest && this->ResponseCode != DPKT_RESPCODE_SUCCESS_REQUEST) {
        this->Length = 0;
        byteout.append(2, 0);
    } else if (this->Identifier == DPKT_MSGID_DISCOVER_SERVICES) {
        if (this->isRequest) {
            this->Length = 0;
            byteout.append(2, 0);
        } else {
            this->Length = this->uuids.size() * 16;
            byteout.append((char)(this->Length >> 8)).append((char)(this->Length));
            foreach (u, this->uuids) {
                if(u >= 1 && u <= 4) {
                    this->uuid_bytes_zwift_play[DPKT_POS_SH8] = (quint8)(u >> 8);
                    this->uuid_bytes_zwift_play[DPKT_POS_SH0] = (quint8)(u);
                    byteout.append((char *)this->uuid_bytes_zwift_play, 16);
                } else {
                    this->uuid_bytes[DPKT_POS_SH8] = (quint8)(u >> 8);
                    this->uuid_bytes[DPKT_POS_SH0] = (quint8)(u);
                    byteout.append((char *)this->uuid_bytes, 16);
                }
            }
        }
    } else if (this->Identifier == DPKT_MSGID_UNKNOWN_0x07) {
        // Unknown message 0x07 - always respond with empty payload
        this->Length = 0;
        byteout.append(2, 0);
    } else if (this->Identifier == DPKT_MSGID_DISCOVER_CHARACTERISTICS && !this->isRequest) {
        this->Length = 16 + this->uuids.size() * 17;
        byteout.append((char)(this->Length >> 8)).append((char)(this->Length));
        u = this->uuid;
        if(u >= 1 && u <= 4) {
            this->uuid_bytes_zwift_play[DPKT_POS_SH8] = (quint8)(u >> 8);
            this->uuid_bytes_zwift_play[DPKT_POS_SH0] = (quint8)(u);
            byteout.append((char *)this->uuid_bytes_zwift_play, 16);
        } else {
            this->uuid_bytes[DPKT_POS_SH8] = (quint8)(u >> 8);
            this->uuid_bytes[DPKT_POS_SH0] = (quint8)(u);
            byteout.append((char *)this->uuid_bytes, 16);
        }
        foreach (u, this->uuids) {
            if(u >= 1 && u <= 4) {
                this->uuid_bytes_zwift_play[DPKT_POS_SH8] = (quint8)(u >> 8);
                this->uuid_bytes_zwift_play[DPKT_POS_SH0] = (quint8)(u);
                byteout.append((char *)this->uuid_bytes_zwift_play, 16);
            } else {
                this->uuid_bytes[DPKT_POS_SH8] = (quint8)(u >> 8);
                this->uuid_bytes[DPKT_POS_SH0] = (quint8)(u);
                byteout.append((char *)this->uuid_bytes, 16);
            }
            byteout.append(this->additional_data.at(i++));
        }
    } else if (((this->Identifier == DPKT_MSGID_READ_CHARACTERISTIC ||
                 this->Identifier == DPKT_MSGID_DISCOVER_CHARACTERISTICS) &&
                this->isRequest) ||
               (this->Identifier == DPKT_MSGID_ENABLE_CHARACTERISTIC_NOTIFICATIONS && !this->isRequest)) {
        this->Length = 16;
        byteout.append((char)(this->Length >> 8)).append((char)(this->Length));
        u = this->uuid;
        if(u >= 1 && u <= 4) {
            this->uuid_bytes_zwift_play[DPKT_POS_SH8] = (quint8)(u >> 8);
            this->uuid_bytes_zwift_play[DPKT_POS_SH0] = (quint8)(u);
            byteout.append((char *)this->uuid_bytes_zwift_play, 16);
        } else {
            this->uuid_bytes[DPKT_POS_SH8] = (quint8)(u >> 8);
            this->uuid_bytes[DPKT_POS_SH0] = (quint8)(u);
            byteout.append((char *)this->uuid_bytes, 16);
        }
    } else if (this->Identifier == DPKT_MSGID_WRITE_CHARACTERISTIC ||
               this->Identifier == DPKT_MSGID_UNSOLICITED_CHARACTERISTIC_NOTIFICATION ||
               (this->Identifier == DPKT_MSGID_READ_CHARACTERISTIC && !this->isRequest) ||
               (this->Identifier == DPKT_MSGID_ENABLE_CHARACTERISTIC_NOTIFICATIONS && this->isRequest)) {
        this->Length = 16 + this->additional_data.size();
        byteout.append((char)(this->Length >> 8)).append((char)(this->Length));
        u = this->uuid;
        if(u >= 1 && u <= 4) {
            this->uuid_bytes_zwift_play[DPKT_POS_SH8] = (quint8)(u >> 8);
            this->uuid_bytes_zwift_play[DPKT_POS_SH0] = (quint8)(u);
            byteout.append((char *)this->uuid_bytes_zwift_play, 16);
        } else {
            this->uuid_bytes[DPKT_POS_SH8] = (quint8)(u >> 8);
            this->uuid_bytes[DPKT_POS_SH0] = (quint8)(u);
            byteout.append((char *)this->uuid_bytes, 16);
        }
        byteout.append(this->additional_data);
    }
    return byteout;
}
