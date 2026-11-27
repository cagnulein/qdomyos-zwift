package com.nettarion.hyperborea.core.model

data class DeviceInfo(
    val name: String,
    val type: DeviceType,
    val supportedMetrics: Set<Metric>,
    val maxResistance: Int,
    val minResistance: Int,
    val minIncline: Float,
    val maxIncline: Float,
    val maxPower: Int,
    val minPower: Int,
    val powerStep: Int,
    val resistanceStep: Float,
    val inclineStep: Float,
    val speedStep: Float,
    val maxSpeed: Float,
) {
    companion object {
        val DEFAULT_INDOOR_BIKE = DeviceInfo(
            name = "Hyperborea",
            type = DeviceType.BIKE,
            supportedMetrics = setOf(
                Metric.POWER, Metric.CADENCE, Metric.SPEED,
                Metric.RESISTANCE, Metric.INCLINE,
                Metric.DISTANCE, Metric.CALORIES,
            ),
            maxResistance = 24, minResistance = 1,
            minIncline = -10f, maxIncline = 20f,
            maxPower = 2000, minPower = 0, powerStep = 1,
            resistanceStep = 1.0f, inclineStep = 0.5f,
            speedStep = 0.5f, maxSpeed = 60f,
        )
    }
}

enum class DeviceType { BIKE, TREADMILL, ROWER, ELLIPTICAL }

/**
 * Belt-driven machines (treadmills and incline trainers) — true only for [DeviceType.TREADMILL]
 * here, since the incline-trainer equipment id is folded into it. Named so the rule has a single
 * home and can be extended if a dedicated incline-trainer type is ever added.
 *
 * The distinction drives two behaviours that genuinely differ on belt machines:
 *  - **Displayed speed source** — belt machines report belt speed in the writable `KPH` field and
 *    leave `ACTUAL_KPH` at 0; bikes/ellipticals leave `KPH` as the target and report a virtual speed
 *    in `ACTUAL_KPH`.
 *  - **Stopping** — a belt machine keeps the belt running on a bare idle transition and must be
 *    explicitly commanded to stop.
 */
val DeviceType.isBeltBased: Boolean
    get() = this == DeviceType.TREADMILL

enum class Metric { POWER, CADENCE, SPEED, RESISTANCE, INCLINE, HEART_RATE, DISTANCE, CALORIES }
