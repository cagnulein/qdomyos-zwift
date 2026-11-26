package com.nettarion.hyperborea.hardware.fitpro.v1

/** USB V1 replies are raw FitPro frames: four header bytes, field data, checksum. */
internal fun bikeUsbReadBatches(fields: Set<V1DataField>): List<Set<V1DataField>> {
    // Keep control readback small and separate from telemetry, including after a write.
    val controls = fields.intersect(setOf(
        V1DataField.RESISTANCE, V1DataField.GEAR,
        V1DataField.IS_CONSTANT_WATTS_MODE, V1DataField.WORKOUT_MODE,
    ))
    val batches = mutableListOf<Set<V1DataField>>()
    if (controls.isNotEmpty()) batches.add(controls)
    var batch = linkedSetOf<V1DataField>()
    var responseBytes = 5
    for (field in (fields - controls).sortedBy { it.fieldIndex }) {
        require(field.sizeBytes <= 59) { "Field cannot fit a USB reply: $field" }
        if (responseBytes + field.sizeBytes > 64) {
            batches.add(batch)
            batch = linkedSetOf()
            responseBytes = 5
        }
        batch.add(field)
        responseBytes += field.sizeBytes
    }
    if (batch.isNotEmpty()) batches.add(batch)
    return batches
}
