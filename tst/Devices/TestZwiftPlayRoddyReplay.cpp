#include <gtest/gtest.h>

#include "zwift_play/zwiftPlayDevice.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QThread>
#include <QVariant>
#include <QVector>
#include <iostream>

// Real notifications from Roddy's iOS build 1448 / Zwift Play firmware 2.0.1
// log of October 14, 2026. Only BLE payloads are included: no personal data.
// Each logged packet has typeZap=RIGHT; the log does not identify which
// physical controller produced which notification. These tests replay the
// parser's settings-dependent behavior, NOT the BLE pairing/linking layer.
namespace {
const char *kIdle = "2308ffffffff0f1a04080010001a04080110001a04080210001a0408031000";
const char *kRight = "2308ffffffff0f1a04080010001a05080110c701"; // 11:53:53.349
const char *kLeft = "2308ffffffff0f1a05080010c7011a04080110001a04080210001a0408031000"; // 11:53:53.367
const char *kLeftShift = "2308fffdffff0f1a04080010001a0408011000"; // 11:53:48.250

class SettingsGuard {
public:
    SettingsGuard() {
        keys = {QZSettings::zwiftplay_swap,
                QZSettings::gears_volume_debouncing,
                QZSettings::zwiftplay_gear_paddle_left,
                QZSettings::zwiftplay_gear_paddle_right,
                QZSettings::zwiftplay_gear_ls1,
                QZSettings::zwiftplay_gear_ls2,
                QZSettings::zwiftplay_gear_rs1,
                QZSettings::zwiftplay_gear_rs2,
                QZSettings::zwiftplay_gear_lb,
                QZSettings::zwiftplay_gear_rb};
        for (const QString &key : keys) {
            existed.append(settings.contains(key));
            values.append(settings.value(key));
        }
    }

    ~SettingsGuard() {
        for (int i = 0; i < keys.size(); ++i) {
            if (existed[i])
                settings.setValue(keys[i], values[i]);
            else
                settings.remove(keys[i]);
        }
    }

    void configure(bool swap, bool debounce, bool secondary) {
        settings.setValue(QZSettings::zwiftplay_swap, swap);
        settings.setValue(QZSettings::gears_volume_debouncing, debounce);
        settings.setValue(QZSettings::zwiftplay_gear_paddle_left, 2); // Down
        settings.setValue(QZSettings::zwiftplay_gear_paddle_right, 1); // Up
        settings.setValue(QZSettings::zwiftplay_gear_ls1, secondary ? 2 : 0);
        settings.setValue(QZSettings::zwiftplay_gear_ls2, secondary ? 2 : 0);
        settings.setValue(QZSettings::zwiftplay_gear_rs1, secondary ? 1 : 0);
        settings.setValue(QZSettings::zwiftplay_gear_rs2, secondary ? 1 : 0);
        settings.setValue(QZSettings::zwiftplay_gear_lb, 0);
        settings.setValue(QZSettings::zwiftplay_gear_rb, 0);
    }

private:
    QSettings settings;
    QStringList keys;
    QVector<bool> existed;
    QVector<QVariant> values;
};

struct Events {
    int up = 0;
    int down = 0;
    int leftPaddle = 0;
    int rightPaddle = 0;
    int leftShift = 0;
    int rightShift = 0;

    void attach(ZwiftPlayDevice &device) {
        QObject::connect(&device, &ZwiftPlayDevice::plus, [&]() { ++up; });
        QObject::connect(&device, &ZwiftPlayDevice::minus, [&]() { ++down; });
        QObject::connect(&device, &ZwiftPlayDevice::leftPaddle, [&](int v) {
            if (v < 0) ++leftPaddle;
        });
        QObject::connect(&device, &ZwiftPlayDevice::rightPaddle, [&](int v) {
            if (v > 0) ++rightPaddle;
        });
        QObject::connect(&device, &ZwiftPlayDevice::rideLeftShiftUp, [&](bool v) {
            if (v) ++leftShift;
        });
        QObject::connect(&device, &ZwiftPlayDevice::rideRightShiftUp, [&](bool v) {
            if (v) ++rightShift;
        });
    }
};

void frame(ZwiftPlayDevice &device, const char *hex,
           AbstractZapDevice::ZWIFT_PLAY_TYPE side = AbstractZapDevice::RIGHT) {
    device.processCharacteristic(QStringLiteral("Async"), QByteArray::fromHex(QByteArray(hex)), side);
}

void paddleScenario(bool swap) {
    SettingsGuard settings;
    settings.configure(swap, false, false);
    ZwiftPlayDevice device;
    Events events;
    events.attach(device);

    frame(device, kIdle);
    frame(device, kRight);
    EXPECT_EQ(events.up, swap ? 0 : 1) << "right paddle";
    EXPECT_EQ(events.down, swap ? 1 : 0) << "right paddle";
    frame(device, kIdle);
    frame(device, kLeft);
    EXPECT_EQ(events.up, 1) << "both paddles";
    EXPECT_EQ(events.down, 1) << "both paddles";
    EXPECT_EQ(events.leftPaddle, 1);
    EXPECT_EQ(events.rightPaddle, 1);
}
} // namespace

TEST(ZwiftPlayRoddyReplay, PaddlesOnlySwapOff) {
    paddleScenario(false);
}

TEST(ZwiftPlayRoddyReplay, PaddlesOnlySwapOn) {
    paddleScenario(true);
}

TEST(ZwiftPlayRoddyReplay, SecondaryButtonsDisabled) {
    SettingsGuard settings;
    settings.configure(false, false, false);
    ZwiftPlayDevice device;
    Events events;
    events.attach(device);

    frame(device, kIdle);
    frame(device, kLeftShift);
    frame(device, kIdle);
    // RS1 as seen in the current Zwift Ride button-map format.
    frame(device, "2308ffdfffff0f1a04080010001a04080110001a04080210001a0408031000");
    EXPECT_EQ(events.leftShift, 1);
    EXPECT_EQ(events.up, 0);
    EXPECT_EQ(events.down, 0);
}

TEST(ZwiftPlayRoddyReplay, SecondaryButtonsMapped) {
    SettingsGuard settings;
    settings.configure(false, false, true);
    ZwiftPlayDevice device;
    Events events;
    events.attach(device);

    frame(device, kIdle);
    frame(device, kLeftShift);
    EXPECT_EQ(events.down, 1);
    frame(device, kIdle);
    frame(device, "2308ffdfffff0f1a04080010001a04080110001a04080210001a0408031000");
    EXPECT_EQ(events.up, 1);
}

TEST(ZwiftPlayRoddyReplay, OppositePaddlesWithinDebounceWindow) {
    // Recorded order: right press (11:53:53.349), neutral (11:53:53.365),
    // left press (11:53:53.367). The real interval was 18 ms.
    // The replay preserves packet order but not original wall-clock timing.
    QThread::msleep(270); // clear previous test's static 250-ms gear guard
    SettingsGuard settings;
    settings.configure(false, true, false);
    ZwiftPlayDevice device;
    Events events;
    events.attach(device);

    frame(device, kIdle);
    frame(device, kRight);
    frame(device, kIdle);
    frame(device, kLeft);
    EXPECT_EQ(events.up, 1);
    EXPECT_EQ(events.down, 1);
}

TEST(ZwiftPlayRoddyReplay, AnalogLocationIndependentOfDetectedControllerSide) {
    // Manufacturer bytes 14/15 are both classified RIGHT, but 0x23 analog
    // inputs use embedded location ID 0/1, rather than detected controller side.
    SettingsGuard settings;
    settings.configure(false, false, false);
    for (auto side : {AbstractZapDevice::LEFT, AbstractZapDevice::RIGHT}) {
        ZwiftPlayDevice device;
        Events events;
        events.attach(device);
        frame(device, kIdle, side);
        frame(device, kRight, side);
        frame(device, kIdle, side);
        frame(device, kLeft, side);
        EXPECT_EQ(events.up, 1) << "side=" << side;
        EXPECT_EQ(events.down, 1) << "side=" << side;
    }
}


namespace {
struct StreamResult {
    int frames = 0;
    int up = 0;
    int down = 0;
    int left = 0;
    int right = 0;
};

StreamResult replayCompleteCapturedStream(bool swap) {
    // Replay all 553 packets in the captured order. Debouncing is disabled
    // here because processing them without the original 51.5-second delay
    // would otherwise change QDateTime-dependent filtering decisions.
    SettingsGuard settings;
    settings.configure(swap, false, true);
    ZwiftPlayDevice device;
    Events events;
    events.attach(device);

    const QString fixtureName = QStringLiteral("zwift-play-roddy-2026-10-14.frames");
    QFile trace(QStringLiteral("Devices/fixtures/") + fixtureName);
    if (!trace.exists()) {
        const QDir sourceDir(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
        trace.setFileName(sourceDir.filePath(QStringLiteral("fixtures/") + fixtureName));
    }
    StreamResult result;
    if (!trace.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ADD_FAILURE() << "Could not open BLE replay fixture: " << trace.fileName().toStdString();
        return result;
    }
    qint64 lastTimeMs = -1;
    while (!trace.atEnd()) {
        const QByteArray line = trace.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;
        const int sep = line.indexOf('\t');
        if (sep <= 0) {
            ADD_FAILURE() << "Invalid frame fixture row: " << line.constData();
            continue;
        }
        bool ok = false;
        const qint64 relativeMs = line.left(sep).toLongLong(&ok);
        EXPECT_TRUE(ok);
        EXPECT_GE(relativeMs, lastTimeMs);
        lastTimeMs = relativeMs;
        const QByteArray payload = QByteArray::fromHex(line.mid(sep + 1));
        if (payload.isEmpty()) {
            ADD_FAILURE() << "Empty payload in BLE replay fixture";
            continue;
        }
        EXPECT_EQ(static_cast<quint8>(payload.at(0)), 0x23);
        device.processCharacteristic(QStringLiteral("Async"), payload, AbstractZapDevice::RIGHT);
        ++result.frames;
    }
    result.up = events.up;
    result.down = events.down;
    result.left = events.leftPaddle;
    result.right = events.rightPaddle;
    return result;
}
} // namespace

TEST(ZwiftPlayRoddyReplay, All553CapturedFramesBothSwapModes) {
    const StreamResult normal = replayCompleteCapturedStream(false);
    const StreamResult swapped = replayCompleteCapturedStream(true);
    EXPECT_EQ(normal.frames, 553);
    EXPECT_EQ(swapped.frames, 553);
    EXPECT_GT(normal.left, 0);
    EXPECT_GT(normal.right, 0);
    EXPECT_GT(normal.up, 0);
    EXPECT_GT(normal.down, 0);
    // Swap should invert direction but never change recognition of a paddle
    // when replaying the very same input sequence.
    EXPECT_EQ(normal.up, swapped.down);
    EXPECT_EQ(normal.down, swapped.up);
    EXPECT_EQ(normal.left, swapped.left);
    EXPECT_EQ(normal.right, swapped.right);

    std::cout << "Roddy 553-frame replay (debounce OFF for offline timing): "
              << "normal: up=" << normal.up << ", down=" << normal.down
              << ", left=" << normal.left << ", right=" << normal.right
              << "; swapped: up=" << swapped.up << ", down=" << swapped.down
              << std::endl;
}
