package com.nettarion.hyperborea.hardware.fitpro.v1

/**
 * Adapt Hyperborea 156e7ee: a manual/SIM target must clear a latched MCU ERG mode.
 * Only V1 bikes declaring this field opt in. Do not depend on a potentially
 * truncated poll readback or re-send a target periodically (physical keys must win).
 */
internal fun bikeManualErgExitFields(
    isBike: Boolean,
    supportedBitFields: Set<Int>,
): Map<V1DataField, Float> =
    if (isBike && V1DataField.IS_CONSTANT_WATTS_MODE.fieldIndex in supportedBitFields) {
        mapOf(V1DataField.IS_CONSTANT_WATTS_MODE to 0f)
    } else {
        emptyMap()
    }
