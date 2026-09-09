package com.nettarion.hyperborea.hardware.fitpro.v2

import java.nio.ByteBuffer
import java.nio.ByteOrder

object V2Codec {

    private const val COMM_TYPE: Byte = 0x02

    // Outgoing command types (V2Command)
    private const val CMD_SUBSCRIBE: Int = 0x01
    private const val CMD_WRITE: Int = 0x02
    private const val CMD_SUPPORTED_FEATURES: Int = 0x06
    private const val CMD_UNSUBSCRIBE: Int = 0x07

    // Incoming response types (V2ResponseType) — different numbering from commands
    private const val RSP_FEATURES: Int = 0x01
    private const val RSP_ACKNOWLEDGE: Int = 0x03
    private const val RSP_ERROR: Int = 0x04
    private const val RSP_EVENT: Int = 0x05

    fun encode(message: V2Message.Outgoing): ByteArray = when (message) {
        is V2Message.Outgoing.QueryFeatures -> encodePacket(
            source = message.source,
            type = CMD_SUPPORTED_FEATURES,
            payload = byteArrayOf(),
        )
        is V2Message.Outgoing.Subscribe -> encodePacket(
            source = message.source,
            type = CMD_SUBSCRIBE,
            payload = encodeFeatureList(message.features),
        )
        is V2Message.Outgoing.Unsubscribe -> encodePacket(
            source = message.source,
            type = CMD_UNSUBSCRIBE,
            payload = encodeFeatureList(message.features),
        )
        is V2Message.Outgoing.WriteFeature -> encodePacket(
            source = message.source,
            type = CMD_WRITE,
            payload = encodeFeatureValue(message.feature, message.value),
        )
    }

    fun decode(data: ByteArray): V2Message.Incoming? {
        if (data.size < 3) return null
        if (data[0] != COMM_TYPE) return null

        val sourceAndType = data[1].toInt() and 0xFF
        val type = sourceAndType and 0x0F
        val length = data[2].toInt() and 0xFF
        val payload = if (data.size > 3) data.copyOfRange(3, minOf(3 + length, data.size)) else byteArrayOf()

        return when (type) {
            RSP_FEATURES -> decodeSupportedFeatures(payload)
            RSP_ACKNOWLEDGE -> V2Message.Incoming.Acknowledge(sourceAndType)
            RSP_ERROR -> V2Message.Incoming.Error(if (payload.isNotEmpty()) payload[0].toInt() and 0xFF else 0)
            RSP_EVENT -> decodeEvent(payload)
            else -> V2Message.Incoming.Unknown(data.copyOf())
        }
    }

    private fun encodePacket(source: Int, type: Int, payload: ByteArray): ByteArray {
        val sourceType = ((source and 0x0F) shl 4) or (type and 0x0F)
        return byteArrayOf(
            COMM_TYPE,
            sourceType.toByte(),
            payload.size.toByte(),
            *payload,
        )
    }

    private fun encodeFeatureList(features: List<V2FeatureId>): ByteArray {
        val buf = ByteArray(features.size * 2)
        features.forEachIndexed { i, feature ->
            buf[i * 2] = feature.wireLo
            buf[i * 2 + 1] = feature.wireHi
        }
        return buf
    }

    private fun encodeFeatureValue(feature: V2FeatureId, value: Float): ByteArray {
        val buf = ByteBuffer.allocate(6).order(ByteOrder.LITTLE_ENDIAN)
        buf.put(feature.wireLo)
        buf.put(feature.wireHi)
        buf.putFloat(value)
        return buf.array()
    }

    private fun decodeEvent(payload: ByteArray): V2Message.Incoming {
        if (payload.size < 6) return V2Message.Incoming.Unknown(payload.copyOf())
        val feature = V2FeatureId.fromWireBytes(payload[0], payload[1])
            ?: return V2Message.Incoming.Unknown(payload.copyOf())
        val value = ByteBuffer.wrap(payload, 2, 4).order(ByteOrder.LITTLE_ENDIAN).float
        return V2Message.Incoming.Event(feature, value)
    }

    private fun decodeSupportedFeatures(payload: ByteArray): V2Message.Incoming {
        val features = mutableListOf<V2FeatureId>()
        var i = 0
        while (i + 1 < payload.size) {
            V2FeatureId.fromWireBytes(payload[i], payload[i + 1])?.let { features.add(it) }
            i += 2
        }
        return V2Message.Incoming.SupportedFeatures(features)
    }
}
