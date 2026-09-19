package com.nettarion.hyperborea.hardware.fitpro.v2

import com.nettarion.hyperborea.core.AppLogger
import com.nettarion.hyperborea.core.model.ConsoleKey
import com.nettarion.hyperborea.core.model.DeviceCommand
import com.nettarion.hyperborea.core.model.DeviceIdentity
import com.nettarion.hyperborea.core.model.DeviceInfo
import com.nettarion.hyperborea.core.model.DeviceType
import com.nettarion.hyperborea.core.model.ExerciseData
import com.nettarion.hyperborea.core.model.isBeltBased
import com.nettarion.hyperborea.hardware.fitpro.session.ExerciseDataAccumulator
import com.nettarion.hyperborea.hardware.fitpro.session.FitProSession
import com.nettarion.hyperborea.hardware.fitpro.session.GripHeartRateFilter
import com.nettarion.hyperborea.hardware.fitpro.session.SessionState
import com.nettarion.hyperborea.hardware.fitpro.transport.HidTransport
import kotlin.math.roundToInt
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.filterNotNull
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.launch
import kotlinx.coroutines.withTimeoutOrNull

class V2Session(
    private val transport: HidTransport,
    private val logger: AppLogger,
    private val scope: CoroutineScope,
    private val deviceInfo: DeviceInfo,
    private val accumulator: ExerciseDataAccumulator = ExerciseDataAccumulator(),
) : FitProSession {

    private val _exerciseData = MutableStateFlow<ExerciseData?>(null)
    override val exerciseData: StateFlow<ExerciseData?> = _exerciseData.asStateFlow()

    private val _deviceIdentity = MutableStateFlow<DeviceIdentity?>(null)
    override val deviceIdentity: StateFlow<DeviceIdentity?> = _deviceIdentity.asStateFlow()

    private val _sessionState = MutableStateFlow<SessionState>(SessionState.Disconnected)
    override val sessionState: StateFlow<SessionState> = _sessionState.asStateFlow()

    private val _consoleKeyPresses = MutableSharedFlow<ConsoleKey>(extraBufferCapacity = 8)
    override val consoleKeyPresses: SharedFlow<ConsoleKey> = _consoleKeyPresses

    private val _degradedReason = MutableStateFlow<String?>(null)
    override val degradedReason: StateFlow<String?> = _degradedReason.asStateFlow()

    /** Latest WORKOUT_STATE value reported by the console. */
    private val _workoutMode = MutableStateFlow<Float?>(null)

    /** Complete supported feature set, published only after the list terminator arrives. */
    private val _supportedFeatures = MutableStateFlow<Set<V2FeatureId>?>(null)

    /** Union of supported-features frames received so far. */
    private val featureAccumulator = mutableSetOf<V2FeatureId>()
    private val unknownFeatureCodes = mutableSetOf<Int>()
    private var declaredFeatures: Set<V2FeatureId>? = null

    override var detectedDeviceType: DeviceType = DeviceType.BIKE
        private set

    private var receiveJob: Job? = null
    private var startRequestJob: Job? = null
    private var lastKeyCode = 0
    private var lastSentGrade = 0f
    private var lastSentSpeed = 0f
    private val gripHeartRate = GripHeartRateFilter()

    override suspend fun start() {
        if (_sessionState.value is SessionState.Streaming || _sessionState.value is SessionState.Connecting) return

        try {
            _sessionState.value = SessionState.Connecting
            transport.open()

            _sessionState.value = SessionState.Handshaking
            startReceiveLoop()

            // LargeX consoles send SupportedFeatures over multiple frames. Read the complete list
            // before subscribing so unsupported feature ids are never sent back to the console.
            val supported = querySupportedFeatures(QUERY_FEATURES_ATTEMPTS)
            declaredFeatures = supported
            if (supported != null) {
                detectedDeviceType = deriveDeviceType(supported)
                logger.i(TAG, "Detected device type: $detectedDeviceType (from ${supported.size} features)")
            } else {
                logger.w(TAG, "Console never declared supported features; assuming $detectedDeviceType, subscribing unfiltered")
            }

            configureSubscriptions(supported)
            _deviceIdentity.value = DeviceIdentity()

            // Mirror the stock one-shot bring-up. The idle-lock value depends on whether this is a
            // belt console and whether START_REQUESTED is available, so do this after type detection.
            writeInitConfiguration(supported)

            transitionToWorkout()

            accumulator.start()
            _sessionState.value = SessionState.Streaming
            logger.i(TAG, "V2 session started")
        } catch (e: CancellationException) {
            throw e
        } catch (e: Exception) {
            logger.e(TAG, "Failed to start V2 session", e)
            receiveJob?.cancel()
            receiveJob = null
            startRequestJob?.cancel()
            startRequestJob = null
            try { transport.close() } catch (_: Exception) {}
            _sessionState.value = SessionState.Error(e.message ?: "V2 session failed", e)
        }
    }

    override suspend fun stop() {
        receiveJob?.cancel()
        receiveJob = null
        startRequestJob?.cancel()
        startRequestJob = null

        try {
            if (transport.isOpen) {
                haltForTeardown()
                transport.write(V2Codec.encode(
                    V2Message.Outgoing.WriteFeature(V2FeatureId.WORKOUT_STATE, V2WorkoutMode.NONE.raw),
                ))
                transport.write(V2Codec.encode(V2Message.Outgoing.Unsubscribe(V2FeatureId.subscribable)))
            }
        } catch (e: CancellationException) {
            throw e
        } catch (e: Exception) {
            logger.w(TAG, "Error during V2 session stop: ${e.message}")
        } finally {
            try { transport.close() } catch (e: Exception) {
                logger.w(TAG, "Transport close failed: ${e.message}")
            }
        }

        accumulator.reset()
        featureAccumulator.clear()
        unknownFeatureCodes.clear()
        declaredFeatures = null
        _supportedFeatures.value = null
        _exerciseData.value = null
        _deviceIdentity.value = null
        _degradedReason.value = null
        _sessionState.value = SessionState.Disconnected
        logger.i(TAG, "V2 session stopped")
    }

    private suspend fun haltForTeardown() {
        if (!detectedDeviceType.isBeltBased) return
        transport.write(V2Codec.encode(V2Message.Outgoing.WriteFeature(V2FeatureId.TARGET_KPH, 0f)))
        transport.write(V2Codec.encode(V2Message.Outgoing.WriteFeature(V2FeatureId.TARGET_GRADE, 0f)))
        transport.write(V2Codec.encode(
            V2Message.Outgoing.WriteFeature(V2FeatureId.WORKOUT_STATE, V2WorkoutMode.PAUSED.raw),
        ))
        delay(BELT_HALT_SETTLE_MS)
    }

    override suspend fun identify(): DeviceIdentity? {
        try {
            transport.open()
            startReceiveLoop()
            val supported = querySupportedFeatures(attempts = 1)
            logger.i(TAG, if (supported != null) {
                "Console replied with ${supported.size} features"
            } else {
                "Console didn't reply to features query"
            })
            _deviceIdentity.value = DeviceIdentity()
            return _deviceIdentity.value
        } catch (e: CancellationException) {
            throw e
        } catch (e: Exception) {
            logger.e(TAG, "Identify failed", e)
            return null
        } finally {
            receiveJob?.cancel()
            receiveJob = null
            try { transport.close() } catch (_: Exception) {}
        }
    }

    override suspend fun calibrate() {
        throw UnsupportedOperationException("CalibrateIncline not supported on V2")
    }

    override suspend fun writeFeature(command: DeviceCommand) {
        if (_sessionState.value !is SessionState.Streaming) return

        val message = when (command) {
            is DeviceCommand.SetResistance -> V2Message.Outgoing.WriteFeature(
                V2FeatureId.TARGET_RESISTANCE,
                command.level.toFloat(),
            )
            is DeviceCommand.SetIncline -> {
                lastSentGrade = roundToStep(command.percent, deviceInfo.inclineStep)
                V2Message.Outgoing.WriteFeature(V2FeatureId.TARGET_GRADE, lastSentGrade)
            }
            is DeviceCommand.SetTargetSpeed -> {
                lastSentSpeed = command.kph
                V2Message.Outgoing.WriteFeature(V2FeatureId.TARGET_KPH, command.kph)
            }
            is DeviceCommand.AdjustIncline -> {
                lastSentGrade += if (command.increase) deviceInfo.inclineStep else -deviceInfo.inclineStep
                lastSentGrade = lastSentGrade.coerceIn(deviceInfo.minIncline, deviceInfo.maxIncline)
                V2Message.Outgoing.WriteFeature(V2FeatureId.TARGET_GRADE, lastSentGrade)
            }
            is DeviceCommand.AdjustSpeed -> {
                lastSentSpeed += if (command.increase) deviceInfo.speedStep else -deviceInfo.speedStep
                lastSentSpeed = lastSentSpeed.coerceIn(0f, deviceInfo.maxSpeed)
                V2Message.Outgoing.WriteFeature(V2FeatureId.TARGET_KPH, lastSentSpeed)
            }
            is DeviceCommand.SetTargetPower -> V2Message.Outgoing.WriteFeature(
                V2FeatureId.GOAL_WATTS,
                command.watts.toFloat(),
            )
            is DeviceCommand.PauseWorkout,
            is DeviceCommand.StopWorkout -> {
                accumulator.pause()
                V2Message.Outgoing.WriteFeature(V2FeatureId.WORKOUT_STATE, V2WorkoutMode.PAUSED.raw)
            }
            is DeviceCommand.ResumeWorkout -> {
                val mode = _workoutMode.value?.let { V2WorkoutMode.fromRaw(it) }
                if (detectedDeviceType == DeviceType.TREADMILL) {
                    if (mode == V2WorkoutMode.PAUSED) {
                        requestWorkoutStart("ResumeWorkout")
                    } else {
                        logger.i(TAG, "Ignoring host ResumeWorkout while treadmill is idle; waiting for physical Start")
                    }
                    return
                }
                accumulator.resume()
                V2Message.Outgoing.WriteFeature(V2FeatureId.WORKOUT_STATE, V2WorkoutMode.RUNNING.raw)
            }
            is DeviceCommand.CalibrateIncline -> {
                logger.w(TAG, "CalibrateIncline not supported on V2")
                return
            }
            is DeviceCommand.SetFanSpeed -> {
                logger.w(TAG, "SetFanSpeed not supported on V2")
                return
            }
            is DeviceCommand.SetVolume,
            is DeviceCommand.SetGear,
            is DeviceCommand.SetDistanceGoal,
            is DeviceCommand.SetWarmupTimeout,
            is DeviceCommand.SetCooldownTimeout,
            is DeviceCommand.SetPauseTimeout,
            is DeviceCommand.SetWarmUpMode,
            is DeviceCommand.SetCoolDownMode,
            is DeviceCommand.SetErgMode -> {
                logger.w(TAG, "${command::class.simpleName} not supported on V2")
                return
            }
        }

        try {
            transport.write(V2Codec.encode(message))
        } catch (e: CancellationException) {
            throw e
        } catch (e: Exception) {
            logger.e(TAG, "Failed to write feature", e)
        }
    }

    private suspend fun querySupportedFeatures(attempts: Int): Set<V2FeatureId>? {
        repeat(attempts) { attempt ->
            transport.write(V2Codec.encode(V2Message.Outgoing.QueryFeatures()))
            val features = withTimeoutOrNull(SUPPORTED_FEATURES_TIMEOUT_MS) {
                _supportedFeatures.filterNotNull().first()
            }
            if (features != null) return features
            logger.w(
                TAG,
                "No complete supported-features reply within ${SUPPORTED_FEATURES_TIMEOUT_MS}ms (attempt ${attempt + 1}/$attempts)",
            )
        }
        if (featureAccumulator.isNotEmpty()) {
            logger.w(TAG, "Supported-features list never terminated; using ${featureAccumulator.size} accumulated features")
            return featureAccumulator.toSet()
        }
        return null
    }

    private suspend fun configureSubscriptions(supported: Set<V2FeatureId>?) {
        transport.write(V2Codec.encode(V2Message.Outgoing.Unsubscribe(emptyList())))

        val wanted = V2FeatureId.subscribable.filter { supported == null || it in supported }
        if (wanted.isEmpty()) {
            logger.w(TAG, "Console supports none of the features we want; no subscriptions made")
            return
        }

        val batches = wanted.chunked(MAX_SUBSCRIBE_BATCH)
        logger.d(TAG, "Subscribing to ${wanted.size} features in ${batches.size} batches")
        for (batch in batches) {
            transport.write(V2Codec.encode(V2Message.Outgoing.Subscribe(batch)))
        }
    }

    private suspend fun writeInitConfiguration(supported: Set<V2FeatureId>?) {
        if (supported == null) return

        if (V2FeatureId.HEART_BEAT_INTERVAL in supported) {
            logger.i(TAG, "Bring-up: HEART_BEAT_INTERVAL=$HEART_BEAT_INTERVAL_MS")
            writeFeatureRaw(V2FeatureId.HEART_BEAT_INTERVAL, HEART_BEAT_INTERVAL_MS)
        }

        if (V2FeatureId.IDLE_SYSTEM_MODE_LOCK in supported) {
            val lock = if (detectedDeviceType.isBeltBased && !consoleDeclaresStartRequested) {
                IDLE_MODE_LOCKED
            } else {
                IDLE_MODE_UNLOCKED
            }
            logger.i(TAG, "Bring-up: IDLE_SYSTEM_MODE_LOCK=${if (lock == IDLE_MODE_LOCKED) "locked" else "unlocked"}")
            writeFeatureRaw(V2FeatureId.IDLE_SYSTEM_MODE_LOCK, lock)
        }
    }

    private fun startReceiveLoop() {
        receiveJob = scope.launch {
            try {
                transport.incoming().collect { data ->
                    handleIncoming(data)
                }
            } catch (e: CancellationException) {
                throw e
            } catch (e: Exception) {
                logger.e(TAG, "Receive loop error", e)
            }

            if (_sessionState.value is SessionState.Streaming) {
                logger.w(TAG, "Transport disconnected")
                _sessionState.value = SessionState.Disconnected
            }
        }
    }

    private var lastLogTimeMs = 0L

    private fun handleIncoming(data: ByteArray) {
        val message = V2Codec.decode(data) ?: return

        when (message) {
            is V2Message.Incoming.Event -> {
                applyEvent(message.feature, message.value)
                _exerciseData.value = accumulator.snapshot()

                val now = System.currentTimeMillis()
                if (now - lastLogTimeMs >= 1000L) {
                    lastLogTimeMs = now
                    val snap = _exerciseData.value
                    if (snap != null) {
                        logger.d(
                            TAG,
                            "power=${snap.power}W cadence=${snap.cadence}rpm speed=${snap.speed}kph resistance=${snap.resistance} incline=${snap.incline}%",
                        )
                    }
                }
            }
            is V2Message.Incoming.SupportedFeatures -> {
                if (message.isEndOfList) {
                    if (unknownFeatureCodes.isNotEmpty()) {
                        logger.i(TAG, "Console declared ${unknownFeatureCodes.size} feature ids we don't use: $unknownFeatureCodes")
                    }
                    logger.i(TAG, "Supported features complete: ${featureAccumulator.map { it.name }}")
                    _supportedFeatures.value = featureAccumulator.toSet()
                } else {
                    featureAccumulator += message.features
                    unknownFeatureCodes += message.unknownCodes
                    logger.d(
                        TAG,
                        "Supported-features frame: ${message.features.map { it.name }}" +
                            if (message.unknownCodes.isNotEmpty()) " + unknown ${message.unknownCodes}" else "",
                    )
                }
            }
            is V2Message.Incoming.Acknowledge ->
                logger.d(TAG, "ACK: ${message.type}")
            is V2Message.Incoming.Error ->
                logger.w(TAG, "Console rejected ${message.describe()}")
            is V2Message.Incoming.Unknown ->
                logger.d(TAG, "Unknown message: ${message.raw.size} bytes")
        }
    }

    private fun applyEvent(feature: V2FeatureId, value: Float) {
        when (feature) {
            V2FeatureId.KEY_COOKED -> handleKeypadCode(value.toInt())
            V2FeatureId.WATTS -> accumulator.updatePower(value.toInt())
            V2FeatureId.RPM -> accumulator.updateCadence(value.toInt())
            V2FeatureId.CURRENT_KPH -> accumulator.updateSpeed(value)
            V2FeatureId.TARGET_RESISTANCE -> accumulator.updateResistance(value.toInt())
            V2FeatureId.CURRENT_GRADE -> accumulator.updateIncline(value)
            V2FeatureId.PULSE -> accumulator.updateHeartRate(gripHeartRate.update(value.toInt()))
            V2FeatureId.DISTANCE -> accumulator.updateDistance(value)
            V2FeatureId.CURRENT_CALORIES -> accumulator.updateCalories(value.toInt())
            V2FeatureId.RUNNING_TIME -> accumulator.updateElapsedTime(value.toLong())
            V2FeatureId.TARGET_KPH -> {
                accumulator.updateTargetSpeed(value)
                if (detectedDeviceType.isBeltBased) accumulator.updateSpeed(value)
            }
            V2FeatureId.TARGET_GRADE -> accumulator.updateTargetIncline(value)
            V2FeatureId.HEART_BEAT_INTERVAL,
            V2FeatureId.IDLE_SYSTEM_MODE_LOCK,
            V2FeatureId.START_REQUESTED,
            V2FeatureId.DISPLAY_UNITS,
            V2FeatureId.GOAL_TIME,
            V2FeatureId.WARM_UP_TIMEOUT,
            V2FeatureId.COOL_DOWN_TIMEOUT,
            V2FeatureId.PAUSE_TIMEOUT -> { /* Write-only configuration echoes. */ }
            V2FeatureId.SYSTEM_MODE -> { /* System state, not exercise data. */ }
            V2FeatureId.WORKOUT_STATE -> {
                val mode = V2WorkoutMode.fromRaw(value)
                _workoutMode.value = value
                logger.i(TAG, "Console workout state event: $mode (raw=$value)")
                if (mode == V2WorkoutMode.READY_TO_START) {
                    requestWorkoutStart("console reported READY_TO_START")
                }
                accumulator.updateWorkoutMode(v2WorkoutStateToV1Code(mode))
            }
            V2FeatureId.MAX_RESISTANCE -> { /* Device capability. */ }
            V2FeatureId.GOAL_WATTS -> accumulator.updateTargetPower(value.toInt())
        }
    }

    private fun handleKeypadCode(code: Int) {
        if (code == lastKeyCode) return
        lastKeyCode = code
        if (code == 0) return

        val key = when (code) {
            KEY_STOP -> ConsoleKey.STOP
            KEY_START -> ConsoleKey.START
            KEY_SPEED_UP -> ConsoleKey.SPEED_UP
            KEY_SPEED_DOWN -> ConsoleKey.SPEED_DOWN
            KEY_INCLINE_UP -> ConsoleKey.INCLINE_UP
            KEY_INCLINE_DOWN -> ConsoleKey.INCLINE_DOWN
            else -> null
        }

        logger.d(TAG, "Console keypad: code=$code${key?.let { " ($it)" } ?: ""}")
        if (key == null) return
        _consoleKeyPresses.tryEmit(key)

        if (key == ConsoleKey.START) {
            requestWorkoutStart("physical Start key")
            return
        }
        routeTreadmillKey(key)
    }

    private fun routeTreadmillKey(key: ConsoleKey) {
        if (detectedDeviceType != DeviceType.TREADMILL || consoleDeclaresStartRequested) return
        val mode = _workoutMode.value?.let { V2WorkoutMode.fromRaw(it) }
        if (mode != V2WorkoutMode.RUNNING) return

        val command = when (key) {
            ConsoleKey.SPEED_UP -> DeviceCommand.AdjustSpeed(increase = true)
            ConsoleKey.SPEED_DOWN -> DeviceCommand.AdjustSpeed(increase = false)
            ConsoleKey.INCLINE_UP -> DeviceCommand.AdjustIncline(increase = true)
            ConsoleKey.INCLINE_DOWN -> DeviceCommand.AdjustIncline(increase = false)
            ConsoleKey.STOP -> DeviceCommand.PauseWorkout
            else -> return
        }
        scope.launch { writeFeature(command) }
    }

    private val consoleDeclaresStartRequested: Boolean
        get() = declaredFeatures?.contains(V2FeatureId.START_REQUESTED) == true

    private fun requestWorkoutStart(trigger: String) {
        if (detectedDeviceType != DeviceType.TREADMILL) return
        if (_sessionState.value !is SessionState.Streaming) return

        val from = _workoutMode.value?.let { V2WorkoutMode.fromRaw(it) }
        if (from == V2WorkoutMode.RUNNING) return
        if (startRequestJob?.isActive == true) return

        val acked = consoleDeclaresStartRequested
        startRequestJob = scope.launch {
            try {
                if (acked) {
                    logger.i(TAG, "Start requested ($trigger, from=${from ?: "?"}); acking with START_REQUESTED")
                    writeFeatureRaw(V2FeatureId.START_REQUESTED, START_REQUESTED_TRUE)
                } else {
                    logger.i(TAG, "Start requested ($trigger, from=${from ?: "?"}); host-driving RUNNING without WARM_UP")
                    releaseIdleModeLock()
                    if (from != V2WorkoutMode.PAUSED) writePreWorkoutConfig()
                    writeWorkoutState(V2WorkoutMode.RUNNING)
                    writeFeature(DeviceCommand.SetTargetSpeed(maxOf(lastSentSpeed, INITIAL_BELT_KPH)))
                    if (V2FeatureId.TARGET_GRADE in (declaredFeatures ?: emptySet())) {
                        writeFeature(DeviceCommand.SetIncline(lastSentGrade))
                    }
                }
            } catch (e: CancellationException) {
                throw e
            } catch (e: Exception) {
                logger.e(TAG, "Start-request writes failed", e)
                return@launch
            }

            if (confirmWorkoutMode("reach RUNNING after Start") { it == V2WorkoutMode.RUNNING }) {
                logger.i(TAG, "Console reached RUNNING (${if (acked) "START_REQUESTED honored" else "host-driven"})")
                _degradedReason.value = null
            } else {
                logger.w(TAG, "Console didn't reach RUNNING after start ($trigger)")
                _degradedReason.value = START_NOT_CONFIRMED_REASON
            }
        }
    }

    private suspend fun releaseIdleModeLock() {
        if (V2FeatureId.IDLE_SYSTEM_MODE_LOCK !in (declaredFeatures ?: emptySet())) return
        logger.i(TAG, "Start: releasing IDLE_SYSTEM_MODE_LOCK before commanding RUNNING")
        writeFeatureRaw(V2FeatureId.IDLE_SYSTEM_MODE_LOCK, IDLE_MODE_UNLOCKED)
    }

    private suspend fun writePreWorkoutConfig() {
        val declared = declaredFeatures ?: emptySet()
        val config = listOf(
            V2FeatureId.DISPLAY_UNITS to DISPLAY_UNITS_METRIC,
            V2FeatureId.WARM_UP_TIMEOUT to WARM_UP_TIMEOUT_S,
            V2FeatureId.COOL_DOWN_TIMEOUT to COOL_DOWN_TIMEOUT_S,
            V2FeatureId.PAUSE_TIMEOUT to PAUSE_TIMEOUT_S,
            V2FeatureId.GOAL_TIME to GOAL_TIME_S,
        )
        val sent = config.filter { (feature, _) -> feature in declared }
        logger.i(TAG, "Pre-workout config: ${sent.joinToString { "${it.first}=${it.second}" }}")
        for ((feature, value) in sent) {
            writeFeatureRaw(feature, value)
        }
    }

    private suspend fun writeFeatureRaw(feature: V2FeatureId, value: Float) {
        transport.write(V2Codec.encode(V2Message.Outgoing.WriteFeature(feature, value)))
    }

    private fun roundToStep(value: Float, step: Float): Float =
        (value / step).roundToInt() * step

    private suspend fun transitionToWorkout() {
        if (detectedDeviceType == DeviceType.TREADMILL) {
            // Important for LargeX: do not write WARM_UP while arming. Older firmware rejects it
            // from idle, while other belt consoles can start moving immediately when it is written.
            logger.i(TAG, "Treadmill armed; staying idle and awaiting the physical Start key")
            _degradedReason.value = null
            return
        }

        writeWorkoutState(V2WorkoutMode.WARM_UP)
        confirmWorkoutMode("leave idle") {
            it != V2WorkoutMode.NONE && it != V2WorkoutMode.READY_TO_START
        }
        writeWorkoutState(V2WorkoutMode.RUNNING)
        val running = confirmWorkoutMode("reach RUNNING") { it == V2WorkoutMode.RUNNING }
        logger.i(
            TAG,
            "Console workout state: NONE -> WARM_UP -> ${if (running) V2WorkoutMode.RUNNING else V2WorkoutMode.UNKNOWN}",
        )
        _degradedReason.value = if (running) {
            null
        } else {
            "The console didn't confirm the workout started; resistance/speed may not respond"
        }
    }

    private fun deriveDeviceType(features: Set<V2FeatureId>): DeviceType {
        val hasResistance = V2FeatureId.TARGET_RESISTANCE in features ||
            V2FeatureId.MAX_RESISTANCE in features
        val hasBeltSpeed = V2FeatureId.TARGET_KPH in features || V2FeatureId.CURRENT_KPH in features
        val hasGrade = V2FeatureId.TARGET_GRADE in features || V2FeatureId.CURRENT_GRADE in features
        return when {
            !hasResistance && (hasBeltSpeed || hasGrade) -> DeviceType.TREADMILL
            else -> DeviceType.BIKE
        }
    }

    private fun v2WorkoutStateToV1Code(mode: V2WorkoutMode): Int = when (mode) {
        V2WorkoutMode.RUNNING -> V1_WORKOUT_MODE_RUNNING
        V2WorkoutMode.PAUSED -> V1_WORKOUT_MODE_PAUSE
        V2WorkoutMode.OFF_MACHINE -> V1_WORKOUT_MODE_DMK
        V2WorkoutMode.WARM_UP -> V1_WORKOUT_MODE_WARM_UP
        V2WorkoutMode.COOL_DOWN -> V1_WORKOUT_MODE_COOL_DOWN
        V2WorkoutMode.NONE,
        V2WorkoutMode.READY_TO_START,
        V2WorkoutMode.RESULTS -> V1_WORKOUT_MODE_IDLE
        V2WorkoutMode.UNKNOWN -> V1_WORKOUT_MODE_UNKNOWN
    }

    private suspend fun writeWorkoutState(mode: V2WorkoutMode) {
        transport.write(V2Codec.encode(
            V2Message.Outgoing.WriteFeature(V2FeatureId.WORKOUT_STATE, mode.raw),
        ))
    }

    private suspend fun confirmWorkoutMode(
        what: String,
        accept: (V2WorkoutMode) -> Boolean,
    ): Boolean {
        val ok = withTimeoutOrNull(STATE_CONFIRM_TIMEOUT_MS) {
            _workoutMode.filterNotNull().map { V2WorkoutMode.fromRaw(it) }.first { accept(it) }
            true
        } != null
        if (!ok) logger.w(TAG, "Console didn't $what; workout may be inactive; continuing")
        return ok
    }

    companion object {
        private const val TAG = "V2Session"
        private const val BELT_HALT_SETTLE_MS = 200L
        private const val MAX_SUBSCRIBE_BATCH = 8
        private const val STATE_CONFIRM_TIMEOUT_MS = 5_000L
        private const val SUPPORTED_FEATURES_TIMEOUT_MS = 4_000L
        private const val QUERY_FEATURES_ATTEMPTS = 3

        private const val START_REQUESTED_TRUE = 1f
        private const val INITIAL_BELT_KPH = 0.5f
        private const val HEART_BEAT_INTERVAL_MS = 720f
        private const val IDLE_MODE_UNLOCKED = 0f
        private const val IDLE_MODE_LOCKED = 1f
        private const val DISPLAY_UNITS_METRIC = 1f
        private const val WARM_UP_TIMEOUT_S = 180f
        private const val COOL_DOWN_TIMEOUT_S = 600f
        private const val PAUSE_TIMEOUT_S = 600f
        private const val GOAL_TIME_S = 10_800f
        private const val START_NOT_CONFIRMED_REASON = "The console didn't confirm the treadmill start; press Start again"

        private const val KEY_STOP = 1
        private const val KEY_START = 2
        private const val KEY_SPEED_UP = 3
        private const val KEY_SPEED_DOWN = 4
        private const val KEY_INCLINE_UP = 5
        private const val KEY_INCLINE_DOWN = 6

        private const val V1_WORKOUT_MODE_UNKNOWN = 0
        private const val V1_WORKOUT_MODE_IDLE = 1
        private const val V1_WORKOUT_MODE_RUNNING = 2
        private const val V1_WORKOUT_MODE_PAUSE = 3
        private const val V1_WORKOUT_MODE_DMK = 8
        private const val V1_WORKOUT_MODE_WARM_UP = 10
        private const val V1_WORKOUT_MODE_COOL_DOWN = 11
    }
}
