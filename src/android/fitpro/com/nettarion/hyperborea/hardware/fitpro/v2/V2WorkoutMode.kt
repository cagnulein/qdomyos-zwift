package com.nettarion.hyperborea.hardware.fitpro.v2

/**
 * FitPro V2 console workout state — the value carried by the [V2FeatureId.WORKOUT_STATE] feature.
 * Mirrors the stock service's V2WorkoutState. Note V2 numbers these differently from V1's
 * [com.nettarion.hyperborea.hardware.fitpro.v1.WorkoutMode] (e.g. RUNNING is 3 here, 2 there).
 *
 * The app drives transitions by writing this feature; the MCU reports the current state via events.
 * A workout is brought up `NONE → WARM_UP → RUNNING` (`RUNNING ↔ PAUSED` during the workout).
 */
enum class V2WorkoutMode(val raw: Float) {
    UNKNOWN(-1f),
    NONE(0f),            // idle — no workout
    READY_TO_START(1f),
    WARM_UP(2f),
    RUNNING(3f),         // the active workout state
    COOL_DOWN(4f),
    PAUSED(5f),
    RESULTS(6f),
    OFF_MACHINE(7f),
    ;

    companion object {
        fun fromRaw(raw: Float): V2WorkoutMode = entries.firstOrNull { it.raw == raw } ?: UNKNOWN
    }
}
