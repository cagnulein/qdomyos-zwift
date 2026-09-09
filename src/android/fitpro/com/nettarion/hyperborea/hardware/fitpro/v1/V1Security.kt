package com.nettarion.hyperborea.hardware.fitpro.v1

/**
 * Security-hash calculation for the FitPro V1 handshake.
 */
object V1Security {

    fun calculateHash(serialNumber: Int, partNumber: Int, model: Int): ByteArray {
        val hash = ByteArray(32)
        for (i in 0 until 32) {
            val pos = (i + 1).toByte()
            hash[i] = if ((serialNumber shr i) and 1 == 1) {
                if (i < 16) {
                    (((partNumber shl 16) or (partNumber ushr 16)) shr i).toByte() xor pos
                } else {
                    (partNumber shr i).toByte() xor pos
                }
            } else {
                ((model + i) * pos.toInt()).toByte() xor pos
            }
        }
        return hash
    }
}

private infix fun Byte.xor(other: Byte): Byte = (this.toInt() xor other.toInt()).toByte()
