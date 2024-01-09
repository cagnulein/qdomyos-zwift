#ifndef MAPCOORDINATE_H
#define MAPCOORDINATE_H

#include "ZwiftWorldConstants.h"

class MapCoordinate {
public:
    double X;
    double Y;
    double Altitude;
    double WorldId;

    MapCoordinate(double X, double Y, double Altitude, double WorldID) {
        this->X = X;
        this->Y = Y;
        this->Altitude = Altitude;
        this->WorldId = WorldID;
    };
};

#endif // MAPCOORDINATE_H
