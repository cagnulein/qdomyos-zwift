package com.nettarion.hyperborea.hardware.fitpro.session

import com.nettarion.hyperborea.core.model.ExerciseData

class ExerciseDataAccumulator(
    private val clock: () -> Long = System::currentTimeMillis,
    initialElapsedSeconds: Long = 0L,
) {
    private var power: Int? = null
    private var cadence: Int? = null
    private var speed: Float? = null
    private var resistance: Int? = null
    private var incline: Float? = null
    private var heartRate: Int? = null
    private var distance: Float? = null
    private var calories: Int? = null
    private var targetSpeed: Float? = null
    private var targetIncline: Float? = null
    private var targetPower: Int? = null
    private var targetResistance: Int? = null
    private var workoutMode: Int? = null
    private var verticalGain: Float? = null
    private var verticalNet: Float? = null
    private var averageWatts: Int? = null
    private var averageIncline: Float? = null
    private var lapTime: Long? = null
    private var pausedTime: Long? = null
    private var startRequested: Boolean? = null
    private var goalTime: Long? = null
    private var strokeCount: Int? = null
    private var strokeRate: Int? = null
    private var splitTime: Int? = null
    private var avgSplitTime: Int? = null

    // Elapsed time tracking — own clock, pausable
    private var accumulatedSeconds: Long = initialElapsedSeconds
    private var runningStartTime: Long = 0L
    private var paused: Boolean = false

    fun start() {
        // Clock starts lazily on first non-zero cadence
    }

    fun startTimer() {
        if (runningStartTime == 0L && !paused) runningStartTime = clock()
    }

    fun pause() {
        if (!paused && runningStartTime > 0L) {
            accumulatedSeconds += (clock() - runningStartTime) / 1000L
            runningStartTime = 0L
            paused = true
        }
    }

    fun resume() {
        if (paused) {
            runningStartTime = clock()
            paused = false
        }
    }

    fun updatePower(value: Int) { power = value }
    fun updateCadence(value: Int) {
        cadence = value
        if (value > 0 && runningStartTime == 0L && !paused) runningStartTime = clock()
    }
    fun updateSpeed(value: Float) { speed = value }
    fun updateResistance(value: Int) { resistance = value }
    fun updateIncline(value: Float) { incline = value }
    /** A `null` value clears the reading — used when a grip sensor loses contact. */
    fun updateHeartRate(value: Int?) { heartRate = value }
    fun updateDistance(value: Float) { distance = value }
    fun updateCalories(value: Int) { calories = value }
    fun updateElapsedTime(seconds: Long) { /* ignored — we track our own clock */ }
    fun updateTargetSpeed(value: Float) { targetSpeed = value }
    fun updateTargetIncline(value: Float) { targetIncline = value }
    fun updateTargetPower(value: Int) { targetPower = value }
    fun updateTargetResistance(value: Int) { targetResistance = value }
    fun updateWorkoutMode(value: Int) { workoutMode = value }
    fun updateVerticalGain(meters: Float) { verticalGain = meters }
    fun updateVerticalNet(meters: Float) { verticalNet = meters }
    fun updateAverageWatts(watts: Int) { averageWatts = watts }
    fun updateAverageIncline(percent: Float) { averageIncline = percent }
    fun updateLapTime(seconds: Long) { lapTime = seconds }
    fun updatePausedTime(seconds: Long) { pausedTime = seconds }
    fun updateStartRequested(requested: Boolean) { startRequested = requested }
    fun updateGoalTime(seconds: Long) { goalTime = seconds }
    fun updateStrokeCount(value: Int) { strokeCount = value }
    fun updateStrokeRate(value: Int) { strokeRate = value }
    fun updateSplitTime(value: Int) { splitTime = value }
    fun updateAvgSplitTime(value: Int) { avgSplitTime = value }

    private fun elapsedSeconds(): Long {
        val running = if (runningStartTime > 0L) (clock() - runningStartTime) / 1000L else 0L
        return accumulatedSeconds + running
    }

    fun snapshot(): ExerciseData = ExerciseData(
        power = power,
        cadence = cadence,
        speed = speed,
        resistance = resistance,
        incline = incline,
        heartRate = heartRate,
        distance = distance,
        calories = calories,
        elapsedTime = elapsedSeconds(),
        targetSpeed = targetSpeed,
        targetIncline = targetIncline,
        targetPower = targetPower,
        targetResistance = targetResistance,
        workoutMode = workoutMode,
        verticalGain = verticalGain,
        verticalNet = verticalNet,
        averageWatts = averageWatts,
        averageIncline = averageIncline,
        lapTime = lapTime,
        pausedTime = pausedTime,
        startRequested = startRequested,
        goalTime = goalTime,
        strokeCount = strokeCount,
        strokeRate = strokeRate,
        splitTime = splitTime,
        avgSplitTime = avgSplitTime,
    )

    fun reset() {
        power = null
        cadence = null
        speed = null
        resistance = null
        incline = null
        heartRate = null
        distance = null
        calories = null
        accumulatedSeconds = 0L
        runningStartTime = 0L
        paused = false
        targetSpeed = null
        targetIncline = null
        targetPower = null
        targetResistance = null
        workoutMode = null
        verticalGain = null
        verticalNet = null
        averageWatts = null
        averageIncline = null
        lapTime = null
        pausedTime = null
        startRequested = null
        goalTime = null
        strokeCount = null
        strokeRate = null
        splitTime = null
        avgSplitTime = null
    }
}
