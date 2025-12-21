#!/usr/bin/env python3
import argparse
import asyncio
import sys
import io
import json
import time
from collections import deque
from threading import Lock, Thread
from pathlib import Path
from bleak import BleakScanner


def is_valid_name(name, mac):
    if not name:
        return False
    clean_mac = mac.replace(':', '').replace('-', '').lower()
    clean_name = name.replace(':', '').replace('-', '').lower()
    if clean_name == clean_mac:
        return False
    # Reject tech noise
    if any(x in name for x in ["RSSI", "Manufacturer", "Pairable", "Device", "Unknown"]):
        return False
    return True


class BufferedWriter:
    """Thread-safe buffered output writer optimized for high-frequency updates"""

    def __init__(self, flush_interval=0.1, max_buffer_size=100):
        self.buffer = deque(maxlen=max_buffer_size)
        self.lock = Lock()
        self.flush_interval = flush_interval
        self.last_flush = time.time()
        self._active = True
        self._start_flush_thread()

    def write(self, line):
        with self.lock:
            self.buffer.append(line)
            if len(self.buffer) >= self.buffer.maxlen:
                self._flush_now()

    def _flush_now(self):
        with self.lock:
            if not self.buffer:
                return
            try:
                sys.stdout.write(''.join(self.buffer))
                sys.stdout.flush()
            except Exception:
                pass
            self.buffer.clear()
            # record last flush time for adaptive backoff logic
            try:
                self.last_flush = time.time()
            except Exception:
                pass

    def _start_flush_thread(self):
        def flush_loop():
            while self._active:
                time.sleep(self.flush_interval)
                now = time.time()
                # If idle for a bit, backoff to reduce wakeups
                if now - self.last_flush > 2.0:
                    time.sleep(0.5)
                self._flush_now()

        t = Thread(target=flush_loop, daemon=True)
        t.start()

    def stop(self):
        self._active = False
        # Give the flush thread a short moment to finish
        time.sleep(0.05)
        self._flush_now()


writer = BufferedWriter(flush_interval=0.1, max_buffer_size=50)

# Simple on-disk cache stored in tmpfs to speed cold-starts
CACHE_FILE = Path("/dev/shm/qz_bt_cache.json")
CACHE_MAX_AGE = 300  # seconds

def load_cache():
    if not CACHE_FILE.exists():
        return {}
    try:
        with CACHE_FILE.open('r') as f:
            data = json.load(f)
            now = time.time()
            # only keep fresh entries
            return {mac: d for mac, d in data.items() if now - d.get('timestamp', 0) < CACHE_MAX_AGE}
    except Exception:
        return {}

def save_cache(devices):
    try:
        CACHE_FILE.parent.mkdir(parents=True, exist_ok=True)
        with CACHE_FILE.open('w') as f:
            json.dump(devices, f)
    except Exception:
        pass

# In-memory registry of recent discoveries (pre-populated from cache)
discovered_devices = load_cache()


def detection_callback(device, advertisement_data):
    mac = device.address
    rssi = getattr(advertisement_data, 'rssi', None)
    raw_name = advertisement_data.local_name or device.name

    # Only publish devices with a validated human-friendly name
    if not is_valid_name(raw_name, mac):
        return

    label = raw_name
    # update in-memory registry for cache persistence and quick pre-population
    discovered_devices[mac] = {
        'rssi': rssi,
        'label': label,
        'timestamp': time.time()
    }

    writer.write(f"{mac}|{rssi}|{label}\n")


def update_heartbeat(path: Path):
    try:
        if path:
            path.touch(exist_ok=True)
    except Exception:
        pass


async def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--heartbeat', type=str, help='Heartbeat file path')
    args = parser.parse_args()

    HEARTBEAT_FILE = Path(args.heartbeat) if args.heartbeat else None

    try:
        # Emit cached devices immediately to allow zero-latency pre-population
        for mac, data in discovered_devices.items():
            try:
                writer.write(f"{mac}|{data.get('rssi')}|{data.get('label')}\n")
            except Exception:
                pass

        # Signal initialization state
        writer.write("STATUS|0|INITIALIZING\n")
        writer.write("STATUS|0|CONNECTING\n")

        scanner = BleakScanner(detection_callback)
        await scanner.start()

        # Scanning is active
        writer.write("STATUS|0|CONNECTED\n")

        # Heartbeat/maintenance loop: faster heartbeat and periodic cache save
        last_cache_save = time.time()
        while True:
            await asyncio.sleep(2)
            # touch heartbeat file for supervisor
            update_heartbeat(HEARTBEAT_FILE)
            # also emit a compact heartbeat line for consumers
            try:
                writer.write(f"HEARTBEAT|{int(time.time())}|\n")
            except Exception:
                pass
            # persist cache periodically (every 10s)
            try:
                now = time.time()
                if now - last_cache_save >= 10:
                    save_cache(discovered_devices)
                    last_cache_save = now
            except Exception:
                pass

    except Exception as e:
        writer.write(f"ERROR|0|{str(e)}\n")
        writer.stop()
        sys.exit(1)
    finally:
        try:
            await scanner.stop()
        except Exception:
            pass
        # Final cache flush
        try:
            save_cache(discovered_devices)
        except Exception:
            pass
        writer.stop()


if __name__ == '__main__':
    asyncio.run(main())