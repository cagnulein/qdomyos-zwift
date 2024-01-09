#ifndef ZWIFTWORLDCONSTANTS_H
#define ZWIFTWORLDCONSTANTS_H

class ZwiftWorldConstants
{
public:
   double MetersBetweenLatitudeDegree;
   double MetersBetweenLongitudeDegree;
   double MetersBetweenLatitudeDegreeMul;
   double MetersBetweenLongitudeDegreeMul;
   double CenterLatitudeFromOrigin;
   double CenterLongitudeFromOrigin;

   ZwiftWorldConstants(double metersBetweenLatitudeDegree, double metersBetweenLongitudeDegree, double centerLatitudeFromOrigin, double centerLongitudeFromOrigin)
   {
       MetersBetweenLatitudeDegree = metersBetweenLatitudeDegree;
       MetersBetweenLongitudeDegree = metersBetweenLongitudeDegree;
       MetersBetweenLatitudeDegreeMul = 1 / metersBetweenLatitudeDegree;
       MetersBetweenLongitudeDegreeMul = 1 / metersBetweenLongitudeDegree;
       CenterLatitudeFromOrigin = centerLatitudeFromOrigin * metersBetweenLatitudeDegree * 100;
       CenterLongitudeFromOrigin = centerLongitudeFromOrigin * metersBetweenLongitudeDegree * 100;
   }
};

#endif // ZWIFTWORLDCONSTANTS_H
