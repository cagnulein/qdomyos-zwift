package com.nettarion.hyperborea.hardware.fitpro.v2

sealed interface V2Message {

    sealed interface Outgoing : V2Message {
        data class QueryFeatures(val source: Int = SOURCE_APP) : Outgoing
        data class Subscribe(val features: List<V2FeatureId>, val source: Int = SOURCE_APP) : Outgoing
        data class Unsubscribe(val features: List<V2FeatureId>, val source: Int = SOURCE_APP) : Outgoing
        data class WriteFeature(val feature: V2FeatureId, val value: Float, val source: Int = SOURCE_APP) : Outgoing
    }

    sealed interface Incoming : V2Message {
        data class Acknowledge(val type: Int) : Incoming
        data class Error(val code: Int) : Incoming
        data class Event(val feature: V2FeatureId, val value: Float) : Incoming
        data class SupportedFeatures(val features: List<V2FeatureId>) : Incoming
        data class Unknown(val raw: ByteArray) : Incoming {
            override fun equals(other: Any?) = other is Unknown && raw.contentEquals(other.raw)
            override fun hashCode() = raw.contentHashCode()
        }
    }

    companion object {
        const val SOURCE_APP = 0x00
    }
}
