package com.che.zap.utils

import com.google.protobuf.ByteString
import com.google.protobuf.CodedInputStream
import com.google.protobuf.InvalidProtocolBufferException
import com.google.protobuf.WireFormat
import java.io.IOException
import java.nio.charset.StandardCharsets
import javax.xml.transform.OutputKeys

object ProtoDecode {

    // https://stackoverflow.com/a/48868239
    // note if this fails check there isn't a byte unrelated to the protobuf on the start...

    // https://stackoverflow.com/a/68088642 very useful also

    @Throws(IOException::class)
    fun decodeProto(data: ByteArray?, singleLine: Boolean): String {
        return decodeProto(ByteString.copyFrom(data), 0, singleLine)
    }

    @Throws(IOException::class)
    fun decodeProto(data: ByteString, depth: Int, singleLine: Boolean): String {
        val input: CodedInputStream = CodedInputStream.newInstance(data.asReadOnlyByteBuffer())
        return decodeProtoInput(input, depth, singleLine)
    }

    @Throws(IOException::class)
    private fun decodeProtoInput(input: CodedInputStream, depth: Int, singleLine: Boolean): String {
        val s = StringBuilder("{ ")
        var foundFields = false
        while (true) {
            val tag = input.readTag()
            val type: Int = WireFormat.getTagWireType(tag)
            if (tag == 0 || type == WireFormat.WIRETYPE_END_GROUP) {
                break
            }
            foundFields = true
            protoNewline(depth, s, singleLine)
            val number: Int = WireFormat.getTagFieldNumber(tag)
            s.append(number).append(": ")
            when (type) {
                WireFormat.WIRETYPE_VARINT -> s.append(input.readInt64())
                WireFormat.WIRETYPE_FIXED64 -> s.append(java.lang.Double.longBitsToDouble(input.readFixed64()))
                WireFormat.WIRETYPE_LENGTH_DELIMITED -> {
                    val data: ByteString = input.readBytes()
                    try {
                        val subMessage = decodeProto(data, depth + 1, singleLine)
                        if (data.size() < 30) {
                            var probablyString = true
                            val str = String(data.toByteArray(), StandardCharsets.UTF_8)
                            for (c in str.toCharArray()) {
                                if (c < '\n') {
                                    probablyString = false
                                    break
                                }
                            }
                            if (probablyString)
                                s.append("\"").append(str).append("\" ")
                        }
                        s.append(subMessage)
                    } catch (e: IOException) {
                        s.append('"').append(String(data.toByteArray())).append('"')
                    }
                }

                WireFormat.WIRETYPE_START_GROUP -> s.append(decodeProtoInput(input, depth + 1, singleLine))
                WireFormat.WIRETYPE_FIXED32 -> s.append(java.lang.Float.intBitsToFloat(input.readFixed32()))
                else -> throw InvalidProtocolBufferException("Invalid wire type")
            }
        }
        if (foundFields) {
            protoNewline(depth - 1, s, singleLine)
        }
        return s.append('}').toString()
    }

    private fun protoNewline(depth: Int, s: StringBuilder, noNewline: Boolean) {
        if (noNewline) {
            s.append(" ")
            return
        }
        s.append('\n')
        for (i in 0..depth) {
            s.append(OutputKeys.INDENT)
        }
    }
}

object ProtoUtils {

    fun getSignedValue(value: Int): Int {
        val negativeBit = value and 0b1
        val num = value shr 1
        return if (negativeBit == 1) -num else num
    }

}