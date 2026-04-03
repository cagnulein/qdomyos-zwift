#!/usr/bin/env python3
# =============================================================================
# QDomyos-Zwift: Bluetooth Device Discovery Provider
#
# Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
# Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
#
# Asynchronous BLE scanner with RAM-based caching and buffered output.
# Runs as a supervised subprocess controlled by `setup-dashboard.sh`.
# =============================================================================

import argparse
import re
import asyncio
import sys
import json
import time
import signal
from collections import deque
import threading
from pathlib import Path
import os
try:
    from bleak import BleakScanner
except Exception:
    print("- bleak not installed")
    sys.exit(2)
import atexit

HEARTBEAT_DISK_INTERVAL = 30.0 
SHOW_ALL_DEVICES = False

# ============================================================================
# FILTERING
# ============================================================================

_ANSI_PATTERN = re.compile(r'\\(?:x1b|033|e)\\\[[0-9;]*[mK]|\x1b\[[0-9;]*[mK]')
_SANITIZE_CACHE = {}

def is_valid_device(name: str, mac: str) -> bool:
    if SHOW_ALL_DEVICES: return True
    if not name: return False
    
    clean_name = name.strip()
    if len(clean_name) < 1: return False
        
    lower_name = clean_name.lower()
    clean_mac = mac.replace(':', '').replace('-', '').lower()
    
    if lower_name.replace(':', '').replace('-', '') == clean_mac:
        return False
    
    # Explicitly reject "Unknown" to keep list clean
    if lower_name == "unknown":
        return False

    return True

def sanitize_label(label: str) -> str:
    if not label: return ""
    if label in _SANITIZE_CACHE: return _SANITIZE_CACHE[label]
    try:
        cleaned = _ANSI_PATTERN.sub('', label)
        result = ''.join(ch for ch in cleaned if ch.isprintable()).strip()
    except Exception:
        result = ''.join(ch for ch in label if ch.isprintable()).strip()
    if len(_SANITIZE_CACHE) > 200: _SANITIZE_CACHE.clear()
    _SANITIZE_CACHE[label] = result
    return result

class BufferedWriter:
    def __init__(self):
        self.buffer = deque(maxlen=50)
        self.lock = threading.Lock()
        self.flush_event = threading.Event()
        self._active = True
        self._flush_thread = threading.Thread(target=self._flush_loop, daemon=True)
        self._flush_thread.start()

    def write(self, line):
        with self.lock:
            self.buffer.append(line)
            if len(self.buffer) > 10: self.flush_event.set()

    def _flush_loop(self):
        while self._active:
            self.flush_event.wait(0.1)
            self.flush_event.clear()
            with self.lock:
                if self.buffer:
                    try:
                        sys.stdout.write(''.join(self.buffer))
                        sys.stdout.flush()
                    except: pass
                    self.buffer.clear()

    def flush_immediate(self):
        self.flush_event.set()
        time.sleep(0.01)

    def stop(self):
        self._active = False
        self.flush_event.set()

writer = BufferedWriter()
atexit.register(writer.stop)

# ==========================================================================
# MAIN LOGIC
# ==========================================================================
CACHE_FILE = Path(f"/dev/shm/qz_bt_cache_{os.getpid()}.json")

def load_cache():
    if not CACHE_FILE.exists(): return {}
    try:
        with CACHE_FILE.open('r') as f: return json.load(f)
    except: return {}

def save_cache(devices):
    try:
        with CACHE_FILE.open('w') as f: json.dump(devices, f)
    except: pass

discovered_devices = load_cache()
_last_emitted = {} 

def detection_callback(device, advertisement_data):
    mac = device.address
    rssi = getattr(advertisement_data, 'rssi', 0)
    now = time.time()
    
    raw_name = advertisement_data.local_name or device.name
    
    if not raw_name or not raw_name.strip():
        if not SHOW_ALL_DEVICES: return
        raw_name = "Unknown"

    label = sanitize_label(raw_name)
    
    if not is_valid_device(label, mac):
        return
    
    # Throttling: 1 update per second per device
    if mac in _last_emitted:
        if (now - _last_emitted[mac]) < 1.0:
            return
    _last_emitted[mac] = now
    
    discovered_devices[mac] = {'rssi': rssi, 'label': label, 'timestamp': now}
    writer.write(f"{mac}|{rssi}|{label}\n")

shutdown_event = asyncio.Event()

async def main():
    global SHOW_ALL_DEVICES
    parser = argparse.ArgumentParser()
    parser.add_argument('--heartbeat', type=str)
    parser.add_argument('--all', action='store_true')
    args = parser.parse_args()
    SHOW_ALL_DEVICES = args.all
    
    loop = asyncio.get_running_loop()
    for sig in (signal.SIGINT, signal.SIGTERM):
        loop.add_signal_handler(sig, lambda: shutdown_event.set())

    try:
        for mac, data in discovered_devices.items():
             if is_valid_device(data.get('label'), mac):
                 writer.write(f"{mac}|{data.get('rssi')}|{data.get('label')}\n")

        writer.write("STATUS|0|INITIALIZING\n")
        writer.flush_immediate()
        
        scanner = BleakScanner(detection_callback)
        await scanner.start()
        
        writer.write("STATUS|0|SCANNING\n")
        
        while not shutdown_event.is_set():
            try:
                await asyncio.wait_for(shutdown_event.wait(), timeout=1.0)
            except asyncio.TimeoutError:
                writer.write(f"HEARTBEAT|{int(time.time())}|\n")
                save_cache(discovered_devices)
    finally:
        try: await scanner.stop()
        except: pass
        try: CACHE_FILE.unlink(missing_ok=True)
        except: pass
        writer.stop()

if __name__ == '__main__':
    asyncio.run(main())