package com.nettarion.hyperborea.hardware.fitpro.v1

import com.nettarion.hyperborea.core.AppLogger
import com.nettarion.hyperborea.core.model.ConsoleKey
import com.nettarion.hyperborea.core.model.DeviceCommand
import com.nettarion.hyperborea.core.model.DeviceIdentity
import com.nettarion.hyperborea.core.model.DeviceInfo
import com.nettarion.hyperborea.core.model.DeviceType
import com.nettarion.hyperborea.core.model.ExerciseData
import com.nettarion.hyperborea.core.model.isBeltBased
import com.nettarion.hyperborea.hardware.fitpro.session.DeviceDatabase
import com.nettarion.hyperborea.hardware.fitpro.session.ExerciseDataAccumulator
import com.nettarion.hyperborea.hardware.fitpro.session.FitProSession
import com.nettarion.hyperborea.hardware.fitpro.session.GripHeartRateFilter
import com.nettarion.hyperborea.hardware.fitpro.session.PowerEstimator
import com.nettarion.hyperborea.hardware.fitpro.session.SessionState
import com.nettarion.hyperborea.hardware.fitpro.transport.HidTransport
import kotlin.math.roundToInt
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.channels.BufferOverflow
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withTimeoutOrNull

class V1Session(
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

    private val _consoleKeyPresses =
        MutableSharedFlow<ConsoleKey>(extraBufferCapacity = 8, onBufferOverflow = BufferOverflow.DROP_OLDEST)
    override val consoleKeyPresses: SharedFlow<ConsoleKey> = _consoleKeyPresses.asSharedFlow()

    private val _degradedReason = MutableStateFlow<String?>(null)
    override val degradedReason: StateFlow<String?> = _degradedReason.asStateFlow()

    private var pollJob: Job? = null
    private var pendingWriteFields: Map<V1DataField, Float> = emptyMap()
    private val pendingWriteMutex = Mutex()
    @Volatile private var pendingCalibration: CompletableDeferred<Unit>? = null
    private var lastLogTimeMs = 0L
    private var consecutivePollErrors = 0
    private var lastSentGrade = 0f
    private var lastSentSpeed = 0f
    private var lastKeyCode = -1 // for KEY_OBJECT press-edge detection
    private val resistance = ResistanceConverter(deviceInfo.maxResistance)
    private val gripHeartRate = GripHeartRateFilter()

    /** Device capabilities read from MCU during handshake. */
    var capabilities: V1Capabilities? = null
        private set

    /** Power-curve table index for this device, resolved during the handshake. */
    var powerCurveIndex: Int? = null
        private set

    // Security handshake state — stored for SECURITY_BLOCK re-verification
    private var softwareVersion: Int = 0
    private var hardwareVersion: Int = 0
    private var serialNumber: Int = 0
    private var partNumber: Int = 0
    private var model: Int = 0
    private var masterLibraryVersion: Int = 0

    /** Bitfield indices ([V1DataField.fieldIndex]) the device declared it supports; empty if it couldn't be read. */
    private var supportedBitFields: Set<Int> = emptySet()

    /**
     * Equipment type as detected from the MCU's own `Connect` device-id response. The constructor's
     * [deviceInfo] arrives here from [com.nettarion.hyperborea.hardware.fitpro.session.DeviceDatabase.fromProductId],
     * which only knows the USB product id and defaults [DeviceType.BIKE] for every FitPro device —
     * so its `.type` cannot be trusted during [prepareConsole]/[transitionToActive]. The MCU's
     * equipment id, captured in [handshake] and mapped via [DeviceDatabase.deviceTypeFromEquipmentId],
     * is the ground truth at this stage. [com.nettarion.hyperborea.hardware.fitpro.FitProAdapter]
     * reads this back through [FitProSession.detectedDeviceType] after [start] returns and uses it
     * to refine the adapter-level [DeviceInfo.type].
     */
    override var detectedDeviceType: DeviceType = DeviceType.BIKE
        private set

    /**
     * The actual set we poll for each loop iteration, narrowed to what the device claims to support.
     * Filtering matters because [V1Codec.decodeDataResponseForFields] decodes the response as a flat
     * blob in field-index order with no per-field presence check, so asking for a field the MCU
     * doesn't supply causes every subsequent field to land on the wrong offset (the bug behind the
     * NordicTrack 2950 Argon-firmware -10595 kcal / 139 km screenshot).
     */
    private var pollFields: Set<V1DataField> = V1DataField.periodicReadFields

    /** Tracks whether the previous poll's response was flagged truncated, so we log only on the edge. */
    private var lastTruncatedSeen: Boolean = false

    override suspend fun start() {
        if (_sessionState.value is SessionState.Streaming || _sessionState.value is SessionState.Connecting) return

        try {
            _sessionState.value = SessionState.Connecting
            transport.open()

            _sessionState.value = SessionState.Handshaking
            transport.clearBuffer()
            handshake()

            // Console init (done while still in IDLE), then bring the workout up the way the
            // console firmware expects (device-type-dependent — treadmills stop at WARM_UP and
            // wait for the physical Start key, see transitionToActive).
            prepareConsole()
            accumulator.start()
            transitionToActive()

            _sessionState.value = SessionState.Streaming
            startPollLoop()
            logger.i(TAG, "V1 session started")
        } catch (e: CancellationException) {
            throw e
        } catch (e: Exception) {
            logger.e(TAG, "Failed to start V1 session", e)
            try { transport.close() } catch (_: Exception) {}
            _sessionState.value = SessionState.Error(e.message ?: "V1 session failed", e)
        }
    }

    override suspend fun stop() {
        // Stop the poll loop and wait (briefly) for it to actually exit before we touch the transport
        // — gracefulEndForDisconnect does its own request/response round-trips and must not race the
        // poll's reads. Bounded so a wedged MCU read can't hang teardown.
        pollJob?.let { job ->
            job.cancel()
            withTimeoutOrNull(POLL_JOIN_TIMEOUT_MS) { job.join() }
        }
        pollJob = null

        try {
            if (transport.isOpen) {
                gracefulEndForDisconnect()
                writeMessage(V1Message.Outgoing.Disconnect())
                delay(COMMAND_DELAY_MS)
                transport.close()
            }
        } catch (e: CancellationException) {
            throw e
        } catch (e: Exception) {
            logger.w(TAG, "Error during V1 session stop: ${e.message}")
        }

        accumulator.reset()
        _exerciseData.value = null
        _deviceIdentity.value = null
        _degradedReason.value = null
        _sessionState.value = SessionState.Disconnected
        logger.i(TAG, "V1 session stopped")
    }

    /**
     * Brings the equipment to a safe stopped state, then waits for the MCU to confirm it's ready to
     * drop the USB link, before [stop] disconnects. Three phases:
     *
     * 1. **Belt halt (belt machines only, safety-critical):** a bare `WORKOUT_MODE=IDLE` write does
     *    NOT stop the belt — the firmware treats the idle transition as advisory, so the belt keeps
     *    running until the user hits the physical Stop key (the user-reported bug). We command belt
     *    speed to 0 *and* `PAUSE` (both halt the belt) and confirm the `KPH` read-back reaches 0
     *    first ([haltBeltConfirmed]).
     * 2. **Clean end:** write `WORKOUT_MODE=IDLE` (and `GRADE=0` on incline-capable belt machines so
     *    they don't park raised). This tells the MCU the run is over so it runs its end-of-workout
     *    housekeeping — which we never used to do, so we'd disconnect while it still thought a workout
     *    was live.
     * 3. **Ready-to-disconnect wait:** poll `IS_READY_TO_DISCONNECT` until the MCU asserts it
     *    (bounded — a wedged MCU must not hang teardown). Closing the bus before the MCU is ready
     *    leaves its USB state inconsistent until a full re-enumeration, which is why a run that ended
     *    on the console could previously only be recovered by force-stopping the app.
     */
    private suspend fun gracefulEndForDisconnect() {
        val isBelt = detectedDeviceType.isBeltBased
        if (isBelt) haltBeltConfirmed()

        // Clean end (write-only): tell the MCU the run is over so it does its end-of-workout
        // housekeeping. GRADE=0 on belt machines so an incline trainer doesn't park raised.
        writeMessage(V1Message.Outgoing.ReadWriteData(
            writeFields = buildMap {
                put(V1DataField.WORKOUT_MODE, WorkoutMode.IDLE.raw)
                if (isBelt) put(V1DataField.GRADE, 0f)
            },
        ))
        delay(COMMAND_DELAY_MS)

        repeat(READY_POLL_ATTEMPTS) {
            val response = sendReadWrite(readFields = setOf(V1DataField.IS_READY_TO_DISCONNECT))
            val ready = response?.fields?.get(V1DataField.IS_READY_TO_DISCONNECT)
            if (ready != null && ready >= READY_TO_DISCONNECT_TRUE) {
                logger.i(TAG, "MCU ready to disconnect")
                return
            }
            delay(READY_POLL_MS)
        }
        logger.w(TAG, "MCU never asserted IS_READY_TO_DISCONNECT within ${READY_POLL_ATTEMPTS * READY_POLL_MS}ms — proceeding")
    }

    /**
     * Belt-machine halt loop: command `KPH=0` + `PAUSE` once, then poll the `KPH` read-back until the
     * MCU acknowledges speed 0 (or we run out of attempts). Belt machines only — callers gate on
     * [DeviceType.isBeltBased].
     */
    private suspend fun haltBeltConfirmed() {
        repeat(BELT_HALT_CONFIRM_ATTEMPTS) { attempt ->
            val response = sendReadWrite(
                writeFields = if (attempt == 0) mapOf(
                    V1DataField.KPH to 0f,
                    V1DataField.WORKOUT_MODE to WorkoutMode.PAUSE.raw,
                ) else emptyMap(),
                readFields = setOf(V1DataField.KPH),
            )
            val commandedKph = response?.fields?.get(V1DataField.KPH)
            if (commandedKph != null && commandedKph <= BELT_STOPPED_KPH) {
                logger.i(TAG, "Belt halt confirmed (KPH=$commandedKph) before disconnect")
                return
            }
            delay(STATE_CONFIRM_POLL_MS)
        }
        logger.w(TAG, "Belt halt not confirmed after $BELT_HALT_CONFIRM_ATTEMPTS attempts — sent KPH=0 + PAUSE, proceeding")
    }

    override suspend fun identify(): DeviceIdentity? {
        try {
            transport.open()
            transport.clearBuffer()
            handshake()
            return _deviceIdentity.value
        } catch (e: CancellationException) { throw e }
        catch (e: Exception) {
            logger.e(TAG, "Identify failed", e)
            return null
        } finally {
            try { transport.close() } catch (_: Exception) {}
        }
    }

    override suspend fun calibrate() {
        try {
            transport.open()
            transport.clearBuffer()
            handshake()

            // Calibration runs from idle — no RUNNING mode needed.
            // Connect → handshake → calibrate commands at 4s intervals → disconnect.
            runCalibration()
        } finally {
            try { transport.close() } catch (_: Exception) {}
        }
    }

    override suspend fun writeFeature(command: DeviceCommand) {
        if (command is DeviceCommand.CalibrateIncline) {
            if (_sessionState.value !is SessionState.Streaming) {
                throw IllegalStateException("Not connected")
            }
            val deferred = CompletableDeferred<Unit>()
            pendingCalibration = deferred
            deferred.await()
            return
        }

        if (_sessionState.value !is SessionState.Streaming) return

        val fields = commandToFields(command)

        pendingWriteMutex.withLock {
            pendingWriteFields = pendingWriteFields + fields
        }
    }

    internal fun commandToFields(command: DeviceCommand): Map<V1DataField, Float> = when (command) {
        is DeviceCommand.SetResistance -> {
            mapOf(V1DataField.RESISTANCE to resistance.levelToRaw(command.level).toFloat())
        }
        is DeviceCommand.SetIncline -> {
            lastSentGrade = roundToStep(command.percent, deviceInfo.inclineStep)
            mapOf(V1DataField.GRADE to lastSentGrade)
        }
        is DeviceCommand.SetTargetSpeed -> {
            lastSentSpeed = command.kph
            mapOf(V1DataField.KPH to command.kph)
        }
        is DeviceCommand.AdjustIncline -> {
            lastSentGrade += if (command.increase) deviceInfo.inclineStep else -deviceInfo.inclineStep
            lastSentGrade = lastSentGrade.coerceIn(deviceInfo.minIncline, deviceInfo.maxIncline)
            mapOf(V1DataField.GRADE to lastSentGrade)
        }
        is DeviceCommand.AdjustSpeed -> {
            lastSentSpeed += if (command.increase) deviceInfo.speedStep else -deviceInfo.speedStep
            lastSentSpeed = lastSentSpeed.coerceIn(0f, deviceInfo.maxSpeed)
            mapOf(V1DataField.KPH to lastSentSpeed)
        }
        is DeviceCommand.SetTargetPower -> {
            mapOf(
                V1DataField.WATT_GOAL to command.watts.toFloat(),
                V1DataField.IS_CONSTANT_WATTS_MODE to 1f,
            )
        }
        is DeviceCommand.PauseWorkout -> {
            mapOf(V1DataField.WORKOUT_MODE to WorkoutMode.PAUSE.raw)
        }
        is DeviceCommand.ResumeWorkout -> {
            mapOf(V1DataField.WORKOUT_MODE to WorkoutMode.RUNNING.raw)
        }
        is DeviceCommand.CalibrateIncline -> emptyMap()
        is DeviceCommand.SetFanSpeed -> mapOf(V1DataField.FAN_STATE to command.level.toFloat())
        is DeviceCommand.SetVolume -> mapOf(V1DataField.VOLUME to command.level.toFloat())
        is DeviceCommand.SetGear -> mapOf(V1DataField.GEAR to command.gear.toFloat())
        is DeviceCommand.SetDistanceGoal -> mapOf(V1DataField.DISTANCE_GOAL to command.meters.toFloat())
        is DeviceCommand.SetWarmupTimeout -> mapOf(V1DataField.WARMUP_TIMEOUT to command.seconds.toFloat())
        is DeviceCommand.SetCooldownTimeout -> mapOf(V1DataField.COOLDOWN_TIMEOUT to command.seconds.toFloat())
        is DeviceCommand.SetPauseTimeout -> mapOf(V1DataField.PAUSE_TIMEOUT to command.seconds.toFloat())
        is DeviceCommand.SetWarmUpMode -> mapOf(V1DataField.WORKOUT_MODE to WorkoutMode.WARM_UP.raw)
        is DeviceCommand.SetCoolDownMode -> mapOf(V1DataField.WORKOUT_MODE to WorkoutMode.COOL_DOWN.raw)
        is DeviceCommand.SetErgMode -> mapOf(V1DataField.IS_CONSTANT_WATTS_MODE to if (command.enable) 1f else 0f)
    }

    private suspend fun handshake() {
        // 0. DeviceInfo (from MAIN) → serialNumber, softwareVersion, and the real equipment
        //    device ID: the MCU echoes its own device type in byte 0 of the response.
        // DeviceInfo is the gatekeeper: every real controller answers it, and we need its sw /
        // supportedBitFields / equipment id to do anything useful. With no Connect step after it,
        // this is also where we detect "nothing is responding" — so a missing/garbled response is a
        // hard failure rather than something we paper over with defaults.
        val deviceInfo = sendAndAwait(V1Message.Outgoing.DeviceInfo()) as? V1Message.Incoming.DeviceInfoResponse
            ?: throw IllegalStateException("No DeviceInfo response — controller not responding")
        softwareVersion = deviceInfo.softwareVersion
        hardwareVersion = deviceInfo.hardwareVersion
        serialNumber = deviceInfo.serialNumber
        supportedBitFields = deviceInfo.supportedBitFields
        val equipmentDeviceId = deviceInfo.deviceId.takeIf { it in V1Message.EQUIPMENT_DEVICE_IDS }
            ?: V1Message.DEVICE_FITNESS_BIKE
        logger.i(
            TAG,
            "Device info: sw=$softwareVersion, hw=$hardwareVersion, serial=$serialNumber, " +
                "equipmentDeviceId=$equipmentDeviceId, supportedBitFields=${supportedBitFields.size}",
        )
        pollFields = computePollFields(supportedBitFields)
        detectedDeviceType = DeviceDatabase.deviceTypeFromEquipmentId(equipmentDeviceId)
        logger.d(TAG, "Detected device type: $detectedDeviceType")

        delay(COMMAND_DELAY_MS)

        // 1. SupportedCommands → the request opcodes this controller accepts; gates every optional
        //    step below. We deliberately send NO Connect command first: the stock console firmware
        //    never sends one (it brings the link up at the transport layer), and sending our legacy
        //    Connect makes some controllers — the NordicTrack S15i spin bike — stop answering this
        //    and the following meta-queries, which then wedges the USB link. Addressed to the
        //    equipment device id, mirroring the stock bring-up. If the controller doesn't answer (or
        //    doesn't list a command) we skip the optional steps and go straight to the data poll,
        //    exactly as the stock firmware does — sending a controller a command it doesn't
        //    implement wedges it.
        val supportedCommands = querySupportedCommands(equipmentDeviceId)

        delay(COMMAND_DELAY_MS)

        // 2. SystemInfo → partNumber, model (skipped if the controller doesn't declare it)
        if (isCommandSupported(supportedCommands, V1Message.CMD_SYSTEM_INFO)) {
            val systemInfo = sendAndAwait(V1Message.Outgoing.SystemInfo())
            if (systemInfo is V1Message.Incoming.SystemInfoResponse) {
                partNumber = systemInfo.partNumber
                model = systemInfo.model
                logger.d(TAG, "System info: partNumber=$partNumber, model=$model")
                powerCurveIndex = DeviceDatabase.powerCurveIndexForPartNumber(partNumber)
                if (powerCurveIndex != null) {
                    logger.i(TAG, "Power curve table: $powerCurveIndex (from part number $partNumber)")
                }
            } else {
                logger.w(TAG, "Expected SystemInfoResponse, got: $systemInfo")
            }
            delay(COMMAND_DELAY_MS)
        } else {
            logger.i(TAG, "Controller doesn't support SystemInfo — skipping (power-curve lookup unavailable)")
        }

        // 3. VersionInfo → masterLibraryVersion (skipped if the controller doesn't declare it)
        if (isCommandSupported(supportedCommands, V1Message.CMD_VERSION_INFO)) {
            val versionInfo = sendAndAwait(V1Message.Outgoing.VersionInfo())
            if (versionInfo is V1Message.Incoming.VersionInfoResponse) {
                masterLibraryVersion = versionInfo.masterLibraryVersion
                logger.d(TAG, "Version info: masterLib=$masterLibraryVersion, build=${versionInfo.masterLibraryBuild}")
            } else {
                logger.w(TAG, "Expected VersionInfoResponse, got: $versionInfo")
            }
            delay(COMMAND_DELAY_MS)
        } else {
            logger.i(TAG, "Controller doesn't support VersionInfo — skipping")
        }

        _deviceIdentity.value = DeviceIdentity(
            serialNumber = serialNumber.toString(),
            firmwareVersion = softwareVersion.toString(),
            hardwareVersion = hardwareVersion.toString(),
            model = model.toString(),
            partNumber = partNumber.toString(),
        )

        // 4. VerifySecurity (only if SW version > 75 and the controller declares it — the security
        //    hash is derived from SystemInfo/VersionInfo values, so a controller that omits those
        //    can't be unlocked this way and doesn't ask to be).
        if (softwareVersion > 75 && isCommandSupported(supportedCommands, V1Message.CMD_VERIFY_SECURITY)) {
            verifySecurity()
            delay(COMMAND_DELAY_MS)
        } else {
            logger.d(TAG, "Skipping security verification (sw=$softwareVersion, declared=${supportedCommands?.contains(V1Message.CMD_VERIFY_SECURITY) ?: false})")
        }

        // 5. Read startup fields (device limits + equipment stats)
        readStartupFields(equipmentDeviceId)
    }

    /**
     * Asks the controller which request command opcodes it accepts. Returns the declared set, or
     * `null` if the controller didn't answer (or returned something unparseable). Callers treat
     * `null` (and any command not in the set) as "not supported, skip it" via [isCommandSupported]
     * — matching the stock console, which only sends a command the controller lists and otherwise
     * goes straight to the data poll. Addressed to the equipment device id.
     */
    private suspend fun querySupportedCommands(equipmentDeviceId: Int): Set<Int>? {
        val response = sendAndAwait(V1Message.Outgoing.SupportedCommands(equipmentDeviceId))
        return if (response is V1Message.Incoming.SupportedCommandsResponse) {
            logger.i(TAG, "Supported commands: ${response.commandIds.sorted().joinToString { "0x%02X".format(it) }}")
            response.commandIds
        } else {
            logger.w(TAG, "SupportedCommands query failed ($response) — skipping optional commands, going straight to poll")
            null
        }
    }

    /** True only when [supportedCommands] was read and declares [commandId]; `null`/unlisted → skip. */
    private fun isCommandSupported(supportedCommands: Set<Int>?, commandId: Int): Boolean =
        supportedCommands != null && commandId in supportedCommands

    private suspend fun verifySecurity() {
        val hash = V1Security.calculateHash(serialNumber, partNumber, model)
        val secretKey = masterLibraryVersion * 8
        val response = sendAndAwait(V1Message.Outgoing.VerifySecurity(hash = hash, secretKey = secretKey))

        if (response is V1Message.Incoming.SecurityResponse) {
            if (!response.isUnlocked) {
                throw IllegalStateException("Security verification failed (key=${response.unlockedKey})")
            }
            logger.i(TAG, "Security verified (key=${response.unlockedKey})")
        } else {
            logger.w(TAG, "Unexpected security response: $response")
        }
    }

    private suspend fun readStartupFields(equipmentDeviceId: Int) {
        val response = sendReadWrite(readFields = V1DataField.startupReadFields)
        if (response == null || response.status != V1Message.STATUS_DONE || response.fields.isEmpty()) {
            logger.d(TAG, "Startup field read returned no data: $response")
            return
        }
        val fields = response.fields
        capabilities = V1Capabilities(
            maxGrade = fields[V1DataField.MAX_GRADE],
            minGrade = fields[V1DataField.MIN_GRADE],
            maxKph = fields[V1DataField.MAX_KPH],
            minKph = fields[V1DataField.MIN_KPH],
            maxResistance = fields[V1DataField.MAX_RESISTANCE_LEVEL]?.toInt()?.takeIf { it > 0 },
            equipmentDeviceId = equipmentDeviceId,
        )
        logger.i(TAG, "Capabilities: $capabilities")
        // TOTAL_TIME / MOTOR_TOTAL_DISTANCE units are device-dependent: bikes report seconds and
        // metres, but belt machines (e.g. the NordicTrack 2950) report milliseconds and millimetres
        // — which, read as raw s/m, would show a ~13-year runtime and an 833,757 km odometer. Scale
        // belt-machine values to seconds/metres so the lifetime stats read sanely. (Empirical from
        // observed hardware; a plausibility heuristic can't work because a lightly-used ms-device is
        // indistinguishable from a heavily-used s-device — only the device type separates them.)
        val lifetimeScale = if (detectedDeviceType.isBeltBased) 1000 else 1
        val eqHours = fields[V1DataField.TOTAL_TIME]?.toLong()?.let { it / lifetimeScale }
        val eqDist = fields[V1DataField.MOTOR_TOTAL_DISTANCE]?.let { it / lifetimeScale }
        _deviceIdentity.value = _deviceIdentity.value?.copy(equipmentHours = eqHours, equipmentDistance = eqDist)
        logger.i(TAG, "Equipment stats: totalTime=${eqHours}s, totalDistance=${eqDist}m")
    }

    /**
     * Console init, done while the console is still IDLE (before the workout transition).
     * Branches by device type:
     *
     * - **Treadmill / incline trainer**: only `REQUIRE_START_REQUESTED` is asserted, and even that
     *   only if the device supports the bitfield. `IDLE_MODE_LOCKOUT` is deliberately left alone —
     *   on a belt-driven machine the MCU itself gates belt motion on the physical Start key, and
     *   locking out idle-mode on top of that would fight the safety interlock.
     * - **Bike / elliptical / rower**: both `REQUIRE_START_REQUESTED` and `IDLE_MODE_LOCKOUT` are
     *   asserted (if supported). `IDLE_MODE_LOCKOUT=ENABLED` here keeps a Zwift-style session
     *   streaming when the rider briefly stops pedalling — without it the MCU auto-pauses the
     *   workout. [transitionToActive] re-disables it immediately before writing
     *   `WORKOUT_MODE=RUNNING`, which the firmware requires.
     */
    private suspend fun prepareConsole() {
        val isTreadmill = detectedDeviceType == DeviceType.TREADMILL
        val supportsRequireStart = supportedBitFields.isEmpty() ||
            V1DataField.REQUIRE_START_REQUESTED.fieldIndex in supportedBitFields
        val supportsIdleLockout = supportedBitFields.isEmpty() ||
            V1DataField.IDLE_MODE_LOCKOUT.fieldIndex in supportedBitFields

        if (supportsRequireStart) {
            writeConsoleField(V1DataField.REQUIRE_START_REQUESTED, FIELD_ENABLED)
        }
        if (!isTreadmill && supportsIdleLockout) {
            writeConsoleField(V1DataField.IDLE_MODE_LOCKOUT, FIELD_ENABLED)
        }
    }

    private suspend fun writeConsoleField(field: V1DataField, value: Float) {
        sendReadWrite(writeFields = mapOf(field to value), readFields = setOf(field))
        delay(COMMAND_DELAY_MS)
    }

    /**
     * Builds the per-loop read set from [V1DataField.periodicReadFields] intersected with the
     * device's self-declared [supportedBitFields]. We trust the device's declaration: if it didn't
     * claim a field, the MCU won't include bytes for it in the response, and asking anyway would
     * misalign every later field's offset (the bug that produced -10595 kcal / 139 km on the
     * NordicTrack 2950 Argon screenshot).
     *
     * Falls back to the full periodicReadFields set if [supportedBitFields] is empty (handshake
     * couldn't parse the device's bitmask) — that preserves the pre-fix behavior for devices
     * we've always worked with, and the warning makes the fallback visible.
     */
    private fun computePollFields(supportedBitFields: Set<Int>): Set<V1DataField> {
        if (supportedBitFields.isEmpty()) {
            logger.w(
                TAG,
                "Device declared no supportedBitFields; polling the full periodicReadFields set. " +
                    "If the MCU omits any of these fields the decoder will misalign — watch for isTruncated.",
            )
            return V1DataField.periodicReadFields
        }
        val filtered = V1DataField.periodicReadFields.filterTo(mutableSetOf()) { it.fieldIndex in supportedBitFields }
        val omitted = V1DataField.periodicReadFields - filtered
        if (omitted.isNotEmpty()) {
            logger.i(
                TAG,
                "Filtering ${omitted.size} unsupported field(s) from poll: ${omitted.joinToString { it.name }}",
            )
        }
        return filtered
    }

    /**
     * Brings the console up to the workout-active state the way the firmware expects. Two paths,
     * because treadmills and aerobic machines have fundamentally different start safety:
     *
     * - **Treadmill / incline trainer**: arm the console in WARM_UP and stop. Writing
     *   `WORKOUT_MODE=RUNNING` from the app does *not* move the belt — the MCU gates belt motion
     *   on a rising edge of the read-only `START_REQUESTED` telemetry (set when the user presses
     *   the physical Start key). Writing RUNNING anyway would just time out the confirmation poll
     *   and surface as a (semantically wrong) "console didn't confirm the workout started"
     *   degraded warning. Instead, the orchestrator parks in
     *   [com.nettarion.hyperborea.core.orchestration.OrchestratorState.AwaitingConsoleStart] and
     *   the running [pollOnce] loop picks up `WORKOUT_MODE=RUNNING` once the MCU completes the
     *   transition.
     * - **Bike / elliptical / rower**: drive the state machine ourselves —
     *   `IDLE → WARM_UP(10) → RUNNING(2)` with confirmation polling. `IDLE_MODE_LOCKOUT` must be
     *   disabled immediately before writing RUNNING (the firmware refuses the RUNNING transition
     *   while idle-mode is locked, even though we needed it locked through [prepareConsole] for
     *   the streaming-without-auto-pause behaviour). If the MCU never confirms a step we log a
     *   warning and continue degraded — that warning is the thing to look for in logs when
     *   resistance / speed controls don't respond.
     */
    private suspend fun transitionToActive() {
        if (detectedDeviceType == DeviceType.TREADMILL) {
            val mode = writeAndConfirmWorkoutMode(WorkoutMode.WARM_UP) { it != WorkoutMode.IDLE }
            logger.i(TAG, "Console state: IDLE → ${mode ?: WorkoutMode.UNKNOWN} (awaiting physical Start key)")
            _degradedReason.value = null
            return
        }

        val supportsIdleLockout = supportedBitFields.isEmpty() ||
            V1DataField.IDLE_MODE_LOCKOUT.fieldIndex in supportedBitFields
        if (supportsIdleLockout) {
            writeConsoleField(V1DataField.IDLE_MODE_LOCKOUT, FIELD_DISABLED)
        }
        writeAndConfirmWorkoutMode(WorkoutMode.WARM_UP) { it != WorkoutMode.IDLE }
        val running = writeAndConfirmWorkoutMode(WorkoutMode.RUNNING) { it == WorkoutMode.RUNNING }
        logger.i(TAG, "Console state: IDLE → WARM_UP → ${running ?: WorkoutMode.UNKNOWN}")
        _degradedReason.value =
            if (running == WorkoutMode.RUNNING) null
            else "The console didn't confirm the workout started — resistance/speed may not respond"
    }

    private suspend fun writeAndConfirmWorkoutMode(target: WorkoutMode, accept: (WorkoutMode) -> Boolean): WorkoutMode? {
        repeat((STATE_CONFIRM_TIMEOUT_MS / STATE_CONFIRM_POLL_MS).toInt()) { attempt ->
            // Assert the target on the first attempt; subsequent attempts just poll the read-back.
            val response = sendReadWrite(
                writeFields = if (attempt == 0) mapOf(V1DataField.WORKOUT_MODE to target.raw) else emptyMap(),
                readFields = setOf(V1DataField.WORKOUT_MODE),
            )
            val mode = response?.fields?.get(V1DataField.WORKOUT_MODE)?.let { WorkoutMode.fromRaw(it) }
            if (mode != null && accept(mode)) return mode
            delay(STATE_CONFIRM_POLL_MS)
        }
        logger.w(TAG, "Console didn't reach $target — workout may be inactive; continuing")
        return null
    }

    /**
     * Sends one ReadWriteData (writes [writeFields], requests [readFields]) and decodes the single-
     * packet response, returning it (or null on no/garbled response). Used for the startup-field read
     * and the workout-state writes/confirmations — not the poll loop, which reads the multi-packet
     * [V1DataField.periodicReadFields] via [pollOnce].
     */
    private suspend fun sendReadWrite(
        writeFields: Map<V1DataField, Float> = emptyMap(),
        readFields: Set<V1DataField> = emptySet(),
    ): V1Message.Incoming.DataResponse? {
        writeMessage(V1Message.Outgoing.ReadWriteData(writeFields = writeFields, readFields = readFields))
        delay(READ_DELAY_MS)
        val raw = readPacketOrNull() ?: return null
        return V1Codec.decodeSingleDataResponse(raw, readFields.ifEmpty { V1DataField.periodicReadFields })
    }

    private suspend fun writeMessage(message: V1Message.Outgoing) {
        for (packet in V1Codec.encode(message)) transport.write(packet)
    }

    private fun startPollLoop() {
        pollJob = scope.launch {
            while (isActive && _sessionState.value is SessionState.Streaming) {
                val calibDeferred = pendingCalibration
                if (calibDeferred != null) {
                    pendingCalibration = null
                    try {
                        runCalibration()
                        calibDeferred.complete(Unit)
                    } catch (e: CancellationException) {
                        calibDeferred.cancel(e)
                        throw e
                    } catch (e: Exception) {
                        calibDeferred.completeExceptionally(e)
                    }
                } else {
                    try {
                        pollOnce()
                    } catch (e: CancellationException) {
                        throw e
                    } catch (e: Exception) {
                        consecutivePollErrors++
                        logger.w(TAG, "Poll error ($consecutivePollErrors/$MAX_CONSECUTIVE_POLL_ERRORS): ${e.message}")
                        if (consecutivePollErrors >= MAX_CONSECUTIVE_POLL_ERRORS) {
                            _sessionState.value = SessionState.Error("Repeated poll failures", e)
                        }
                    }
                }
                delay(POLL_INTERVAL_MS)
            }

            if (_sessionState.value is SessionState.Streaming) {
                logger.w(TAG, "Poll loop ended unexpectedly")
                _sessionState.value = SessionState.Disconnected
            }
        }
    }

    private suspend fun pollOnce() {
        val writeFields: Map<V1DataField, Float>
        pendingWriteMutex.withLock {
            writeFields = pendingWriteFields
            pendingWriteFields = emptyMap()
        }

        // ReadWriteData targets DEVICE_MAIN (0x02) — FITNESS_BIKE (0x07) returns DEV_NOT_SUPPORTED.
        // pollFields is periodicReadFields ∩ supportedBitFields so the response payload size matches
        // the decoder's positional read; see V1Codec.decodeDataResponseForFields.
        writeMessage(V1Message.Outgoing.ReadWriteData(
            writeFields = writeFields,
            readFields = pollFields,
        ))

        delay(READ_DELAY_MS)

        // No timeout here on purpose: the poll loop is steady-state and just waits for the next reply.
        val firstPacket = transport.readPacket()
        if (firstPacket == null) {
            if (writeFields.isNotEmpty()) {
                pendingWriteMutex.withLock {
                    pendingWriteFields = writeFields + pendingWriteFields
                }
            }
            return
        }

        val decoded = try {
            readResponse(firstPacket, pollFields)
        } catch (e: Exception) {
            logger.w(TAG, "Malformed response (${firstPacket.size} bytes): ${e.message}")
            // Re-queue write fields so commands aren't lost
            if (writeFields.isNotEmpty()) {
                pendingWriteMutex.withLock {
                    pendingWriteFields = writeFields + pendingWriteFields
                }
            }
            return
        }
        if (decoded is V1Message.Incoming.DataResponse) {
            if (decoded.status == V1Message.STATUS_SECURITY_BLOCK) {
                logger.w(TAG, "Security block — re-verifying")
                if (writeFields.isNotEmpty()) {
                    pendingWriteMutex.withLock {
                        pendingWriteFields = writeFields + pendingWriteFields
                    }
                }
                verifySecurity()
                return
            }
            if (decoded.status != V1Message.STATUS_DONE) {
                return
            }

            if (decoded.fields.isEmpty()) {
                logger.w(TAG, "DataResponse OK but empty fields (payload size mismatch)")
                return
            }

            // Edge-triggered: log once when the response shape stops matching the request shape,
            // not every 100ms. Means the MCU is supplying a different field set than its DeviceInfo
            // bitmask declared — keep going (lenient decode produced what it could) but surface it.
            if (decoded.isTruncated && !lastTruncatedSeen) {
                logger.w(
                    TAG,
                    "DataResponse payload size doesn't match the requested ${pollFields.size}-field shape " +
                        "(decoded ${decoded.fields.size} field(s)) — later field offsets may be unreliable.",
                )
            } else if (!decoded.isTruncated && lastTruncatedSeen) {
                logger.i(TAG, "DataResponse payload size now matches the requested field shape again.")
            }
            lastTruncatedSeen = decoded.isTruncated

            applyDataResponse(decoded.fields)
            handleKeyObject(decoded.keyObject)
            estimatePowerIfNeeded()
            consecutivePollErrors = 0
            _exerciseData.value = accumulator.snapshot()

            val now = System.currentTimeMillis()
            if (now - lastLogTimeMs >= 1000L) {
                lastLogTimeMs = now
                val snap = _exerciseData.value
                if (snap != null) {
                    logger.d(TAG, "power=${snap.power}W cadence=${snap.cadence}rpm speed=${snap.speed}kph resistance=${snap.resistance} incline=${snap.incline}%")
                }
            }
        }
    }

    private fun applyDataResponse(fields: Map<V1DataField, Float>) {
        for ((field, value) in fields) {
            when (field) {
                V1DataField.WATTS -> accumulator.updatePower(value.toInt())
                V1DataField.RPM -> {
                    val prev = accumulator.snapshot().cadence
                    accumulator.updateCadence(value.toInt())
                    if ((prev == null || prev == 0) && value.toInt() > 0) {
                        logger.d(TAG, "Cadence went non-zero: ${value.toInt()} rpm")
                    }
                }
                // Speed source is device-type-dependent: belt machines report belt speed in KPH
                // (ACTUAL_KPH stays 0); other machines report a virtual speed in ACTUAL_KPH and
                // leave KPH as an unused setpoint (a bike has no commandable speed), so we don't
                // surface it as a target — no meaningless blue speed arrow on a bike.
                V1DataField.ACTUAL_KPH -> if (!detectedDeviceType.isBeltBased) accumulator.updateSpeed(value)
                V1DataField.KPH -> if (detectedDeviceType.isBeltBased) accumulator.updateSpeed(value)
                V1DataField.RESISTANCE -> accumulator.updateResistance(resistance.rawToLevel(value.toInt()))
                V1DataField.ACTUAL_INCLINE -> accumulator.updateIncline(value)
                V1DataField.GRADE -> accumulator.updateTargetIncline(value)
                // Grip HR is a noisy analog contact reading — gate + smooth it, and clear (null) on
                // contact loss. External BLE HRMs bypass this and are merged in the orchestrator.
                V1DataField.PULSE -> accumulator.updateHeartRate(gripHeartRate.update(value.toInt()))
                // CURRENT_DISTANCE is meters on the wire, but ExerciseData.distance — and every
                // consumer (FTMS ×1000→m, dashboard "KM", ride recorder distanceKm) — is kilometers.
                // Convert here, or distance reads 1000× high.
                V1DataField.CURRENT_DISTANCE -> accumulator.updateDistance(value / 1000f)
                V1DataField.CURRENT_CALORIES -> accumulator.updateCalories(value.toInt())
                V1DataField.CURRENT_TIME -> accumulator.updateElapsedTime(value.toLong())
                V1DataField.WORKOUT_MODE -> {
                    val mode = value.toInt()
                    val previousMode = accumulator.snapshot().workoutMode
                    if (previousMode != mode) {
                        when (WorkoutMode.fromRaw(mode)) {
                            WorkoutMode.PAUSE -> accumulator.pause()
                            WorkoutMode.RUNNING -> {
                                accumulator.resume()
                                accumulator.startTimer()
                            }
                            else -> {}
                        }
                    }
                    accumulator.updateWorkoutMode(mode)
                }
                V1DataField.VERTICAL_METER_GAIN -> accumulator.updateVerticalGain(value)
                V1DataField.VERTICAL_METER_NET -> accumulator.updateVerticalNet(value)
                V1DataField.AVERAGE_WATTS -> accumulator.updateAverageWatts(value.toInt())
                V1DataField.AVERAGE_GRADE -> accumulator.updateAverageIncline(value)
                V1DataField.LAP_TIME -> accumulator.updateLapTime(value.toLong())
                V1DataField.RECOVERABLE_PAUSED_TIME -> accumulator.updatePausedTime(value.toLong())
                V1DataField.START_REQUESTED -> accumulator.updateStartRequested(value.toInt() != 0)
                V1DataField.GOAL_TIME -> accumulator.updateGoalTime(value.toLong())
                V1DataField.STROKES -> accumulator.updateStrokeCount(value.toInt())
                V1DataField.STROKES_PER_MINUTE -> accumulator.updateStrokeRate(value.toInt())
                V1DataField.FIVE_HUNDRED_SPLIT -> accumulator.updateSplitTime(value.toInt())
                V1DataField.AVG_FIVE_HUNDRED_SPLIT -> accumulator.updateAvgSplitTime(value.toInt())
                // KEY_OBJECT is decoded onto DataResponse.keyObject and handled in handleKeyObject(),
                // so it never reaches this map — this case only keeps the `when` exhaustive.
                V1DataField.KEY_OBJECT,
                V1DataField.RUNNING_TIME,
                V1DataField.DISTANCE,
                V1DataField.CALORIES,
                V1DataField.MAX_RESISTANCE_LEVEL,
                V1DataField.WATT_GOAL,
                V1DataField.FAN_STATE,
                V1DataField.IDLE_MODE_LOCKOUT,
                V1DataField.REQUIRE_START_REQUESTED,
                V1DataField.VOLUME,
                V1DataField.GEAR,
                V1DataField.PAUSE_TIMEOUT,
                V1DataField.WARMUP_TIMEOUT,
                V1DataField.COOLDOWN_TIMEOUT,
                V1DataField.DISTANCE_GOAL,
                V1DataField.IS_CONSTANT_WATTS_MODE,
                V1DataField.MAX_GRADE,
                V1DataField.MIN_GRADE,
                V1DataField.MAX_KPH,
                V1DataField.MIN_KPH,
                V1DataField.MAX_PULSE,
                V1DataField.MAX_RPM,
                V1DataField.SYSTEM_UNITS,
                V1DataField.MOTOR_TOTAL_DISTANCE,
                V1DataField.TOTAL_TIME,
                V1DataField.IS_READY_TO_DISCONNECT -> { /* write-only, capability, or unprocessed fields */ }
            }
        }
    }

    /**
     * Emits a [ConsoleKey] on each fresh press of the console membrane keypad. KEY_OBJECT reports the
     * *currently-pressed* key (and 0 on release), so we edge-detect: emit when the code changes to a
     * new non-zero value. The equipment's own MCU acts on every one of these keys directly (changing
     * resistance/incline/speed, transitioning the workout state machine on START/STOP, etc.) and the
     * new state flows up through normal polling — so we don't drive anything from this stream, it's
     * pure UI / diagnostic plumbing.
     */
    private fun handleKeyObject(keyObject: KeyObject?) {
        val code = keyObject?.code ?: 0
        if (code == lastKeyCode) return
        lastKeyCode = code
        if (code == 0) return
        val key = fitProKeyToConsoleKey(code)
        logger.d(TAG, "Console keypad: code=$code held=${keyObject?.timeHeld ?: 0}ms${key?.let { " ($it)" } ?: ""}")
        key?.let { _consoleKeyPresses.tryEmit(it) }
    }

    private fun fitProKeyToConsoleKey(code: Int): ConsoleKey? = when (code) {
        KEY_START -> ConsoleKey.START
        KEY_STOP -> ConsoleKey.STOP
        KEY_SPEED_UP -> ConsoleKey.SPEED_UP
        KEY_SPEED_DOWN -> ConsoleKey.SPEED_DOWN
        KEY_INCLINE_UP -> ConsoleKey.INCLINE_UP
        KEY_INCLINE_DOWN -> ConsoleKey.INCLINE_DOWN
        // GEAR_UP/DOWN map to resistance — on bike consoles the +/- buttons are the resistance/gear
        // selector and there's no separate "gear" the app tracks.
        KEY_RESISTANCE_UP, KEY_GEAR_UP -> ConsoleKey.RESISTANCE_UP
        KEY_RESISTANCE_DOWN, KEY_GEAR_DOWN -> ConsoleKey.RESISTANCE_DOWN
        else -> null // fan / volume / etc. — not mapped (yet)
    }

    private fun estimatePowerIfNeeded() {
        val snapshot = accumulator.snapshot()
        if (snapshot.power != null && snapshot.power != 0) return // MCU provides power
        val speed = snapshot.speed ?: return
        val resistance = snapshot.resistance ?: return
        val maxRes = deviceInfo.maxResistance
        if (maxRes <= 0) return

        val estimated = powerCurveIndex?.let {
            PowerEstimator.estimate(it, speed, resistance, maxRes, deviceInfo.type)
        } ?: PowerEstimator.estimateFallback(speed, resistance, maxRes)

        if (estimated != null && estimated > 0) {
            accumulator.updatePower(estimated)
        }
    }

    private suspend fun runCalibration() {
        logger.i(TAG, "Starting incline calibration")
        var attempts = 0
        while (attempts < MAX_CALIBRATION_ATTEMPTS) {
            val response = sendAndAwait(V1Message.Outgoing.Calibrate())
            logger.d(TAG, "Calibration poll $attempts: $response")
            if (response is V1Message.Incoming.GenericResponse) {
                when (response.status) {
                    V1Message.STATUS_DONE -> {
                        logger.i(TAG, "Incline calibration complete")
                        return
                    }
                    V1Message.STATUS_IN_PROGRESS -> {
                        attempts++
                        delay(CALIBRATION_POLL_MS)
                    }
                    V1Message.STATUS_SECURITY_BLOCK -> {
                        logger.w(TAG, "Security block during calibration — re-verifying")
                        verifySecurity()
                        attempts++
                    }
                    else -> throw IllegalStateException("Calibration failed: status=${response.status}")
                }
            } else {
                throw IllegalStateException("Unexpected calibration response: $response")
            }
        }
        throw IllegalStateException("Calibration timed out after $MAX_CALIBRATION_ATTEMPTS attempts")
    }

    private suspend fun sendAndAwait(message: V1Message.Outgoing): V1Message.Incoming? {
        writeMessage(message)
        delay(READ_DELAY_MS)
        val firstPacket = readPacketOrNull() ?: return null
        return readResponse(firstPacket)
    }

    /** [transport.readPacket] with a safety timeout — a non-responsive MCU must not hang the session. */
    private suspend fun readPacketOrNull(): ByteArray? =
        withTimeoutOrNull(RESPONSE_TIMEOUT_MS) { transport.readPacket() }

    private suspend fun readResponse(
        firstPacket: ByteArray,
        dataResponseFields: Set<V1DataField>? = null,
    ): V1Message.Incoming? {
        if (V1Codec.isMultiPacketHeader(firstPacket)) {
            val expected = V1Codec.expectedPacketCount(firstPacket)
            val packets = mutableListOf(firstPacket)
            repeat(expected) {
                val dataPacket = transport.readPacket() ?: return null
                packets.add(dataPacket)
            }
            return V1Codec.decode(packets, dataResponseFields)
        }
        return V1Codec.decodeSingle(firstPacket, dataResponseFields)
    }

    private fun roundToStep(value: Float, step: Float): Float =
        (value / step).roundToInt() * step

    companion object {
        private const val TAG = "V1Session"
        private const val POLL_INTERVAL_MS = 100L
        private const val COMMAND_DELAY_MS = 100L
        private const val READ_DELAY_MS = 0L
        // Safety timeout for a single MCU response — it normally replies immediately; if it ever
        // doesn't, fail/degrade gracefully instead of hanging the session.
        private const val RESPONSE_TIMEOUT_MS = 1000L
        private const val MAX_CONSECUTIVE_POLL_ERRORS = 10
        private const val CALIBRATION_POLL_MS = 4000L // 4-second poll interval during calibration
        private const val MAX_CALIBRATION_ATTEMPTS = 60 // 4-minute timeout at 4s intervals

        // Confirming a WORKOUT_MODE transition: re-read WORKOUT_MODE every STATE_CONFIRM_POLL_MS,
        // for up to STATE_CONFIRM_TIMEOUT_MS, before giving up and continuing degraded.
        private const val STATE_CONFIRM_POLL_MS = 150L
        private const val STATE_CONFIRM_TIMEOUT_MS = 5_000L

        // Teardown: bound how long stop() waits for the poll loop to exit before touching the transport.
        private const val POLL_JOIN_TIMEOUT_MS = 500L

        // Belt-machine halt on stop: command KPH=0 + PAUSE, then poll the KPH read-back until it
        // reaches ~0 (confirming the MCU accepted the halt) before disconnecting.
        private const val BELT_HALT_CONFIRM_ATTEMPTS = 8
        private const val BELT_STOPPED_KPH = 0.1f

        // Graceful teardown: after the clean-end write, poll IS_READY_TO_DISCONNECT until the MCU
        // asserts it (BYTE field, so ~1 = ready), bounded so a wedged MCU can't hang teardown.
        private const val READY_TO_DISCONNECT_TRUE = 0.5f
        private const val READY_POLL_MS = 150L
        private const val READY_POLL_ATTEMPTS = 10

        // Console-init field values: 1 = ENABLED (REQUIRE_START_REQUESTED) / LOCKED (IDLE_MODE_LOCKOUT),
        // 0 = DISABLED / UNLOCKED.
        private const val FIELD_ENABLED = 1f
        private const val FIELD_DISABLED = 0f

        // KEY_OBJECT key codes for the console-keypad buttons we surface as [ConsoleKey] events.
        // Hyperborea acts on none of them directly — the MCU does the work and the resulting
        // state flows up through the WORKOUT_MODE poll.
        private const val KEY_STOP = 1
        private const val KEY_START = 2
        private const val KEY_SPEED_UP = 3
        private const val KEY_SPEED_DOWN = 4
        private const val KEY_INCLINE_UP = 5
        private const val KEY_INCLINE_DOWN = 6
        private const val KEY_RESISTANCE_UP = 7
        private const val KEY_RESISTANCE_DOWN = 8
        private const val KEY_GEAR_UP = 9
        private const val KEY_GEAR_DOWN = 10
    }
}
