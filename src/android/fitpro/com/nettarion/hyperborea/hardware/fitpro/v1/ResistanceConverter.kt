package com.nettarion.hyperborea.hardware.fitpro.v1

import kotlin.math.ceil

/** Stock FitPro V1 uses integer division, not fractional scaling with rounding.
 * Ported from Hyperborea 2899c97 (MIT; see LICENSE.Hyperborea).
 */
class ResistanceConverter(maxResistance: Int) {
    private val scale: Int = if (maxResistance > 0) (10_000 / maxResistance).coerceAtLeast(1) else 1

    fun levelToRaw(level: Int): Int = maxOf(0, level * scale - 1)

    fun rawToLevel(raw: Int): Int = ceil(raw.toDouble() / scale).toInt()
}
