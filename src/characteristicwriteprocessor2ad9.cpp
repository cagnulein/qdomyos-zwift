#include "characteristicwriteprocessor2ad9.h"
#include "elliptical.h"
#include "ftmsbike.h"
#include "treadmill.h"
#include <QSettings>
#include <QtMath>

CharacteristicWriteProcessor2AD9::CharacteristicWriteProcessor2AD9(double bikeResistanceGain,
                                                                   uint8_t bikeResistanceOffset, bluetoothdevice *bike,
                                                                   CharacteristicNotifier2AD9 *notifier,
                                                                   QObject *parent)
    : CharacteristicWriteProcessor(parent), bikeResistanceOffset(bikeResistanceOffset),
    bikeResistanceGain(bikeResistanceGain), Bike(bike), notifier(notifier) {}

int CharacteristicWriteProcessor2AD9::writeProcess(quint16 uuid, const QByteArray &data, QByteArray &reply) {
    if (data.size()) {
        bluetoothdevice::BLUETOOTH_TYPE dt = Bike->deviceType();
        if (dt == bluetoothdevice::BIKE) {
            QSettings settings;
            bool force_resistance = settings.value(QZSettings::virtualbike_forceresistance, QZSettings::default_virtualbike_forceresistance).toBool();
            bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool();
            char cmd = data.at(0);
            emit ftmsCharacteristicChanged(QLowEnergyCharacteristic(), data);
            if (cmd == FTMS_SET_TARGET_RESISTANCE_LEVEL) {

                // Set Target Resistance
                resistance_t uresistance = data.at(1);
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
                changeSlope(iresistance);
            } else if (cmd == FTMS_SET_TARGET_POWER) // erg mode

            {
                qDebug() << QStringLiteral("erg mode");
                reply.append((quint8)FTMS_RESPONSE_CODE);
                reply.append((quint8)FTMS_SET_TARGET_POWER);
                reply.append((quint8)FTMS_SUCCESS);

                uint16_t power = (((uint8_t)data.at(1)) + (data.at(2) << 8));
                changePower(power);
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
        } else if (dt == bluetoothdevice::TREADMILL || dt == bluetoothdevice::ELLIPTICAL) {
            char a, b;
            if ((char)data.at(0) == 0x02) {
                // Set Target Speed
                a = data.at(1);
                b = data.at(2);

                uint16_t uspeed = a + (((uint16_t)b) << 8);
                double requestSpeed = (double)uspeed / 100.0;
                if (dt == bluetoothdevice::TREADMILL) {
                    ((treadmill *)Bike)->changeSpeed(requestSpeed);
                }
                qDebug() << QStringLiteral("new requested speed ") + QString::number(requestSpeed);
            } else if ((char)data.at(0) == 0x03) // Set Target Inclination
            {
                a = data.at(1);
                b = data.at(2);

                int16_t sincline = a + (((int16_t)b) << 8);
                double requestIncline = (double)sincline / 10.0;
                if (requestIncline < 0)
                    requestIncline = 0;

                if (dt == bluetoothdevice::TREADMILL)
                    ((treadmill *)Bike)->changeInclination(requestIncline, requestIncline);
                // Resistance as incline on Sole E95s Elliptical #419
                else if (dt == bluetoothdevice::ELLIPTICAL)
                    ((elliptical *)Bike)->changeInclination(requestIncline, requestIncline);
                qDebug() << "new requested incline " + QString::number(requestIncline);
            } else if ((char)data.at(0) == 0x07) // Start request
            {
                // Bike->start();
                qDebug() << QStringLiteral("request to start");
            } else if ((char)data.at(0) == 0x08) // Stop request
            {
                // Bike->stop();
                qDebug() << QStringLiteral("request to stop");
            } else if ((char)data.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS) // simulation parameter
            {
                qDebug() << QStringLiteral("indoor bike simulation parameters");
                int16_t iresistance = (((uint8_t)data.at(3)) + (data.at(4) << 8));
                changeSlope(iresistance);
            }
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)data.at(0));
            reply.append((quint8)FTMS_SUCCESS);
        }
        if(notifier) {
            notifier->answer = reply;
        }
        return CP_OK;
    } else
        return CP_INVALID;
}

void CharacteristicWriteProcessor2AD9::changePower(uint16_t power) { Bike->changePower(power); }

void CharacteristicWriteProcessor2AD9::changeSlope(int16_t iresistance) {
    bluetoothdevice::BLUETOOTH_TYPE dt = Bike->deviceType();
    if (dt == bluetoothdevice::BIKE) {
      QSettings settings;
      bool force_resistance = settings.value(QZSettings::virtualbike_forceresistance, QZSettings::default_virtualbike_forceresistance).toBool();
      bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool();
      bool zwift_negative_inclination_x2 =
          settings.value(QZSettings::zwift_negative_inclination_x2, QZSettings::default_zwift_negative_inclination_x2).toBool();
      double offset = settings.value(QZSettings::zwift_inclination_offset, QZSettings::default_zwift_inclination_offset).toDouble();
      double gain = settings.value(QZSettings::zwift_inclination_gain, QZSettings::default_zwift_inclination_gain).toDouble();

      qDebug() << QStringLiteral("new requested resistance zwift erg grade ") + QString::number(iresistance) +
                      QStringLiteral(" enabled ") + force_resistance;
      double resistance = ((double)iresistance * 1.5) / 100.0;
      qDebug() << QStringLiteral("calculated erg grade ") + QString::number(resistance);

      double grade = ((iresistance / 100.0) * gain) + offset;
      // if the bike doesn't have the inclination by hardware, i'm simulating inclination with the value received form Zwift
      if(!((bike*)Bike)->inclinationAvailableByHardware())
          Bike->setInclination(grade);

      if (iresistance >= 0 || !zwift_negative_inclination_x2)
          emit changeInclination(grade,
                                 ((qTan(qDegreesToRadians(iresistance / 100.0)) * 100.0) * gain) + offset);
      else
          emit changeInclination((((iresistance / 100.0) * 2.0) * gain) + offset,
                                 (((qTan(qDegreesToRadians(iresistance / 100.0)) * 100.0) * 2.0) * gain) + offset);

      if (force_resistance && !erg_mode) {
          // same on the training program
          Bike->changeResistance((resistance_t)(round(resistance * bikeResistanceGain)) + bikeResistanceOffset +
                                 1); // resistance start from 1
      }
    } else if (dt == bluetoothdevice::TREADMILL || dt == bluetoothdevice::ELLIPTICAL) {
      QSettings settings;
      double offset = settings.value(QZSettings::zwift_inclination_offset, QZSettings::default_zwift_inclination_offset).toDouble();
      double gain = settings.value(QZSettings::zwift_inclination_gain, QZSettings::default_zwift_inclination_gain).toDouble();

      qDebug() << QStringLiteral("new requested resistance zwift erg grade ") + QString::number(iresistance);
      double resistance = ((double)iresistance * 1.5) / 100.0;
      qDebug() << QStringLiteral("calculated erg grade ") + QString::number(resistance);

      emit changeInclination(((iresistance / 100.0) * gain) + offset,
                             ((qTan(qDegreesToRadians(iresistance / 100.0)) * 100.0) * gain) + offset);
    }
    emit slopeChanged();
}
