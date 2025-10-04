#include "characteristicnotifier2a63.h"

CharacteristicNotifier2A63::CharacteristicNotifier2A63(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(0x2a63, parent), Bike(Bike) {}

int CharacteristicNotifier2A63::notify(QByteArray &value) {
    double normalizeWattage = Bike->wattsMetric().value();
    if (normalizeWattage < 0)
        normalizeWattage = 0;
    
    if (Bike->deviceType() == BIKE) {
        /*
         // set measurement
         measurement[2] = power & 0xFF;
         measurement[3] = (power >> 8) & 0xFF;

         measurement[4] = wheelrev & 0xFF;
         measurement[5] = (wheelrev >> 8) & 0xFF;
         measurement[6] = (wheelrev >> 16) & 0xFF;
         measurement[7] = (wheelrev >> 24) & 0xFF;

         measurement[8] = lastwheel & 0xFF;
         measurement[9] = (lastwheel >> 8) & 0xFF;

         measurement[10] = crankrev & 0xFF;
         measurement[11] = (crankrev >> 8) & 0xFF;

         measurement[12] = lastcrank & 0xFF;
         measurement[13] = (lastcrank >> 8) & 0xFF;
         
         // speed & distance
         // NOTE : based on Apple Watch default wheel dimension 700c x 2.5mm
         // NOTE : 3 is theoretical crank:wheel gear ratio
         // NOTE : 2.13 is circumference of 700c in meters

         wheelCount = crankCount * 3;
              speed = cadence * 3 * 2.13 * 60 / 1000;
           distance = wheelCount * 2.13 / 1000;

         #if defined(USEPOWER)
           lastWheelK = lastCrankK * 2;  // 1/2048 s granularity
         #else
           lastWheelK = lastCrankK * 1;  // 1/1024 s granularity
         #endif
         
         */
        
      uint32_t wheelCount = (uint32_t)Bike->currentCrankRevolutions() * 3;
      uint16_t lastWheelK = Bike->lastCrankEventTime() * 2;
        
      value.append((char)0x30); // crank data present and wheel for apple watch
      value.append((char)0x00);
      value.append((char)(((uint16_t)normalizeWattage) & 0xFF));                     // watt
      value.append((char)(((uint16_t)normalizeWattage) >> 8) & 0xFF);                // watt
        
        value.append((char)(((uint32_t)wheelCount) & 0xFF));      // revs count
        value.append((char)(((uint32_t)wheelCount) >> 8) & 0xFF); // revs count
        value.append((char)(((uint32_t)wheelCount) >> 16) & 0xFF); // revs count
        value.append((char)(((uint32_t)wheelCount) >> 24) & 0xFF); // revs count
        value.append((char)(lastWheelK & 0xff));                       // eventtime
        value.append((char)(lastWheelK >> 8) & 0xFF);                  // eventtime
        
      value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) & 0xFF));      // revs count
      value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) >> 8) & 0xFF); // revs count
      value.append((char)(Bike->lastCrankEventTime() & 0xff));                       // eventtime
      value.append((char)(Bike->lastCrankEventTime() >> 8) & 0xFF);                  // eventtime
      return CN_OK;
    } else
        return CN_INVALID;
}
