package com.nettarion.hyperborea.hardware.fitpro.v1

/**
 * FitPro V1 console workout state — the value of the [V1DataField.WORKOUT_MODE] bitfield.
 *
 * The MCU runs the workout state machine; the app drives transitions by writing this field, and
 * reads it back to see the current state. A workout is brought up `IDLE → WARM_UP → RUNNING`
 * (`RUNNING ↔ PAUSE` during the workout); `DMK` is the safety-key-removed state. Wire values are
 * the raw byte the [V1Converter.BYTE] converter carries.
 */
enum class WorkoutMode(val raw: Float) {
    UNKNOWN(0f),
    IDLE(1f),
    RUNNING(2f),
    PAUSE(3f),
    DMK(8f),
    WARM_UP(10f),
    COOL_DOWN(11f),
    ;

    companion object {
        fun fromRaw(raw: Int): WorkoutMode = entries.firstOrNull { it.raw.toInt() == raw } ?: UNKNOWN
        fun fromRaw(raw: Float): WorkoutMode = fromRaw(raw.toInt())
    }
}
