package com.nettarion.hyperborea.core.model

sealed interface DeviceCommand {
    data class SetResistance(val level: Int) : DeviceCommand
    data class SetIncline(val percent: Float) : DeviceCommand
    data class SetTargetSpeed(val kph: Float) : DeviceCommand
    data class SetTargetPower(val watts: Int) : DeviceCommand
    data class AdjustIncline(val increase: Boolean) : DeviceCommand
    data class AdjustSpeed(val increase: Boolean) : DeviceCommand
    data object PauseWorkout : DeviceCommand
    data object ResumeWorkout : DeviceCommand
    data object CalibrateIncline : DeviceCommand
    data class SetFanSpeed(val level: Int) : DeviceCommand
    data class SetVolume(val level: Int) : DeviceCommand
    data class SetGear(val gear: Int) : DeviceCommand
    data class SetDistanceGoal(val meters: Int) : DeviceCommand
    data class SetWarmupTimeout(val seconds: Int) : DeviceCommand
    data class SetCooldownTimeout(val seconds: Int) : DeviceCommand
    data class SetPauseTimeout(val seconds: Int) : DeviceCommand
    data class SetWarmUpMode(val enable: Boolean) : DeviceCommand
    data class SetCoolDownMode(val enable: Boolean) : DeviceCommand
    data class SetErgMode(val enable: Boolean) : DeviceCommand
}
