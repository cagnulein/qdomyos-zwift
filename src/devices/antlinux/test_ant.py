#!/usr/bin/env python3
# -----------------------------------------------------------------------------
# QDomyos-Zwift: ANT+ Virtual Footpod Feature
# Standalone ANT+ Test Script
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor(s): bassai-sho
# Development assisted by AI analysis tools
# Licensed under GPL-3.0 - see project repository for full license
#
# This script tests ANT+ functionality independently of the QDomyos-Zwift binary.
# It simulates a running treadmill broadcasting data to help troubleshoot ANT+
# hardware, Python environment, and watch pairing issues.
#
# Usage:
#   cd ~/qdomyos-zwift-x86-64-ant  # or qdomyos-zwift-arm64-ant
#   sudo ~/ant_venv/bin/python3 ./test_ant.py
# -----------------------------------------------------------------------------

import time
import sys
import argparse
import os

# Ensure the ant_broadcaster module can be found in the same directory
script_dir = os.path.dirname(os.path.abspath(__file__))
if script_dir not in sys.path:
    sys.path.insert(0, script_dir)

# Ensure pyusb symbols are imported into the module namespace for USB ops
try:
    import usb.core
    import usb.util
except Exception:
    # If check_required_packages passed earlier, this should not fail.
    # Leave errors to be handled by the caller.
    pass


def check_required_packages():
    """Self-check for required Python packages used by the test script.
    Prints compact one-line diagnostics (prefixed with '-') so callers
    (the TUI) can display a concise summary.
    Returns True if all packages are present, False otherwise.
    """
    required = [
        ("openant", "openant"),
        ("pyusb", "usb.core"),
        ("pybind11", "pybind11"),
    ]
    missing = []
    for label, modname in required:
        try:
            __import__(modname)
        except Exception:
            missing.append(label)

    if missing:
        # Emit compact bullets for the TUI to capture and show
        for m in missing:
            if m == "pyusb":
                print(f"- pyusb not installed")
            elif m == "openant":
                print(f"- openant not installed")
            elif m == "pybind11":
                print(f"- pybind11 not installed")
            else:
                print(f"- {m} not installed")
        return False
    return True


# Early self-check: if any runtime packages are missing, print and exit
if not check_required_packages():
    # Informational exit (non-zero) so callers know the test didn't run
    sys.exit(2)

# Attempt direct import and provide a clear error message if missing
try:
    from ant_broadcaster import AntBroadcaster
except Exception as e:
    sys.stderr.write("ERROR: Cannot import 'ant_broadcaster'. Ensure 'ant_broadcaster.py' is next to this script.\n")
    sys.stderr.write(f"Details: {e}\n")
    sys.exit(1)

"""
Improved Cadence Estimation Model
Based on biomechanics research and walk-to-run transition studies

Research-backed transition points:
- Walk-to-run transition: 7.0-7.2 km/h at ~140 SPM
- Casual walking: 90-110 SPM (3-5 km/h)
- Brisk walking: 110-140 SPM (5-7 km/h)
- Easy running: 160-170 SPM (7-10 km/h)
- Moderate running: 170-180 SPM (10-14 km/h)
- Fast running: 180-190 SPM (14+ km/h)
"""
def estimate_cadence(speed_kmh: float) -> int:
    """
    Estimates realistic cadence in SPM based on biomechanics research.
    Improved Cadence Estimation Model:
    Based on biomechanics research and walk-to-run transition studies
    Research-backed transition points:
    - Walk-to-run transition: 7.0-7.2 km/h at ~140 SPM
    - Casual walking: 90-110 SPM (3-5 km/h)
    - Brisk walking: 110-140 SPM (5-7 km/h)
    - Easy running: 160-170 SPM (7-10 km/h)
    - Moderate running: 170-180 SPM (10-14 km/h)
    - Fast running: 180-190 SPM (14+ km/h)  
    """
    if speed_kmh < 0.5:
        return 0
    
    # WALKING ZONE (0.5 - 7.0 km/h)
    elif speed_kmh < 7.0:
        if speed_kmh < 3.0:
            # Very slow walking: 0 → 90 SPM
            cadence = speed_kmh * 30.0
        else:
            # Normal to brisk walking: 90 → 140 SPM
            cadence = (speed_kmh - 3.0) * 12.5 + 90.0
    
    # RUNNING ZONE (7.0+ km/h)
    else:
        # Easy jog to fast run: 160 → 200 SPM
        cadence = (speed_kmh - 7.0) * 4.0 + 160.0
        cadence = min(cadence, 200.0)
    
    # Round to nearest even number (Garmin displays even values)
    return int(cadence + 0.5) & ~1



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
        (0x0fcf, 0x1009),  # Garmin USB-m Stick
        (0x0fcf, 0x1008),  # Garmin USB2 Stick
        (0x0fcf, 0x100c),  # Garmin USB3 Stick
        (0x0fcf, 0x100e),  # Garmin mini Stick
        (0x0fcf, 0x88a4),  # Garmin development stick
        (0x0fcf, 0x1004),  # Dynastream/Generic ANTUSB2 Stick (Cycplus, Anself, etc.)
        (0x11fd, 0x0001)   # Suunto ANT+ Dongle
    ]
    print("--- Pre-Test USB Reset ---")
    # Retry loop to handle transient kernel/device races
    attempts = 6
    for attempt in range(1, attempts + 1):
        try:
            dongle = None
            for v, p in SUPPORTED_DONGLES:
                try:
                    d = usb.core.find(idVendor=v, idProduct=p)
                except Exception:
                    d = None
                if d:
                    dongle = d
                    break

            if dongle is None:
                print(f"- Dongle missing/in use (attempt {attempt}/{attempts})")
                if attempt < attempts:
                    time.sleep(0.6)
                    continue
                return False

            # Try to safely detach kernel driver interfaces if present
            try:
                for cfg in dongle:
                    for intf in cfg:
                        try:
                            if hasattr(dongle, 'is_kernel_driver_active') and dongle.is_kernel_driver_active(intf.bInterfaceNumber):
                                try:
                                    dongle.detach_kernel_driver(intf.bInterfaceNumber)
                                except Exception:
                                    pass
                        except Exception:
                            pass
            except Exception:
                pass

            # Informational
            man = getattr(dongle, 'manufacturer', '') or ''
            prod = getattr(dongle, 'product', '') or ''
            print(f"Found dongle: {man} {prod}")

            # Attempt reset with backoff on transient USB errors
            try:
                dongle.reset()
                time.sleep(0.5)
                print("- Reset complete")
                return True
            except usb.core.USBError as e:
                msg = str(e)
                if 'Access' in msg or 'Permission' in msg or (hasattr(e, 'errno') and e.errno == 13):
                    print("- Permission error (run sudo)")
                    return False
                print(f"- USB reset error (attempt {attempt}/{attempts}): {e}")
                # Try to show which process holds the device (best-effort)
                try:
                    import subprocess
                    bus = getattr(dongle, 'bus', None)
                    addr = getattr(dongle, 'address', None)
                    if bus and addr:
                        bus_s = f"{int(bus):03d}" if isinstance(bus, int) else str(bus)
                        addr_s = f"{int(addr):03d}" if isinstance(addr, int) else str(addr)
                        out = subprocess.check_output(["/usr/bin/which", "lsof"], stderr=subprocess.DEVNULL).strip()
                        if out:
                            try:
                                l = subprocess.check_output(["lsof", f"/dev/bus/usb/{bus_s}/{addr_s}"], stderr=subprocess.DEVNULL, text=True)
                                if l:
                                    print("- Device held by process:")
                                    for ln in l.splitlines()[:4]:
                                        print(f"- {ln}")
                            except Exception:
                                pass
                except Exception:
                    pass

                if attempt < attempts:
                    time.sleep(0.8)
                    continue
                return False
        except Exception as e:
            print(f"- Unexpected error during USB probe/reset: {e}")
            if attempt < attempts:
                time.sleep(0.6)
                continue
            return False

def main():
    """Main test function."""
    ant_device_id = 54321
    ant_verbose = False

    if not reset_ant_dongle():
        sys.exit(1)

    print("\n--- ANT+ Broadcaster Test (C++ Architecture Simulation) ---")
    
    broadcaster = AntBroadcaster()

    parser = argparse.ArgumentParser(description='ANT+ test runner')
    parser.add_argument('--dashboard', action='store_true', help='Emit compact status lines for dashboard consumption')
    args = parser.parse_args()

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
            # Precompute a compact pace string (choose upper bound)
            pace_km_str, _ = calculate_and_format_pace_range(speed_kmh)
            pace_upper = pace_km_str
            if '-' in pace_km_str:
                pace_upper = pace_km_str.split('-')[-1].strip()
                pace_upper = pace_upper.lstrip('~')

            while True:
                loop_start_time = time.monotonic()
                stage_elapsed = loop_start_time - stage_start_time

                if stage_elapsed >= duration:
                    break

                speed_mps = speed_kmh / 3.6

                broadcaster.send_ant_data(speed_mps, expected_cadence)

                # Compact dashboard-friendly status line (CR-terminated)
                if args.dashboard:
                    # Prefix with the human-readable stage name so the
                    # dashboard can show which stage is active.
                    status = f"{stage_name} | Cadence:{expected_cadence} Speed:{speed_kmh:.1f} Pace:{pace_upper} [ {int(stage_elapsed):>2}s / {duration}s ]"
                    # Pad to avoid remnants of previous longer lines
                    print(f"{status:<80}", end="\r")
                    sys.stdout.flush()
                else:
                    progress_str = f"  Running... [ {int(stage_elapsed):>2}s / {duration}s ]"
                    print(f"{progress_str:<50}", end="\r")
                    sys.stdout.flush()

                # Self-correcting timer to ensure a precise 4Hz loop rate
                work_duration = time.monotonic() - loop_start_time
                sleep_duration = 0.250 - work_duration
                if sleep_duration > 0:
                    time.sleep(sleep_duration)

            # Ensure we end the stage with a newline so logs remain readable
            if args.dashboard:
                print("")

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