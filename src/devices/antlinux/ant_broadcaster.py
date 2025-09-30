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
        self._total_time = 0.0
        self._stride_count = 0
        self._last_tick = 0.0
        self._last_log_time = 0.0

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

            if current_speed > 0.1:
                self._stride_count = (self._stride_count + 1) & 0xFF
            
            elapsed_ms = int(self._total_time * 1000)
            time_field_1 = (elapsed_ms // 5) & 0xFF
            time_field_2 = (elapsed_ms // 1000) & 0xFF
            speed_int = int(current_speed)
            speed_frac = int(round((current_speed - speed_int) * 256.0)) & 0xFF
            
            # --- Use struct.pack to build the payload ---
            # Format: '<' = little-endian
            #         'B' = unsigned char (1 byte)
            # This creates a robust 8-byte 'bytes' object.
            try:
                # 1. Pack data into a binary bytes object.
                packed_payload = struct.pack('<BBBBBBBB', 
                                             0x01, time_field_1, time_field_2, 0x00, 
                                             speed_int, speed_frac, self._stride_count, 0x00)

                # 2. Convert the bytes object into a list of integers.
                list_payload = list(packed_payload)
                
                if self._ant_channel:
                    self._ant_channel.send_broadcast_data(list_payload)

                    if log.isEnabledFor(logging.DEBUG) and (now - self._last_log_time >= 1.0):
                        self._last_log_time = now # Update the timestamp
                        pace_km, pace_mi = _calculate_pace_range(current_speed)
                        log.debug(f"TX: speed={current_speed:.2f} m/s | Stride={self._stride_count} | Pace/km: {pace_km} | Pace/mi: {pace_mi}")

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
            formatter = logging.Formatter('%(messageL)s')
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

    def send_ant_data(self, speed_mps: float):
        with self._data_lock:
            self._speed_mps = speed_mps