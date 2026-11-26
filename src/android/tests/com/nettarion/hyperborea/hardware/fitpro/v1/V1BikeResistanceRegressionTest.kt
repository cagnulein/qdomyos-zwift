package com.nettarion.hyperborea.hardware.fitpro.v1

import org.junit.Assert.*
import org.junit.Test

class V1BikeResistanceRegressionTest {
    @Test fun stock24LevelScaling() {
        val converter = ResistanceConverter(24)
        assertEquals(0, converter.levelToRaw(0))
        assertEquals(415, converter.levelToRaw(1))
        assertEquals(4991, converter.levelToRaw(12))
        assertEquals(9983, converter.levelToRaw(24))
        for (level in 0..24) {
            assertEquals(level, converter.rawToLevel(converter.levelToRaw(level)))
        }
    }

    @Test fun exactDivisorAndUnknownLimit() {
        val converter = ResistanceConverter(20)
        assertEquals(9999, converter.levelToRaw(20))
        assertEquals(20, converter.rawToLevel(9999))
        assertEquals(0, ResistanceConverter(0).levelToRaw(0))
    }

    @Test fun gearWidthPreservesFollowingFields() {
        val payload = byteArrayOf(0, 0, 0, 0, 7, 24, 0, 1, 0x34, 0x12, 1)
        val fields = setOf(V1DataField.GEAR, V1DataField.WATT_GOAL, V1DataField.IS_CONSTANT_WATTS_MODE)
        val decoded = V1Codec.decodeDataResponse(V1Message.STATUS_DONE, payload, fields)
            as V1Message.Incoming.DataResponse
        assertFalse(decoded.isTruncated)
        assertEquals(7f, decoded.fields.getValue(V1DataField.GEAR), 0f)
        assertEquals(4660f, decoded.fields.getValue(V1DataField.WATT_GOAL), 0f)
        assertEquals(1f, decoded.fields.getValue(V1DataField.IS_CONSTANT_WATTS_MODE), 0f)
        val truncated = V1Codec.decodeDataResponse(V1Message.STATUS_DONE, payload.copyOf(10), fields)
            as V1Message.Incoming.DataResponse
        assertTrue(truncated.isTruncated)
        assertFalse(truncated.fields.containsKey(V1DataField.IS_CONSTANT_WATTS_MODE))
    }

    @Test fun manualExitIsRestrictedToSupportingBikes() {
        val mode = V1DataField.IS_CONSTANT_WATTS_MODE
        assertEquals(mapOf(mode to 0f), bikeManualErgExitFields(true, setOf(mode.fieldIndex)))
        assertTrue(bikeManualErgExitFields(false, setOf(mode.fieldIndex)).isEmpty())
        assertTrue(bikeManualErgExitFields(true, emptySet()).isEmpty())
        assertTrue(bikeManualErgExitFields(true, setOf(V1DataField.RESISTANCE.fieldIndex)).isEmpty())
    }
}
