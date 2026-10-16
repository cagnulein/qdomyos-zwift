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

        /** Console rejection. [command] is the rejected outgoing command id; [code] is the reason. */
        data class Error(val command: Int, val code: Int) : Incoming {
            fun describe(): String = "command=0x%02x reason=%s".format(command, describeCode(code))

            companion object {
                fun describeCode(code: Int): String = when (code) {
                    0 -> "NO_ERROR"
                    1 -> "UNASSIGNED"
                    2 -> "FRAMING"
                    3 -> "FEATURES_NOT_SUPPORTED"
                    4 -> "WRITE_NOT_SUPPORTED"
                    5 -> "DATA_OUT_OF_RANGE"
                    6 -> "COMMAND_NOT_SUPPORTED"
                    7 -> "WRITE_VALUE_NOT_ALLOWED"
                    else -> "UNKNOWN($code)"
                }
            }
        }

        data class Event(val feature: V2FeatureId, val value: Float) : Incoming

        /**
         * One frame of the console's supported-features list. The list is complete only after an
         * empty terminator frame. Unknown feature ids still count as list content.
         */
        data class SupportedFeatures(
            val features: List<V2FeatureId>,
            val unknownCodes: List<Int> = emptyList(),
        ) : Incoming {
            val isEndOfList: Boolean get() = features.isEmpty() && unknownCodes.isEmpty()
        }

        data class Unknown(val raw: ByteArray) : Incoming {
            override fun equals(other: Any?) = other is Unknown && raw.contentEquals(other.raw)
            override fun hashCode() = raw.contentHashCode()
        }
    }

    companion object {
        const val SOURCE_APP = 0x00
    }
}
