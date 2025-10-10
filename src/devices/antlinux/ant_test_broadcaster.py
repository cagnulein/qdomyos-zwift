#!/usr/bin/env python3
# -----------------------------------------------------------------------------
# QDomyos-Zwift: ANT+ Virtual Footpod Feature
# Test Script for ant_broadcaster.py (C++ Architecture Simulation)
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor(s): bassai-sho
# Licensed under GPL-3.0 - see project repository for full license
#
# This script resets the ANT+ USB dongle and tests the ant_broadcaster.py
# module by simulating the behavior of the C++ host application. It is used
# for standalone validation of the Python and ANT+ hardware functionality.
#
# Command to run:
# sudo $HOME/ant_venv/bin/python3 $HOME/qdomyos-zwift/src/devices/antlinux/ant_test_broadcaster.py
# -----------------------------------------------------------------------------

import time
import sys
import math
from pathlib import Path
import struct #

# Ensure the ant_broadcaster module can be found in the same directory
sys.path.append(str(Path(__file__).parent.absolute()))

try:
    import usb.core
    import usb.util
except ImportError:
    print("ERROR: pyusb library not found. Please run 'pip install pyusb' in your venv.")
    sys.exit(1)

from ant_broadcaster import AntBroadcaster

def estimate_cadence(speed_kmh: float) -> int:
    """
    Estimates a realistic running cadence (in Steps Per Minute) from speed.
    This function mirrors the logic in the C++ AntWorker.
    
    NOTE: Returns SPM (Steps Per Minute), not strides per minute.
    The broadcaster will convert SPM to stride rate internally.
    """
    if speed_kmh < 5.0:
        return int(speed_kmh * 15.0 + 45.0)
    cadence = int(speed_kmh * 5.0 + 110.0)
    return min(cadence, 200)

def calculate_and_format_pace_range(speed_kmh: float) -> (str, str):
    """Calculates and formats the expected pace range in min/km and min/mi."""
    if speed_kmh < 0.5:
        return "--:--", "--:--"
    pace_min_per_km_float = 60.0 / speed_kmh
    total_seconds_km = pace_min_per_km_float * 60
    lower_bound_sec_km = (total_seconds_km // 5) * 5
    upper_bound_sec_km = lower_bound_sec_km + 5
    km_range_str = "~{}:{:02d}-{}:{:02d}".format(
        int(lower_bound_sec_km // 60), int(lower_bound_sec_km % 60),
        int(upper_bound_sec_km // 60), int(upper_bound_sec_km % 60)
    )
    KM_TO_MILES = 1.60934
    speed_mph = speed_kmh / KM_TO_MILES
    pace_min_per_mi_float = 60.0 / speed_mph
    total_seconds_mi = pace_min_per_mi_float * 60
    lower_bound_sec_mi = (total_seconds_mi // 5) * 5
    upper_bound_sec_mi = lower_bound_sec_mi + 5
    mi_range_str = "~{}:{:02d}-{}:{:02d}".format(
        int(lower_bound_sec_mi // 60), int(lower_bound_sec_mi % 60),
        int(upper_bound_sec_mi // 60), int(upper_bound_sec_mi % 60)
    )
    return km_range_str, mi_range_str

def reset_ant_dongle():
    """Finds and resets the first available ANT+ USB dongle."""
    SUPPORTED_DONGLES = [(0x0fcf, 0x1009), (0x0fcf, 0x1008), (0x11fd, 0x0001)]
    print("--- Pre-Test USB Reset ---")
    try:
        dongle = next((usb.core.find(idVendor=v, idProduct=p) for v, p in SUPPORTED_DONGLES if usb.core.find(idVendor=v, idProduct=p)), None)
        if dongle is None:
            print("No ANT+ dongle found to reset. Proceeding...")
            return True
        print(f"Found dongle: {dongle.manufacturer} {dongle.product}")
        print("Attempting to reset USB device...")
        dongle.reset()
        time.sleep(1)
        print("Reset complete.")
        return True
    except usb.core.USBError as e:
        print(f"\nERROR during USB reset: {e}\nThis is likely a permissions issue. Please run this script with 'sudo'.")
        return False
    except Exception as e:
        print(f"\nAn unexpected error occurred during USB reset: {e}")
        return False

def main():
    """
    Main test function.
    
    NOTE: This test runs the broadcaster on the main thread, while the actual
    C++ integration runs it on a dedicated worker thread. Thread-related
    issues may not be detected by this test, but the startup and data flow
    logic are designed to be as close as possible to the C++ implementation.
    """
    # 1. Simulate the global variables used by the C++ integration
    ant_device_id = 54321  # This should match the default in main.cpp
    ant_verbose = True     # Enable verbose logging to match the C++ app's test flag

    if not reset_ant_dongle():
        sys.exit(1)

    print("\n--- ANT+ Broadcaster Test (C++ Architecture Simulation) ---")
    
    broadcaster = AntBroadcaster()

    try:
        # 2. Start the broadcaster using the simulated global variables
        print(f"Starting ANT+ broadcaster with device ID {ant_device_id}...")
        if not broadcaster.start(ant_device_id, ant_verbose):
            print("\nERROR: Failed to start the broadcaster (simulating C++ failure mode).")
            print("Possible causes:")
            print("- ANT+ dongle not connected or is in use by another application (e.g., Garmin Express).")
            print("- Permission issues (this script should be run with sudo).")
            print("- The 'openant' library is not correctly installed in the venv.")
            return

        print("Broadcaster started successfully. Simulating C++ worker loop at 4Hz...")
        
        start_time = time.monotonic()
        while True:
            elapsed_time = time.monotonic() - start_time
            if elapsed_time > 120:  # Run test for 2 minutes
                break

            # Simulate a fluctuating treadmill speed
            speed_kmh = 8.0 + 4.0 * math.sin(elapsed_time * 2 * math.pi / 60.0)
            speed_mps = speed_kmh / 3.6

            estimated_cadence = 0
            if speed_kmh > 0.5:
                estimated_cadence = estimate_cadence(speed_kmh)

            # 3. This mirrors the C++ call from AntWorker::doWork()
            broadcaster.send_ant_data(speed_mps, estimated_cadence)

            pace_km_str, pace_mi_str = calculate_and_format_pace_range(speed_kmh)
            
            # Show both the estimated cadence (SPM) and the resulting stride rate (strides/min)
            # NOTE: Stride rate = cadence / 2 (since 2 steps = 1 stride)
            stride_rate_per_min = estimated_cadence / 2.0
            
            output = (f"Time: {int(elapsed_time):>3}s | "
                      f"Speed: {speed_kmh:5.2f} km/h | "
                      f"Cadence: {estimated_cadence:>3} SPM | "
                      f"Stride Rate: {stride_rate_per_min:5.1f} strides/min | "
                      f"Pace/km: {pace_km_str:<12}")
            print(output, end="\r")
            sys.stdout.flush()

            # 4. Match the C++ timer frequency exactly
            time.sleep(0.250)

    except KeyboardInterrupt:
        print("\nUser interrupted test...")
    except Exception as e:
        print(f"\nTest failed with an unexpected exception: {e}")
        print("This may indicate an integration issue that would also affect the C++ code.")
    finally:
        try:
            broadcaster.stop()
            print("\nTest completed - broadcaster stopped cleanly.")
        except Exception as e:
            print(f"\nWarning: An error occurred during broadcaster cleanup: {e}")

if __name__ == "__main__":
    main()