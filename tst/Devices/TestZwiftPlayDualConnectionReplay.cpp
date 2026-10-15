#include <gtest/gtest.h>

#include "zwift_play/zwiftclickremote.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStringList>
#include <QThread>
#include <QVariant>
#include <QVector>
#include <iostream>

// Verify the real zwiftclickremote BLE characteristic dispatch, where QZ
// gives every discovered Zwift Play its own ZwiftPlayDevice and connects both
// parsers' plus/minus signals to the same bike. No real BLE hardware required.
// The fixture does NOT contain a peripheral UUID per notification; splitting
// packets based on their shape below is a what-if, not an identified device.
namespace {

const char *kIdle = "2308ffffffff0f1a04080010001a04080110001a04080210001a0408031000";
const char *kRight = "2308ffffffff0f1a04080010001a05080110c701"; // right paddle
const char *kLeft = "2308ffffffff0f1a05080010c7011a04080110001a04080210001a0408031000"; // left paddle

class DualReplaySettings {
public:
    DualReplaySettings() {
        keys << QZSettings::zwiftplay_swap
             << QZSettings::gears_volume_debouncing
             << QZSettings::zwiftplay_gear_paddle_left
             << QZSettings::zwiftplay_gear_paddle_right
             << QZSettings::zwiftplay_gear_ls1
             << QZSettings::zwiftplay_gear_ls2
             << QZSettings::zwiftplay_gear_rs1
             << QZSettings::zwiftplay_gear_rs2
             << QZSettings::zwiftplay_gear_lb
             << QZSettings::zwiftplay_gear_rb;
        for (const QString &key : keys) {
            present.append(settings.contains(key));
            original.append(settings.value(key));
        }
    }

    ~DualReplaySettings() {
        for (int i = 0; i < keys.size(); ++i) {
            if (present.at(i))
                settings.setValue(keys.at(i), original.at(i));
            else
                settings.remove(keys.at(i));
        }
    }

    void configure(bool swap, bool debounce) {
        settings.setValue(QZSettings::zwiftplay_swap, swap);
        settings.setValue(QZSettings::gears_volume_debouncing, debounce);
        settings.setValue(QZSettings::zwiftplay_gear_paddle_left, 2);
        settings.setValue(QZSettings::zwiftplay_gear_paddle_right, 1);
        // Disable secondary buttons to isolate the analog paddles.
        for (const QString &key : {QZSettings::zwiftplay_gear_ls1,
                                   QZSettings::zwiftplay_gear_ls2,
                                   QZSettings::zwiftplay_gear_rs1,
                                   QZSettings::zwiftplay_gear_rs2,
                                   QZSettings::zwiftplay_gear_lb,
                                   QZSettings::zwiftplay_gear_rb})
            settings.setValue(key, 0);
    }

private:
    QSettings settings;
    QStringList keys;
    QVector<bool> present;
    QVector<QVariant> original;
};

struct GearSink {
    int up = 0;
    int down = 0;

    void connectTo(zwiftclickremote &remote) {
        QObject::connect(remote.playDevice, &ZwiftPlayDevice::plus, [&]() { ++up; });
        QObject::connect(remote.playDevice, &ZwiftPlayDevice::minus, [&]() { ++down; });
    }
};

QBluetoothUuid asyncUUID() {
    return QBluetoothUuid(QStringLiteral("00000002-19CA-4651-86E5-FA29DCDD09D1"));
}

void inject(zwiftclickremote &remote, const char *frameHex) {
    remote.handleCharacteristicValueChanged(asyncUUID(), QByteArray::fromHex(frameHex));
}

struct ShapeCounts {
    int shortFrames = 0;
    int longFrames = 0;
    int singleUp = 0;
    int singleDown = 0;
    int splitUp = 0;
    int splitDown = 0;
};

ShapeCounts splitRoddyTraceByLength() {
    DualReplaySettings settings;
    settings.configure(false, false);

    zwiftclickremote single(nullptr, AbstractZapDevice::RIGHT);
    zwiftclickremote shortRemote(nullptr, AbstractZapDevice::RIGHT);
    zwiftclickremote longRemote(nullptr, AbstractZapDevice::RIGHT);
    GearSink singleSink;
    GearSink splitSink;
    singleSink.connectTo(single);
    splitSink.connectTo(shortRemote);
    splitSink.connectTo(longRemote);

    const QString name = QStringLiteral("zwift-play-roddy-2026-10-14.frames");
    QFile trace(QStringLiteral("Devices/fixtures/") + name);
    if (!trace.exists()) {
        const QDir source(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
        trace.setFileName(source.filePath(QStringLiteral("fixtures/") + name));
    }

    ShapeCounts counts;
    if (!trace.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ADD_FAILURE() << "Cannot load captured stream: " << trace.fileName().toStdString();
        return counts;
    }

    while (!trace.atEnd()) {
        const QByteArray line = trace.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;
        const int tab = line.indexOf('\t');
        if (tab <= 0) { ADD_FAILURE() << "Malformed capture line"; continue; }
        const QByteArray frame = QByteArray::fromHex(line.mid(tab + 1));
        if (frame.isEmpty()) {
            ADD_FAILURE() << "Empty frame in capture";
            continue;
        }
        single.handleCharacteristicValueChanged(asyncUUID(), frame);
        if (frame.size() <= 20) {
            shortRemote.handleCharacteristicValueChanged(asyncUUID(), frame);
            ++counts.shortFrames;
        } else {
            longRemote.handleCharacteristicValueChanged(asyncUUID(), frame);
            ++counts.longFrames;
        }
    }
    counts.singleUp = singleSink.up;
    counts.singleDown = singleSink.down;
    counts.splitUp = splitSink.up;
    counts.splitDown = splitSink.down;
    return counts;
}

} // namespace

TEST(ZwiftPlayDualConnectionReplay, TwoControllersDispatchIntoSameGearSink) {
    DualReplaySettings settings;
    settings.configure(false, false);
    zwiftclickremote first(nullptr, AbstractZapDevice::RIGHT);
    zwiftclickremote second(nullptr, AbstractZapDevice::RIGHT);
    GearSink sink;
    sink.connectTo(first);
    sink.connectTo(second);

    inject(first, kIdle);
    inject(second, kIdle);
    inject(first, kRight);
    inject(second, kLeft);
    EXPECT_EQ(sink.up, 1);
    EXPECT_EQ(sink.down, 1);
}

TEST(ZwiftPlayDualConnectionReplay, DuplicateRightPaddleOnBothConnectionsCausesTwoUpsWithoutDebouncing) {
    DualReplaySettings settings;
    settings.configure(false, false);
    zwiftclickremote first(nullptr, AbstractZapDevice::RIGHT);
    zwiftclickremote second(nullptr, AbstractZapDevice::RIGHT);
    GearSink sink;
    sink.connectTo(first);
    sink.connectTo(second);

    inject(first, kIdle);
    inject(second, kIdle);
    inject(first, kRight);
    inject(second, kRight);
    EXPECT_EQ(sink.up, 2);
    EXPECT_EQ(sink.down, 0);
}

TEST(ZwiftPlayDualConnectionReplay, SharedDebounceSuppressesDuplicateSameDirectionAcrossConnections) {
    // AbstractZapDevice::lastRideGearFrame is static across both devices.
    // Wait until any preceding test's global 250 ms guard has expired.
    QThread::msleep(270);
    DualReplaySettings settings;
    settings.configure(false, true);
    zwiftclickremote first(nullptr, AbstractZapDevice::RIGHT);
    zwiftclickremote second(nullptr, AbstractZapDevice::RIGHT);
    GearSink sink;
    sink.connectTo(first);
    sink.connectTo(second);

    inject(first, kIdle);
    inject(second, kIdle);
    inject(first, kRight);
    inject(second, kRight);
    EXPECT_EQ(sink.up, 1);
    EXPECT_EQ(sink.down, 0);
}

TEST(ZwiftPlayDualConnectionReplay, SharedDebounceAcceptsOppositeDirectionsFromSeparateControllers) {
    QThread::msleep(270);
    DualReplaySettings settings;
    settings.configure(false, true);
    zwiftclickremote first(nullptr, AbstractZapDevice::RIGHT);
    zwiftclickremote second(nullptr, AbstractZapDevice::RIGHT);
    GearSink sink;
    sink.connectTo(first);
    sink.connectTo(second);

    inject(first, kIdle);
    inject(second, kIdle);
    inject(first, kRight);
    inject(second, kLeft);
    EXPECT_EQ(sink.up, 1);
    EXPECT_EQ(sink.down, 1);
}

TEST(ZwiftPlayDualConnectionReplay, SwapReversesBothDirectionsWithoutChangingConnections) {
    DualReplaySettings settings;
    for (bool swap : {false, true}) {
        settings.configure(swap, false);
        zwiftclickremote first(nullptr, AbstractZapDevice::RIGHT);
        zwiftclickremote second(nullptr, AbstractZapDevice::RIGHT);
        GearSink sink;
        sink.connectTo(first);
        sink.connectTo(second);

        inject(first, kIdle);
        inject(second, kIdle);
        inject(first, kRight);
        inject(second, kLeft);
        EXPECT_EQ(sink.up, 1);
        EXPECT_EQ(sink.down, 1);
    }
}

TEST(ZwiftPlayDualConnectionReplay, SplitFullCapturedTraceByShapeComparedWithSingleParser) {
    // This deliberately tests a hypothesis. The real trace has no peripheral
    // identity: 19/20 and 31/32-byte frames could be from one or two remotes.
    const ShapeCounts counts = splitRoddyTraceByLength();
    EXPECT_EQ(counts.shortFrames + counts.longFrames, 553);
    EXPECT_GT(counts.shortFrames, 0);
    EXPECT_GT(counts.longFrames, 0);
    EXPECT_GT(counts.singleUp + counts.singleDown, 0);
    EXPECT_GT(counts.splitUp + counts.splitDown, 0);

    std::cout << "Zwift Play two-stream hypothesis (debounce OFF): "
              << counts.shortFrames << " short frames, "
              << counts.longFrames << " long frames; "
              << "single parser up=" << counts.singleUp
              << ", down=" << counts.singleDown
              << "; independent parsers up=" << counts.splitUp
              << ", down=" << counts.splitDown << std::endl;
}
