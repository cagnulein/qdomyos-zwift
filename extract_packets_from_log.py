#!/usr/bin/env python3
"""
Extract BLE packets from octane treadmill debug log
and generate a test that passes them directly to the parsing code
"""

import re
import sys
from pathlib import Path

def extract_packets_from_log(log_file):
    """Extract all BLE packets from the debug log"""
    packets = []

    with open(log_file, 'r') as f:
        for line in f:
            # Look for lines like: " << 20 a5 04 02 00 01 fd 00..."
            match = re.search(r' << 20 (.+)$', line)
            if match:
                hex_str = match.group(1).strip()
                # Convert hex string to bytes
                hex_bytes = hex_str.split()
                if len(hex_bytes) == 20:
                    packets.append(hex_bytes)

    return packets

def generate_test_file(packets, output_file):
    """Generate a C++ test file that processes all packets"""

    # Generate the test function
    test_code = '''#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QByteArray>
#include <QVector>
#include <QString>
#include <iostream>
#include <iomanip>
#include <cmath>

#include "octanetreadmill/octanetreadmill.h"

/**
 * @file TestOctaneTreadmillPacketParsing.h
 * @brief Direct packet parsing test using real BLE data from debug log
 *
 * This test injects all BLE packets captured in the debug log directly
 * into the octanetreadmill parsing code and captures the extracted speeds.
 *
 * Used to validate that the fix for buffer mismatch (newValue vs value)
 * produces reasonable speed values instead of garbage like:
 * - 0.156243 km/h
 * - 14.0078 km/h
 * - 0.0803571 km/h
 */

class SignalCapture : public QObject {
    Q_OBJECT
public:
    QVector<QString> debugMessages;
    QVector<double> speeds;

public slots:
    void onDebug(const QString &msg) {
        debugMessages.append(msg);
        if (msg.contains("Current speed:")) {
            QString speedStr = msg.split(':').last().trimmed();
            bool ok;
            double speed = speedStr.toDouble(&ok);
            if (ok) {
                speeds.append(speed);
            }
        }
    }
};

class OctanePacketParsingTest : public testing::Test {
protected:
    octanetreadmill *device;
    SignalCapture *capture;

    void SetUp() override {
        capture = new SignalCapture();
        device = new octanetreadmill(200, true, true, 0.0, 0.0);
        QObject::connect(device, &octanetreadmill::debug, capture, &SignalCapture::onDebug);
    }

    void TearDown() override {
        if (device) delete device;
        if (capture) delete capture;
    }

    void injectPacket(const QVector<uint8_t> &packet) {
        QByteArray data;
        for (uint8_t byte : packet) {
            data.append((char)byte);
        }
        QLowEnergyCharacteristic mockChar;
        QMetaObject::invokeMethod(device, "characteristicChanged", Qt::DirectConnection,
                                 Q_ARG(QLowEnergyCharacteristic, mockChar),
                                 Q_ARG(QByteArray, data));
    }
};

TEST_F(OctanePacketParsingTest, DirectLogPacketParsing) {
    /**
     * CRITICAL: Test that all real BLE packets from the debug log
     * produce reasonable speed values, not garbage values
     */

    // Real BLE packets extracted from debug-Sun_Jan_18_21_04_26_2026.log
    QVector<QVector<uint8_t>> packets = {
'''

    # Add all packets
    for i, packet in enumerate(packets):
        packet_bytes = ', '.join(f'0x{b}' for b in packet)
        test_code += f'        {{ {packet_bytes} }},\n'

    test_code += '''    };

    qDebug() << "Testing" << packets.size() << "real BLE packets from debug log";
    qDebug() << "\\n=== PACKET PARSING RESULTS ===\\n";

    double minSpeed = 999.0;
    double maxSpeed = 0.0;
    double sumSpeed = 0.0;
    int validSpeeds = 0;
    int garbageSpeeds = 0;

    std::vector<double> allSpeeds;

    for (size_t i = 0; i < packets.size(); i++) {
        capture->speeds.clear();
        capture->debugMessages.clear();

        injectPacket(packets[i]);
        QCoreApplication::processEvents();

        if (!capture->speeds.isEmpty()) {
            double speed = capture->speeds.last();
            allSpeeds.push_back(speed);

            std::cout << "Packet " << std::setw(3) << (i+1)
                      << ": Speed = " << std::fixed << std::setprecision(6)
                      << speed << " km/h";

            // Check for garbage values
            bool isGarbage = false;
            if (speed < 0.5) {
                std::cout << " [GARBAGE: too low]";
                isGarbage = true;
                garbageSpeeds++;
            } else if (speed > 40.0) {
                std::cout << " [GARBAGE: too high for running]";
                isGarbage = true;
                garbageSpeeds++;
            } else if (speed >= 0.5 && speed <= 40.0) {
                validSpeeds++;
                minSpeed = std::min(minSpeed, speed);
                maxSpeed = std::max(maxSpeed, speed);
                sumSpeed += speed;
            }
            std::cout << std::endl;
        }
    }

    std::cout << "\\n=== SUMMARY ===\\n";
    std::cout << "Total packets: " << packets.size() << "\\n";
    std::cout << "Packets with speed data: " << allSpeeds.size() << "\\n";
    std::cout << "Valid speeds (0.5-40 km/h): " << validSpeeds << "\\n";
    std::cout << "Garbage speeds: " << garbageSpeeds << "\\n";

    if (validSpeeds > 0) {
        double avgSpeed = sumSpeed / validSpeeds;
        std::cout << "Speed range: " << std::fixed << std::setprecision(2)
                  << minSpeed << " - " << maxSpeed << " km/h\\n";
        std::cout << "Average speed: " << avgSpeed << " km/h\\n";
    }

    std::cout << "\\n";

    // ASSERTIONS
    // With the fix: should have reasonable speeds, not garbage
    EXPECT_GT(validSpeeds, 0)
        << "Should extract valid speeds from packets";

    EXPECT_EQ(garbageSpeeds, 0)
        << "Should NOT have garbage speeds after fix";

    if (validSpeeds > 0) {
        double avgSpeed = sumSpeed / validSpeeds;

        // Speeds should cluster in a reasonable range for running
        EXPECT_GE(minSpeed, 0.5) << "Min speed should be >= 0.5 km/h";
        EXPECT_LE(maxSpeed, 40.0) << "Max speed should be <= 40 km/h for running";

        // Average should be reasonable
        EXPECT_GE(avgSpeed, 4.0) << "Average speed too low (likely parsing error)";
        EXPECT_LE(avgSpeed, 30.0) << "Average speed too high for running";
    }
}
'''

    with open(output_file, 'w') as f:
        f.write(test_code)

    print(f"Generated test file: {output_file}")
    print(f"Total packets extracted: {len(packets)}")

if __name__ == '__main__':
    log_file = '/mnt/c/Users/violarob/Downloads/debug-Sun_Jan_18_21_04_26_2026 (1).log'
    output_file = '/mnt/c/Work/qdomyos-zwift/tst/Devices/TestOctaneTreadmillDirectPackets.h'

    print(f"Extracting packets from: {log_file}")
    packets = extract_packets_from_log(log_file)
    print(f"Found {len(packets)} packets")

    if packets:
        generate_test_file(packets, output_file)
        print(f"Test file generated successfully!")

        # Also generate a CSV for manual analysis
        csv_file = '/mnt/c/Work/qdomyos-zwift/octane_packets.csv'
        with open(csv_file, 'w') as f:
            f.write("packet_num,hex_data\n")
            for i, packet in enumerate(packets, 1):
                hex_str = ' '.join(packet)
                f.write(f"{i},{hex_str}\n")
        print(f"CSV file generated: {csv_file}")
    else:
        print("No packets found in log file")
        sys.exit(1)
