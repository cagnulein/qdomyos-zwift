package com.nettarion.hyperborea.hardware.fitpro.v1

import com.nettarion.hyperborea.core.AppLogger
import com.nettarion.hyperborea.core.model.DeviceCommand
import com.nettarion.hyperborea.core.model.DeviceInfo
import com.nettarion.hyperborea.core.model.DeviceType
import com.nettarion.hyperborea.hardware.fitpro.session.PowerEstimator
import com.nettarion.hyperborea.hardware.fitpro.transport.HidTransport
import kotlin.coroutines.Continuation
import kotlin.coroutines.intrinsics.suspendCoroutineUninterceptedOrReturn
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.emptyFlow
import kotlinx.coroutines.runBlocking
import org.junit.Assert.*
import org.junit.Test

class V1S22iGearRegressionTest {
    private val gear = V1DataField.GEAR
    private val motor = V1DataField.RESISTANCE
    private val erg = V1DataField.IS_CONSTANT_WATTS_MODE
    private val controlReads = setOf(motor, V1DataField.WORKOUT_MODE, gear, erg)
    private val capturedGear = listOf<Byte>(7, 2, 3, 4, 1, 24, 6, 1)

    @Test fun s22iUsesClampedGearTargetsAndExitsErg() {
        val session = session(FakeController())
        set(session, "lastGearPayload", capturedGear)
        for (target in 1..24) {
            val fields = session.commandToFields(DeviceCommand.SetResistance(target))
            assertEquals(mapOf(gear to target.toFloat(), erg to 0f), fields)
            assertFalse(fields.containsKey(motor))
        }
        assertEquals(1f, session.commandToFields(DeviceCommand.SetResistance(0))[gear])
        assertEquals(24f, session.commandToFields(DeviceCommand.SetResistance(200))[gear])
    }

    @Test fun profileRequiresTheExactControllerAndDeclaredGearSupport() {
        val changes: List<Pair<String, Any>> = listOf(
            "partNumber" to 426459, "model" to 2121,
            "detectedDeviceType" to DeviceType.TREADMILL,
            "detectedDeviceType" to DeviceType.ROWER,
            "detectedDeviceType" to DeviceType.ELLIPTICAL,
            "capabilities" to V1Capabilities(equipmentDeviceId = 7, maxResistance = 24),
            "supportedBitFields" to setOf(motor.fieldIndex, erg.fieldIndex),
        )
        for ((field, value) in changes) {
            val session = session(FakeController())
            set(session, "lastGearPayload", capturedGear)
            set(session, field, value)
            val target = session.commandToFields(DeviceCommand.SetResistance(1))
            assertEquals("$field=$value", 415f, target[motor])
            assertFalse("$field=$value", target.containsKey(gear))
        }
    }

    @Test fun waitsForValidatedGearStateBeforeWriting() = runBlocking {
        val controller = FakeController()
        val session = session(controller)
        assertTrue(session.commandToFields(DeviceCommand.SetResistance(2)).isEmpty())
        controller.corruptNext = true
        poll(session)
        assertTrue(session.commandToFields(DeviceCommand.SetResistance(2)).isEmpty())
        poll(session)
        assertEquals(2f, session.commandToFields(DeviceCommand.SetResistance(2))[gear])
    }

    @Test fun realPollWritesGearOncePreservesMetadataAndReadsBackSelectedLevel() = runBlocking {
        val controller = FakeController()
        val session = session(controller)
        poll(session)
        assertEquals(1, session.exerciseData.value!!.resistance)
        set(session, "pendingWriteFields", session.commandToFields(DeviceCommand.SetResistance(2)))
        poll(session)
        assertEquals(2, session.exerciseData.value!!.resistance)
        assertEquals(0, get(session, "actualResistanceLevel"))
        assertEquals(capturedGear.toMutableList().also { it[4] = 2 }, controller.lastWrittenGear)
        assertEquals(0, controller.motorWrites)
        assertEquals(1, controller.gearWrites)
        assertEquals(0, controller.ergMode)
        poll(session)
        assertEquals(1, controller.gearWrites)
    }

    @Test fun physicalGearChangesAppearEvenWhenMotorResistanceIsUnchanged() = runBlocking {
        val controller = FakeController()
        val session = session(controller)
        poll(session)
        for (selected in 2..8) {
            controller.selectedGear = selected
            poll(session)
            assertEquals(selected, session.exerciseData.value!!.resistance)
            assertEquals(0, get(session, "actualResistanceLevel"))
        }
        controller.selectedGear = 0
        controller.rawResistance = 415
        poll(session)
        assertEquals(8, session.exerciseData.value!!.resistance)
        assertEquals(1, get(session, "actualResistanceLevel"))
    }

    @Test fun unknownBikeStillReportsMotorResistanceRatherThanGear() = runBlocking {
        val controller = FakeController().also { it.selectedGear = 8; it.rawResistance = 415 }
        val session = session(controller)
        set(session, "partNumber", 123456)
        poll(session)
        assertEquals(1, session.exerciseData.value!!.resistance)
        assertEquals(415f, session.commandToFields(DeviceCommand.SetResistance(1))[motor])
    }

    @Test fun powerFallbackUsesMotorResistanceRatherThanSelectedGear() = runBlocking {
        val controller = FakeController().also { it.selectedGear = 12 }
        val session = session(controller)
        set(session, "pollReadBatches", listOf(controlReads + setOf(
            V1DataField.WATTS, V1DataField.RPM, V1DataField.ACTUAL_KPH)))
        poll(session)
        assertEquals(12, session.exerciseData.value!!.resistance)
        val expected = PowerEstimator.estimateFallback(20f, 0, 24)
        assertNotEquals(expected, PowerEstimator.estimateFallback(20f, 12, 24))
        assertEquals(expected, session.exerciseData.value!!.power)
    }

    @Test fun gearCodecPreservesAllSevenOtherBytesAndDefaultEncoding() {
        val request = V1Message.Outgoing.ReadWriteData(writeFields = mapOf(gear to 9f), gearPayload = capturedGear)
        val frame = V1Codec.encode(request).single()
        val payloadOffset = 4 + (frame[3].toInt() and 0xff)
        assertEquals(capturedGear.toMutableList().also { it[4] = 9 },
            frame.copyOfRange(payloadOffset, payloadOffset + 8).toList())
        assertTrue(V1Codec.verifyChecksum(frame))
        val default = V1Codec.encode(request.copy(gearPayload = null)).single()
        assertEquals(listOf<Byte>(0, 0, 0, 0, 9, 0, 0, 0),
            default.copyOfRange(payloadOffset, payloadOffset + 8).toList())
    }

    @Test fun inclineAndPositivePowerCommandsKeepTheirOriginalMeaning() {
        val session = session(FakeController())
        assertEquals(mapOf(V1DataField.GRADE to 2.5f, erg to 0f),
            session.commandToFields(DeviceCommand.SetIncline(2.5f)))
        assertEquals(mapOf(V1DataField.WATT_GOAL to 150f, erg to 1f),
            session.commandToFields(DeviceCommand.SetTargetPower(150)))
    }

    private fun session(controller: FakeController): V1Session {
        val logger = object : AppLogger {
            override fun d(tag: String, message: String) {}
            override fun i(tag: String, message: String) {}
            override fun w(tag: String, message: String) {}
            override fun e(tag: String, message: String, throwable: Throwable?) {}
        }
        return V1Session(controller, logger, CoroutineScope(Job()), DeviceInfo.DEFAULT_INDOOR_BIKE).also {
            set(it, "partNumber", 426458); set(it, "model", 2120)
            set(it, "capabilities", V1Capabilities(equipmentDeviceId = 8, maxResistance = 24))
            set(it, "supportedBitFields", controlReads.map { field -> field.fieldIndex }.toSet())
            set(it, "pollReadBatches", listOf(controlReads))
        }
    }

    private fun set(session: V1Session, name: String, value: Any) {
        V1Session::class.java.getDeclaredField(name).also { it.isAccessible = true }.set(session, value)
    }
    private fun get(session: V1Session, name: String): Any? =
        V1Session::class.java.getDeclaredField(name).also { it.isAccessible = true }.get(session)
    private suspend fun poll(session: V1Session): Unit = suspendCoroutineUninterceptedOrReturn { continuation ->
        V1Session::class.java.getDeclaredMethod("pollOnceLocked", Continuation::class.java)
            .also { it.isAccessible = true }.invoke(session, continuation)
    }

    private inner class FakeController : HidTransport {
        override val isOpen = true
        var selectedGear = 1
        var rawResistance = 0
        var ergMode = 1
        var corruptNext = false
        var motorWrites = 0
        var gearWrites = 0
        var lastWrittenGear: List<Byte>? = null
        private var reply = byteArrayOf()
        override suspend fun open() {}
        override suspend fun close() {}
        override suspend fun clearBuffer() {}
        override fun incoming() = emptyFlow<ByteArray>()
        override suspend fun write(data: ByteArray) {
            assertTrue(data.size <= 64)
            val sections = data[3].toInt() and 0xff
            var offset = 4 + sections
            for (field in mask(data, 4, sections)) {
                when (field) {
                    V1DataField.RESISTANCE -> motorWrites++
                    V1DataField.GEAR -> {
                        lastWrittenGear = data.copyOfRange(offset, offset + 8).toList()
                        selectedGear = data[offset + 4].toInt() and 0xff
                        gearWrites++
                    }
                    V1DataField.IS_CONSTANT_WATTS_MODE -> ergMode = data[offset].toInt() and 0xff
                    else -> {}
                }
                offset += field.sizeBytes
            }
            val reads = mask(data, offset + 1, data[offset].toInt() and 0xff)
            reply = ByteArray(5 + reads.sumOf { it.sizeBytes })
            reply[0] = 8; reply[1] = reply.size.toByte(); reply[2] = 2; reply[3] = 2
            offset = 4
            for (field in reads) {
                when (field) {
                    V1DataField.RESISTANCE -> { reply[offset] = rawResistance.toByte(); reply[offset + 1] = (rawResistance shr 8).toByte() }
                    V1DataField.GEAR -> capturedGear.toByteArray().also { it[4] = selectedGear.toByte() }.copyInto(reply, offset)
                    V1DataField.WORKOUT_MODE -> reply[offset] = 2
                    V1DataField.IS_CONSTANT_WATTS_MODE -> reply[offset] = ergMode.toByte()
                    V1DataField.RPM -> reply[offset] = 70
                    V1DataField.ACTUAL_KPH -> { reply[offset] = 0xd0.toByte(); reply[offset + 1] = 7 }
                    else -> {}
                }
                offset += field.sizeBytes
            }
            reply[reply.lastIndex] = V1Codec.checksum(reply.copyOf(reply.lastIndex))
        }
        override suspend fun readPacket(): ByteArray = reply.copyOf(64).also {
            if (corruptNext) { it[4] = (it[4].toInt() xor 1).toByte(); corruptNext = false }
        }
        private fun mask(data: ByteArray, start: Int, count: Int): List<V1DataField> =
            V1DataField.entries.filter { it.fieldIndex / 8 < count &&
                (data[start + it.fieldIndex / 8].toInt() and (1 shl (it.fieldIndex % 8))) != 0 }
                .sortedBy { it.fieldIndex }
    }
}
