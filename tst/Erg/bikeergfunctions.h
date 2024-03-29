#pragma once

#include "erginterface.h"
#include "bike.h"

class bikeergfunctions : public virtual erginterface {
  protected:
    bike* device;

    void setCadence(int32_t cadence);
public:
    bikeergfunctions(bike *device);


    minmax<int16_t> getCadenceLimits() const override { return this->device->cadenceLimits(); }

    minmax<resistance_t> getResistanceLimits() const override { return this->device->resistanceLimits(); }


    double getPower(const int32_t cadence, const resistance_t resistance) override;


    int32_t getResistance(const int32_t cadence, const double power) override;

    double toPeloton(const resistance_t resistance) override;
    resistance_t fromPeloton(const int pelotonResistance) override;

};
