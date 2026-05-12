#include <gtest/gtest.h>

#include "zwift_play/zwiftPlayDevice.h"

#include <QByteArray>
#include <QObject>

namespace {

struct ButtonEvents {
    int gearPlus = 0;
    int gearMinus = 0;
    int leftUpPressed = 0;
    int leftUpReleased = 0;
    int leftDownPressed = 0;
    int leftDownReleased = 0;
    int leftLeftPressed = 0;
    int leftLeftReleased = 0;
    int leftRightPressed = 0;
    int leftRightReleased = 0;
    int leftShoulderPressed = 0;
    int leftShoulderReleased = 0;
    int leftPowerPressed = 0;
    int leftPowerReleased = 0;
    int rightYPressed = 0;
    int rightYReleased = 0;
    int rightZPressed = 0;
    int rightZReleased = 0;
    int rightAPressed = 0;
    int rightAReleased = 0;
    int rightBPressed = 0;
    int rightBReleased = 0;
    int rightShoulderPressed = 0;
    int rightShoulderReleased = 0;
    int rightPowerPressed = 0;
    int rightPowerReleased = 0;
};

void connectButtonEvents(ZwiftPlayDevice &device, ButtonEvents &events) {
    QObject::connect(&device, &ZwiftPlayDevice::plus, [&events]() {
        ++events.gearPlus;
    });
    QObject::connect(&device, &ZwiftPlayDevice::minus, [&events]() {
        ++events.gearMinus;
    });
    QObject::connect(&device, &ZwiftPlayDevice::leftUp, [&events](bool pressed) {
        pressed ? ++events.leftUpPressed : ++events.leftUpReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::leftDown, [&events](bool pressed) {
        pressed ? ++events.leftDownPressed : ++events.leftDownReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::leftLeft, [&events](bool pressed) {
        pressed ? ++events.leftLeftPressed : ++events.leftLeftReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::leftRight, [&events](bool pressed) {
        pressed ? ++events.leftRightPressed : ++events.leftRightReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::leftShoulder, [&events](bool pressed) {
        pressed ? ++events.leftShoulderPressed : ++events.leftShoulderReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::leftPower, [&events](bool pressed) {
        pressed ? ++events.leftPowerPressed : ++events.leftPowerReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rightY, [&events](bool pressed) {
        pressed ? ++events.rightYPressed : ++events.rightYReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rightZ, [&events](bool pressed) {
        pressed ? ++events.rightZPressed : ++events.rightZReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rightA, [&events](bool pressed) {
        pressed ? ++events.rightAPressed : ++events.rightAReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rightB, [&events](bool pressed) {
        pressed ? ++events.rightBPressed : ++events.rightBReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rightShoulder, [&events](bool pressed) {
        pressed ? ++events.rightShoulderPressed : ++events.rightShoulderReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rightPower, [&events](bool pressed) {
        pressed ? ++events.rightPowerPressed : ++events.rightPowerReleased;
    });
}

QByteArray rideFrame(quint32 activeLowButtonMap) {
    QByteArray frame;
    frame.append(QByteArray::fromHex("2308"));

    quint32 value = activeLowButtonMap;
    while (value >= 0x80) {
        frame.append(static_cast<char>((value & 0x7f) | 0x80));
        value >>= 7;
    }
    frame.append(static_cast<char>(value));

    frame.append(QByteArray::fromHex("1a04080010001a04080110001a04080210001a0408031000"));
    return frame;
}

void processRideFrame(ZwiftPlayDevice &device, quint32 activeLowButtonMap) {
    device.processCharacteristic(QStringLiteral("Async"), rideFrame(activeLowButtonMap), AbstractZapDevice::RIGHT);
}

} // namespace

TEST(ZwiftRideControllerTest, IdleAnalogGroupDoesNotEmitFalseRightY) {
    ZwiftPlayDevice device;
    ButtonEvents events;
    connectButtonEvents(device, events);

    device.processCharacteristic(QStringLiteral("Async"),
                                 QByteArray::fromHex("2308ffffffff0f1a04080010001a04080110001a04080210001a0408031000"),
                                 AbstractZapDevice::RIGHT);

    EXPECT_EQ(events.rightYPressed, 0);
    EXPECT_EQ(events.rightYReleased, 0);
    EXPECT_EQ(events.gearPlus, 0);
    EXPECT_EQ(events.gearMinus, 0);
}

TEST(ZwiftRideControllerTest, ParsesObservedActiveLowButtonMapsFromUpdatedFirmwareLog) {
    ZwiftPlayDevice device;
    ButtonEvents events;
    connectButtonEvents(device, events);

    const quint32 idle = 0xffffffff;
    processRideFrame(device, idle);

    processRideFrame(device, 0xfffffffe); // LEFT_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xfffffffd); // UP_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xfffffffb); // RIGHT_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xfffffff7); // DOWN_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xffffffef); // A_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xffffffdf); // B_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xffffffbf); // Y_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xffffff7f); // Newer firmware Z-like button bit.
    processRideFrame(device, idle);
    processRideFrame(device, 0xfffffeff); // Z_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xfffffdff); // SHFT_UP_L_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xfffffbff); // SHFT_DN_L_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xfffff7ff); // POWERUP_L_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xffffefff); // ONOFF_L_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xffffdfff); // SHFT_UP_R_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xffffbfff); // SHFT_DN_R_BTN
    processRideFrame(device, idle);
    processRideFrame(device, 0xffff7fff); // Newer firmware right power-like bit.
    processRideFrame(device, idle);

    EXPECT_EQ(events.leftLeftPressed, 1);
    EXPECT_EQ(events.leftLeftReleased, 1);
    EXPECT_EQ(events.leftUpPressed, 1);
    EXPECT_EQ(events.leftUpReleased, 1);
    EXPECT_EQ(events.leftRightPressed, 1);
    EXPECT_EQ(events.leftRightReleased, 1);
    EXPECT_EQ(events.leftDownPressed, 1);
    EXPECT_EQ(events.leftDownReleased, 1);
    EXPECT_EQ(events.rightAPressed, 1);
    EXPECT_EQ(events.rightAReleased, 1);
    EXPECT_EQ(events.rightBPressed, 1);
    EXPECT_EQ(events.rightBReleased, 1);
    EXPECT_EQ(events.rightYPressed, 1);
    EXPECT_EQ(events.rightYReleased, 1);
    EXPECT_EQ(events.rightZPressed, 2);
    EXPECT_EQ(events.rightZReleased, 2);
    EXPECT_EQ(events.leftShoulderPressed, 2);
    EXPECT_EQ(events.leftShoulderReleased, 2);
    EXPECT_EQ(events.leftPowerPressed, 2);
    EXPECT_EQ(events.leftPowerReleased, 2);
    EXPECT_EQ(events.rightShoulderPressed, 2);
    EXPECT_EQ(events.rightShoulderReleased, 2);
    EXPECT_EQ(events.rightPowerPressed, 1);
    EXPECT_EQ(events.rightPowerReleased, 1);

    EXPECT_EQ(events.gearPlus, 1);
    EXPECT_EQ(events.gearMinus, 3);
}

TEST(ZwiftRideControllerTest, RightPowerButtonDoesNotTriggerGearDown) {
    ZwiftPlayDevice device;
    ButtonEvents events;
    connectButtonEvents(device, events);

    const quint32 idle = 0xffffffff;
    processRideFrame(device, idle);
    processRideFrame(device, 0xffff7fff); // Observed right power button bit in updated firmware log.
    processRideFrame(device, idle);

    EXPECT_EQ(events.rightPowerPressed, 1);
    EXPECT_EQ(events.rightPowerReleased, 1);
    EXPECT_EQ(events.gearPlus, 0);
    EXPECT_EQ(events.gearMinus, 0);
}
