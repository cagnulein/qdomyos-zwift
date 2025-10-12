#!/usr/bin/env python3
# -----------------------------------------------------------------------------
# QDomyos-Zwift: ANT+ Virtual Footpod Feature
# Test Script for ant_broadcaster.py (C++ Architecture Simulation)
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor(s): bassai-sho
# AI analysis tools (Claude, Gemini) were used to assist coding and debugging
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
from pathlib import Path

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
    Estimates a realistic running cadence and ensures it is an even number
    to match the display behavior of Garmin watches.
    """
    if speed_kmh < 5.0:
        cadence = int(speed_kmh * 15.0 + 45.0)
    else:
        cadence = int(speed_kmh * 5.0 + 110.0)
    
    # Round down to the nearest even number
    return min(cadence, 200) & ~1

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
    SUPPORTED_DONGLES = [
        (0x0fcf, 0x1009),  # Garmin USB-m Stick (Modern)
        (0x0fcf, 0x1008),  # Garmin USB2 Stick (Older)
        (0x0fcf, 0x1004),  # Dynastream/Generic ANTUSB2 Stick (Cycplus, Anself, etc.)
        (0x11fd, 0x0001)   # Suunto ANT+ Dongle
    ]
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
    """Main test function."""
    ant_device_id = 54321
    ant_verbose = False

    if not reset_ant_dongle():
        sys.exit(1)

    print("\n--- ANT+ Broadcaster Test (C++ Architecture Simulation) ---")
    
    broadcaster = AntBroadcaster()

    try:
        print(f"Starting ANT+ broadcaster with device ID {ant_device_id}...")
        if not broadcaster.start(ant_device_id, ant_verbose):
            print("\nERROR: Failed to start the broadcaster.")
            print("Possible causes:")
            print("- ANT+ dongle not connected or is in use by another application.")
            print("- Permission issues (run with sudo).")
            print("- 'openant' library not installed in the venv.")
            return

        print("Broadcaster started successfully. Simulating a structured workout...")
        print("Please check your watch and compare with the EXPECTED values below.")

        test_plan = [
            ("Warm-up Walk", 5.0, 20),
            ("Jogging", 8.5, 30),
            ("Running", 12.0, 30),
            ("Jogging", 8.5, 20),
            ("Cool-down Walk", 5.0, 20),
            ("Stopping", 0.0, 10),
        ]

        for i, (stage_name, speed_kmh, duration) in enumerate(test_plan):
            
            expected_cadence = estimate_cadence(speed_kmh) if speed_kmh > 0.5 else 0
            pace_km_str, _ = calculate_and_format_pace_range(speed_kmh)
            
            print("\n" + "="*50)
            print(f" STAGE {i+1}/{len(test_plan)}: {stage_name}")
            print("-"*50)
            print(f"  TARGET SPEED:   {speed_kmh:.1f} km/h")
            print(f"  EXPECTED PACE:  {pace_km_str}")
            print(f"  EXPECTED CADENCE: {expected_cadence} SPM")
            print("="*50)
            
            stage_start_time = time.monotonic()
            while True:
                loop_start_time = time.monotonic()
                stage_elapsed = loop_start_time - stage_start_time
                
                if stage_elapsed >= duration:
                    break

                speed_mps = speed_kmh / 3.6
                
                broadcaster.send_ant_data(speed_mps, expected_cadence)
                
                progress_str = f"  Running... [ {int(stage_elapsed):>2}s / {duration}s ]"
                print(f"{progress_str:<50}", end="\r")
                sys.stdout.flush()

                # Self-correcting timer to ensure a precise 4Hz loop rate
                work_duration = time.monotonic() - loop_start_time
                sleep_duration = 0.250 - work_duration
                if sleep_duration > 0:
                    time.sleep(sleep_duration)

    except KeyboardInterrupt:
        print("\n\nUser interrupted test...")
    except Exception as e:
        print(f"\n\nTest failed with an unexpected exception: {e}")
    finally:
        try:
            broadcaster.stop()
            print("\n\nTest completed - broadcaster stopped cleanly.")
        except Exception as e:
            print(f"\n\nWarning: An error occurred during broadcaster cleanup: {e}")

if __name__ == "__main__":
    main()