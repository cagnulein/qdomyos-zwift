package org.cagnulen.qdomyoszwift

import androidx.health.connect.client.records.CyclingPedalingCadenceRecord
import androidx.health.connect.client.records.DistanceRecord
import androidx.health.connect.client.records.ElevationGainedRecord
import androidx.health.connect.client.records.ExerciseSessionRecord
import androidx.health.connect.client.records.HeartRateRecord
import androidx.health.connect.client.records.HeartRateVariabilityRmssdRecord
import androidx.health.connect.client.records.PowerRecord
import androidx.health.connect.client.records.SpeedRecord
import androidx.health.connect.client.records.StepsCadenceRecord
import androidx.health.connect.client.records.StepsRecord
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

    // ── buildRecords: active time only ───────────────────────────────────────

    private val T0 = 1_700_000_000_000L
    private val MIN = 60_000L

    private fun sessionOf(records: List<androidx.health.connect.client.records.Record>) =
        records.first() as ExerciseSessionRecord

    // A walk recorded from the moment the treadmill connected: two idle minutes, three minutes
    // of walking, two idle minutes after Stop
    private fun walkWithIdleTime() = JSONArray().apply {
        put(makeSample(T0, heart = 70))
        put(makeSample(T0 + MIN, heart = 71))
        put(makeSample(T0 + 2 * MIN, heart = 72, speed = 3.0, elapsedTime = 1L, distance = 0.01, calories = 1.0))
        put(makeSample(T0 + 3 * MIN, heart = 95, speed = 3.5, elapsedTime = 61L, distance = 0.06, calories = 6.0))
        put(makeSample(T0 + 4 * MIN, heart = 100, speed = 3.5, elapsedTime = 121L, distance = 0.12, calories = 12.0))
        put(makeSample(T0 + 5 * MIN, heart = 90, elapsedTime = 121L, distance = 0.12, calories = 12.0))
        put(makeSample(T0 + 6 * MIN, heart = 80, elapsedTime = 121L, distance = 0.12, calories = 12.0))
    }

    @Test
    fun `treadmill session starts at the first sample with speed`() {
        val session = sessionOf(HealthConnectHelper.buildRecords(walkWithIdleTime(), "t", TREADMILL, null))
        assertEquals(Instant.ofEpochMilli(T0 + 2 * MIN), session.startTime)
    }

    @Test
    fun `treadmill session ends at the sample where the belt stopped`() {
        val session = sessionOf(HealthConnectHelper.buildRecords(walkWithIdleTime(), "t", TREADMILL, null))
        assertEquals(Instant.ofEpochMilli(T0 + 5 * MIN), session.endTime)
    }

    @Test
    fun `trimmed session keeps the totals of the last sample`() {
        val records = HealthConnectHelper.buildRecords(walkWithIdleTime(), "t", TREADMILL, null)
        assertEquals(0.12, records.filterIsInstance<DistanceRecord>().single().distance.inKilometers, 1e-9)
        assertEquals(12.0, records.filterIsInstance<TotalCaloriesBurnedRecord>().single().energy.inKilocalories, 1e-9)
    }

    @Test
    fun `heart rate is written only for the active time`() {
        val records = HealthConnectHelper.buildRecords(walkWithIdleTime(), "t", TREADMILL, null)
        val heart = records.filterIsInstance<HeartRateRecord>().single()
        assertEquals(listOf(72L, 95L, 100L, 90L), heart.samples.map { it.beatsPerMinute })
    }

    @Test
    fun `elapsed time running at zero speed does not start the session (continuous moving)`() {
        val samples = JSONArray().apply {
            put(makeSample(T0, elapsedTime = 1L))
            put(makeSample(T0 + MIN, elapsedTime = 60L))
            put(makeSample(T0 + 2 * MIN, speed = 3.0, elapsedTime = 120L, distance = 0.05))
            put(makeSample(T0 + 3 * MIN, speed = 3.0, elapsedTime = 180L, distance = 0.1))
            put(makeSample(T0 + 4 * MIN, elapsedTime = 240L, distance = 0.1))
            put(makeSample(T0 + 5 * MIN, elapsedTime = 300L, distance = 0.1))
        }
        val session = sessionOf(HealthConnectHelper.buildRecords(samples, "t", TREADMILL, null))
        assertEquals(Instant.ofEpochMilli(T0 + 2 * MIN), session.startTime)
        assertEquals(Instant.ofEpochMilli(T0 + 4 * MIN), session.endTime)
    }

    @Test
    fun `whole session kept, last samples included, when nothing ever moves`() {
        val samples = JSONArray().apply {
            put(makeSample(T0, heart = 70))
            put(makeSample(T0 + MIN, heart = 71, distance = 0.5))
            put(makeSample(T0 + 2 * MIN, heart = 72, distance = 0.5))
            put(makeSample(T0 + 3 * MIN, heart = 73, distance = 0.5))
        }
        for (type in listOf(TREADMILL, BIKE, ROWING, ELLIPTICAL)) {
            val records = HealthConnectHelper.buildRecords(samples, "t", type, null)
            val session = sessionOf(records)
            assertEquals(Instant.ofEpochMilli(T0), session.startTime)
            assertEquals(Instant.ofEpochMilli(T0 + 3 * MIN), session.endTime)
            assertEquals(4, records.filterIsInstance<HeartRateRecord>().single().samples.size)
        }
    }

    @Test
    fun `other equipment keeps the whole session`() {
        val samples = JSONArray().apply {
            put(makeSample(T0))
            put(makeSample(T0 + MIN, speed = 2.0, cadence = 60.0))
            put(makeSample(T0 + 2 * MIN))
        }
        for (type in listOf(UNKNOWN, JUMPROPE, STAIRCLIMBER)) {
            val session = sessionOf(HealthConnectHelper.buildRecords(samples, "t", type, null))
            assertEquals(Instant.ofEpochMilli(T0), session.startTime)
            assertEquals(Instant.ofEpochMilli(T0 + 2 * MIN), session.endTime)
        }
    }

    @Test
    fun `bike session follows the cadence, with its power and cadence samples`() {
        val samples = JSONArray().apply {
            put(makeSample(T0, heart = 70))
            put(makeSample(T0 + MIN, cadence = 80.0, watt = 150.0, distance = 0.5))
            put(makeSample(T0 + 2 * MIN, cadence = 85.0, watt = 160.0, distance = 1.0))
            put(makeSample(T0 + 3 * MIN, distance = 1.0))
            put(makeSample(T0 + 4 * MIN, distance = 1.0))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        val session = sessionOf(records)
        assertEquals(Instant.ofEpochMilli(T0 + MIN), session.startTime)
        assertEquals(Instant.ofEpochMilli(T0 + 3 * MIN), session.endTime)
        assertEquals(listOf(150.0, 160.0), records.filterIsInstance<PowerRecord>().single().samples.map { it.power.inWatts })
        assertEquals(listOf(80.0, 85.0),
            records.filterIsInstance<CyclingPedalingCadenceRecord>().single().samples.map { it.revolutionsPerMinute })
        assertEquals(1.0, records.filterIsInstance<DistanceRecord>().single().distance.inKilometers, 1e-9)
    }

    @Test
    fun `bike with a power sensor but no cadence keeps the whole session`() {
        val samples = JSONArray().apply {
            put(makeSample(T0))
            put(makeSample(T0 + MIN, watt = 150.0))
            put(makeSample(T0 + 2 * MIN, watt = 160.0))
            put(makeSample(T0 + 3 * MIN))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        val session = sessionOf(records)
        assertEquals(Instant.ofEpochMilli(T0), session.startTime)
        assertEquals(Instant.ofEpochMilli(T0 + 3 * MIN), session.endTime)
        assertEquals(2, records.filterIsInstance<PowerRecord>().single().samples.size)
    }

    @Test
    fun `treadmill with an external power sensor keeps its power for the active time`() {
        val samples = JSONArray().apply {
            put(makeSample(T0))
            put(makeSample(T0 + MIN, speed = 8.0, watt = 210.0, distance = 0.1))
            put(makeSample(T0 + 2 * MIN, speed = 8.5, watt = 230.0, distance = 0.3))
            put(makeSample(T0 + 3 * MIN, distance = 0.3))
            put(makeSample(T0 + 4 * MIN, distance = 0.3))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", TREADMILL, null)
        val session = sessionOf(records)
        assertEquals(Instant.ofEpochMilli(T0 + MIN), session.startTime)
        assertEquals(Instant.ofEpochMilli(T0 + 3 * MIN), session.endTime)
        assertEquals(listOf(210.0, 230.0), records.filterIsInstance<PowerRecord>().single().samples.map { it.power.inWatts })
    }

    @Test
    fun `rower session follows the stroke cadence`() {
        val samples = JSONArray().apply {
            put(makeSample(T0))
            put(makeSample(T0 + MIN, cadence = 24.0, watt = 120.0, distance = 0.2))
            put(makeSample(T0 + 2 * MIN, cadence = 26.0, watt = 130.0, distance = 0.5))
            put(makeSample(T0 + 3 * MIN, distance = 0.5))
            put(makeSample(T0 + 4 * MIN, distance = 0.5))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", ROWING, null)
        val session = sessionOf(records)
        assertEquals(Instant.ofEpochMilli(T0 + MIN), session.startTime)
        assertEquals(Instant.ofEpochMilli(T0 + 3 * MIN), session.endTime)
        assertEquals(0.5, records.filterIsInstance<DistanceRecord>().single().distance.inKilometers, 1e-9)
    }

    @Test
    fun `elliptical session follows the speed`() {
        val samples = JSONArray().apply {
            put(makeSample(T0))
            put(makeSample(T0 + MIN, speed = 6.0, cadence = 50.0))
            put(makeSample(T0 + 2 * MIN))
            put(makeSample(T0 + 3 * MIN))
        }
        val session = sessionOf(HealthConnectHelper.buildRecords(samples, "t", ELLIPTICAL, null))
        assertEquals(Instant.ofEpochMilli(T0 + MIN), session.startTime)
        assertEquals(Instant.ofEpochMilli(T0 + 2 * MIN), session.endTime)
    }

    @Test
    fun `a pause in the middle stays inside the session`() {
        val samples = JSONArray().apply {
            put(makeSample(T0, heart = 70))
            put(makeSample(T0 + MIN, speed = 3.0, heart = 90, distance = 0.05))
            put(makeSample(T0 + 2 * MIN, heart = 85, distance = 0.05))
            put(makeSample(T0 + 3 * MIN, heart = 84, distance = 0.05))
            put(makeSample(T0 + 4 * MIN, speed = 3.0, heart = 95, distance = 0.1))
            put(makeSample(T0 + 5 * MIN, heart = 88, distance = 0.1))
            put(makeSample(T0 + 6 * MIN, heart = 75, distance = 0.1))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", TREADMILL, null)
        val session = sessionOf(records)
        assertEquals(Instant.ofEpochMilli(T0 + MIN), session.startTime)
        assertEquals(Instant.ofEpochMilli(T0 + 5 * MIN), session.endTime)
        assertEquals(listOf(90L, 85L, 84L, 95L, 88L),
            records.filterIsInstance<HeartRateRecord>().single().samples.map { it.beatsPerMinute })
    }

    @Test
    fun `bike pause in the middle stays inside the session`() {
        val samples = JSONArray().apply {
            put(makeSample(T0, cadence = 80.0))
            put(makeSample(T0 + MIN))
            put(makeSample(T0 + 2 * MIN, cadence = 82.0))
            put(makeSample(T0 + 3 * MIN))
        }
        val session = sessionOf(HealthConnectHelper.buildRecords(samples, "t", BIKE, null))
        assertEquals(Instant.ofEpochMilli(T0), session.startTime)
        assertEquals(Instant.ofEpochMilli(T0 + 3 * MIN), session.endTime)
    }

    @Test
    fun `bike that loses cadence halfway keeps the session while power and distance go on`() {
        val samples = JSONArray().apply {
            put(makeSample(T0))
            put(makeSample(T0 + MIN, cadence = 80.0, watt = 150.0, distance = 0.5))
            put(makeSample(T0 + 2 * MIN, watt = 155.0, distance = 1.0)) // cadence sensor lost
            put(makeSample(T0 + 3 * MIN, watt = 150.0, distance = 1.5))
            put(makeSample(T0 + 4 * MIN, distance = 1.5)) // stopped
            put(makeSample(T0 + 5 * MIN, distance = 1.5))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        val session = sessionOf(records)
        assertEquals(Instant.ofEpochMilli(T0 + MIN), session.startTime)
        assertEquals(Instant.ofEpochMilli(T0 + 4 * MIN), session.endTime)
        assertEquals(listOf(150.0, 155.0, 150.0), records.filterIsInstance<PowerRecord>().single().samples.map { it.power.inWatts })
        assertEquals(1.5, records.filterIsInstance<DistanceRecord>().single().distance.inKilometers, 1e-9)
    }

    @Test
    fun `bike that loses cadence and power keeps the session while the distance grows`() {
        val samples = JSONArray().apply {
            put(makeSample(T0, cadence = 80.0, distance = 0.1))
            put(makeSample(T0 + MIN, distance = 0.6))
            put(makeSample(T0 + 2 * MIN, distance = 1.1))
            put(makeSample(T0 + 3 * MIN, distance = 1.1))
            put(makeSample(T0 + 4 * MIN, distance = 1.1))
        }
        val session = sessionOf(HealthConnectHelper.buildRecords(samples, "t", BIKE, null))
        assertEquals(Instant.ofEpochMilli(T0), session.startTime)
        assertEquals(Instant.ofEpochMilli(T0 + 3 * MIN), session.endTime)
    }

    @Test
    fun `treadmill that loses speed halfway keeps the session while the distance grows`() {
        val samples = JSONArray().apply {
            put(makeSample(T0))
            put(makeSample(T0 + MIN, speed = 5.0, distance = 0.1))
            put(makeSample(T0 + 2 * MIN, distance = 0.2)) // speed not reported
            put(makeSample(T0 + 3 * MIN, distance = 0.3))
            put(makeSample(T0 + 4 * MIN, distance = 0.3))
            put(makeSample(T0 + 5 * MIN, distance = 0.3))
        }
        val session = sessionOf(HealthConnectHelper.buildRecords(samples, "t", TREADMILL, null))
        assertEquals(Instant.ofEpochMilli(T0 + MIN), session.startTime)
        assertEquals(Instant.ofEpochMilli(T0 + 4 * MIN), session.endTime)
    }

    @Test
    fun `calories still counting after Stop do not extend the session`() {
        val samples = JSONArray().apply {
            put(makeSample(T0, speed = 3.0, distance = 0.1, calories = 5.0))
            put(makeSample(T0 + MIN, distance = 0.1, calories = 6.0)) // heart-rate calories at rest
            put(makeSample(T0 + 2 * MIN, distance = 0.1, calories = 7.0))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", TREADMILL, null)
        assertEquals(Instant.ofEpochMilli(T0 + MIN), sessionOf(records).endTime)
        assertEquals(7.0, records.filterIsInstance<TotalCaloriesBurnedRecord>().single().energy.inKilocalories, 1e-9)
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

    // ── buildRecords: elevation ───────────────────────────────────────────────

    @Test
    fun `buildRecords includes ElevationGainedRecord when last sample has elevationGain greater than 0`() {
        val samples = twoSamples(1_000L, 2_000L, elevationGain = 150.0)
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertTrue(records.any { it is ElevationGainedRecord })
    }

    @Test
    fun `buildRecords omits ElevationGainedRecord when elevationGain is 0`() {
        val samples = twoSamples(1_000L, 2_000L)
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertFalse(records.any { it is ElevationGainedRecord })
    }

    // ── buildRecords: steps ───────────────────────────────────────────────────

    @Test
    fun `buildRecords includes StepsRecord when last sample has stepCount greater than 0`() {
        val samples = twoSamples(1_000L, 2_000L, stepCount = 500.0)
        val records = HealthConnectHelper.buildRecords(samples, "t", TREADMILL, null)
        assertTrue(records.any { it is StepsRecord })
    }

    @Test
    fun `buildRecords omits StepsRecord when stepCount is 0`() {
        val samples = twoSamples(1_000L, 2_000L)
        val records = HealthConnectHelper.buildRecords(samples, "t", TREADMILL, null)
        assertFalse(records.any { it is StepsRecord })
    }

    // ── buildRecords: steps cadence vs cycling cadence ────────────────────────

    @Test
    fun `buildRecords uses StepsCadenceRecord for treadmill cadence`() {
        val samples = JSONArray().apply {
            put(makeSample(1_000L, cadence = 160.0))
            put(makeSample(2_000L, cadence = 165.0))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", TREADMILL, null)
        assertTrue(records.any { it is StepsCadenceRecord })
        assertFalse(records.any { it is CyclingPedalingCadenceRecord })
    }

    @Test
    fun `buildRecords uses CyclingPedalingCadenceRecord for bike cadence`() {
        val samples = JSONArray().apply {
            put(makeSample(1_000L, cadence = 80.0))
            put(makeSample(2_000L, cadence = 85.0))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertTrue(records.any { it is CyclingPedalingCadenceRecord })
        assertFalse(records.any { it is StepsCadenceRecord })
    }

    // ── buildRecords: HRV ─────────────────────────────────────────────────────

    @Test
    fun `buildRecords includes HeartRateVariabilityRmssdRecord when hrv greater than 0`() {
        val samples = JSONArray().apply {
            put(makeSample(1_000L, hrv = 42.5))
            put(makeSample(2_000L, hrv = 38.0))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        val hrvRecords = records.filterIsInstance<HeartRateVariabilityRmssdRecord>()
        assertEquals(2, hrvRecords.size)
        assertEquals(42.5, hrvRecords[0].heartRateVariabilityMillis, 0.001)
    }

    @Test
    fun `buildRecords omits HeartRateVariabilityRmssdRecord when hrv is below health connect minimum`() {
        val samples = JSONArray().apply {
            put(makeSample(1_000L, hrv = 0.9067155184426361))
            put(makeSample(2_000L, hrv = 1.0))
        }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        val hrvRecords = records.filterIsInstance<HeartRateVariabilityRmssdRecord>()
        assertEquals(1, hrvRecords.size)
        assertEquals(1.0, hrvRecords[0].heartRateVariabilityMillis, 0.001)
    }

    @Test
    fun `buildRecords omits HeartRateVariabilityRmssdRecord when hrv is 0`() {
        val samples = JSONArray().apply { put(makeSample(1_000L)) }
        val records = HealthConnectHelper.buildRecords(samples, "t", BIKE, null)
        assertFalse(records.any { it is HeartRateVariabilityRmssdRecord })
    }

    // ── helpers ───────────────────────────────────────────────────────────────

    private fun makeSample(
        time: Long,
        speed: Double = 0.0,
        watt: Double = 0.0,
        cadence: Double = 0.0,
        heart: Int = 0,
        distance: Double = 0.0,
        calories: Double = 0.0,
        elevationGain: Double = 0.0,
        stepCount: Double = 0.0,
        hrv: Double = 0.0,
        elapsedTime: Long = 0L
    ) = JSONObject().apply {
        put("time", time)
        put("speed", speed)
        put("watt", watt)
        put("cadence", cadence)
        put("heart", heart)
        put("distance", distance)
        put("calories", calories)
        put("elevationGain", elevationGain)
        put("stepCount", stepCount)
        put("hrv", hrv)
        put("elapsedTime", elapsedTime)
    }

    private fun singleSample(time: Long = System.currentTimeMillis()) =
        JSONArray().apply { put(makeSample(time, watt = 100.0, cadence = 80.0, speed = 25.0)) }

    private fun twoSamples(
        startMs: Long,
        endMs: Long,
        distance: Double = 0.0,
        calories: Double = 0.0,
        elevationGain: Double = 0.0,
        stepCount: Double = 0.0
    ) = JSONArray().apply {
        put(makeSample(startMs, speed = 25.0, watt = 150.0, cadence = 80.0))
        put(makeSample(endMs, speed = 28.0, watt = 180.0, cadence = 85.0,
                       distance = distance, calories = calories,
                       elevationGain = elevationGain, stepCount = stepCount))
    }
}
