#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

// The Paragon X reports speed three times in its 0x17 status packet.
// Some iOS connections deliver only the first three 20-byte BLE notifications
// (60 bytes) before the next status packet starts. The missing bytes are at
// the end; speed and inclination are already present in the received prefix.
namespace ParagonXTelemetry {
struct Metrics {
    double speedKmh;
    double inclinePercent;
    // 0x01 = running, 0x02 = paused, 0x04 = stopped in the real
    // 2026-11-25/2026-11-24 Paragon X BLE captures.
    uint8_t machineState = 0;
};

// The firmware keeps the last commanded belt speed in telemetry even while
// paused/stopped. Publish zero to QZ (and therefore Zwift) until there is an
// explicit restart, or a physical console restart (0x02/0x04 -> 0x01).
// This state applies only to the Paragon X proprietary protocol.
class SessionState {
  public:
    void stopRequested() { localStop_ = true; }
    void startRequested() { localStop_ = false; }

    void observe(uint8_t machineState) {
        if (machineState != 0x01 && machineState != 0x02 && machineState != 0x04)
            return;
        if (machineState == 0x01 &&
            (lastMachineState_ == 0x02 || lastMachineState_ == 0x04)) {
            // The user may have restarted the belt from its own console.
            localStop_ = false;
        }
        machineStopped_ = (machineState == 0x02 || machineState == 0x04);
        lastMachineState_ = machineState;
    }

    bool commandsSuspended() const { return localStop_ || machineStopped_; }
    double publishedSpeed(double reportedSpeedKmh) const {
        return commandsSuspended() ? 0.0 : reportedSpeedKmh;
    }
    void reset() {
        localStop_ = false;
        machineStopped_ = false;
        lastMachineState_ = 0;
    }

  private:
    bool localStop_ = false;
    bool machineStopped_ = false;
    uint8_t lastMachineState_ = 0;
};

inline bool decode(const uint8_t *packet, std::size_t size, bool allowIncomplete, Metrics &metrics) {
    if (!packet || size < 31 || packet[0] != 0x55 || packet[1] != 0xaa || packet[4] != 0x02 ||
        packet[5] != 0x17 || packet[6] != 0x40 || packet[7] != 0x00) {
        return false;
    }

    const std::size_t expected = 10 + (static_cast<std::size_t>(packet[7]) << 8) + packet[6];
    if (size < expected) {
        // Never decode incomplete data until the *next* packet header has
        // proven that the previous packet will not receive more fragments.
        if (!allowIncomplete || size < 60)
            return false;
        // An incomplete frame lacks its trailing checksum/footer: check the
        // three redundant speed copies to avoid promoting corrupted data.
        if (packet[24] != packet[26] || packet[25] != packet[27] ||
            packet[24] != packet[28] || packet[25] != packet[29])
            return false;
    }

    const uint16_t rawSpeed = static_cast<uint16_t>(packet[24]) |
                              (static_cast<uint16_t>(packet[25]) << 8);
    const double speed = (static_cast<double>(rawSpeed) / 100.0) * 1.60934;
    const double incline = static_cast<double>(packet[30]) / 10.0;
    if (speed > 40.0 || incline > 30.0)
        return false;

    metrics.speedKmh = speed;
    metrics.inclinePercent = incline;
    metrics.machineState = packet[14];
    return true;
}

// Only a subsequent frame header makes a truncated frame eligible for the
// conservative fallback. A BLE gap or a 20-byte fragment alone is not enough.
inline bool decodePreviousAtBoundary(const uint8_t *previous, std::size_t previousSize,
                                     int remainingBytes, const uint8_t *next,
                                     std::size_t nextSize, Metrics &metrics) {
    if (remainingBytes <= 0 || !next || nextSize <= 7 || next[0] != 0x55 || next[1] != 0xaa)
        return false;
    return decode(previous, previousSize, true, metrics);
}
// Mirrors the BLE notification/frame boundary behavior used by the Paragon X
// production driver. The callback is the actual packetReceived() emission in
// horizontreadmill; tests can count callbacks without a Bluetooth device.
struct NotificationResult {
    bool completed = false;
    bool recovered = false;
    Metrics recoveredMetrics{};
};

class NotificationAssembler {
  public:
    template <typename PacketReceivedCallback>
    NotificationResult accept(const uint8_t *data, std::size_t size, PacketReceivedCallback packetReceived) {
        NotificationResult result{};
        if (!data || size == 0)
            return result;

        const bool newHeader = data[0] == 0x55 && size > 7;
        if (newHeader) {
            if (decodePreviousAtBoundary(frame_.data(), frame_.size(), remaining_, data, size,
                                         result.recoveredMetrics)) {
                result.recovered = true;
                packetReceived();
            }
            frame_.clear();
            remaining_ = static_cast<int>((static_cast<unsigned>(data[7]) << 8) |
                                          static_cast<unsigned>(data[6])) + 10;
        }
        // Keep the original Horizon behavior for headerless notifications:
        // initialization replies can be delivered as standalone short packets.
        // The legacy driver signals packetReceived() whenever remaining_ <= 0,
        // even when no new 0x55aa header was present.

        frame_.insert(frame_.end(), data, data + size);
        remaining_ -= static_cast<int>(size);
        if (remaining_ <= 0) {
            result.completed = true;
            remaining_ = 0;
            packetReceived();
        }
        return result;
    }

    const std::vector<uint8_t> &frame() const { return frame_; }
    int remaining() const { return remaining_; }
    void reset() {
        frame_.clear();
        remaining_ = 0;
    }

  private:
    std::vector<uint8_t> frame_;
    int remaining_ = 0;
};

} // namespace ParagonXTelemetry
