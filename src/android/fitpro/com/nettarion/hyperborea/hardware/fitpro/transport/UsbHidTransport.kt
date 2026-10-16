package com.nettarion.hyperborea.hardware.fitpro.transport

import android.hardware.usb.UsbDeviceConnection
import android.hardware.usb.UsbEndpoint
import android.hardware.usb.UsbInterface
import android.os.SystemClock
import com.nettarion.hyperborea.core.AppLogger
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.currentCoroutineContext
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.flowOn
import kotlinx.coroutines.isActive
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext

class UsbHidTransport(
    private val connection: UsbDeviceConnection,
    private val usbInterface: UsbInterface,
    private val inEndpoint: UsbEndpoint,
    private val outEndpoint: UsbEndpoint,
    private val logger: AppLogger,
) : HidTransport {

    @Volatile
    private var _isOpen = false
    override val isOpen: Boolean get() = _isOpen

    /**
     * FitPro MCUs behave as strict request/response peers. Keep IN and OUT transfers half-duplex so
     * a blocking read cannot overlap a write.
     */
    private val transferMutex = Mutex()

    override suspend fun open() {
        if (_isOpen) return
        claimInterfaceWithRetry()
        _isOpen = true
        logger.d(TAG, "Transport opened")
    }

    override suspend fun close() {
        if (!_isOpen) return
        _isOpen = false
        try {
            connection.releaseInterface(usbInterface)
            connection.close()
            logger.d(TAG, "Transport closed")
        } catch (e: Exception) {
            logger.w(TAG, "USB close error: ${e.message}")
        }
    }

    /** The MCU may intermittently refuse interface claims, so retry before giving up. */
    private suspend fun claimInterfaceWithRetry() {
        repeat(CLAIM_ATTEMPTS) { attempt ->
            if (connection.claimInterface(usbInterface, true)) {
                if (attempt > 0) {
                    logger.i(TAG, "Claimed USB interface after ${attempt + 1} attempts")
                }
                return
            }
            logger.w(TAG, "claimInterface failed (attempt ${attempt + 1}/$CLAIM_ATTEMPTS)")
            delay(CLAIM_RETRY_DELAY_MS)
        }
        throw IllegalStateException("Failed to claim USB interface after $CLAIM_ATTEMPTS attempts")
    }

    override suspend fun write(data: ByteArray) {
        if (!_isOpen) throw IllegalStateException("Transport not open")
        require(data.size <= MAX_PACKET_SIZE) { "Packet too large: ${data.size} > $MAX_PACKET_SIZE" }
        // Pad to 64 bytes because the MCU expects full-size USB packets.
        val padded = if (data.size < MAX_PACKET_SIZE) data.copyOf(MAX_PACKET_SIZE) else data

        // FitPro MCUs can refuse OUT transfers for short stretches. A single -1 is therefore not
        // fatal. Retry over a bounded window, and probe a possible halted OUT endpoint on the first
        // refusal.
        withContext(Dispatchers.IO) {
            var attempts = 0
            while (true) {
                if (!_isOpen) throw IllegalStateException("Transport closed during write")
                val transferred = transferMutex.withLock {
                    connection.bulkTransfer(outEndpoint, padded, padded.size, WRITE_TIMEOUT_MS)
                }
                if (transferred >= 0) {
                    if (attempts > 0) {
                        logger.i(TAG, "USB write succeeded after ${attempts + 1} attempts")
                    }
                    return@withContext
                }

                attempts++
                if (attempts >= WRITE_MAX_ATTEMPTS) {
                    throw IllegalStateException("USB write failed after $attempts attempts")
                }
                if (attempts == 1 || attempts % 10 == 0) {
                    logger.w(TAG, "USB write refused, retrying (attempt $attempts/$WRITE_MAX_ATTEMPTS)")
                }
                if (attempts == 1) {
                    val cleared = transferMutex.withLock { clearHalt(outEndpoint) }
                    logger.w(
                        TAG,
                        if (cleared) {
                            "OUT endpoint halt cleared"
                        } else {
                            "OUT endpoint clear-halt refused; device mute at control level"
                        },
                    )
                }
                delay(WRITE_RETRY_DELAY_MS)
            }
        }
    }

    /** Standard CLEAR_FEATURE(ENDPOINT_HALT) request. */
    private fun clearHalt(endpoint: UsbEndpoint): Boolean =
        connection.controlTransfer(
            0x02, // host-to-device | standard | endpoint recipient
            0x01, // CLEAR_FEATURE
            0x00, // ENDPOINT_HALT
            endpoint.address,
            null,
            0,
            CONTROL_TIMEOUT_MS,
        ) >= 0

    override suspend fun readPacket(): ByteArray? {
        if (!_isOpen) return null
        return withContext(Dispatchers.IO) {
            val buffer = ByteArray(MAX_PACKET_SIZE)
            val transferred = transferMutex.withLock {
                connection.bulkTransfer(inEndpoint, buffer, buffer.size, READ_TIMEOUT_MS)
            }
            if (transferred > 0) buffer.copyOf(transferred) else null
        }
    }

    override suspend fun clearBuffer() {
        if (!_isOpen) return
        val clearCmd = ByteArray(MAX_PACKET_SIZE).also { it[0] = 0xFF.toByte() }
        val readBuf = ByteArray(MAX_PACKET_SIZE)
        var consecutiveFf = 0
        var attempts = 0

        withContext(Dispatchers.IO) {
            while (consecutiveFf < 2 && attempts < MAX_CLEAR_ATTEMPTS) {
                transferMutex.withLock {
                    connection.bulkTransfer(outEndpoint, clearCmd, clearCmd.size, 500)
                }
                delay(50)
                val n = transferMutex.withLock {
                    connection.bulkTransfer(inEndpoint, readBuf, readBuf.size, 500)
                }
                if (n > 0 && readBuf[0] == 0xFF.toByte()) {
                    consecutiveFf++
                } else {
                    consecutiveFf = 0
                }
                attempts++
            }
        }

        logger.i(TAG, "Buffer cleared after $attempts attempts (ff=$consecutiveFf)")
    }

    override fun incoming(): Flow<ByteArray> = flow {
        val buffer = ByteArray(MAX_PACKET_SIZE)
        var failedPolls = 0
        while (currentCoroutineContext().isActive && _isOpen) {
            val pollStart = SystemClock.elapsedRealtime()
            val transferred = transferMutex.withLock {
                connection.bulkTransfer(inEndpoint, buffer, buffer.size, POLL_TIMEOUT_MS)
            }
            if (transferred > 0) {
                failedPolls = 0
                emit(buffer.copyOf(transferred))
            } else {
                // Android returns -1 both for a quiet poll timeout and for some endpoint errors.
                // Log prolonged silence and pace immediately failing reads so they do not spin.
                failedPolls++
                if (failedPolls % SILENT_POLL_LOG_EVERY == 0) {
                    logger.w(TAG, "No USB data for $failedPolls consecutive polls")
                }
                if (SystemClock.elapsedRealtime() - pollStart < POLL_TIMEOUT_MS / 2) {
                    delay(POLL_TIMEOUT_MS.toLong())
                }
            }
        }
    }.flowOn(Dispatchers.IO)

    private companion object {
        const val TAG = "UsbHidTransport"
        const val MAX_PACKET_SIZE = 64

        const val READ_TIMEOUT_MS = 1000
        const val POLL_TIMEOUT_MS = 50
        const val SILENT_POLL_LOG_EVERY = 200

        const val WRITE_TIMEOUT_MS = 50
        const val WRITE_MAX_ATTEMPTS = 50
        const val WRITE_RETRY_DELAY_MS = 20L

        const val CLAIM_ATTEMPTS = 20
        const val CLAIM_RETRY_DELAY_MS = 500L

        const val CONTROL_TIMEOUT_MS = 200
        const val MAX_CLEAR_ATTEMPTS = 10
    }
}
