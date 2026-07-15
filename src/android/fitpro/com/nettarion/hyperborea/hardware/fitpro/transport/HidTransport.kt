package com.nettarion.hyperborea.hardware.fitpro.transport

import kotlinx.coroutines.flow.Flow

interface HidTransport {
    suspend fun open()
    suspend fun close()
    suspend fun write(data: ByteArray)
    fun incoming(): Flow<ByteArray>
    /** Read a single USB packet, returning null on timeout. */
    suspend fun readPacket(): ByteArray?
    /** Clear stale data from USB buffer before init. */
    suspend fun clearBuffer()
    val isOpen: Boolean
}
