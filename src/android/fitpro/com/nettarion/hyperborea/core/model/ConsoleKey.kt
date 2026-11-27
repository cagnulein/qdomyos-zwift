package com.nettarion.hyperborea.core.model

/**
 * A physical-console keypad press, abstracted away from the equipment-specific key codes. The
 * hardware adapter exposes a stream of these — one per press — on
 * [com.nettarion.hyperborea.core.adapter.HardwareAdapter.consoleKeyPresses].
 *
 * This is observe-only: the equipment's own MCU acts on these keys directly (changing
 * resistance/incline/speed, transitioning the workout state machine on START/STOP, etc.) and the
 * new state flows up through normal polling. Nothing in the Hyperborea pipeline drives the
 * hardware from this stream — it exists for UI/diagnostics.
 *
 * One firmware-level subtlety: on a FitPro V1 treadmill the MCU itself gates belt motion on the
 * physical [START] key (the rising edge of the read-only `START_REQUESTED` telemetry field, set
 * by the MCU when this key is pressed). Writing `WORKOUT_MODE=RUNNING` from the app alone will
 * not move the belt — the workout transitions to RUNNING only once the MCU has seen both the
 * mode write AND a Start-key press. The orchestrator models this with
 * [com.nettarion.hyperborea.core.orchestration.OrchestratorState.AwaitingConsoleStart].
 */
enum class ConsoleKey {
    START,
    STOP,
    RESISTANCE_UP,
    RESISTANCE_DOWN,
    INCLINE_UP,
    INCLINE_DOWN,
    SPEED_UP,
    SPEED_DOWN,
}
