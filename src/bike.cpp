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

    const uint16_t watt_cad40_min = 15;
    const uint16_t watt_cad40_max = 30;

    const uint16_t watt_cad45_min = 20;
    const uint16_t watt_cad45_max = 35;

    const uint16_t watt_cad50_min = 25;
    const uint16_t watt_cad50_max = 45;

    const uint16_t watt_cad55_min = 30;
    const uint16_t watt_cad55_max = 55;

    const uint16_t watt_cad60_min = 35;
    const uint16_t watt_cad60_max = 70;

    const uint16_t watt_cad65_min = 40;
    const uint16_t watt_cad65_max = 90;

    const uint16_t watt_cad70_min = 50;
    const uint16_t watt_cad70_max = 110;

    const uint16_t watt_cad75_min = 55;
    const uint16_t watt_cad75_max = 135;

    const uint16_t watt_cad80_min = 65;
    const uint16_t watt_cad80_max = 160;

    const uint16_t watt_cad85_min = 75;
    const uint16_t watt_cad85_max = 190;

    const uint16_t watt_cad90_min = 85;
    const uint16_t watt_cad90_max = 225;

    const uint16_t watt_cad95_min = 100;
    const uint16_t watt_cad95_max = 260;

    const uint16_t watt_cad100_min = 110;
    const uint16_t watt_cad100_max = 300;

    const uint16_t watt_cad105_min = 125;
    const uint16_t watt_cad105_max = 340;

    const uint16_t watt_cad110_min = 140;
    const uint16_t watt_cad110_max = 385;

    const uint16_t watt_cad115_min = 155;
    const uint16_t watt_cad115_max = 435;

    const uint16_t watt_cad120_min = 170;
    const uint16_t watt_cad120_max = 485;

    const uint16_t watt_cad125_min = 190;
    const uint16_t watt_cad125_max = 540;

    const uint16_t watt_cad130_min = 210;
    const uint16_t watt_cad130_max = 595;

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
