#!/usr/bin/env python3
# -----------------------------------------------------------------------------
# QDomyos-Zwift: ANT+ Virtual Footpod Feature
# ANT+ SDM Broadcaster - Python Core
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor(s): bassai-sho
# AI analysis tools (Claude, Gemini) were used to assist coding and debugging
#
# Licensed under GPL-3.0 - see project repository for full license
#
# This script handles the low-level ANT+ communication using the `openant`
# library. It runs in a dedicated thread, broadcasting treadmill speed data
# using a specific ANT+ SDM (footpod) payload structure determined to be
# compatible with Garmin watch receivers. This module is designed to be
# embedded and controlled by the C++ AntWorker class.
# -----------------------------------------------------------------------------

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
        (0x0fcf, 0x1004),  # Generic
        (0x11fd, 0x0001)   # Suunto
    ]

    log.info("--- Starting ANT+ Dongle Reset Sequence (one-time) ---")
    try:
        dongle = next((usb.core.find(idVendor=v, idProduct=p) for v, p in SUPPORTED_DONGLES if usb.core.find(idVendor=v, idProduct=p)), None)
        if dongle is None:
            log.info("No supported ANT+ dongle found for reset. Continuing.")
            return True

        log.info(f"Found dongle for reset: {dongle.manufacturer} {dongle.product}")
        dongle.reset()
        time.sleep(1)
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

            except Exception as e:
                log.error("Broadcast error: %s. Stopping thread.", e, exc_info=True)
                self._running.set()
                break
            
            sleep_duration = 0.250 - (time.monotonic() - now)
            if sleep_duration > 0:
                time.sleep(sleep_duration)
            elif sleep_duration < -0.25: 
                if log.isEnabledFor(logging.DEBUG):
                    log.info("Broadcast thread is running: %.3fs behind schedule", -sleep_duration)

        log.info("ANT+ broadcasting thread finished.")

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
        if self._running.is_set():
            return
        
        log.info("Stopping ANT+ broadcaster...")
        self._running.set()

        if self._thread is not None:
            self._thread.join(timeout=2.0) # Increased timeout for safety
        
        try:
            if self._ant_channel: self._ant_channel.close()
            if self._ant_node: self._ant_node.stop()
        except Exception as e:
            log.warning("Non-critical error during ANT+ resource cleanup: %s", e)

        self._ant_channel, self._ant_node, self._thread = None, None, None
        
        # Reset all state by calling __init__
        self.__init__()
        
        log.info("ANT+ broadcaster stopped and resources released.")

    def send_ant_data(self, speed_mps: float, cadence_spm: int):
        with self._data_lock:
            self._speed_mps = speed_mps
            self._target_cadence = float(cadence_spm)
            if self._speed_mps < 0.1:
                self._current_cadence = 0.0
                self._target_cadence = 0.0