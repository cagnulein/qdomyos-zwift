package com.che.zap.proto

import com.google.protobuf.ByteString
import com.google.protobuf.CodedInputStream
import com.google.protobuf.InvalidProtocolBufferException
import com.google.protobuf.WireFormat

class BatteryStatus(message: ByteArray) {

    var level: Int = 0

    init {

        val input: CodedInputStream = CodedInputStream.newInstance(ByteString.copyFrom(message).asReadOnlyByteBuffer())
        while (true) {
            val tag = input.readTag()
            val type: Int = WireFormat.getTagWireType(tag)
            if (tag == 0 || type == WireFormat.WIRETYPE_END_GROUP)
                break
            val number: Int = WireFormat.getTagFieldNumber(tag)
            when (type) {
                WireFormat.WIRETYPE_VARINT -> {
                    val value = input.readInt64().toInt() // biggest number we expect is an int
                    when (number) {
                        2 -> level = value // tag starts at 2. I guess thats legal protobuf, you can remove fields after all
                        else -> throw InvalidProtocolBufferException("Unexpected tag")
                    }
                }
                else -> throw InvalidProtocolBufferException("Unexpected wire type")
            }
        }
    }

    override fun toString(): String {
        return "BatteryStatus(level=$level)"
    }

}