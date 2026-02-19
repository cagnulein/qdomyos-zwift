#!/usr/bin/env python3
# =============================================================================
# QDomyos-Zwift: ANT+ SDM Broadcaster - Python Core
#
# Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
# Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
#
# Core ANT+ communication module implementing Stride-Based Speed & Distance
# Monitor (SDM) protocol for broadcasting treadmill data to Garmin watches.
# Runs in a dedicated thread and is controlled by the C++ `AntWorker`.
#
# Key features: real-time priority scheduling, cadence ramping, phased shutdown
# and one-time dongle reset with multi-vendor support.
#
# Dependencies: openant, pyusb, pybind11, Python 3.9+
# =============================================================================

import logging
import time
import threading
from typing import Optional, Tuple
import struct
import os

try:
    os.environ['PYUSB_BACKEND'] = 'libusb1'
    import usb.core
    from openant.easy.node import Node
    from openant.easy.channel import Channel
    _openant_available = True
except Exception:
    _openant_available = False

log = logging.getLogger("AntBroadcaster")

# Timing / timeout constants (tweak per-platform as needed)
BROADCAST_PERIOD = 0.250
THREAD_JOIN_TIMEOUT = 3.0
CHANNEL_CLOSE_DELAY = 0.1
NODE_STOP_DELAY = 0.2
USB_DISPOSE_DELAY = 0.05
DONGLE_RESET_SETTLE = 1.0
LOG_SKEW_WARNING_THRESHOLD = 0.25

# Precompiled struct formats for performance
_PAGE_STRUCT = struct.Struct('<BBBBBBBB')

# One-time reset guard to avoid repeated USB device scans/resets
_reset_done_lock = threading.Lock()
_reset_done = False

def _calculate_pace_range(speed_mps: float) -> Tuple[str, str]:
    """Calculates and formats the expected pace range in min/km and min/mi."""
    if speed_mps < 0.2:
        return "--:--", "--:--"
    speed_kmh = speed_mps * 3.6
    pace_min_per_km = 60.0 / speed_kmh
    total_seconds_km = pace_min_per_km * 60
    lower_bound_sec_km = (total_seconds_km // 5) * 5
    upper_bound_sec_km = lower_bound_sec_km + 5
    km_range_str = f"~{int(lower_bound_sec_km // 60)}:{int(lower_bound_sec_km % 60):02d}-{int(upper_bound_sec_km // 60)}:{int(upper_bound_sec_km % 60):02d}"
    return km_range_str, ""

def _reset_ant_dongle_once() -> bool:
    """One-time dongle reset attempt."""
    global _reset_done
    with _reset_done_lock:
        if _reset_done:
            return True
        _reset_done = True

    SUPPORTED_DONGLES = [
        (0x0fcf, 0x1009),  # Garmin USB-m
        (0x0fcf, 0x1008),  # Garmin USB2
        (0x0fcf, 0x100c),  # Garmin USB3
        (0x0fcf, 0x100e),  # Garmin mini
        (0x0fcf, 0x88a4),  # Garmin development stick
        (0x0fcf, 0x1004),  # Generic
        (0x11fd, 0x0001)   # Suunto
    ]

    log.debug("--- Starting ANT+ Dongle Reset Sequence (one-time) ---")
    try:
        dongle = next((usb.core.find(idVendor=v, idProduct=p) for v, p in SUPPORTED_DONGLES if usb.core.find(idVendor=v, idProduct=p)), None)
        if dongle is None:
            log.info("No supported ANT+ dongle found for reset. Continuing.")
            return True

        log.debug(f"Found dongle for reset: {dongle.manufacturer} {dongle.product}")
        dongle.reset()
        time.sleep(DONGLE_RESET_SETTLE)
        log.info("Dongle reset sequence finished.")
        return True
    except Exception as e:
        log.error(f"Error during ANT+ dongle reset: {e}. This may be a permissions issue.", exc_info=True)
        return True # Non-fatal

class AntBroadcaster:
    DEVICE_TYPE, TX_TYPE, PERIOD, FREQ = 124, 1, 8134, 57
    ANT_NETWORK_KEY = [0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45]

    def __init__(self):
        self._ant_node: Optional[Node] = None
        self._ant_channel: Optional[Channel] = None
        self._thread: Optional[threading.Thread] = None
        self._running = threading.Event()
        self._data_lock = threading.Lock()
        self._speed_mps = 0.0
        self._target_cadence = 0.0
        self._current_cadence = 0.0
        self._stride_accumulator = 0.0
        self._total_time = 0.0
        self._stride_count = 0
        self._last_tick = 0.0
        self._last_log_time = 0.0
        self._last_broadcast_time = 0
        self._broadcast_counter = 0
        self._total_distance = 0.0
        self._cadence_ramp_step = 12.5
        self._time_mod_base = 256000
        
        # Metrics for dashboard monitoring (RAM disk only, avoid SD card wear)
        self._metrics_file = None
        self._last_metrics_write = 0.0
        if os.path.isdir('/dev/shm') and os.access('/dev/shm', os.W_OK):
            self._metrics_file = '/dev/shm/qz_ant_metrics.json'
            log.info("Metrics enabled: writing to %s", self._metrics_file)
        else:
            log.warning("/dev/shm not writable, metrics disabled (SD card protection)")

    def _set_realtime_priority(self):
        """On Linux, attempt to set the current thread to a real-time scheduling policy."""
        if hasattr(os, 'sched_setscheduler'):
            try:
                policy = os.SCHED_FIFO
                params = os.sched_param(1)
                os.sched_setscheduler(0, policy, params) # 0 for current thread
                log.info("Successfully set broadcaster thread to real-time priority.")
            except PermissionError:
                log.warning("Could not set real-time thread priority: must run as root.")
            except Exception as e:
                log.warning(f"Failed to set real-time thread priority: {e}")

    def _broadcasting_loop(self):
        """Main loop running on a dedicated thread to send data at a fixed 4Hz."""
        self._set_realtime_priority()
        
        log.info("ANT+ broadcasting thread started.")
        self._last_tick = time.monotonic()

        while not self._running.is_set():
            now = time.monotonic()
            dt = now - self._last_tick
            self._last_tick = now

            with self._data_lock:
                speed = self._speed_mps
                target_cadence = self._target_cadence

            if target_cadence > self._current_cadence:
                self._current_cadence = min(target_cadence, self._current_cadence + self._cadence_ramp_step)
            elif target_cadence < self._current_cadence:
                self._current_cadence = max(target_cadence, self._current_cadence - self._cadence_ramp_step)
            current_cadence = self._current_cadence

            self._total_time += dt
            self._total_distance += speed * dt

            if current_cadence > 0:
                stride_rate_sps = (current_cadence * 0.5) / 60.0
                self._stride_accumulator += stride_rate_sps * dt
                if self._stride_accumulator >= 1.0:
                    num_strides = int(self._stride_accumulator)
                    self._stride_count = (self._stride_count + num_strides) & 0xFF
                    self._stride_accumulator -= num_strides

            elapsed_ms = int(self._total_time * 1000)
            speed_int = int(speed)
            speed_frac = int((speed - speed_int) * 256.0) & 0xFF
            distance_int = int(self._total_distance) & 0xFF

            try:
                if (self._broadcast_counter & 0x07) == 7:
                    cadence_half = current_cadence * 0.5
                    cadence_int = int(cadence_half)
                    cadence_frac = int((cadence_half - cadence_int) * 256.0)
                    packed_payload = _PAGE_STRUCT.pack(0x02, self._stride_count, cadence_frac, cadence_int, speed_int, speed_frac, self._stride_count, 0x00)
                else:
                    time_field_1 = ((elapsed_ms % self._time_mod_base) // 5) & 0xFF
                    time_field_2 = ((elapsed_ms % self._time_mod_base) // 1000) & 0xFF
                    delta_time_ms = elapsed_ms - self._last_broadcast_time
                    delta_time_field = (delta_time_ms >> 5) & 0xFF
                    self._last_broadcast_time = elapsed_ms
                    packed_payload = _PAGE_STRUCT.pack(0x01, time_field_1, time_field_2, distance_int, speed_int, speed_frac, self._stride_count, delta_time_field)

                if self._ant_channel:
                    self._ant_channel.send_broadcast_data(list(packed_payload))

                self._broadcast_counter += 1

                if log.isEnabledFor(logging.DEBUG) and (now - self._last_log_time >= 1.0):
                    self._last_log_time = now
                    pace_km, _ = _calculate_pace_range(speed)
                    log.debug("TX: speed=%.2f m/s | Cadence=%.1f→%.1f SPM | Stride=%d | Pace/km: %s",
                              speed, current_cadence, target_cadence, self._stride_count, pace_km)
                
                # Write metrics to RAM disk at 1Hz for dashboard monitoring (throttled from 4Hz broadcast)
                if self._metrics_file and (now - self._last_metrics_write >= 1.0):
                    self._last_metrics_write = now
                    self._write_metrics(speed, current_cadence, self._total_distance / 1000.0)

            except Exception as e:
                log.error("Broadcast error: %s. Stopping thread.", e, exc_info=True)
                self._running.set()
                break
            
            sleep_duration = BROADCAST_PERIOD - (time.monotonic() - now)
            if sleep_duration > 0:
                time.sleep(sleep_duration)
            elif sleep_duration < -LOG_SKEW_WARNING_THRESHOLD:
                if log.isEnabledFor(logging.DEBUG):
                    log.info("Broadcast thread is running: %.3fs behind schedule", -sleep_duration)

        log.info("ANT+ broadcasting thread finished.")
    
    def _write_metrics(self, speed_mps: float, cadence_spm: float, distance_km: float):
        """Atomically write metrics to RAM disk for dashboard monitoring.
        
        Uses tmp+rename for atomic writes to prevent partial reads.
        Silently skips on write failures to avoid log spam.
        """
        if not self._metrics_file:
            return
        
        import json
        
        metrics = {
            "speed_kmh": round(speed_mps * 3.6, 2),
            "cadence_spm": int(cadence_spm),
            "distance_km": round(distance_km, 3),
            "timestamp_ms": int(time.time() * 1000)
        }
        
        try:
            tmp_file = self._metrics_file + '.tmp'
            with open(tmp_file, 'w') as f:
                json.dump(metrics, f, indent=2)
                f.write('\n')  # trailing newline for cleaner cat
            
            # Atomic rename prevents partial reads
            os.replace(tmp_file, self._metrics_file)
        except Exception:
            pass  # Silently skip on failure to avoid log spam

    def start(self, device_id: int, verbose: bool) -> bool:
        level = logging.DEBUG if verbose else logging.INFO
        if not log.handlers:
            handler = logging.StreamHandler()
            formatter = logging.Formatter('%(message)s')
            handler.setFormatter(formatter)
            log.addHandler(handler)
            log.propagate = False
        log.setLevel(level)

        if not _openant_available:
            log.error("The 'openant' library is not available.")
            return False

        try:
            _reset_ant_dongle_once()
            log.info("Initializing ANT+ Node...")
            self._ant_node = Node()
            self._ant_node.set_network_key(0x00, self.ANT_NETWORK_KEY)
            self._ant_channel = self._ant_node.new_channel(Channel.Type.BIDIRECTIONAL_TRANSMIT)
            self._ant_channel.set_id(device_id, self.DEVICE_TYPE, self.TX_TYPE)
            self._ant_channel.set_period(self.PERIOD)
            self._ant_channel.set_rf_freq(self.FREQ)
            self._ant_channel.open()
            
            self._running.clear()
            self._thread = threading.Thread(target=self._broadcasting_loop, daemon=True)
            self._thread.start()
            log.info("ANT+ broadcaster started successfully (Device ID %d).", device_id)
            return True
        except Exception as e:
            log.error("Failed to initialize ANT+ broadcaster: %s", e, exc_info=verbose)
            self.stop()
            return False

    def stop(self):
        """Enhanced shutdown with explicit USB release and state verification.

        This phased shutdown ensures the ANT+ channel and USB device are
        released cleanly before allowing a subsequent start().
        """
        # PHASE 0: Ensure we signal shutdown and allow cleanup to proceed
        if not getattr(self, '_running', None) or not hasattr(self, '_running'):
            # If state missing, try a safe reset
            try:
                self._running = threading.Event()
            except Exception:
                pass

        # If not already set, signal stop
        try:
            if not self._running.is_set():
                self._running.set()
        except Exception:
            # best effort
            pass

        log.info("Stopping ANT+ broadcaster...")

        # PHASE 1: Wait for the broadcast thread to finish
        try:
                if self._thread is not None and getattr(self._thread, 'is_alive', lambda: False)():
                    self._thread.join(timeout=THREAD_JOIN_TIMEOUT)
                    if getattr(self._thread, 'is_alive', lambda: False)():
                        log.warning("Broadcast thread did not terminate cleanly within timeout")
        except Exception as e:
            log.warning("Error while joining broadcast thread: %s", e)

        # PHASE 2: Close ANT+ channel and allow USB layer to propagate
        try:
            if getattr(self, '_ant_channel', None):
                try:
                    self._ant_channel.close()
                except Exception as e:
                    log.warning("Channel close warning: %s", e)
                try:
                    time.sleep(CHANNEL_CLOSE_DELAY)
                except Exception:
                    pass
                self._ant_channel = None
        except Exception as e:
            log.warning("Unexpected channel cleanup error: %s", e)

        # PHASE 3: Stop node and allow kernel driver to detach
        try:
            if getattr(self, '_ant_node', None):
                try:
                    self._ant_node.stop()
                except Exception as e:
                    log.warning("Node stop warning: %s", e)
                try:
                    time.sleep(NODE_STOP_DELAY)
                except Exception:
                    pass
                self._ant_node = None
        except Exception as e:
            log.warning("Unexpected node cleanup error: %s", e)

        # PHASE 4: Force garbage collection to release PyUSB references
        try:
            import gc
            gc.collect()
        except Exception:
            pass

        # PHASE 4b: Attempt explicit USB resource disposal/reset for known dongles.
        # This helps ensure libusb releases kernel claims on some platforms.
        try:
            try:
                import usb.core as _usb_core
                import usb.util as _usb_util
            except Exception:
                _usb_core = None
                _usb_util = None

            if _usb_core is not None:
                SUPPORTED = [
                    (0x0fcf, 0x1009), (0x0fcf, 0x1008), (0x0fcf, 0x100c),
                    (0x0fcf, 0x100e), (0x0fcf, 0x88a4), (0x0fcf, 0x1004),
                    (0x11fd, 0x0001)
                ]
                for (v, p) in SUPPORTED:
                    try:
                        d = _usb_core.find(idVendor=v, idProduct=p)
                        if d is None:
                            continue
                        # Dispose python-side resources if available
                        try:
                            if _usb_util is not None:
                                _usb_util.dispose_resources(d)
                        except Exception:
                            pass
                        # Try a lightweight reset to nudge kernel driver detach
                        try:
                            d.reset()
                        except Exception:
                            pass
                        try:
                            time.sleep(USB_DISPOSE_DELAY)
                        except Exception:
                            pass
                    except Exception:
                        continue
        except Exception:
            pass

        # PHASE 5: Reset internal state and clear running flag for next start
        try:
            self._thread = None
        except Exception:
            pass
        try:
            if hasattr(self, '_running'):
                self._running.clear()
        except Exception:
            pass

        # Reset counters and state variables used by broadcasting loop
        try:
            self._speed_mps = 0.0
            self._target_cadence = 0.0
            self._current_cadence = 0.0
            self._stride_accumulator = 0.0
            self._total_time = 0.0
            self._stride_count = 0
            self._last_tick = 0.0
            self._last_log_time = 0.0
            self._last_broadcast_time = 0
            self._broadcast_counter = 0
            self._total_distance = 0.0
        except Exception:
            pass
        
        # Clean up metrics file
        try:
            if self._metrics_file:
                if os.path.exists(self._metrics_file):
                    os.remove(self._metrics_file)
                tmp = self._metrics_file + '.tmp'
                if os.path.exists(tmp):
                    os.remove(tmp)
        except Exception:
            pass

        log.info("ANT+ broadcaster stopped and USB device released.")

    def send_ant_data(self, speed_mps: float, cadence_spm: int):
        with self._data_lock:
            self._speed_mps = speed_mps
            self._target_cadence = float(cadence_spm)
            if self._speed_mps < 0.1:
                self._current_cadence = 0.0
                self._target_cadence = 0.0