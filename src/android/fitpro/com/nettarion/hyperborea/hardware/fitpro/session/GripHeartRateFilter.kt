package com.nettarion.hyperborea.hardware.fitpro.session

import kotlin.math.roundToInt

/**
 * Smooths the FitPro grip (hand-contact) heart-rate signal.
 *
 * The MCU reports the raw contact-sensor reading in the `PULSE` field on every poll (~100 ms). That
 * signal is electrically noisy and swings wildly until the sensor gets a stable lock — users have
 * seen it jump anywhere from 60 to 150 bpm on a treadmill. Two cheap filters tame it:
 *
 *  1. **Validity gate** — drop physiologically implausible samples and the contact-loss `0`. A
 *     dropped sample also forgets the running average, so re-gripping starts clean and the displayed
 *     bpm blanks while there's no contact (which is what users expect when they let go).
 *  2. **Exponential moving average** — once a valid stream is flowing, smooth it so the number
 *     doesn't lurch on every poll.
 *
 * External BLE heart-rate monitors deliver clean, already-debounced values and must **not** go
 * through this filter — they're merged in the orchestrator, downstream of the hardware adapter.
 */
class GripHeartRateFilter(
    private val smoothing: Float = DEFAULT_SMOOTHING,
    private val plausibleBpm: IntRange = PLAUSIBLE_BPM,
) {
    private var ema: Float? = null

    /**
     * Feeds one raw grip-HR sample (the byte the MCU reports in `PULSE`). Returns the smoothed bpm
     * to display, or `null` when there's no usable reading (no contact / out of range) — callers
     * should clear the displayed heart rate on `null`.
     */
    fun update(rawBpm: Int): Int? {
        if (rawBpm !in plausibleBpm) {
            ema = null
            return null
        }
        val previous = ema
        val next = if (previous == null) rawBpm.toFloat() else previous + smoothing * (rawBpm - previous)
        ema = next
        return next.roundToInt()
    }

    companion object {
        /** EMA weight for each new sample; lower = smoother but slower to track real changes. */
        private const val DEFAULT_SMOOTHING = 0.3f

        /** Resting-low to athletic-max bounds; anything outside is sensor noise, not a heartbeat. */
        private val PLAUSIBLE_BPM = 30..230
    }
}
