#pragma once

#include <optional>
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
     * @brief The maximum cadence the object understands.
     * @return
     */
    virtual std::optional<int32_t> getMaxCadence() const = 0;

    /**
     * @brief The minimum cadence the object understands.
     * @return
     */
    virtual std::optional<int32_t> getMinCadence() const = 0;

    /**
     * @brief The maximum resistance the device accepts.
     * @return
     */
    virtual std::optional<resistance_t> getMaxResistance() const = 0;

    /**
     * @brief The minimum resistance level.
     * @return
     */
    virtual std::optional<resistance_t> getMinResistance() const = 0;


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
     * @brief Destructor
     */
    virtual ~erginterface() = default;
};
