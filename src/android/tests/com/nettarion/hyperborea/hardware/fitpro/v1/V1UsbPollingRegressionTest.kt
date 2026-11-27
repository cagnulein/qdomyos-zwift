package com.nettarion.hyperborea.hardware.fitpro.v1

import com.nettarion.hyperborea.core.AppLogger
import com.nettarion.hyperborea.core.model.DeviceInfo
import com.nettarion.hyperborea.hardware.fitpro.transport.HidTransport
import kotlin.coroutines.Continuation
import kotlin.coroutines.intrinsics.suspendCoroutineUninterceptedOrReturn
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.emptyFlow
import kotlinx.coroutines.runBlocking
import org.junit.Assert.*
import org.junit.Test

class V1UsbPollingRegressionTest {
    // The 30-field request in Scott's Nov 26 log, before splitting.
    private val scottFields = V1DataField.periodicReadFields - setOf(
        V1DataField.STROKES, V1DataField.STROKES_PER_MINUTE,
        V1DataField.FIVE_HUNDRED_SPLIT, V1DataField.AVG_FIVE_HUNDRED_SPLIT,
    )
    private val controls = setOf(V1DataField.RESISTANCE, V1DataField.GEAR,
        V1DataField.WORKOUT_MODE, V1DataField.IS_CONSTANT_WATTS_MODE)

    @Test fun scottRequestFitsUsbWithoutLosingTailFields() {
        assertEquals(92, 5 + scottFields.sumOf { it.sizeBytes })
        val batches = bikeUsbReadBatches(scottFields)
        assertEquals(controls, batches.first())
        assertEquals(scottFields, batches.flatten().toSet())
        assertEquals(scottFields.size, batches.sumOf { it.size })
        for (batch in batches) {
            assertTrue(5 + batch.sumOf { it.sizeBytes } <= 64)
            val request = V1Codec.encode(V1Message.Outgoing.ReadWriteData(readFields = batch))
            assertEquals(1, request.size)
            assertTrue(request.single().size <= 64)
        }
    }

    @Test fun subsetsAndUnknownCapabilitiesKeepEveryRequestedFieldOnce() {
        for (fields in listOf(emptySet(), controls, setOf(V1DataField.RPM), V1DataField.periodicReadFields)) {
            val batches = bikeUsbReadBatches(fields)
            assertEquals(fields, batches.flatten().toSet())
            assertEquals(fields.size, batches.sumOf { it.size })
            assertTrue(batches.all { it.isNotEmpty() && 5 + it.sumOf { f -> f.sizeBytes } <= 64 })
        }
    }

    @Test fun controlReadbackIncludesResistanceGearAndErgFlag() {
        val reply = response(controls)
        val decoded = V1Codec.decodeUsbDataResponse(reply, controls)!!
        assertFalse(decoded.isTruncated)
        assertEquals(415f, decoded.fields.getValue(V1DataField.RESISTANCE), 0f)
        assertEquals(6f, decoded.fields.getValue(V1DataField.GEAR), 0f)
        assertEquals(0f, decoded.fields.getValue(V1DataField.IS_CONSTANT_WATTS_MODE), 0f)
    }

    @Test fun acceptsUsbPaddingButRejectsChecksumLengthAndOpcodeErrors() {
        val good = response(controls)
        assertNotNull(V1Codec.decodeUsbDataResponse(good.copyOf(64), controls))
        assertNull(V1Codec.decodeUsbDataResponse(good.copyOf(good.size - 1), controls))
        val corrupt = good.copyOf().also { it[4] = (it[4].toInt() xor 1).toByte() }
        assertNull(V1Codec.decodeUsbDataResponse(corrupt, controls))
        val wrongOpcode = good.copyOf().also { it[2] = 0x81.toByte(); checksum(it) }
        assertNull(V1Codec.decodeUsbDataResponse(wrongOpcode, controls))
        assertNull(V1Codec.decodeUsbDataResponse(response(controls - V1DataField.GEAR), controls))
    }

    @Test fun rejectsTheOldOversizedReplyAndDoesNotPublishItsPrefix() {
        val full = response(scottFields)
        assertEquals(92, full.size)
        assertNull(V1Codec.decodeUsbDataResponse(full.copyOf(64), scottFields))
        // Even a checksum-correct short frame cannot masquerade as the 92-byte response.
        val short = full.copyOf(64).also { it[1] = 64; checksum(it) }
        assertNull(V1Codec.decodeUsbDataResponse(short, scottFields))
        // Existing lenient codec remains available to other V1 equipment.
        val legacy = V1Codec.decodeSingleDataResponse(short, scottFields)!!
        assertTrue(legacy.isTruncated)
        assertEquals(15, legacy.fields.size)
    }

    @Test fun securityErrorsArePreservedWithoutTreatingThemAsTelemetry() {
        val error = byteArrayOf(8, 5, 2, 8, 0).also { checksum(it) }
        val decoded = V1Codec.decodeUsbDataResponse(error, controls)!!
        assertEquals(V1Message.STATUS_SECURITY_BLOCK, decoded.status)
        assertTrue(decoded.fields.isEmpty())
    }

    @Test fun realSessionPollSendsCommandOnceAndRetainsMetricsAcrossBatches() = runBlocking {
        val transport = FakeUsb()
        val session = session(transport)
        setField(session, "pendingWriteFields", mapOf(V1DataField.RESISTANCE to 415f,
            V1DataField.IS_CONSTANT_WATTS_MODE to 0f))
        poll(session)
        assertEquals(1, transport.writesWithData)
        assertEquals(80, session.exerciseData.value!!.cadence)
        assertEquals(150, session.exerciseData.value!!.power)
        assertEquals(1, session.exerciseData.value!!.resistance)
        poll(session)
        assertEquals(1, transport.writesWithData)
        assertTrue(transport.replyLengths.all { it <= 64 })
    }

    @Test fun corruptReplyDoesNotChangeMetricsAndQueuedCommandIsRetried() = runBlocking {
        val transport = FakeUsb()
        val session = session(transport)
        poll(session)
        val previous = session.exerciseData.value
        setField(session, "pendingWriteFields", mapOf(V1DataField.RESISTANCE to 415f))
        transport.corruptNext = true
        poll(session)
        assertEquals(previous, session.exerciseData.value)
        poll(session)
        assertEquals(2, transport.writesWithData)
        assertEquals(80, session.exerciseData.value!!.cadence)
    }

    private fun session(transport: FakeUsb): V1Session {
        val logger = object : AppLogger {
            override fun d(tag: String, message: String) {}
            override fun i(tag: String, message: String) {}
            override fun w(tag: String, message: String) {}
            override fun e(tag: String, message: String, throwable: Throwable?) {}
        }
        return V1Session(transport, logger, CoroutineScope(Job()), DeviceInfo.DEFAULT_INDOOR_BIKE).also {
            setField(it, "pollReadBatches", bikeUsbReadBatches(scottFields))
        }
    }

    private fun setField(session: V1Session, name: String, value: Any) {
        V1Session::class.java.getDeclaredField(name).also { it.isAccessible = true }.set(session, value)
    }

    // Exercise the production transaction loop without starting a background job or moving hardware.
    private suspend fun poll(session: V1Session): Unit = suspendCoroutineUninterceptedOrReturn { continuation ->
        V1Session::class.java.getDeclaredMethod("pollOnceLocked", Continuation::class.java)
            .also { it.isAccessible = true }.invoke(session, continuation)
    }

    private inner class FakeUsb : HidTransport {
        override val isOpen = true
        var writesWithData = 0
        var corruptNext = false
        val replyLengths = mutableListOf<Int>()
        private var reply = byteArrayOf()
        override suspend fun open() {}
        override suspend fun close() {}
        override suspend fun clearBuffer() {}
        override fun incoming() = emptyFlow<ByteArray>()
        override suspend fun write(data: ByteArray) {
            assertTrue(data.size <= 64)
            assertEquals(2, data[2].toInt())
            val writeSections = data[3].toInt() and 0xFF
            val written = fieldsFromMask(data, 4, writeSections)
            if (written.isNotEmpty()) writesWithData++
            val readSectionOffset = 4 + writeSections + written.sumOf { it.sizeBytes }
            val readSections = data[readSectionOffset].toInt() and 0xFF
            val reads = fieldsFromMask(data, readSectionOffset + 1, readSections)
            reply = response(reads)
            replyLengths.add(reply.size)
        }
        override suspend fun readPacket(): ByteArray = reply.copyOf(64).also {
            if (corruptNext) { it[4] = (it[4].toInt() xor 1).toByte(); corruptNext = false }
        }
    }

    private fun fieldsFromMask(data: ByteArray, offset: Int, sections: Int): Set<V1DataField> =
        V1DataField.entries.filterTo(linkedSetOf()) {
            it.fieldIndex / 8 < sections &&
                (data[offset + it.fieldIndex / 8].toInt() and (1 shl (it.fieldIndex % 8))) != 0
        }

    private fun response(fields: Set<V1DataField>): ByteArray {
        val frame = ByteArray(5 + fields.sumOf { it.sizeBytes })
        frame[0] = 8; frame[1] = frame.size.toByte(); frame[2] = 2; frame[3] = 2
        var offset = 4
        for (field in fields.sortedBy { it.fieldIndex }) {
            when (field) {
                V1DataField.RESISTANCE -> { frame[offset] = 0x9f.toByte(); frame[offset + 1] = 1 }
                V1DataField.GEAR -> frame[offset + 4] = 6
                V1DataField.WORKOUT_MODE -> frame[offset] = 2
                V1DataField.WATTS -> frame[offset] = 150.toByte()
                V1DataField.RPM -> frame[offset] = 80
                else -> {}
            }
            offset += field.sizeBytes
        }
        checksum(frame)
        return frame
    }

    private fun checksum(frame: ByteArray) {
        frame[frame.lastIndex] = V1Codec.checksum(frame.copyOf(frame.lastIndex))
    }
}
