package com.nettarion.hyperborea.hardware.fitpro.session

import com.nettarion.hyperborea.core.model.ConsoleKey
import com.nettarion.hyperborea.core.model.DeviceCommand
import com.nettarion.hyperborea.core.model.DeviceIdentity
import com.nettarion.hyperborea.core.model.DeviceType
import com.nettarion.hyperborea.core.model.ExerciseData
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.StateFlow

interface FitProSession {
    val exerciseData: StateFlow<ExerciseData?>
    val deviceIdentity: StateFlow<DeviceIdentity?>
    val sessionState: StateFlow<SessionState>

    /** Console membrane-keypad presses (resistance ± / incline ± / speed ±), one event per press. */
    val consoleKeyPresses: SharedFlow<ConsoleKey>

    /** Non-null while streaming but degraded — e.g. the console never confirmed the workout started. */
    val degradedReason: StateFlow<String?>

    /**
     * Equipment type as derived from the session's own handshake (V1: MCU-reported equipment id;
     * V2: supported-features heuristic). Defaults to [DeviceType.BIKE] before the handshake
     * resolves. The adapter reads this after [start] returns to refine the public [DeviceType] on
     * its [com.nettarion.hyperborea.core.adapter.HardwareAdapter.deviceInfo].
     */
    val detectedDeviceType: DeviceType

    suspend fun start()
    suspend fun stop()
    suspend fun identify(): DeviceIdentity?
    suspend fun calibrate()
    suspend fun writeFeature(command: DeviceCommand)
}

sealed interface SessionState {
    data object Connecting : SessionState
    data object Handshaking : SessionState
    data object Streaming : SessionState
    data object Disconnected : SessionState
    data class Error(val message: String, val cause: Throwable? = null) : SessionState
}
