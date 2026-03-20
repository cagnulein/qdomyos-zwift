#include "devices/bike.h"
#include "devices/elliptical.h"
#include "characteristicwriteprocessor.h"
#include <QSettings>

CharacteristicWriteProcessor::CharacteristicWriteProcessor(double bikeResistanceGain, int8_t bikeResistanceOffset,
                                                           bluetoothdevice *bike, QObject *parent)
    : QObject(parent), bikeResistanceOffset(bikeResistanceOffset), bikeResistanceGain(bikeResistanceGain), Bike(bike) {}

void CharacteristicWriteProcessor::changePower(uint16_t power) { Bike->changePower(power); }

void CharacteristicWriteProcessor::changeSlope(int16_t iresistance, uint8_t crr, uint8_t cw) {
    BLUETOOTH_TYPE dt = Bike->deviceType();
    QSettings settings;
    bool force_resistance =
        settings.value(QZSettings::virtualbike_forceresistance, QZSettings::default_virtualbike_forceresistance)
            .toBool();
    bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool();
    bool zwift_negative_inclination_x2 =
        settings.value(QZSettings::zwift_negative_inclination_x2, QZSettings::default_zwift_negative_inclination_x2)
            .toBool();
    double offset =
        settings.value(QZSettings::zwift_inclination_offset, QZSettings::default_zwift_inclination_offset).toDouble();
    double gain =
        settings.value(QZSettings::zwift_inclination_gain, QZSettings::default_zwift_inclination_gain).toDouble();
    double CRRGain = settings.value(QZSettings::CRRGain, QZSettings::default_CRRGain).toDouble();
    double CWGain = settings.value(QZSettings::CWGain, QZSettings::default_CWGain).toDouble();
    bool zwift_play_emulator = settings.value(QZSettings::zwift_play_emulator, QZSettings::default_zwift_play_emulator).toBool();
    double min_inclination = settings.value(QZSettings::min_inclination, QZSettings::default_min_inclination).toDouble();

    qDebug() << QStringLiteral("new requested resistance zwift erg grade ") + QString::number(iresistance) +
                    QStringLiteral(" enabled ") + force_resistance;
    double resistance = ((double)iresistance * 1.5) / 100.0;
    qDebug() << QStringLiteral("calculated erg grade ") + QString::number(resistance);

    double grade = ((iresistance / 100.0) * gain) + offset;
    double percentage = ((qTan(qDegreesToRadians(iresistance / 100.0)) * 100.0) * gain) + offset;
    if (zwift_negative_inclination_x2 && iresistance < 0) {
        grade = (((iresistance / 100.0) * 2.0) * gain) + offset;
        percentage = (((qTan(qDegreesToRadians(iresistance / 100.0)) * 100.0) * 2.0) * gain) + offset;
    }

    if(min_inclination > grade) {
        grade = min_inclination;
        qDebug() << "grade override due to min_inclination " << min_inclination;
    }

    /*
    Surface	Road Crr	MTB Crr	Gravel Crr (Namebrand)	Zwift Gravel Crr
    Pavement	.004	.01	.008	.008
        Sand	.004	.01	.008	.008
        Brick	.0055	.01	.008	.008
        Wood	.0065	.01	.008	.008
        Cobbles	.0065	.01	.008	.008
        Ice/Snow	.0075	.014	.018	.018
        Dirt	.025	.014	.016	.018
        Grass	 	.042
    */
    const double fCRR = crr / 10000.0;
    const double CRR_offset = ((crr - 40) * 0.05) * CRRGain;

    const double fCW = cw / 100.0;
    const double CW_offset = ((crr - 40) * 0.05) * CWGain;

    qDebug() << "changeSlope CRR = " << fCRR << CRR_offset << "CW = " << fCW;

    if (dt == BIKE) {

        // if the bike doesn't have the inclination by hardware, i'm simulating inclination with the value received
        // from Zwift
        if (!((bike *)Bike)->inclinationAvailableByHardware()) {
            if(zwift_play_emulator)
                Bike->setInclination(grade);
            else
                Bike->setInclination(grade + CRR_offset + CW_offset);
        }

        emit changeInclination(grade, percentage);

        if (force_resistance && !erg_mode) {
            // same on the training program
            Bike->changeResistance((resistance_t)(round(resistance * bikeResistanceGain)) + bikeResistanceOffset + 1 +
                                   CRR_offset + CW_offset); // resistance start from 1
        }
    } else if (dt == TREADMILL) {
        emit changeInclination(grade, percentage);
    } else if (dt == ELLIPTICAL) {
        bool inclinationAvailableByHardware = ((elliptical *)Bike)->inclinationAvailableByHardware();
        qDebug() << "inclinationAvailableByHardware" << inclinationAvailableByHardware << "erg_mode" << erg_mode;
        emit changeInclination(grade, percentage);

        if (!inclinationAvailableByHardware) {
            if (force_resistance && !erg_mode) {
                // same on the training program
                ((elliptical *)Bike)
                    ->changeResistance((resistance_t)(round(resistance * bikeResistanceGain)) + bikeResistanceOffset +
                                       1 + CRR_offset + CW_offset); // resistance start from 1
            }
        }
    }
    emit slopeChanged();
}
