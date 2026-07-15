package com.nettarion.hyperborea.core.model

data class ExerciseData(
    val power: Int?,
    val cadence: Int?,
    val speed: Float?,
    val resistance: Int?,
    val incline: Float?,
    val heartRate: Int?,
    val distance: Float?,
    val calories: Int?,
    val elapsedTime: Long,
    val targetSpeed: Float? = null,
    val targetIncline: Float? = null,
    val targetPower: Int? = null,
    val targetResistance: Int? = null,
    val workoutMode: Int? = null,
    val verticalGain: Float? = null,
    val verticalNet: Float? = null,
    val averageWatts: Int? = null,
    val averageIncline: Float? = null,
    val lapTime: Long? = null,
    val pausedTime: Long? = null,
    val startRequested: Boolean? = null,
    val goalTime: Long? = null,
    val strokeCount: Int? = null,
    val strokeRate: Int? = null,
    val splitTime: Int? = null,
    val avgSplitTime: Int? = null,
) {
    companion object {
        /**
         * All-zero seed sample. Broadcast adapters use this so their FTMS / CPS / Training-Status
         * notification streams are alive (steady all-zero frames) even before a workout has started,
         * which is what real FTMS hardware does and what clients like Zwift expect.
         */
        val ZERO = ExerciseData(
            power = 0,
            cadence = 0,
            speed = 0f,
            resistance = null,
            incline = null,
            heartRate = null,
            distance = null,
            calories = null,
            elapsedTime = 0L,
            workoutMode = null,
        )
    }
}
