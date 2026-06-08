package org.cagnulen.qdomyoszwift

import androidx.health.connect.client.records.CyclingPedalingCadenceRecord
import androidx.health.connect.client.records.DistanceRecord
import androidx.health.connect.client.records.ExerciseSessionRecord
import androidx.health.connect.client.records.HeartRateRecord
import androidx.health.connect.client.records.PowerRecord
import androidx.health.connect.client.records.SpeedRecord
import androidx.health.connect.client.records.TotalCaloriesBurnedRecord
import org.json.JSONArray
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.time.Instant

class HealthConnectHelperTest {

    // Device type constants matching bluetoothdevicetype.h
    private val UNKNOWN = 0
    private val TREADMILL = 1
    private val BIKE = 2
    private val ROWING = 3
    private val ELLIPTICAL = 4
    private val JUMPROPE = 5
    private val STAIRCLIMBER = 6

    // ── exerciseType mapping ─────────────────────────────────────────────────

    @Test
    fun `exerciseType maps TREADMILL to RUNNING_TREADMILL`() {
        assertEquals(
            ExerciseSessionRecord.EXERCISE_TYPE_RUNNING_TREADMILL,
            HealthConnectHelper.exerciseType(TREADMILL)
        )
    }

    @Test
    fun `exerciseType maps BIKE to BIKING_STATIONARY`() {
        assertEquals(
            ExerciseSessionRecord.EXERCISE_TYPE_BIKING_STATIONARY,
            HealthConnectHelper.exerciseType(BIKE)
        )
    }

    @Test
    fun `exerciseType maps ROWING to ROWING_MACHINE`() {
        assertEquals(
            ExerciseSessionRecord.EXERCISE_TYPE_ROWING_MACHINE,
            HealthConnectHelper.exerciseType(ROWING)
        )
    }

    @Test
    fun `exerciseType maps ELLIPTICAL to ELLIPTICAL`() {
        assertEquals(
            ExerciseSessionRecord.EXERCISE_TYPE_ELLIPTICAL,
            HealthConnectHelper.exerciseType(ELLIPTICAL)
        )
    }

    @Test
    fun `exerciseType maps STAIRCLIMBER to STAIR_CLIMBING_MACHINE`() {
        assertEquals(
            ExerciseSessionRecord.EXERCISE_TYPE_STAIR_CLIMBING_MACHINE,
            HealthConnectHelper.exerciseType(STAIRCLIMBER)
        )
    }

    @Test
    fun `exerciseType maps UNKNOWN to OTHER_WORKOUT`() {
        assertEquals(
            ExerciseSessionRecord.EXERCISE_TYPE_OTHER_WORKOUT,
            HealthConnectHelper.exerciseType(UNKNOWN)
        )
    }

    // ── buildRecords: session record ─────────────────────────────────────────

    @Test
    fun `buildRecords always includes ExerciseSessionRecord as first record`() {
        val records = HealthConnectHelper.buildRecords(singleSample(), "test", BIKE, "TestBike")
        assertTrue(records.first() is ExerciseSessionRecord)
    }

    @Test
    fun `buildRecords sets correct exercise type on session`() {
        val records = HealthConnectHelper.buildRecords(singleSample(), "test", BIKE, "TestBike")
        val session = records.first() as ExerciseSessionRecord
        assertEquals(ExerciseSessionRecord.EXERCISE_TYPE_BIKING_STATIONARY, session.exerciseType)
    }

    @Test
    fun `buildRecords uses provided title on session`() {
        val records = HealthConnectHelper.buildRecords(singleSample(), "My Ride", BIKE, "TestBike")
        val session = records.first() as ExerciseSessionRecord
        assertEquals("My Ride", session.title)
    }

    @Test
    fun `buildRecords uses default title when blank`() {
        val records = HealthConnectHelper.buildRecords(singleSample(), "", BIKE, "TestBike")
        val session = records.first() as ExerciseSessionRecord
        assertEquals("QZ workout", session.title)
    }

    @Test
    fun `buildRecords sets device name as notes on session`() {
        val records = HealthConnectHelper.buildRecords(singleSample(), "title", BIKE, "MyBike")
        val session = records.first() as ExerciseSessionRecord
        assertEquals("MyBike", session.notes)
    }

    // ── buildRecords: end time guard ─────────────────────────────────────────

    @Test
    fun `buildRecords bumps end 1 second when start equals end`() {
        val t = System.currentTimeMillis()
        val samples = JSONArray().apply { put(makeSample(t, watt = 100.0)) }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        val session = records.first() as ExerciseSessionRecord
        assertTrue(session.endTime.isAfter(session.startTime))
        assertEquals(1L, session.endTime.epochSecond - session.startTime.epochSecond)
    }

    @Test
    fun `buildRecords uses correct start and end from samples`() {
        val start = 1_700_000_000_000L
        val end = 1_700_000_060_000L
        val samples = twoSamples(start, end)
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        val session = records.first() as ExerciseSessionRecord
        assertEquals(Instant.ofEpochMilli(start), session.startTime)
        assertEquals(Instant.ofEpochMilli(end), session.endTime)
    }

    // ── buildRecords: power ───────────────────────────────────────────────────

    @Test
    fun `buildRecords includes PowerRecord when watt greater than 0`() {
        val samples = JSONArray().apply {
            put(makeSample(1_000L, watt = 150.0))
            put(makeSample(2_000L, watt = 200.0))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertTrue(records.any { it is PowerRecord })
    }

    @Test
    fun `buildRecords PowerRecord has correct number of samples`() {
        val samples = JSONArray().apply {
            put(makeSample(1_000L, watt = 150.0))
            put(makeSample(2_000L, watt = 200.0))
            put(makeSample(3_000L, watt = 0.0))  // zero — should be skipped
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        val powerRecord = records.filterIsInstance<PowerRecord>().first()
        assertEquals(2, powerRecord.samples.size)
    }

    @Test
    fun `buildRecords omits PowerRecord when all watts are 0`() {
        val samples = JSONArray().apply {
            put(makeSample(1_000L, watt = 0.0))
            put(makeSample(2_000L, watt = 0.0))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertFalse(records.any { it is PowerRecord })
    }

    // ── buildRecords: cadence ─────────────────────────────────────────────────

    @Test
    fun `buildRecords includes CyclingPedalingCadenceRecord when cadence greater than 0`() {
        val samples = JSONArray().apply {
            put(makeSample(1_000L, cadence = 80.0))
            put(makeSample(2_000L, cadence = 85.0))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertTrue(records.any { it is CyclingPedalingCadenceRecord })
    }

    @Test
    fun `buildRecords CyclingPedalingCadenceRecord has correct RPM values`() {
        val samples = JSONArray().apply {
            put(makeSample(1_000L, cadence = 90.0))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        val cadenceRecord = records.filterIsInstance<CyclingPedalingCadenceRecord>().first()
        assertEquals(90.0, cadenceRecord.samples.first().revolutionsPerMinute, 0.001)
    }

    @Test
    fun `buildRecords omits CyclingPedalingCadenceRecord when all cadence values are 0`() {
        val samples = JSONArray().apply {
            put(makeSample(1_000L, cadence = 0.0))
            put(makeSample(2_000L, cadence = 0.0))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertFalse(records.any { it is CyclingPedalingCadenceRecord })
    }

    // ── buildRecords: heart rate ──────────────────────────────────────────────

    @Test
    fun `buildRecords includes HeartRateRecord when heart greater than 0`() {
        val samples = JSONArray().apply {
            put(makeSample(1_000L, heart = 140))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertTrue(records.any { it is HeartRateRecord })
    }

    @Test
    fun `buildRecords omits HeartRateRecord when all heart values are 0`() {
        val samples = JSONArray().apply { put(makeSample(1_000L, heart = 0)) }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertFalse(records.any { it is HeartRateRecord })
    }

    // ── buildRecords: speed ───────────────────────────────────────────────────

    @Test
    fun `buildRecords includes SpeedRecord when speed greater than 0`() {
        val samples = JSONArray().apply {
            put(makeSample(1_000L, speed = 30.0))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertTrue(records.any { it is SpeedRecord })
    }

    // ── buildRecords: distance and calories ───────────────────────────────────

    @Test
    fun `buildRecords includes DistanceRecord when last sample has distance greater than 0`() {
        val samples = twoSamples(1_000L, 2_000L, distance = 5.0)
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertTrue(records.any { it is DistanceRecord })
    }

    @Test
    fun `buildRecords includes TotalCaloriesBurnedRecord when last sample has calories greater than 0`() {
        val samples = twoSamples(1_000L, 2_000L, calories = 300.0)
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertTrue(records.any { it is TotalCaloriesBurnedRecord })
    }

    @Test
    fun `buildRecords omits DistanceRecord when distance is 0`() {
        val samples = twoSamples(1_000L, 2_000L, distance = 0.0)
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertFalse(records.any { it is DistanceRecord })
    }

    // ── helpers ───────────────────────────────────────────────────────────────

    private fun makeSample(
        time: Long,
        speed: Double = 0.0,
        watt: Double = 0.0,
        cadence: Double = 0.0,
        heart: Int = 0,
        distance: Double = 0.0,
        calories: Double = 0.0
    ) = JSONObject().apply {
        put("time", time)
        put("speed", speed)
        put("watt", watt)
        put("cadence", cadence)
        put("heart", heart)
        put("distance", distance)
        put("calories", calories)
    }

    private fun singleSample(time: Long = System.currentTimeMillis()) =
        JSONArray().apply { put(makeSample(time, watt = 100.0, cadence = 80.0, speed = 25.0)) }

    private fun twoSamples(
        startMs: Long,
        endMs: Long,
        distance: Double = 0.0,
        calories: Double = 0.0
    ) = JSONArray().apply {
        put(makeSample(startMs, speed = 25.0, watt = 150.0, cadence = 80.0))
        put(makeSample(endMs, speed = 28.0, watt = 180.0, cadence = 85.0,
                       distance = distance, calories = calories))
    }
}
