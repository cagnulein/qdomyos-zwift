package com.che.zap.utils

import java.util.UUID

object BleUuids {

    private const val BT_SIG_UUID_PREFIX = "0000"
    private const val BT_SIG_UUID_SUFFIX = "-0000-1000-8000-00805F9B34FB"

    fun uuidFromShortString(shortUuid: String): UUID {
        return UUID.fromString("$BT_SIG_UUID_PREFIX$shortUuid$BT_SIG_UUID_SUFFIX")
    }
}