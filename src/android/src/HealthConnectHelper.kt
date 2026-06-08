package org.cagnulen.qdomyoszwift

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.os.Build
import androidx.health.connect.client.HealthConnectClient
import androidx.health.connect.client.permission.HealthPermission
import androidx.health.connect.client.records.CyclingPedalingCadenceRecord
import androidx.health.connect.client.records.DistanceRecord
import androidx.health.connect.client.records.ElevationGainedRecord
import androidx.health.connect.client.records.ExerciseSessionRecord
import androidx.health.connect.client.records.HeartRateRecord
import androidx.health.connect.client.records.HeartRateVariabilityRmssdRecord
import androidx.health.connect.client.records.PowerRecord
import androidx.health.connect.client.records.Record
import androidx.health.connect.client.records.SpeedRecord
import androidx.health.connect.client.records.StepsCadenceRecord
import androidx.health.connect.client.records.StepsRecord
import androidx.health.connect.client.records.TotalCaloriesBurnedRecord
import androidx.health.connect.client.records.metadata.Metadata
import androidx.health.connect.client.units.Energy
import androidx.health.connect.client.units.Length
import androidx.health.connect.client.units.Power
import androidx.health.connect.client.units.Velocity
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import org.json.JSONArray
import java.time.Instant
import java.time.ZoneId

class HealthConnectHelper {
    companion object {
        private const val TAG = "HealthConnectHelper"
        private const val MIN_SUPPORTED_SDK = 26
        private const val HEALTH_CONNECT_PROVIDER = "com.google.android.apps.healthdata"
        private const val PREFS_NAME = "qz_health_connect"
        private const val PREF_PERMISSION_PROMPT_PENDING = "permission_prompt_pending_v2"

        private var initialized = false

        @JvmStatic
        fun initialize(context: Context?) {
            if (initialized) {
                return
            }

            initialized = true

            if (context == null) {
                QLog.d(TAG, "Skipping Health Connect init: context is null")
                return
            }

            if (Build.VERSION.SDK_INT < MIN_SUPPORTED_SDK) {
                QLog.d(TAG, "Skipping Health Connect init: unsupported Android API ${Build.VERSION.SDK_INT}")
                return
            }

            try {
                val sdkStatus = HealthConnectClient.getSdkStatus(context.applicationContext, HEALTH_CONNECT_PROVIDER)
                QLog.d(TAG, "Health Connect SDK status: $sdkStatus")
                if (sdkStatus == HealthConnectClient.SDK_AVAILABLE) {
                    HealthConnectClient.getOrCreate(context.applicationContext, HEALTH_CONNECT_PROVIDER)
                    QLog.d(TAG, "Health Connect client initialized")
                    requestPermissionsIfNeeded(context)
                }
            } catch (t: Throwable) {
                QLog.w(TAG, "Health Connect init was skipped", t)
            }
        }

        @JvmStatic
        fun writeWorkoutJson(context: Context?, title: String?, deviceType: Int, deviceName: String?, samplesJson: String?) {
            if (context == null || samplesJson.isNullOrEmpty()) {
                return
            }

            if (Build.VERSION.SDK_INT < MIN_SUPPORTED_SDK) {
                QLog.d(TAG, "Skipping Health Connect write: unsupported Android API ${Build.VERSION.SDK_INT}")
                return
            }

            CoroutineScope(Dispatchers.IO).launch {
                try {
                    val appContext = context.applicationContext
                    if (HealthConnectClient.getSdkStatus(appContext, HEALTH_CONNECT_PROVIDER) != HealthConnectClient.SDK_AVAILABLE) {
                        QLog.d(TAG, "Skipping Health Connect write: SDK is not available")
                        return@launch
                    }

                    val client = HealthConnectClient.getOrCreate(appContext, HEALTH_CONNECT_PROVIDER)
                    val granted = client.permissionController.getGrantedPermissions()

                    val requiredPermission = HealthPermission.getWritePermission(ExerciseSessionRecord::class)
                    if (!granted.contains(requiredPermission)) {
                        QLog.d(TAG, "Skipping Health Connect write: missing required permission WRITE_EXERCISE")
                        requestPermissionsIfNeeded(context)
                        return@launch
                    }

                    val samples = JSONArray(samplesJson)
                    if (samples.length() == 0) {
                        QLog.d(TAG, "Skipping Health Connect write: empty sample list")
                        return@launch
                    }

                    val records = buildRecords(samples, title, deviceType, deviceName, granted)
                    if (records.isEmpty()) {
                        QLog.d(TAG, "Skipping Health Connect write: no records built")
                        return@launch
                    }

                    client.insertRecords(records)
                    QLog.d(TAG, "Health Connect workout written: ${records.size} records")
                } catch (t: Throwable) {
                    QLog.w(TAG, "Health Connect workout write failed", t)
                }
            }
        }

        @JvmStatic
        fun writePermissions(): Set<String> {
            return setOf(
                HealthPermission.getWritePermission(ExerciseSessionRecord::class),
                HealthPermission.getWritePermission(DistanceRecord::class),
                HealthPermission.getWritePermission(TotalCaloriesBurnedRecord::class),
                HealthPermission.getWritePermission(HeartRateRecord::class),
                HealthPermission.getWritePermission(PowerRecord::class),
                HealthPermission.getWritePermission(SpeedRecord::class),
                HealthPermission.getWritePermission(CyclingPedalingCadenceRecord::class),
                HealthPermission.getWritePermission(ElevationGainedRecord::class),
                HealthPermission.getWritePermission(StepsRecord::class),
                HealthPermission.getWritePermission(StepsCadenceRecord::class),
                HealthPermission.getWritePermission(HeartRateVariabilityRmssdRecord::class)
            )
        }

        // Ask for missing permissions every time the app starts (like iOS HealthKit behaviour).
        // Uses a PENDING flag so we never show two dialogs simultaneously.
        private fun requestPermissionsIfNeeded(context: Context) {
            val activity = context as? Activity
            if (activity == null) {
                QLog.d(TAG, "Skipping Health Connect permission request: context is not an Activity")
                return
            }

            val prefs = activity.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
            if (prefs.getBoolean(PREF_PERMISSION_PROMPT_PENDING, false)) {
                QLog.d(TAG, "Skipping Health Connect permission request: prompt already pending")
                return
            }

            CoroutineScope(Dispatchers.IO).launch {
                try {
                    val client = HealthConnectClient.getOrCreate(activity.applicationContext, HEALTH_CONNECT_PROVIDER)
                    val missingPermissions = writePermissions() - client.permissionController.getGrantedPermissions()
                    QLog.d(TAG, "Health Connect missing permissions: $missingPermissions")
                    if (missingPermissions.isEmpty()) {
                        return@launch
                    }

                    prefs.edit().putBoolean(PREF_PERMISSION_PROMPT_PENDING, true).apply()
                    val intent = Intent(activity, HealthConnectPermissionActivity::class.java)
                    intent.putStringArrayListExtra(
                        HealthConnectPermissionActivity.EXTRA_PERMISSIONS,
                        ArrayList(missingPermissions)
                    )

                    activity.runOnUiThread {
                        try {
                            activity.startActivity(intent)
                            QLog.d(TAG, "Health Connect permission request started")
                        } catch (t: Throwable) {
                            prefs.edit().putBoolean(PREF_PERMISSION_PROMPT_PENDING, false).apply()
                            QLog.w(TAG, "Health Connect permission request failed", t)
                        }
                    }
                } catch (t: Throwable) {
                    QLog.w(TAG, "Health Connect permission check failed", t)
                }
            }
        }

        // Visible for testing — grantedPermissions defaults to all permissions so existing tests pass unchanged.
        @JvmStatic
        fun buildRecords(
            samples: JSONArray,
            title: String?,
            deviceType: Int,
            deviceName: String?,
            grantedPermissions: Set<String> = writePermissions()
        ): List<Record> {
            val first = samples.getJSONObject(0)
            val last = samples.getJSONObject(samples.length() - 1)
            val start = Instant.ofEpochMilli(first.getLong("time"))
            var end = Instant.ofEpochMilli(last.getLong("time"))
            if (!end.isAfter(start)) {
                end = start.plusSeconds(1)
            }
            val zoneOffset = ZoneId.systemDefault().rules.getOffset(start)
            val records = mutableListOf<Record>()

            records += ExerciseSessionRecord(
                startTime = start,
                startZoneOffset = zoneOffset,
                endTime = end,
                endZoneOffset = zoneOffset,
                exerciseType = exerciseType(deviceType),
                title = if (title.isNullOrBlank()) "QZ workout" else title,
                notes = deviceName,
                metadata = Metadata(recordingMethod = Metadata.RECORDING_METHOD_ACTIVELY_RECORDED)
            )

            val hasDistance = grantedPermissions.contains(HealthPermission.getWritePermission(DistanceRecord::class))
            val hasCalories = grantedPermissions.contains(HealthPermission.getWritePermission(TotalCaloriesBurnedRecord::class))
            val hasHeartRate = grantedPermissions.contains(HealthPermission.getWritePermission(HeartRateRecord::class))
            val hasPower = grantedPermissions.contains(HealthPermission.getWritePermission(PowerRecord::class))
            val hasSpeed = grantedPermissions.contains(HealthPermission.getWritePermission(SpeedRecord::class))
            val hasCadence = grantedPermissions.contains(HealthPermission.getWritePermission(CyclingPedalingCadenceRecord::class))
            val hasElevation = grantedPermissions.contains(HealthPermission.getWritePermission(ElevationGainedRecord::class))
            val hasSteps = grantedPermissions.contains(HealthPermission.getWritePermission(StepsRecord::class))
            val hasStepsCadence = grantedPermissions.contains(HealthPermission.getWritePermission(StepsCadenceRecord::class))
            val hasHrv = grantedPermissions.contains(HealthPermission.getWritePermission(HeartRateVariabilityRmssdRecord::class))

            if (hasDistance) {
                val totalDistanceKm = last.optDouble("distance", 0.0)
                if (totalDistanceKm > 0.0) {
                    records += DistanceRecord(
                        startTime = start,
                        startZoneOffset = zoneOffset,
                        endTime = end,
                        endZoneOffset = zoneOffset,
                        distance = Length.kilometers(totalDistanceKm)
                    )
                }
            }

            if (hasCalories) {
                val totalCaloriesKcal = last.optDouble("calories", 0.0)
                if (totalCaloriesKcal > 0.0) {
                    records += TotalCaloriesBurnedRecord(
                        startTime = start,
                        startZoneOffset = zoneOffset,
                        endTime = end,
                        endZoneOffset = zoneOffset,
                        energy = Energy.kilocalories(totalCaloriesKcal)
                    )
                }
            }

            if (hasElevation) {
                val elevationGainM = last.optDouble("elevationGain", 0.0)
                if (elevationGainM > 0.0) {
                    records += ElevationGainedRecord(
                        startTime = start,
                        startZoneOffset = zoneOffset,
                        endTime = end,
                        endZoneOffset = zoneOffset,
                        elevation = Length.meters(elevationGainM)
                    )
                }
            }

            if (hasSteps) {
                val stepCount = last.optDouble("stepCount", 0.0).toLong()
                if (stepCount > 0L) {
                    records += StepsRecord(
                        startTime = start,
                        startZoneOffset = zoneOffset,
                        endTime = end,
                        endZoneOffset = zoneOffset,
                        count = stepCount
                    )
                }
            }

            val isTreadmill = deviceType == 1
            val heartSamples = mutableListOf<HeartRateRecord.Sample>()
            val powerSamples = mutableListOf<PowerRecord.Sample>()
            val speedSamples = mutableListOf<SpeedRecord.Sample>()
            val cadenceSamples = mutableListOf<CyclingPedalingCadenceRecord.Sample>()
            val stepsCadenceSamples = mutableListOf<StepsCadenceRecord.Sample>()
            val hrvRecords = mutableListOf<HeartRateVariabilityRmssdRecord>()

            for (i in 0 until samples.length()) {
                val sample = samples.getJSONObject(i)
                val time = Instant.ofEpochMilli(sample.getLong("time"))
                val heart = sample.optInt("heart", 0)
                val watts = sample.optDouble("watt", 0.0)
                val speed = sample.optDouble("speed", 0.0)
                val cadence = sample.optDouble("cadence", 0.0)
                val hrv = sample.optDouble("hrv", 0.0)

                if (hasHeartRate && heart > 0) {
                    heartSamples += HeartRateRecord.Sample(time = time, beatsPerMinute = heart.toLong())
                }
                if (hasPower && watts > 0.0) {
                    powerSamples += PowerRecord.Sample(time = time, power = Power.watts(watts))
                }
                if (hasSpeed && speed > 0.0) {
                    speedSamples += SpeedRecord.Sample(time = time, speed = Velocity.kilometersPerHour(speed))
                }
                if (cadence > 0.0) {
                    if (hasCadence && !isTreadmill) {
                        cadenceSamples += CyclingPedalingCadenceRecord.Sample(time = time, revolutionsPerMinute = cadence)
                    }
                    if (hasStepsCadence && isTreadmill) {
                        stepsCadenceSamples += StepsCadenceRecord.Sample(time = time, rate = cadence)
                    }
                }
                if (hasHrv && hrv > 0.0) {
                    hrvRecords += HeartRateVariabilityRmssdRecord(
                        time = time,
                        zoneOffset = zoneOffset,
                        heartRateVariabilityMillis = hrv
                    )
                }
            }

            if (heartSamples.isNotEmpty()) {
                records += HeartRateRecord(start, zoneOffset, end, zoneOffset, heartSamples)
            }
            if (powerSamples.isNotEmpty()) {
                records += PowerRecord(start, zoneOffset, end, zoneOffset, powerSamples)
            }
            if (speedSamples.isNotEmpty()) {
                records += SpeedRecord(start, zoneOffset, end, zoneOffset, speedSamples)
            }
            if (cadenceSamples.isNotEmpty()) {
                records += CyclingPedalingCadenceRecord(start, zoneOffset, end, zoneOffset, cadenceSamples)
            }
            if (stepsCadenceSamples.isNotEmpty()) {
                records += StepsCadenceRecord(start, zoneOffset, end, zoneOffset, stepsCadenceSamples)
            }
            records += hrvRecords

            return records
        }

        // Visible for testing
        @JvmStatic
        fun exerciseType(deviceType: Int): Int {
            return when (deviceType) {
                1 -> ExerciseSessionRecord.EXERCISE_TYPE_RUNNING_TREADMILL
                2 -> ExerciseSessionRecord.EXERCISE_TYPE_BIKING_STATIONARY
                3 -> ExerciseSessionRecord.EXERCISE_TYPE_ROWING_MACHINE
                4 -> ExerciseSessionRecord.EXERCISE_TYPE_ELLIPTICAL
                5 -> ExerciseSessionRecord.EXERCISE_TYPE_OTHER_WORKOUT
                6 -> ExerciseSessionRecord.EXERCISE_TYPE_STAIR_CLIMBING_MACHINE
                else -> ExerciseSessionRecord.EXERCISE_TYPE_OTHER_WORKOUT
            }
        }
    }
}
