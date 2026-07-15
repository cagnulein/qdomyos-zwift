package com.nettarion.hyperborea.hardware.fitpro.session

import com.nettarion.hyperborea.core.model.DeviceInfo
import com.nettarion.hyperborea.core.model.DeviceType
import com.nettarion.hyperborea.core.model.Metric

object DeviceDatabase {

    private data class DeviceRecord(
        val name: String,
        val type: DeviceType?,
        val maxResistance: Int?,
        val minIncline: Float?,
        val maxIncline: Float?,
        val maxSpeed: Float?,
        val inclineStep: Float,
        val speedStep: Float,
        val powerCurveIndex: Int? = null,
    )

    fun fromModel(modelNumber: Int): DeviceInfo {
        val record = lookupModelNumber(modelNumber) ?: FALLBACK
        return record.toDeviceInfo()
    }

    fun fromHandshake(modelNumber: Int, partNumber: Int): DeviceInfo {
        val record = lookupPartNumber(partNumber) ?: FALLBACK
        return record.toDeviceInfo()
    }

    fun fallback(): DeviceInfo = FALLBACK.toDeviceInfo()

    /** Returns the power curve table index for a given part number, or null if unknown. */
    fun powerCurveIndexForPartNumber(partNumber: Int): Int? {
        val idx = IfitDeviceCatalog.partNumbers.binarySearch(partNumber)
        if (idx < 0) return null
        val curveIdx = IfitDeviceCatalog.powerCurveIndices[idx].toInt()
        return if (curveIdx < 0) null else curveIdx
    }

    /** Maps a SUPPORTED_DEVICES equipment device ID to a DeviceType. */
    fun deviceTypeFromEquipmentId(equipmentId: Int): DeviceType = when (equipmentId) {
        4, 5 -> DeviceType.TREADMILL
        6, 9, 19 -> DeviceType.ELLIPTICAL
        7, 8 -> DeviceType.BIKE
        20 -> DeviceType.ROWER
        else -> DeviceType.BIKE
    }

    /** Returns type-derived defaults for overlaying after equipment type is known. */
    fun defaultsForType(type: DeviceType): DeviceInfo = DeviceInfo(
        name = "",
        type = type,
        supportedMetrics = metricsForType(type),
        maxResistance = defaultMaxResistance(type),
        minResistance = if (type == DeviceType.TREADMILL) 0 else 1,
        minIncline = defaultMinIncline(type),
        maxIncline = defaultMaxIncline(type),
        maxPower = defaultMaxPower(type),
        minPower = 0,
        powerStep = 1,
        resistanceStep = 1.0f,
        inclineStep = 0.5f,
        speedStep = 0.5f,
        maxSpeed = defaultMaxSpeed(type),
    )

    /**
     * Returns a one-line diagnostic string for a part number, or null if not in catalog.
     *
     * Format: "NAME (ICON_PN) type res=N incline=MIN..MAX speed=Nkph curve=N"
     */
    fun catalogSummary(partNumber: Int): String? {
        val idx = IfitDeviceCatalog.partNumbers.binarySearch(partNumber)
        if (idx < 0) return null

        val name = IfitDeviceCatalog.names[IfitDeviceCatalog.nameIndices[idx].toInt()]
        val iconPnIdx = IfitDeviceCatalog.iconPartNumberIndices[idx].toInt()
        val iconPn = if (iconPnIdx >= 0) IfitDeviceCatalog.iconPartNumbers[iconPnIdx] else null
        val typeOrd = IfitDeviceCatalog.equipmentTypes[idx].toInt()
        val typeName = if (typeOrd >= 0) DeviceType.entries[typeOrd].name.lowercase() else "unknown"
        val maxRes = IfitDeviceCatalog.maxResistances[idx].toInt()
        val minInc = IfitDeviceCatalog.minInclines[idx].toInt()
        val maxInc = IfitDeviceCatalog.maxInclines[idx].toInt()
        val maxSpd = IfitDeviceCatalog.maxSpeedTenthsKph[idx].toInt()
        val spdKph = if (maxSpd > 0) "${maxSpd / 10}.${maxSpd % 10}kph" else "0kph"
        val curve = IfitDeviceCatalog.powerCurveIndices[idx].toInt()
        val maxPower = if (curve >= 0) PowerCurves.maxPower(curve) else null

        return buildString {
            append(name)
            if (iconPn != null) append(" ($iconPn)")
            append(" $typeName")
            append(" res=0-$maxRes")
            append(" incline=$minInc..$maxInc")
            append(" speed=$spdKph")
            append(" power=0-${maxPower ?: "?"}W")
            append(" curve=$curve")
        }
    }

    private val FALLBACK = DeviceRecord(
        name = "FitPro Device",
        type = null,
        maxResistance = null,
        minIncline = null,
        maxIncline = null,
        maxSpeed = null,
        inclineStep = 0.5f,
        speedStep = 0.5f,
    )

    // Model number lookup via ICON part number suffix matching.
    // Model number comes from V1 handshake SystemInfoResponse (e.g. 2117 → "EBNT02117").
    private fun lookupModelNumber(modelNumber: Int): DeviceRecord? {
        val modelStr = modelNumber.toString()
        for (i in IfitDeviceCatalog.iconPartNumberIndices.indices) {
            val iconIdx = IfitDeviceCatalog.iconPartNumberIndices[i].toInt()
            if (iconIdx >= 0) {
                val iconPn = IfitDeviceCatalog.iconPartNumbers[iconIdx]
                if (iconPn.endsWith(modelStr)) {
                    return lookupPartNumber(IfitDeviceCatalog.partNumbers[i])
                }
            }
        }
        return null
    }

    // Part number lookup via IfitDeviceCatalog (sorted parallel arrays, binary search).
    // Part number comes from V1 handshake SystemInfoResponse.
    private fun lookupPartNumber(partNumber: Int): DeviceRecord? {
        val idx = IfitDeviceCatalog.partNumbers.binarySearch(partNumber)
        if (idx < 0) return null

        val typeOrd = IfitDeviceCatalog.equipmentTypes[idx].toInt()
        val type = if (typeOrd >= 0) DeviceType.entries[typeOrd] else null

        val maxRes = IfitDeviceCatalog.maxResistances[idx].toInt()
        val minInc = IfitDeviceCatalog.minInclines[idx].toInt()
        val maxInc = IfitDeviceCatalog.maxInclines[idx].toInt()
        val maxSpd = IfitDeviceCatalog.maxSpeedTenthsKph[idx].toInt()
        val curveIdx = IfitDeviceCatalog.powerCurveIndices[idx].toInt()

        return DeviceRecord(
            name = IfitDeviceCatalog.names[IfitDeviceCatalog.nameIndices[idx].toInt()],
            type = type,
            maxResistance = if (maxRes > 0) maxRes else null,
            minIncline = if (minInc != 0 || maxInc != 0) minInc.toFloat() else null,
            maxIncline = if (minInc != 0 || maxInc != 0) maxInc.toFloat() else null,
            maxSpeed = if (maxSpd > 0) maxSpd / 10f else null,
            inclineStep = 0.5f,
            speedStep = 0.5f,
            powerCurveIndex = if (curveIdx < 0) null else curveIdx,
        )
    }

    fun fromProductId(productId: Int): DeviceInfo? = when (productId) {
        2, 3, 4 -> FALLBACK.toDeviceInfo()
        else -> null
    }

    private fun DeviceRecord.toDeviceInfo(): DeviceInfo {
        val t = type ?: DeviceType.BIKE
        return DeviceInfo(
            name = name,
            type = t,
            supportedMetrics = metricsForType(t),
            maxResistance = maxResistance ?: defaultMaxResistance(t),
            minResistance = if (t == DeviceType.TREADMILL) 0 else 1,
            minIncline = minIncline ?: defaultMinIncline(t),
            maxIncline = maxIncline ?: defaultMaxIncline(t),
            maxPower = powerCurveIndex?.let { PowerCurves.maxPower(it) }
                ?: defaultMaxPower(t),
            minPower = 0,
            powerStep = 1,
            resistanceStep = 1.0f,
            inclineStep = inclineStep,
            speedStep = speedStep,
            maxSpeed = maxSpeed ?: defaultMaxSpeed(t),
        )
    }

    private fun metricsForType(type: DeviceType): Set<Metric> = when (type) {
        DeviceType.BIKE -> setOf(
            Metric.POWER, Metric.CADENCE, Metric.SPEED,
            Metric.RESISTANCE, Metric.INCLINE,
            Metric.DISTANCE, Metric.CALORIES,
        )
        DeviceType.TREADMILL -> setOf(
            Metric.POWER, Metric.SPEED, Metric.INCLINE,
            Metric.DISTANCE, Metric.CALORIES,
        )
        DeviceType.ROWER -> setOf(
            Metric.POWER, Metric.CADENCE, Metric.SPEED,
            Metric.RESISTANCE,
            Metric.DISTANCE, Metric.CALORIES,
        )
        DeviceType.ELLIPTICAL -> setOf(
            Metric.POWER, Metric.CADENCE, Metric.SPEED,
            Metric.RESISTANCE, Metric.INCLINE,
            Metric.DISTANCE, Metric.CALORIES,
        )
    }

    private fun defaultMaxResistance(type: DeviceType): Int = when (type) {
        DeviceType.BIKE -> 24
        DeviceType.TREADMILL -> 0
        DeviceType.ROWER -> 26
        DeviceType.ELLIPTICAL -> 26
    }

    private fun defaultMinIncline(type: DeviceType): Float = when (type) {
        DeviceType.BIKE -> -10f
        DeviceType.TREADMILL -> -6f
        DeviceType.ROWER -> 0f
        DeviceType.ELLIPTICAL -> -10f
    }

    private fun defaultMaxIncline(type: DeviceType): Float = when (type) {
        DeviceType.BIKE -> 20f
        DeviceType.TREADMILL -> 40f
        DeviceType.ROWER -> 0f
        DeviceType.ELLIPTICAL -> 15f
    }

    private fun defaultMaxPower(type: DeviceType): Int = when (type) {
        DeviceType.BIKE -> 2000
        DeviceType.TREADMILL -> 1200
        DeviceType.ROWER -> 1000
        DeviceType.ELLIPTICAL -> 1500
    }

    private fun defaultMaxSpeed(type: DeviceType): Float = when (type) {
        DeviceType.BIKE -> 60f
        DeviceType.TREADMILL -> 24f
        DeviceType.ROWER -> 0f
        DeviceType.ELLIPTICAL -> 60f
    }
}
