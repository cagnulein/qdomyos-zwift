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
from typing import Optional
import struct

try:
    import os
    os.environ['PYUSB_BACKEND'] = 'libusb1'
    import usb.core
    from openant.easy.node import Node
    from openant.easy.channel import Channel
    _openant_available = True
except ImportError:
    _openant_available = False

log = logging.getLogger("AntBroadcaster")

def _calculate_pace_range(speed_mps: float) -> (str, str):
    """Calculates and formats the expected pace range in min/km and min/mi."""
    if speed_mps < 0.2:
        return "--:--", "--:--"
    
    speed_kmh = speed_mps * 3.6
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

def _reset_ant_dongle():
    """Finds and resets the first available ANT+ USB dongle to ensure a clean state."""
    SUPPORTED_DONGLES = [
        (0x0fcf, 0x1009),  # Garmin USB-m Stick (Modern)
        (0x0fcf, 0x1008),  # Garmin USB2 Stick (Older)
        (0x0fcf, 0x1004),  # Dynastream/Generic ANTUSB2 Stick (Cycplus, Anself, etc.)
        (0x11fd, 0x0001)   # Suunto ANT+ Dongle
    ]
    try:
        dongle = next((usb.core.find(idVendor=v, idProduct=p) for v, p in SUPPORTED_DONGLES if usb.core.find(idVendor=v, idProduct=p)), None)
        if dongle is None:
            log.warning("No ANT+ dongle found to reset. Proceeding with initialization.")
            return True
        log.info(f"Found ANT+ dongle for reset: {dongle.manufacturer} {dongle.product}")
        dongle.reset()
        time.sleep(1)
        log.info("ANT+ dongle reset successfully.")
        return True
    except Exception as e:
        log.error(f"Error during ANT+ dongle reset: {e}. This may be a permissions issue.")
        return False

class AntBroadcaster:
    """
    Manages an ANT+ Stride-based Speed and Distance Monitor (SDM) broadcast channel.
    This class is thread-safe and designed to be controlled from a parent application.
    
    PAYLOAD SOLUTION (Test #9):
    - Broadcast Page 1 seven times, then Page 2 (with speed) once.
    - Cadence is sent on Page 2.
    - Cadence is ramped smoothly to prevent instability.
    - Result: Stable pace AND correct cadence.
    """
    DEVICE_TYPE, TX_TYPE, PERIOD, FREQ = 124, 1, 8134, 57
    ANT_NETWORK_KEY = [0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45]
    
    def __init__(self):
        self._ant_node: Optional[Node] = None
        self._ant_channel: Optional[Channel] = None
        self._thread: Optional[threading.Thread] = None
        self._running = threading.Event()
        self._data_lock = threading.Lock()
        
        # State variables
        self._speed_mps = 0.0
        self._stride_accumulator = 0.0        
        self._total_time = 0.0
        self._stride_count = 0
        self._last_tick = 0.0
        self._last_log_time = 0.0
        self._last_broadcast_time = 0
        self._broadcast_counter = 0
        self._total_distance = 0.0
        
        # Cadence ramping variables
        self._target_cadence = 0.0
        self._current_cadence = 0.0
        # Ramp step to achieve ~3 second ramp from 0 to 150 SPM
        self._cadence_ramp_step = 12.5

        # Cached values for performance
        self._time_mod_base = 256000

    def _broadcasting_loop(self):
        """The main loop that runs on a dedicated thread to send data at ~4Hz."""
        log.info("ANT+ broadcasting thread started.")
        self._last_tick = time.monotonic()
        
        while not self._running.is_set():
            now = time.monotonic()
            dt = now - self._last_tick
            self._last_tick = now
            self._total_time += dt

            with self._data_lock:
                # RAMPING: Smoothly move current cadence toward the target cadence
                if self._target_cadence > self._current_cadence:
                    self._current_cadence = min(self._target_cadence, self._current_cadence + self._cadence_ramp_step)
                elif self._target_cadence < self._current_cadence:
                    self._current_cadence = max(self._target_cadence, self._current_cadence - self._cadence_ramp_step)
                
                current_speed = self._speed_mps
                current_cadence = self._current_cadence
                target_cadence_for_log = self._target_cadence # For logging

            # Calculate distance and strides
            self._total_distance += current_speed * dt
            
            # Only perform stride calculations if moving
            if current_cadence > 0:
                stride_rate_sps = current_cadence * 0.5 / 60.0
                strides_this_tick = stride_rate_sps * dt
                self._stride_accumulator += strides_this_tick
                
                if self._stride_accumulator >= 1.0:
                    num_strides = int(self._stride_accumulator)
                    self._stride_count = (self._stride_count + num_strides) & 0xFF
                    self._stride_accumulator -= num_strides

            # Calculate common data fields
            elapsed_ms = int(self._total_time * 1000)
            speed_int = int(current_speed)
            speed_frac = int(round((current_speed - speed_int) * 256.0)) & 0xFF
            distance_int = int(self._total_distance) & 0xFF # Optimized modulo
            
            try:
                # --- 7:1 PAYLOAD SELECTION LOGIC ---
                # Manufacturer info broadcast has been removed for faster data startup.
                # The 7:1 payload cycle now starts immediately.
                if (self._broadcast_counter & 0x07) == 7:
                    # Send Page 2 (Cadence & Speed)
                    page_name = "Cadence/Speed (Page 2)"
                    cadence_strides = current_cadence * 0.5
                    cadence_int = int(cadence_strides)
                    cadence_frac = int((cadence_strides - cadence_int) * 256.0)
                    
                    packed_payload = struct.pack('<BBBBBBBB', 
                                                0x02, self._stride_count, cadence_frac,
                                                cadence_int, speed_int, speed_frac,
                                                self._stride_count, 0x00)
                else:
                    # Send Page 1 (Standard Speed/Distance)
                    page_name = "Speed/Distance (Page 1)"
                    time_field_1 = ((elapsed_ms % self._time_mod_base) // 5) & 0xFF
                    time_field_2 = ((elapsed_ms % self._time_mod_base) // 1000) & 0xFF
                    delta_time_ms = elapsed_ms - self._last_broadcast_time
                    self._last_broadcast_time = elapsed_ms
                    delta_time_field = (delta_time_ms >> 5) & 0xFF
                    
                    packed_payload = struct.pack('<BBBBBBBB', 
                                                0x01, time_field_1, time_field_2,
                                                distance_int, speed_int, speed_frac,
                                                self._stride_count, delta_time_field)

                if self._ant_channel:
                    self._ant_channel.send_broadcast_data(list(packed_payload))

                    if log.isEnabledFor(logging.DEBUG) and (now - self._last_log_time >= 1.0):
                        self._last_log_time = now
                        pace_km, pace_mi = _calculate_pace_range(current_speed)
                        log.debug("TX %s: speed=%.2f m/s | Cadence=%.1f→%.1f SPM | Stride=%d | Pace/km: %s",
                                  page_name, current_speed, target_cadence_for_log, 
                                  self._stride_count, pace_km)
                        
                self._broadcast_counter += 1
                
            except Exception as e:
                log.error(f"Broadcast error: {e}. Stopping thread.", exc_info=True)
                self._running.set()

            # More precise timing for a true 4Hz broadcast rate
            sleep_duration = 0.250 - (time.monotonic() - now)
            if sleep_duration > 0:
                time.sleep(sleep_duration)
            elif sleep_duration < -0.01:
                log.warning(f"Broadcast loop running slow: {-sleep_duration:.3f}s behind")

        log.info("ANT+ broadcasting thread finished.")

    def start(self, device_id: int, verbose: bool) -> bool:
        if self._thread is not None and self._thread.is_alive():
            log.warning("Broadcaster is already running.")
            return True
        
        level = logging.DEBUG if verbose else logging.INFO
        if not log.handlers:
            handler = logging.StreamHandler()
            formatter = logging.Formatter('%(message)s')
            handler.setFormatter(formatter)
            log.addHandler(handler)
            log.propagate = False
        log.setLevel(level)

        if not _openant_available:
            log.error("The 'openant' library is not installed.")
            return False
            
        try:
            if not _reset_ant_dongle():
                return False
            
            log.info("Attempting to initialize ANT+ Node...")
            self._ant_node = Node()
            log.info("ANT+ Node initialized.")
            
            self._ant_node.set_network_key(0x00, self.ANT_NETWORK_KEY)
            self._ant_channel = self._ant_node.new_channel(Channel.Type.BIDIRECTIONAL_TRANSMIT)
            self._ant_channel.set_id(device_id, self.DEVICE_TYPE, self.TX_TYPE)
            self._ant_channel.set_period(self.PERIOD)
            self._ant_channel.set_rf_freq(self.FREQ)
            self._ant_channel.open()
            
            self._running.clear()
            self._thread = threading.Thread(target=self._broadcasting_loop, daemon=True)
            self._thread.start()
            log.info(f"ANT+ broadcaster started successfully with Device ID {device_id}")
            return True
        except Exception as e:
            log.error(f"Failed to find or initialize ANT+ device: {e}", exc_info=verbose)
            self.stop()
            return False

    def stop(self):
        if self._running.is_set():
            return
        
        log.info("Stopping ANT+ broadcaster...")
        self._running.set()
        if self._thread is not None:
            self._thread.join(timeout=1.0)
        
        try:
            if self._ant_channel: self._ant_channel.close()
            if self._ant_node: self._ant_node.stop()
        except Exception as e:
            log.warning(f"Non-critical error during ANT+ resource cleanup: {e}")

        self._ant_channel = None
        self._ant_node = None
        self._thread = None

        # Reset state variables to ensure a clean start next time
        with self._data_lock:
            self._current_cadence = 0.0
            self._target_cadence = 0.0
            self._broadcast_counter = 0
            self._total_distance = 0.0
            self._total_time = 0.0
            self._stride_accumulator = 0.0
            self._stride_count = 0
        
        log.info("ANT+ broadcaster stopped and resources released.")

    def send_ant_data(self, speed_mps: float, cadence_spm: int):
        """
        Receives latest data from the treadmill application.
        The _broadcasting_loop will apply ramping to these values.
        """
        with self._data_lock:
            self._speed_mps = speed_mps
            self._target_cadence = float(cadence_spm)

            # If speed drops to zero, hard reset the current cadence.
            # The next non-zero speed will then ramp up from 0.
            if self._speed_mps < 0.1:
                self._current_cadence = 0.0
                self._target_cadence = 0.0```