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

    int rideLeftShiftUpPressed = 0;
    int rideLeftShiftUpReleased = 0;
    int rideLeftShiftDownPressed = 0;
    int rideLeftShiftDownReleased = 0;
    int rideLeftPowerUpPressed = 0;
    int rideLeftPowerUpReleased = 0;
    int rideLeftOnOffPressed = 0;
    int rideLeftOnOffReleased = 0;
    int rideRightZAltPressed = 0;
    int rideRightZAltReleased = 0;
    int rideRightShiftUpPressed = 0;
    int rideRightShiftUpReleased = 0;
    int rideRightShiftDownPressed = 0;
    int rideRightShiftDownReleased = 0;
    int rideRightPowerPressed = 0;
    int rideRightPowerReleased = 0;
    int rideRightPowerUpPressed = 0;
    int rideRightPowerUpReleased = 0;
    int rideRightOnOffPressed = 0;
    int rideRightOnOffReleased = 0;
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

    QObject::connect(&device, &ZwiftPlayDevice::rideLeftShiftUp, [&events](bool pressed) {
        pressed ? ++events.rideLeftShiftUpPressed : ++events.rideLeftShiftUpReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rideLeftShiftDown, [&events](bool pressed) {
        pressed ? ++events.rideLeftShiftDownPressed : ++events.rideLeftShiftDownReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rideLeftPowerUp, [&events](bool pressed) {
        pressed ? ++events.rideLeftPowerUpPressed : ++events.rideLeftPowerUpReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rideLeftOnOff, [&events](bool pressed) {
        pressed ? ++events.rideLeftOnOffPressed : ++events.rideLeftOnOffReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rideRightZAlt, [&events](bool pressed) {
        pressed ? ++events.rideRightZAltPressed : ++events.rideRightZAltReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rideRightShiftUp, [&events](bool pressed) {
        pressed ? ++events.rideRightShiftUpPressed : ++events.rideRightShiftUpReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rideRightShiftDown, [&events](bool pressed) {
        pressed ? ++events.rideRightShiftDownPressed : ++events.rideRightShiftDownReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rideRightPower, [&events](bool pressed) {
        pressed ? ++events.rideRightPowerPressed : ++events.rideRightPowerReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rideRightPowerUp, [&events](bool pressed) {
        pressed ? ++events.rideRightPowerUpPressed : ++events.rideRightPowerUpReleased;
    });
    QObject::connect(&device, &ZwiftPlayDevice::rideRightOnOff, [&events](bool pressed) {
        pressed ? ++events.rideRightOnOffPressed : ++events.rideRightOnOffReleased;
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

void pressAndRelease(ZwiftPlayDevice &device, quint32 activeLowButtonMap) {
    const quint32 idle = 0xffffffff;
    processRideFrame(device, activeLowButtonMap);
    processRideFrame(device, idle);
}

const quint32 kIdle = 0xffffffff;

const quint32 kMay10ObservedButtons[] = {
    0xfffffffe, // LEFT_BTN
    0xfffffffd, // UP_BTN
    0xfffffffb, // RIGHT_BTN
    0xfffffff7, // DOWN_BTN
    0xffffffef, // A_BTN
    0xffffffdf, // B_BTN
    0xffffffbf, // Y_BTN
    0xffffff7f, // Firmware-specific right Z alt bit
    0xfffffeff, // Z_BTN
    0xfffffdff, // SHFT_UP_L_BTN
    0xfffffbff, // SHFT_DN_L_BTN
    0xfffff7ff, // POWERUP_L_BTN
    0xffffefff, // ONOFF_L_BTN
    0xffffdfff, // SHFT_UP_R_BTN
    0xffffbfff, // SHFT_DN_R_BTN
    0xffff7fff  // Firmware-specific right power bit
};

const quint32 kMay12ObservedButtons[] = {
    0xfffffffe,
    0xfffffffd,
    0xfffffffb,
    0xfffffff7,
    0xffffffef,
    0xffffffdf,
    0xffffffbf,
    0xffffff7f,
    0xfffffeff,
    0xfffff7ff,
    0xffff7fff,
    0xfffffdff,
    0xffffefff,
    0xffffdfff,
    0xfffffbff,
    0xffffbfff
};

void replayObservedButtons(ZwiftPlayDevice &device, const quint32 *buttons, int count) {
    processRideFrame(device, kIdle);
    for (int i = 0; i < count; ++i) {
        pressAndRelease(device, buttons[i]);
    }
}

} // namespace

TEST(ZwiftRideControllerTest, May8IdleFrameDoesNotEmitFalseButtonsOrGears) {
    ZwiftPlayDevice device;
    ButtonEvents events;
    connectButtonEvents(device, events);

    device.processCharacteristic(QStringLiteral("Async"),
                                 QByteArray::fromHex("2308ffffffff0f1a04080010001a04080110001a04080210001a0408031000"),
                                 AbstractZapDevice::RIGHT);

    EXPECT_EQ(events.rightYPressed, 0);
    EXPECT_EQ(events.rightZPressed, 0);
    EXPECT_EQ(events.rideRightZAltPressed, 0);
    EXPECT_EQ(events.rightPowerPressed, 0);
    EXPECT_EQ(events.rideRightPowerPressed, 0);
    EXPECT_EQ(events.gearPlus, 0);
    EXPECT_EQ(events.gearMinus, 0);
}

TEST(ZwiftRideControllerTest, May11RightPowerReplayDoesNotTriggerGearDown) {
    ZwiftPlayDevice device;
    ButtonEvents events;
    connectButtonEvents(device, events);

    processRideFrame(device, kIdle);
    pressAndRelease(device, 0xffff7fff);
    pressAndRelease(device, 0xffff7fff);
    pressAndRelease(device, 0xffff7fff);

    EXPECT_EQ(events.rightPowerPressed, 3);
    EXPECT_EQ(events.rightPowerReleased, 3);
    EXPECT_EQ(events.rideRightPowerPressed, 3);
    EXPECT_EQ(events.rideRightPowerReleased, 3);
    EXPECT_EQ(events.gearPlus, 0);
    EXPECT_EQ(events.gearMinus, 0);
}

TEST(ZwiftRideControllerTest, May10ReplayKeepsRideButtonsDistinctAndGearFree) {
    ZwiftPlayDevice device;
    ButtonEvents events;
    connectButtonEvents(device, events);

    replayObservedButtons(device, kMay10ObservedButtons, int(sizeof(kMay10ObservedButtons) / sizeof(kMay10ObservedButtons[0])));

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
    EXPECT_EQ(events.rightZPressed, 1);
    EXPECT_EQ(events.rightZReleased, 1);
    EXPECT_EQ(events.rideRightZAltPressed, 1);
    EXPECT_EQ(events.rideRightZAltReleased, 1);

    EXPECT_EQ(events.leftShoulderPressed, 1);
    EXPECT_EQ(events.leftShoulderReleased, 1);
    EXPECT_EQ(events.leftPowerPressed, 1);
    EXPECT_EQ(events.leftPowerReleased, 1);
    EXPECT_EQ(events.rightShoulderPressed, 1);
    EXPECT_EQ(events.rightShoulderReleased, 1);
    EXPECT_EQ(events.rightPowerPressed, 1);
    EXPECT_EQ(events.rightPowerReleased, 1);

    EXPECT_EQ(events.rideLeftShiftUpPressed, 1);
    EXPECT_EQ(events.rideLeftShiftUpReleased, 1);
    EXPECT_EQ(events.rideLeftShiftDownPressed, 1);
    EXPECT_EQ(events.rideLeftShiftDownReleased, 1);
    EXPECT_EQ(events.rideLeftPowerUpPressed, 1);
    EXPECT_EQ(events.rideLeftPowerUpReleased, 1);
    EXPECT_EQ(events.rideLeftOnOffPressed, 1);
    EXPECT_EQ(events.rideLeftOnOffReleased, 1);
    EXPECT_EQ(events.rideRightShiftUpPressed, 1);
    EXPECT_EQ(events.rideRightShiftUpReleased, 1);
    EXPECT_EQ(events.rideRightShiftDownPressed, 1);
    EXPECT_EQ(events.rideRightShiftDownReleased, 1);
    EXPECT_EQ(events.rideRightPowerPressed, 1);
    EXPECT_EQ(events.rideRightPowerReleased, 1);
    EXPECT_EQ(events.rideRightPowerUpPressed, 0);
    EXPECT_EQ(events.rideRightPowerUpReleased, 0);
    EXPECT_EQ(events.rideRightOnOffPressed, 0);
    EXPECT_EQ(events.rideRightOnOffReleased, 0);

    EXPECT_EQ(events.gearPlus, 0);
    EXPECT_EQ(events.gearMinus, 0);
}

TEST(ZwiftRideControllerTest, May12ReplayMatchesLegendOrderWithoutAliasesOrGears) {
    ZwiftPlayDevice device;
    ButtonEvents events;
    connectButtonEvents(device, events);

    replayObservedButtons(device, kMay12ObservedButtons, int(sizeof(kMay12ObservedButtons) / sizeof(kMay12ObservedButtons[0])));

    EXPECT_EQ(events.leftShoulderPressed, 1);
    EXPECT_EQ(events.leftShoulderReleased, 1);
    EXPECT_EQ(events.leftPowerPressed, 1);
    EXPECT_EQ(events.leftPowerReleased, 1);
    EXPECT_EQ(events.rightShoulderPressed, 1);
    EXPECT_EQ(events.rightShoulderReleased, 1);
    EXPECT_EQ(events.rightPowerPressed, 1);
    EXPECT_EQ(events.rightPowerReleased, 1);

    EXPECT_EQ(events.rideLeftShiftUpPressed, 1);
    EXPECT_EQ(events.rideLeftShiftUpReleased, 1);
    EXPECT_EQ(events.rideLeftShiftDownPressed, 1);
    EXPECT_EQ(events.rideLeftShiftDownReleased, 1);
    EXPECT_EQ(events.rideLeftPowerUpPressed, 1);
    EXPECT_EQ(events.rideLeftPowerUpReleased, 1);
    EXPECT_EQ(events.rideLeftOnOffPressed, 1);
    EXPECT_EQ(events.rideLeftOnOffReleased, 1);
    EXPECT_EQ(events.rideRightShiftUpPressed, 1);
    EXPECT_EQ(events.rideRightShiftUpReleased, 1);
    EXPECT_EQ(events.rideRightShiftDownPressed, 1);
    EXPECT_EQ(events.rideRightShiftDownReleased, 1);
    EXPECT_EQ(events.rideRightPowerPressed, 1);
    EXPECT_EQ(events.rideRightPowerReleased, 1);

    EXPECT_EQ(events.gearPlus, 0);
    EXPECT_EQ(events.gearMinus, 0);
}
