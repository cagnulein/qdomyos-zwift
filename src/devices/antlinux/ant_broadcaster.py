#!/usr/bin/env python3
# -----------------------------------------------------------------------------
# QDomyos-Zwift: ANT+ Virtual Footpod Feature
# ANT+ SDM Broadcaster - Python Core
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor(s): bassai-sho
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
    if speed_mps < 0.2: # Corresponds to ~0.7 km/h, a reasonable lower limit
        return "--:--", "--:--"
    
    # --- Pace per Kilometer ---
    speed_kmh = speed_mps * 3.6
    pace_min_per_km_float = 60.0 / speed_kmh
    total_seconds_km = pace_min_per_km_float * 60
    lower_bound_sec_km = (total_seconds_km // 5) * 5
    upper_bound_sec_km = lower_bound_sec_km + 5
    km_range_str = "~{}:{:02d}-{}:{:02d}".format(
        int(lower_bound_sec_km // 60), int(lower_bound_sec_km % 60),
        int(upper_bound_sec_km // 60), int(upper_bound_sec_km % 60)
    )
    
    # --- Pace per Mile ---
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
    SUPPORTED_DONGLES = [(0x0fcf, 0x1009), (0x0fcf, 0x1008), (0x11fd, 0x0001)]
    try:
        dongle = next((usb.core.find(idVendor=v, idProduct=p) for v, p in SUPPORTED_DONGLES if usb.core.find(idVendor=v, idProduct=p)), None)
        if dongle is None:
            log.warning("No ANT+ dongle found to reset. Proceeding with initialization.")
            return True
        log.info(f"Found ANT+ dongle for reset: {dongle.manufacturer} {dongle.product}")
        dongle.reset()
        time.sleep(1) # Allow time for the device to re-initialize after reset
        log.info("ANT+ dongle reset successfully.")
        return True
    except Exception as e:
        log.error(f"Error during ANT+ dongle reset: {e}. This may be a permissions issue.")
        return False

class AntBroadcaster:
    """
    Manages an ANT+ Stride-based Speed and Distance Monitor (SDM) broadcast channel.
    This class is thread-safe and designed to be controlled from a parent application.
    """
    DEVICE_TYPE, TX_TYPE, PERIOD, FREQ = 124, 1, 8134, 57
    ANT_NETWORK_KEY = [0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45]
    
    def __init__(self):
        self._ant_node: Optional[Node] = None
        self._ant_channel: Optional[Channel] = None
        self._thread: Optional[threading.Thread] = None
        self._running = threading.Event()
        self._data_lock = threading.Lock()
        self._speed_mps = 0.0
        self._cadence_spm = 0.0
        self._stride_accumulator = 0.0        
        self._total_time = 0.0
        self._stride_count = 0
        self._last_tick = 0.0
        self._last_log_time = 0.0
        self._last_broadcast_time = 0
        self._page_toggle = False

    def _broadcasting_loop(self):
        """The main loop that runs on a dedicated thread to send data at ~4Hz."""
        import struct # Import locally to be thread-safe in embedded environments
        log.info("ANT+ broadcasting thread started.")
        self._last_tick = time.monotonic()
        
        while not self._running.is_set():
            now = time.monotonic()
            dt = now - self._last_tick
            self._last_tick = now
            self._total_time += dt

            with self._data_lock:
                current_speed = self._speed_mps
                current_cadence = self._cadence_spm

            # Calculate how many strides should have occurred in the last time slice (dt).
            stride_rate_sps = (current_cadence / 2.0) / 60.0
            strides_this_tick = stride_rate_sps * dt
            self._stride_accumulator += strides_this_tick

            # If the accumulator has passed a whole number, increment the stride count
            # and subtract the whole number from the accumulator.
            if self._stride_accumulator >= 1.0:
                num_strides = int(self._stride_accumulator)
                self._stride_count = (self._stride_count + num_strides) & 0xFF
                self._stride_accumulator -= num_strides

            if current_speed > 0.1:
                self._stride_count = (self._stride_count + 1) & 0xFF
            
            # Calculate elapsed time in milliseconds
            elapsed_ms = int(self._total_time * 1000)
            
            # Time fields matching Java implementation
            time_field_1 = ((elapsed_ms % 256000) // 5) & 0xFF
            time_field_2 = ((elapsed_ms % 256000) // 1000) & 0xFF
            
            # Speed calculation
            speed_int = int(current_speed)
            speed_frac = int(round((current_speed - speed_int) * 256.0)) & 0xFF
            
            # Calculate delta time field (Byte 7 for Page 1)
            delta_time_ms = elapsed_ms - self._last_broadcast_time
            self._last_broadcast_time = elapsed_ms
            delta_time_field = int(delta_time_ms * 0.03125) & 0xFF
            
            try:
                # Alternate between Page 1 (speed/distance) and Page 2 (cadence)
                if self._page_toggle:
                    # PAGE 2: Cadence page (corrected structure)
                    # Based on ANT+ SDM spec, cadence uses integer + fractional bytes
                    # Byte 0: Page Number (0x02)
                    # Byte 1: Cadence (integer) - strides per minute
                    # Byte 2: Cadence (fractional) - 1/256 strides per minute
                    # Byte 3: Reserved (0xFF)
                    # Byte 4: Speed (integer m/s)
                    # Byte 5: Speed (fractional)
                    # Byte 6: Stride Count
                    # Byte 7: Latency (use 0x00)
                    
                    # Cadence in ANT+ SDM Page 2 is STRIDES per minute (not steps)
                    cadence_strides_per_min = current_cadence / 2.0
                    cadence_int = int(cadence_strides_per_min) & 0xFF
                    cadence_frac = int((cadence_strides_per_min - cadence_int) * 256.0) & 0xFF
                    # Garmin expects a non-standard Speed format [INT, FRAC]
                    # but the standard Cadence format [FRAC, INT].
                    packed_payload = struct.pack('<BBBBBBBB',
                                                0x02,              # Byte 0: Page Number (Correct)
                                                cadence_frac,      # Byte 1: Cadence FRACTIONAL (Corrected Order)
                                                cadence_int,       # Byte 2: Cadence INTEGER (Corrected Order)
                                                0xFF,              # Byte 3: Reserved - must be 0xFF (Corrected)
                                                speed_int,         # Byte 4: Speed INTEGER (Garmin specific, keep as is)
                                                speed_frac,        # Byte 5: Speed FRACTIONAL (Garmin specific, keep as is)
                                                0xFF,              # Byte 6: Reserved - must be 0xFF (Corrected)
                                                0xFF)              # Byte 7: Reserved - must be 0xFF (Corrected)
                else:
                    # PAGE 1: Speed/Distance page (original implementation)
                    packed_payload = struct.pack('<BBBBBBBB', 
                                                 0x01,              # Page 1: Speed/Distance
                                                 time_field_1,      # (time % 256000) / 5
                                                 time_field_2,      # (time % 256000) / 1000
                                                 0x00,              # Reserved
                                                 speed_int,         # Speed integer
                                                 speed_frac,        # Speed fractional
                                                 self._stride_count,# Stride count
                                                 delta_time_field)  # deltaTime * 0.03125

                # Toggle for next iteration
                self._page_toggle = not self._page_toggle

                list_payload = list(packed_payload)
                
                if self._ant_channel:
                    self._ant_channel.send_broadcast_data(list_payload)

                    if log.isEnabledFor(logging.DEBUG) and (now - self._last_log_time >= 1.0):
                        self._last_log_time = now
                        pace_km, pace_mi = _calculate_pace_range(current_speed)
                        page_name = "Page 2 (Cadence)" if self._page_toggle else "Page 1 (Speed)"
                        log.debug("TX %s: speed=%.2f m/s | Cadence=%d SPM | Stride=%d | Pace/km: %s",
                                  page_name, current_speed, int(current_cadence), 
                                  self._stride_count, pace_km)
                        
            except Exception as e:
                log.error(f"Broadcast error: {e}. Stopping thread.", exc_info=True)
                self._running.set()

            
            sleep_duration = 0.248 - (time.monotonic() - now)
            if sleep_duration > 0:
                time.sleep(sleep_duration)
        
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
                # If reset fails, it's likely a fatal permission issue.
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
            return # Already stopping/stopped
        
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
        log.info("ANT+ broadcaster stopped and resources released.")

    def send_ant_data(self, speed_mps: float, cadence_spm: int):
        with self._data_lock:
            self._speed_mps = speed_mps
            self._cadence_spm = cadence_spm