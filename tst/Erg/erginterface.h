#pragma once

#include "minmax.h"
#include "definitions.h"

/**
 * @brief An interface for power calculation functions
 */
class erginterface {
  protected:
    erginterface() {}
    erginterface(const erginterface&) {}
  public:
    /**
     * @brief The cadence domain of the power functions.
     * @return
     */
      virtual minmax<int16_t> getCadenceLimits() const = 0;

    /**
     * @brief The inclusive bounds of resistance the device provides.
     * @return
     */
      virtual minmax<resistance_t> getResistanceLimits() const = 0;

    /**
     * @brief Gets the power in watts for the given cadence and resistance level.
     * @param cadence The cadence in a unit appropriate for the device, e.g. revolutions/strides/strokes per minute.
     * @param resistance The resistance level.
     * @return
     */
    virtual double getPower(const int32_t cadence, const resistance_t resistance) = 0;


    /**
     * @brief Gets the resistance required to acheive the specified power in watts at the specified cadence.
     * @param cadence The cadence in a unit appropriate for the device, e.g. revolutions/strides/strokes per minute.
     * @param power The power in watts to acheive.
     * @return
     */
    virtual int32_t getResistance(const int32_t cadence, const double power) = 0;

    /**
     * @brief Converts the specified device resistance to its corresponding Peloton resistance level.
     * @return
     */
    virtual double toPeloton(const resistance_t resistance) = 0;

    /**
     * @brief Converts the specified Peloton resistance level to its corresponding device resistance level.
     * @return
     */
    virtual resistance_t fromPeloton(const int32_t pelotonResistance) = 0;


    /**
     * @brief Destructor
     */
    virtual ~erginterface() = default;
};
