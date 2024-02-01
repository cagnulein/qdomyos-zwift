package com.che.zap.proto

import com.che.zap.utils.ProtoUtils.getSignedValue
import com.google.protobuf.ByteString
import com.google.protobuf.CodedInputStream
import com.google.protobuf.InvalidProtocolBufferException
import com.google.protobuf.WireFormat

class ControllerNotification(message: ByteArray) {

    companion object {
        const val BTN_PRESSED = 0

        private const val SHOULDER_NAME = "Shoulder"
        private const val POWER_NAME = "Power"
        private const val STEER_NAME = "Steer/Brake"
        private const val UNKNOWN_NAME = "???"
    }

    var isRightController = false

    var buttonYPressed = false // or up on left controller
    var buttonZPressed = false // or left on left controller
    var buttonAPressed = false // or right on left controller
    var buttonBPressed = false // or down on left controller

    var shoulderButtonPressed = false
    var powerButtonPressed = false

    // on the left this will be negative when steering and positive when breaking and vice versa on right
    var steerBrakeValue = 0

    var somethingValue = 0

    init {
        // Taking ProtoDecode and simplifying it as we know the fields are probably enums and varints, (which are both varint wiretypes).

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
                    // NOTE: this is with 1.2.1 firmware. this could change...
                    when (number) {
                        1 -> isRightController = value == BTN_PRESSED
                        2 -> buttonYPressed = value == BTN_PRESSED
                        3 -> buttonZPressed = value == BTN_PRESSED
                        4 -> buttonAPressed = value == BTN_PRESSED
                        5 -> buttonBPressed = value == BTN_PRESSED
                        6 -> shoulderButtonPressed = value == BTN_PRESSED
                        7 -> powerButtonPressed = value == BTN_PRESSED
                        8 -> steerBrakeValue = getSignedValue(value)
                        9 -> somethingValue = value
                        else -> throw InvalidProtocolBufferException("Unexpected tag") // firmware change perhaps?
                    }
                }
                else -> throw InvalidProtocolBufferException("Unexpected wire type")
            }
        }
    }

    fun diff(previousNotification: ControllerNotification): String {
        var diff = ""
        diff += diff(nameY(), buttonYPressed, previousNotification.buttonYPressed)
        diff += diff(nameZ(), buttonZPressed, previousNotification.buttonZPressed)
        diff += diff(nameA(), buttonAPressed, previousNotification.buttonAPressed)
        diff += diff(nameB(), buttonBPressed, previousNotification.buttonBPressed)
        diff += diff(SHOULDER_NAME, shoulderButtonPressed, previousNotification.shoulderButtonPressed)
        diff += diff(POWER_NAME, powerButtonPressed, previousNotification.powerButtonPressed)
        diff += diff(STEER_NAME, steerBrakeValue, previousNotification.steerBrakeValue)
        diff += diff(UNKNOWN_NAME, somethingValue, previousNotification.somethingValue)
        return diff
    }

    private fun diff(title: String, pressedValue: Boolean, oldPressedValue: Boolean): String {
        if (pressedValue != oldPressedValue)
            return "$title=${if (pressedValue) "Pressed" else "Released"} "
        return ""
    }

    private fun diff(title: String, newValue: Int, oldValue: Int): String {
        if (newValue != oldValue)
            return "$title=$newValue "
        return ""
    }

    private fun nameController(): String = if (isRightController) "Right" else "Left"

    private fun nameY(): String = if (isRightController) "Y" else "Up"

    private fun nameZ(): String = if (isRightController) "Z" else "Left"

    private fun nameA(): String = if (isRightController) "A" else "Right"

    private fun nameB(): String = if (isRightController) "B" else "Down"

    override fun toString(): String {

        var text = "ControllerNotification("

        text += "${nameController()} "

        text += if (buttonYPressed) nameY() else ""
        text += if (buttonZPressed) nameZ() else ""
        text += if (buttonAPressed) nameA() else ""
        text += if (buttonBPressed) nameB() else ""

        text += if (shoulderButtonPressed) SHOULDER_NAME else ""
        text += if (powerButtonPressed) POWER_NAME else ""

        text += if (steerBrakeValue != 0) "$STEER_NAME: $steerBrakeValue" else ""

        text += if (somethingValue != 0) "$UNKNOWN_NAME: $somethingValue" else ""

        text += ")"
        return text
    }
}

