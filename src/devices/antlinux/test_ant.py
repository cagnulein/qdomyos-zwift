#!/usr/bin/env python3
# =============================================================================
# QDomyos-Zwift: ANT+ Hardware Test Script
#
# Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
# Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
#
# Standalone test script for validating ANT+ hardware and watch pairing.
# Simulates multi-stage workout data for comparison and troubleshooting.
#
# Usage:
#   sudo ./setup-dashboard.sh (recommended) or
#   sudo ~/ant_venv/bin/python3 ./test_ant.py
# =============================================================================

import time
import sys
import argparse
import os
import signal
import functools

# Force all print statements to flush immediately to stdout.
# This ensures the bash dashboard receives "Signal:Startup" and workout data
# instantly, preventing the "HARDWARE TIMEOUT" error.
print = functools.partial(print, flush=True)

def write_pidfile(path: str) -> None:
    """Write current process PID to `path` (best-effort)."""
    try:
        d = os.path.dirname(path)
        if d and not os.path.isdir(d):
            os.makedirs(d, exist_ok=True)
        with open(path, 'w') as f:
            f.write(str(os.getpid()))
    except Exception:
        pass

# Timing constants
PROBE_SHORT_SLEEP = 0.6
PROBE_LONG_SLEEP = 0.8
RESET_SETTLE = 1.0
LOOP_PERIOD = 0.250

# Ensure the ant_broadcaster module can be found in the same directory
script_dir = os.path.dirname(os.path.abspath(__file__))
if script_dir not in sys.path:
    sys.path.insert(0, script_dir)

# --- EARLY STARTUP SIGNAL ---
# This must run before any package imports (like usb/openant) 
# so the dashboard detects life immediately.
if '--dashboard' in sys.argv:
    print("STAGE 0/1: Initializing | Signal:Startup")

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
# Emit an early startup signal for the dashboard to detect before checking packages
if '--dashboard' in sys.argv:
    # --- CRITICAL FIX: Explicit flush to ensure bash script sees this immediately ---
    print("STAGE 0/1: Initializing | Signal:Startup")
    sys.stdout.flush()

if not check_required_packages():
    # Informational exit (non-zero) so callers know the test didn't run
    sys.exit(2)

# Module-global broadcaster reference so signal handlers can stop it
broadcaster = None

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



def calculate_pace(speed_kmh: float) -> str:
    """Calculates and formats the pace in min/km as MM:SS."""
    if speed_kmh <= 0:
        return "00:00"
    pace_min_per_km = 60.0 / speed_kmh
    minutes = int(pace_min_per_km)
    seconds = int((pace_min_per_km - minutes) * 60)
    return f"{minutes}:{seconds:02d}"

def calculate_pace(speed_kmh: float) -> str:
    """Calculates and formats the expected pace in min/km."""
    if speed_kmh < 0.5:
        return "--:--"
    pace_min_per_km_float = 60.0 / speed_kmh
    total_seconds_km = pace_min_per_km_float * 60
    lower_bound_sec_km = (total_seconds_km // 5) * 5
    km_str = "{}:{:02d}".format(
        int(lower_bound_sec_km // 60), int(lower_bound_sec_km % 60)
    )
    return km_str

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
                    time.sleep(PROBE_SHORT_SLEEP)
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

            try:
                if dongle.is_kernel_driver_active(0):
                    print("Detaching kernel driver...")
                    dongle.detach_kernel_driver(0)
            except Exception as e:
                print(f"Note: Could not detach kernel driver: {e}")

            # Informational
            man = getattr(dongle, 'manufacturer', '') or ''
            prod = getattr(dongle, 'product', '') or ''
            print(f"Found dongle: {man} {prod}")

            # Attempt reset with backoff on transient USB errors
            try:
                dongle.reset()
                time.sleep(RESET_SETTLE)
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
                    time.sleep(PROBE_LONG_SLEEP)
                    continue
                return False
        except Exception as e:
            print(f"- Unexpected error during USB probe/reset: {e}")
            if attempt < attempts:
                time.sleep(PROBE_SHORT_SLEEP)
                continue
            return False

def main():
    global broadcaster
    ant_device_id = 54321
    
    parser = argparse.ArgumentParser(description='ANT+ test runner')
    parser.add_argument('--dashboard', action='store_true', help='Emit compact status lines for dashboard consumption')
    parser.add_argument('--pidfile', type=str, help='Write PID to this file for external monitoring')
    parser.add_argument('--imperial', action='store_true', help='Use imperial units (miles) for pace display')
    args = parser.parse_args()

    # Write PID for the dashboard monitor
    if args.pidfile:
        write_pidfile(args.pidfile)

    # 1. Reset Hardware to clear "Resource Busy" errors
    if not reset_ant_dongle():
        print("Warning: USB reset failed. Trying to proceed anyway...")

    # 2. Start Broadcaster
    print(f"Initializing ANT+ Node with ID {ant_device_id}...")
    try:
        broadcaster = AntBroadcaster()
        if not broadcaster.start(ant_device_id, False):
            print("ERROR: Broadcaster.start() returned False.")
            sys.exit(1)
    except Exception as e:
        print(f"CRITICAL ERROR starting broadcaster: {e}")
        sys.exit(1)

    # 3. Setup Signal Handlers for clean exit
    def _term(signum, frame):
        if broadcaster: broadcaster.stop()
        sys.exit(0)
    signal.signal(signal.SIGTERM, _term)
    signal.signal(signal.SIGINT, _term)

    print("Broadcaster active. Starting workout simulation...")

    test_plan = [
        ("Warm-up Walk", 5.0, 20),
        ("Jogging", 8.5, 30),
        ("Running", 12.0, 30),
        ("Jogging", 8.5, 20),
        ("Cool-down Walk", 5.0, 20),
        ("Stopping", 0.0, 10),
    ]

    try:
        for stage_name, speed, duration in test_plan:
            cadence = estimate_cadence(speed)
            # Use pace range based on unit preference and take upper bound to match Garmin display
            pace_range_km, pace_range_mi = calculate_and_format_pace_range(speed)
            pace_range = pace_range_mi if args.imperial else pace_range_km
            # Extract upper bound: "~12:00-12:05" -> "12:05"
            pace = pace_range.split('-')[1] if '-' in pace_range else pace_range.replace('~', '')

            start = time.monotonic()
            last_print_second = -1  # Track last printed second to avoid duplicates
            
            while (time.monotonic() - start) < duration:
                loop_s = time.monotonic()
                elapsed = loop_s - start
                elapsed_sec = int(elapsed)

                # --- CRITICAL FIX: Catch USB Transmission Errors ---
                # This prevents the script from crashing if a single packet fails.
                try:
                    broadcaster.send_ant_data(speed / 3.6, cadence)
                except Exception:
                    # Ignore transient USB errors; the dashboard will just
                    # see a skipped packet rather than a full crash.
                    pass

                # FIX: Only print when the second actually changes (prevents stuttering)
                if args.dashboard and elapsed_sec != last_print_second:
                    # Format: STAGE | Cadence:X Speed:Y Pace:Z [ E / T ]
                    print(f"{stage_name} | Cadence:{cadence} Speed:{speed:.1f} Pace:{pace} [ {elapsed_sec:>2}s / {duration}s ]", flush=True)
                    last_print_second = elapsed_sec
                elif not args.dashboard:
                    print(f"Running... {elapsed_sec}s", end="\r")

                # Sleep remainder of 4Hz cycle
                rem = LOOP_PERIOD - (time.monotonic() - loop_s)
                if rem > 0: time.sleep(rem)
            
            # FIX: Show final complete state before transitioning to next stage
            # This gives the dashboard UI time to render "20/20" completion
            if args.dashboard:
                print(f"{stage_name} | Cadence:{cadence} Speed:{speed:.1f} Pace:{pace} [ {duration}s / {duration}s ]", flush=True)
            
            # Pause 1 second to allow UI to display completion state
            time.sleep(1.0)

    except KeyboardInterrupt:
        print("\nUser interrupted test.")
    except Exception as e:
        # Catch-all for non-USB errors to print to log before exit
        print(f"FATAL EXCEPTION: {e}")
    finally:
        if broadcaster: broadcaster.stop()
        print("Test finished.")

if __name__ == "__main__":
    main()