#pragma once

#include "erginterface.h"
#include "bike.h"

class bikeergfunctions : public virtual erginterface {
  protected:
    bike* device;

    void setCadence(int32_t cadence);
public:
    bikeergfunctions(bike *device);


    std::optional<int32_t> getMaxCadence() const override;

    std::optional<int32_t> getMinCadence() const override;

    std::optional<resistance_t> getMaxResistance() const override;

    std::optional<resistance_t> getMinResistance() const override;


    double getPower(const int32_t cadence, const resistance_t resistance) override;


    int32_t getResistance(const int32_t cadence, const double power) override;

    int32_t toPeloton(const resistance_t resistance) override;
    resistance_t fromPeloton(const int pelotonResistance) override;

};
