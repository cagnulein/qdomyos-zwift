package com.nettarion.hyperborea.core.model

data class DeviceIdentity(
    val serialNumber: String? = null,
    val firmwareVersion: String? = null,
    val hardwareVersion: String? = null,
    val model: String? = null,
    val partNumber: String? = null,
    val equipmentHours: Long? = null,
    val equipmentDistance: Float? = null,
)
