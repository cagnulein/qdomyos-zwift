package com.nettarion.hyperborea.hardware.fitpro.v1

import kotlin.math.ceil
import kotlin.math.roundToInt

/**
 * Converts between a resistance *level* (1…[maxResistance], what the UI and fitness apps speak) and
 * the *raw* value the FitPro [V1DataField.RESISTANCE] bitfield carries — the way the stock firmware's
 * resistance converter does it: `scale = 10000 / maxResistance` (a fraction, hence float math);
 * `raw = round(level × scale) − 1`, clamped at ≥ 0; `level = ceil(raw ÷ scale)`.
 */
class ResistanceConverter(maxResistance: Int) {
    private val scale: Double = if (maxResistance > 0) 10_000.0 / maxResistance else 1.0

    fun levelToRaw(level: Int): Int = maxOf(0, (level * scale).roundToInt() - 1)

    fun rawToLevel(raw: Int): Int = ceil(raw / scale).toInt()
}
