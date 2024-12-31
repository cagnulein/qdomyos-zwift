#include "bike.h"
#include "characteristicwriteprocessor0003.h"
#include <QDebug>

CharacteristicWriteProcessor0003::CharacteristicWriteProcessor0003(double bikeResistanceGain,
                                                                 int8_t bikeResistanceOffset,
                                                                 bluetoothdevice *bike,
                                                                 CharacteristicNotifier0002 *notifier0002,
                                                                 CharacteristicNotifier0004 *notifier0004,
                                                                 QObject *parent)
    : CharacteristicWriteProcessor(bikeResistanceGain, bikeResistanceOffset, bike, parent), notifier0002(notifier0002), notifier0004(notifier0004) {
}

CharacteristicWriteProcessor0003::VarintResult CharacteristicWriteProcessor0003::decodeVarint(const QByteArray& bytes, int startIndex) {
    qint64 result = 0;
    int shift = 0;
    int bytesRead = 0;

    for (int i = startIndex; i < bytes.size(); i++) {
        quint8 byte = static_cast<quint8>(bytes.at(i));
        result |= static_cast<qint64>(byte & 0x7F) << shift;
        bytesRead++;

        if ((byte & 0x80) == 0) {
            break;
        }
        shift += 7;
    }

    return {result, bytesRead};
}

qint32 CharacteristicWriteProcessor0003::decodeSInt(const QByteArray& bytes) {
    if (static_cast<quint8>(bytes.at(0)) != 0x22) {
        qFatal("Invalid field header");
    }

    int length = static_cast<quint8>(bytes.at(1));

    if (static_cast<quint8>(bytes.at(2)) != 0x10) {
        qFatal("Invalid inner header");
    }

    VarintResult varint = decodeVarint(bytes, 3);

    qint32 decoded = (varint.value >> 1) ^ -(varint.value & 1);

    return decoded;
}

void CharacteristicWriteProcessor0003::handleZwiftGear(const QByteArray &array) {
    uint8_t g = 0;
    if (array.size() >= 2) {
        if ((uint8_t)array[0] == (uint8_t)0xCC && (uint8_t)array[1] == (uint8_t)0x3A) g = 1;
        else if ((uint8_t)array[0] == (uint8_t)0xFC && (uint8_t)array[1] == (uint8_t)0x43) g = 2;
        else if ((uint8_t)array[0] == (uint8_t)0xAC && (uint8_t)array[1] == (uint8_t)0x4D) g = 3;
        else if ((uint8_t)array[0] == (uint8_t)0xDC && (uint8_t)array[1] == (uint8_t)0x56) g = 4;
        else if ((uint8_t)array[0] == (uint8_t)0x8C && (uint8_t)array[1] == (uint8_t)0x60) g = 5;
        else if ((uint8_t)array[0] == (uint8_t)0xE8 && (uint8_t)array[1] == (uint8_t)0x6B) g = 6;
        else if ((uint8_t)array[0] == (uint8_t)0xC4 && (uint8_t)array[1] == (uint8_t)0x77) g = 7;
        else if (array.size() >= 3) {
            if ((uint8_t)array[0] == (uint8_t)0xA0 && (uint8_t)array[1] == (uint8_t)0x83 && (uint8_t)array[2] == (uint8_t)0x01) g = 8;
            else if ((uint8_t)array[0] == (uint8_t)0xA8 && (uint8_t)array[1] == (uint8_t)0x91 && (uint8_t)array[2] == (uint8_t)0x01) g = 9;
            else if ((uint8_t)array[0] == (uint8_t)0xB0 && (uint8_t)array[1] == (uint8_t)0x9F && (uint8_t)array[2] == (uint8_t)0x01) g = 10;
            else if ((uint8_t)array[0] == (uint8_t)0xB8 && (uint8_t)array[1] == (uint8_t)0xAD && (uint8_t)array[2] == (uint8_t)0x01) g = 11;
            else if ((uint8_t)array[0] == (uint8_t)0xC0 && (uint8_t)array[1] == (uint8_t)0xBB && (uint8_t)array[2] == (uint8_t)0x01) g = 12;
            else if ((uint8_t)array[0] == (uint8_t)0xF3 && (uint8_t)array[1] == (uint8_t)0xCB && (uint8_t)array[2] == (uint8_t)0x01) g = 13;
            else if ((uint8_t)array[0] == (uint8_t)0xA8 && (uint8_t)array[1] == (uint8_t)0xDC && (uint8_t)array[2] == (uint8_t)0x01) g = 14;
            else if ((uint8_t)array[0] == (uint8_t)0xDC && (uint8_t)array[1] == (uint8_t)0xEC && (uint8_t)array[2] == (uint8_t)0x01) g = 15;
            else if ((uint8_t)array[0] == (uint8_t)0x90 && (uint8_t)array[1] == (uint8_t)0xFD && (uint8_t)array[2] == (uint8_t)0x01) g = 16;
            else if ((uint8_t)array[0] == (uint8_t)0xD4 && (uint8_t)array[1] == (uint8_t)0x90 && (uint8_t)array[2] == (uint8_t)0x02) g = 17;
            else if ((uint8_t)array[0] == (uint8_t)0x98 && (uint8_t)array[1] == (uint8_t)0xA4 && (uint8_t)array[2] == (uint8_t)0x02) g = 18;
            else if ((uint8_t)array[0] == (uint8_t)0xDC && (uint8_t)array[1] == (uint8_t)0xB7 && (uint8_t)array[2] == (uint8_t)0x02) g = 19;
            else if ((uint8_t)array[0] == (uint8_t)0x9F && (uint8_t)array[1] == (uint8_t)0xCB && (uint8_t)array[2] == (uint8_t)0x02) g = 20;
            else if ((uint8_t)array[0] == (uint8_t)0xD8 && (uint8_t)array[1] == (uint8_t)0xE2 && (uint8_t)array[2] == (uint8_t)0x02) g = 21;
            else if ((uint8_t)array[0] == (uint8_t)0x90 && (uint8_t)array[1] == (uint8_t)0xFA && (uint8_t)array[2] == (uint8_t)0x02) g = 22;
            else if ((uint8_t)array[0] == (uint8_t)0xC8 && (uint8_t)array[1] == (uint8_t)0x91 && (uint8_t)array[2] == (uint8_t)0x03) g = 23;
            else if ((uint8_t)array[0] == (uint8_t)0xF3 && (uint8_t)array[1] == (uint8_t)0xAC && (uint8_t)array[2] == (uint8_t)0x03) g = 24;
            else { return; }
        }
        else { return; }
    }

    if (g < currentZwiftGear) {
        for (int i = 0; i < currentZwiftGear - g; ++i) {
            ((bike*)Bike)->gearDown();
        }
    } else if (g > currentZwiftGear) {
        for (int i = 0; i < g - currentZwiftGear; ++i) {
            ((bike*)Bike)->gearUp();
        }
    }
    currentZwiftGear = g;
}

int CharacteristicWriteProcessor0003::writeProcess(quint16 uuid, const QByteArray &data, QByteArray &reply) {
    static const QByteArray expectedHexArray = QByteArray::fromHex("52696465 4F6E02");
    static const QByteArray expectedHexArray2 = QByteArray::fromHex("410805");
    static const QByteArray expectedHexArray3 = QByteArray::fromHex("00088804");
    static const QByteArray expectedHexArray4 = QByteArray::fromHex("042A0A10 C0BB0120");
    static const QByteArray expectedHexArray5 = QByteArray::fromHex("0422");
    static const QByteArray expectedHexArray6 = QByteArray::fromHex("042A0410");
    static const QByteArray expectedHexArray7 = QByteArray::fromHex("042A0310");
    static const QByteArray expectedHexArray8 = QByteArray::fromHex("0418");

    QByteArray receivedData = data;

    if (receivedData.startsWith(expectedHexArray)) {
        qDebug() << "Zwift Play Processor: Initial connection request";
        reply = QByteArray::fromHex("2a08031211220f4154582030342c2053545820303400");
        notifier0002->addAnswer(reply);
        reply = QByteArray::fromHex("2a0803120d220b524944455f4f4e28322900");
        notifier0002->addAnswer(reply);
        reply = QByteArray::fromHex("526964654f6e0200");
        notifier0004->addAnswer(reply);
    }
    else if (receivedData.startsWith(expectedHexArray2)) {
        qDebug() << "Zwift Play Processor: Device info request";
        reply = QByteArray::fromHex("3c080012320a3008800412040500050"
                                "11a0b4b49434b5220434f524500320f"
                                "3430323431383030393834000000003a01314204080110140");
    }
    else if (receivedData.startsWith(expectedHexArray3)) {
        qDebug() << "Zwift Play Processor: Status request";
        reply = QByteArray::fromHex("3c0888041206 0a0440c0bb01");
    }
    else if (receivedData.startsWith(expectedHexArray5)) {
        qDebug() << "Zwift Play Processor: Slope change request";
        double slopefloat = decodeSInt(receivedData.mid(1));
        QByteArray slope(2, 0);
        slope[0] = quint8(qint16(slopefloat) & 0xFF);
        slope[1] = quint8((qint16(slopefloat) >> 8) & 0x00FF);

        emit ftmsCharacteristicChanged(QLowEnergyCharacteristic(),
                                     QByteArray::fromHex("116901") + slope + QByteArray::fromHex("3228"));

        reply = QByteArray::fromHex("3c0888041206 0a0440c0bb01");
    }
    else if (receivedData.startsWith(expectedHexArray6) ||
             receivedData.startsWith(expectedHexArray7)) {
        qDebug() << "Zwift Play Processor: Gear change request";
        handleZwiftGear(receivedData.mid(4));
        reply = QByteArray::fromHex("03080010001827e7 2000 28 00 3093ed01");
    }
    else if (receivedData.startsWith(expectedHexArray8)) {
        qDebug() << "Zwift Play Processor: Power request";
        VarintResult Power = decodeVarint(receivedData, 2);
        QByteArray power(2, 0);
        power[0] = quint8(qint16(Power.value) & 0xFF);
        power[1] = quint8((qint16(Power.value) >> 8) & 0x00FF);

        emit ftmsCharacteristicChanged(QLowEnergyCharacteristic(),
                                     QByteArray::fromHex("05") + power);

        reply = QByteArray::fromHex("030882011022181020002898523086ed01");
        reply[2] = ((bike*)Bike)->wattsMetric().value();
    }
    else {
        qDebug() << "Zwift Play Processor: Unhandled request:" << receivedData.toHex();
        return -1;
    }

    return 0;
}
