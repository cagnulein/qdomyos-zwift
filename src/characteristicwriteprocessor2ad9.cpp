#include "characteristicwriteprocessor2ad9.h"
#include "ftmsbike.h"
#include <QSettings>
#include <QtMath>

CharacteristicWriteProcessor2AD9::CharacteristicWriteProcessor2AD9(double bikeResistanceGain,
                                                                   uint8_t bikeResistanceOffset, bluetoothdevice *bike,
                                                                   QObject *parent)
    : CharacteristicWriteProcessor(parent), bikeResistanceOffset(bikeResistanceOffset),
      bikeResistanceGain(bikeResistanceGain), Bike(bike) {}

int CharacteristicWriteProcessor2AD9::writeProcess(quint16 uuid, const QByteArray &data, QByteArray &reply) {
    if (data.size()) {
        QSettings settings;
        bool force_resistance = settings.value(QStringLiteral("virtualbike_forceresistance"), true).toBool();
        bool erg_mode = settings.value(QStringLiteral("zwift_erg"), false).toBool();
        char cmd = data.at(0);
        emit ftmsCharacteristicChanged(QLowEnergyCharacteristic(), data);
        if (cmd == FTMS_SET_TARGET_RESISTANCE_LEVEL) {

            // Set Target Resistance
            uint8_t uresistance = data.at(1);
            uresistance = uresistance / 10;
            if (force_resistance && !erg_mode) {
                Bike->changeResistance(uresistance);
            }
            qDebug() << QStringLiteral("new requested resistance ") + QString::number(uresistance) +
                            QStringLiteral(" enabled ") + force_resistance;
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_SET_TARGET_RESISTANCE_LEVEL);
            reply.append((quint8)FTMS_SUCCESS);
        } else if (cmd == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS) // simulation parameter

        {
            qDebug() << QStringLiteral("indoor bike simulation parameters");
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS);
            reply.append((quint8)FTMS_SUCCESS);

            int16_t iresistance = (((uint8_t)data.at(3)) + (data.at(4) << 8));
            slopeChanged(iresistance);
        } else if (cmd == FTMS_SET_TARGET_POWER) // erg mode

        {
            qDebug() << QStringLiteral("erg mode");
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_SET_TARGET_POWER);
            reply.append((quint8)FTMS_SUCCESS);

            uint16_t power = (((uint8_t)data.at(1)) + (data.at(2) << 8));
            powerChanged(power);
        } else if (cmd == FTMS_START_RESUME) {
            qDebug() << QStringLiteral("start simulation!");

            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_START_RESUME);
            reply.append((quint8)FTMS_SUCCESS);
        } else if (cmd == FTMS_REQUEST_CONTROL) {
            qDebug() << QStringLiteral("control requested");

            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((char)FTMS_REQUEST_CONTROL);
            reply.append((quint8)FTMS_SUCCESS);
        } else {
            qDebug() << QStringLiteral("not supported");

            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)cmd);
            reply.append((quint8)FTMS_NOT_SUPPORTED);
        }
        return CP_OK;
    } else
        return CP_INVALID;
}

void CharacteristicWriteProcessor2AD9::powerChanged(uint16_t power) { Bike->changePower(power); }

void CharacteristicWriteProcessor2AD9::slopeChanged(int16_t iresistance) {

    QSettings settings;
    bool force_resistance = settings.value(QStringLiteral("virtualbike_forceresistance"), true).toBool();
    bool erg_mode = settings.value(QStringLiteral("zwift_erg"), false).toBool();

    qDebug() << QStringLiteral("new requested resistance zwift erg grade ") + QString::number(iresistance) +
                    QStringLiteral(" enabled ") + force_resistance;
    double resistance = ((double)iresistance * 1.5) / 100.0;
    qDebug() << QStringLiteral("calculated erg grade ") + QString::number(resistance);

    emit changeInclination(iresistance / 100.0, qTan(qDegreesToRadians(iresistance / 100.0)) * 100.0);

    if (force_resistance && !erg_mode) {
        // same on the training program
        Bike->changeResistance((int8_t)(round(resistance * bikeResistanceGain)) + bikeResistanceOffset +
                               1); // resistance start from 1
    }
}
