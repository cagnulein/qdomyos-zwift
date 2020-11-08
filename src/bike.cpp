#include <QDebug>
#include "bike.h"

bike::bike()
{

}

void bike::changeResistance(int8_t resistance) { requestResistance = resistance;}
double bike::currentCrankRevolutions() { return CrankRevs;}
uint16_t bike::lastCrankEventTime() { return LastCrankEventTime;}
int8_t bike::currentResistance() { return Resistance;}
uint8_t bike::currentCadence() { return Cadence;}
uint8_t bike::fanSpeed() { return FanSpeed; }
bool bike::connected() { return false; }

bluetoothdevice::BLUETOOTH_TYPE bike::deviceType() { return bluetoothdevice::BIKE; }

uint16_t bike::watts()
{
    const uint8_t max_resistance = 15;
    // ref https://translate.google.com/translate?hl=it&sl=en&u=https://support.wattbike.com/hc/en-us/articles/115001881825-Power-Resistance-and-Cadence-Tables&prev=search&pto=aue

    const uint16_t watt_cad40_min = 25;
    const uint16_t watt_cad40_max = 55;

    const uint16_t watt_cad45_min = 35;
    const uint16_t watt_cad45_max = 65;

    const uint16_t watt_cad50_min = 40;
    const uint16_t watt_cad50_max = 80;

    const uint16_t watt_cad55_min = 50;
    const uint16_t watt_cad55_max = 105;

    const uint16_t watt_cad60_min = 60;
    const uint16_t watt_cad60_max = 125;

    const uint16_t watt_cad65_min = 70;
    const uint16_t watt_cad65_max = 160;

    const uint16_t watt_cad70_min = 85;
    const uint16_t watt_cad70_max = 190;

    const uint16_t watt_cad75_min = 100;
    const uint16_t watt_cad75_max = 240;

    const uint16_t watt_cad80_min = 115;
    const uint16_t watt_cad80_max = 280;

    const uint16_t watt_cad85_min = 130;
    const uint16_t watt_cad85_max = 340;

    const uint16_t watt_cad90_min = 150;
    const uint16_t watt_cad90_max = 390;

    const uint16_t watt_cad95_min = 175;
    const uint16_t watt_cad95_max = 450;

    const uint16_t watt_cad100_min = 195;
    const uint16_t watt_cad100_max = 520;

    const uint16_t watt_cad105_min = 210;
    const uint16_t watt_cad105_max = 600;

    const uint16_t watt_cad110_min = 245;
    const uint16_t watt_cad110_max = 675;

    const uint16_t watt_cad115_min = 270;
    const uint16_t watt_cad115_max = 760;

    const uint16_t watt_cad120_min = 300;
    const uint16_t watt_cad120_max = 850;

    const uint16_t watt_cad125_min = 330;
    const uint16_t watt_cad125_max = 945;

    const uint16_t watt_cad130_min = 360;
    const uint16_t watt_cad130_max = 1045;

    if(currentCadence() < 41)
        return((((watt_cad40_max-watt_cad40_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad40_min);
    else if(currentCadence() < 46)
        return((((watt_cad45_max-watt_cad45_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad45_min);
    else if(currentCadence() < 51)
        return((((watt_cad50_max-watt_cad50_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad50_min);
    else if(currentCadence() < 56)
        return((((watt_cad55_max-watt_cad55_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad55_min);
    else if(currentCadence() < 61)
        return((((watt_cad60_max-watt_cad60_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad60_min);
    else if(currentCadence() < 66)
        return((((watt_cad65_max-watt_cad65_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad65_min);
    else if(currentCadence() < 71)
        return((((watt_cad70_max-watt_cad70_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad70_min);
    else if(currentCadence() < 76)
        return((((watt_cad75_max-watt_cad75_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad75_min);
    else if(currentCadence() < 81)
        return((((watt_cad80_max-watt_cad80_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad80_min);
    else if(currentCadence() < 86)
        return((((watt_cad85_max-watt_cad85_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad85_min);
    else if(currentCadence() < 91)
        return((((watt_cad90_max-watt_cad90_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad90_min);
    else if(currentCadence() < 96)
        return((((watt_cad95_max-watt_cad95_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad95_min);
    else if(currentCadence() < 101)
        return((((watt_cad100_max-watt_cad100_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad100_min);
    else if(currentCadence() < 106)
        return((((watt_cad105_max-watt_cad105_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad105_min);
    else if(currentCadence() < 111)
        return((((watt_cad110_max-watt_cad110_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad110_min);
    else if(currentCadence() < 116)
        return((((watt_cad115_max-watt_cad115_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad115_min);
    else if(currentCadence() < 121)
        return((((watt_cad120_max-watt_cad120_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad120_min);
    else if(currentCadence() < 126)
        return((((watt_cad125_max-watt_cad125_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad125_min);
    else
        return((((watt_cad130_max-watt_cad130_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad130_min);
    return 0;
}
