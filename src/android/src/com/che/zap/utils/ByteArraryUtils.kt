package com.che.zap.utils

const val PREFIX = "0x"

fun ByteArray.toHexString(): String {
    val result = asUByteArray().joinToString(" $PREFIX", prefix = PREFIX, postfix = " ") { it.toHex() }
    if (result != "$PREFIX ")
        return result
    return ""
}

fun UByte.toHexString(): String {
    val result = this.toHex()
    return "$PREFIX$result"
}

fun UByte.toHex(): String {
    return this.toString(16).uppercase().padStart(2, '0')
}

fun ByteArray.startsWith(otherByteArray: ByteArray): Boolean {
    if (this.size < otherByteArray.size) return false
    for ((index, byte) in otherByteArray.withIndex()) {
        if (this[index] != byte) return false
    }
    return true
}