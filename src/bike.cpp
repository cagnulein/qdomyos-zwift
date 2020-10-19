#include <QDebug>
#include "bike.h"

bike::bike()
{

}

void bike::changeResistance(double resistance) { requestResistance = resistance;}
double bike::currentResistance() { return Resistance;}
uint8_t bike::currentCadence() { return Cadence;}
uint8_t bike::fanSpeed() { return FanSpeed; }
bool bike::connected() { return false; }

bluetoothdevice::BLUETOOTH_TYPE bike::deviceType() { return bluetoothdevice::BIKE; }

uint16_t bike::watts(double weight)
{
    uint16_t watts=0;
    if(currentSpeed() > 0)
    {
        qDebug() << "BIKE WATT TODO";
    }
    return watts;
}
