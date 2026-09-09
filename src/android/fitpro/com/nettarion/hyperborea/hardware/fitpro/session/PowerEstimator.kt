package com.nettarion.hyperborea.hardware.fitpro.session

import com.nettarion.hyperborea.core.model.DeviceType
import kotlin.math.max
import kotlin.math.pow
import kotlin.math.roundToInt

/**
 * Estimates power output from speed and resistance using per-device calibration curves.
 *
 * Uses bilinear interpolation across RPM rows and relative-resistance columns.
 */
object PowerEstimator {

    /**
     * Estimate power using a device-specific power curve table.
     *
     * @param tableIndex Index into [PowerCurves.tables] (0-31)
     * @param speedKph Current speed in km/h
     * @param resistance Current resistance level (absolute, from MCU)
     * @param maxResistance Maximum resistance level for this device
     * @param deviceType Device type (affects speed→RPM conversion factor)
     * @return Estimated watts, or null if estimation is impossible
     */
    fun estimate(
        tableIndex: Int,
        speedKph: Float,
        resistance: Int,
        maxResistance: Int,
        deviceType: DeviceType,
    ): Int? {
        if (speedKph <= 0f || maxResistance <= 0) return null
        if (tableIndex !in PowerCurves.tables.indices) return null

        val table = PowerCurves.tables[tableIndex]
        if (table.isEmpty()) return null

        // Speed → RPM: ellipticals use 8x, everything else uses 5x
        val rpmFactor = when (deviceType) {
            DeviceType.ELLIPTICAL -> 8
            else -> 5
        }
        val rpm = speedKph * rpmFactor

        // Relative resistance: (resistance / maxResistance) * 100, clamped to 0-100
        val relativeResistance = (resistance.toDouble() / maxResistance * 100.0)
            .coerceIn(0.0, 100.0)

        return interpolate(table, rpm, relativeResistance)
    }

    /**
     * Fallback power estimation when no power curve table is available.
     *
     * Uses a closed-form physics-style estimate:
     *   watts = max(0, speed³ × 0.185 + (relRes/100 + 0.0053) × (weight + 10) × 9.8067 × speed)
     *
     * @param speedKph Current speed in km/h
     * @param resistance Current resistance level
     * @param maxResistance Maximum resistance level
     * @param weightKg Rider weight in kg (default 75kg since weight isn't available in hardware layer)
     * @return Estimated watts, or null if estimation is impossible
     */
    fun estimateFallback(
        speedKph: Float,
        resistance: Int,
        maxResistance: Int,
        weightKg: Double = 75.0,
    ): Int? {
        if (speedKph <= 0f || maxResistance <= 0) return null

        val relRes = (resistance.toDouble() / maxResistance).coerceIn(0.0, 1.0)
        val speed = speedKph.toDouble()

        val watts = max(
            0.0,
            speed.pow(3.0) * 0.185 + (relRes + 0.0053) * (weightKg + 10.0) * 9.8067 * speed,
        )
        return watts.roundToInt()
    }

    private fun interpolate(
        table: List<PowerCurves.Row>,
        rpm: Float,
        relativeResistance: Double,
    ): Int? {
        // Find bounding RPM rows
        var lowerRow: PowerCurves.Row? = null
        var upperRow: PowerCurves.Row? = null

        for (row in table) {
            if (row.rpm <= rpm) {
                lowerRow = row
            }
        }
        for (row in table) {
            if (row.rpm > rpm) {
                upperRow = row
                break
            }
        }

        // Edge cases: RPM below all rows or above all rows
        if (lowerRow == null) lowerRow = table.first()
        if (upperRow == null) upperRow = table.last()

        if (lowerRow.watts.size != 11 || upperRow.watts.size != 11) return null

        // Resistance index: relRes / 10 → 0-10
        val resIdx = (relativeResistance / 10.0).toInt().coerceIn(0, 10)

        if (relativeResistance % 10.0 == 0.0 || resIdx >= 10) {
            // Exact resistance index — interpolate only across RPM
            val idx = resIdx.coerceAtMost(10)
            val lowerWatts = lowerRow.watts[idx]
            val upperWatts = upperRow.watts[idx]

            val rpmRatio = if (lowerRow.rpm == upperRow.rpm) 0.0
            else (rpm - lowerRow.rpm).toDouble() / (upperRow.rpm - lowerRow.rpm)

            return lerp(lowerWatts.toDouble(), upperWatts.toDouble(), rpmRatio)
        }

        // Fractional resistance — bilinear interpolation
        val nextIdx = (resIdx + 1).coerceAtMost(10)
        val resFraction = (relativeResistance - resIdx * 10.0) / 10.0

        // Interpolate resistance within lower RPM row
        val lowerResInterp = lerpDouble(
            lowerRow.watts[resIdx].toDouble(),
            lowerRow.watts[nextIdx].toDouble(),
            resFraction,
        )
        // Interpolate resistance within upper RPM row
        val upperResInterp = lerpDouble(
            upperRow.watts[resIdx].toDouble(),
            upperRow.watts[nextIdx].toDouble(),
            resFraction,
        )

        // Interpolate across RPM
        val rpmRatio = if (lowerRow.rpm == upperRow.rpm) 0.0
        else (rpm - lowerRow.rpm).toDouble() / (upperRow.rpm - lowerRow.rpm)

        return lerp(lowerResInterp, upperResInterp, rpmRatio)
    }

    private fun lerpDouble(a: Double, b: Double, t: Double): Double =
        a + (b - a) * t

    private fun lerp(a: Double, b: Double, t: Double): Int? {
        val result = a + (b - a) * t
        if (result.isNaN()) return null
        return result.roundToInt()
    }
}
