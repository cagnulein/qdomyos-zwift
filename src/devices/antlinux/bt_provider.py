#!/usr/bin/env python3
# =============================================================================
# QDomyos-Zwift: Bluetooth Device Discovery Provider
#
# Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
# Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
#
# Asynchronous BLE scanner with RAM-based caching and buffered output.
# Runs as a supervised subprocess controlled by `setup-dashboard.sh`.
#
# Optimizations: RAM-only cache (/dev/shm), sanitize LRU cache, event-driven
# buffered output, and throttled heartbeat writes.
#
# Dependencies: bleak, Python 3.9+
# =============================================================================

import argparse
import re
import asyncio
import sys
import json
import time
from collections import deque
import threading
from pathlib import Path
try:
    from bleak import BleakScanner
except Exception:
    # Emit a compact, dashboard-friendly diagnostic and exit early.
    # The TUI expects single-line bullets prefixed with '-'.
    print("- bleak not installed")
    sys.exit(2)
import atexit

# Heartbeat optimization: reduce disk writes from 2s to 30s intervals
_last_disk_sync = 0.0
HEARTBEAT_DISK_INTERVAL = 30.0  # Write to disk every 30 seconds


# ============================================================================
# STRING SANITIZATION OPTIMIZATION
# ============================================================================

# Pre-compiled regex pattern for ANSI sequence removal
_ANSI_PATTERN = re.compile(
    r'\\(?:x1b|033|e)\\\[[0-9;]*[mK]|'  # Backslash-escaped ANSI
    r'\x1b\[[0-9;]*[mK]'                  # Real ESC sequences
)

# LRU-like cache for sanitized strings (limit size to prevent memory bloat)
_SANITIZE_CACHE = {}
_SANITIZE_CACHE_MAX_SIZE = 200

# Cache statistics (for monitoring)
_cache_hits = 0
_cache_misses = 0


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


def sanitize_label(label: str) -> str:
    """
    Remove ANSI sequences and non-printable characters from device labels.
    OPTIMIZED: Single-pass processing with a small sanitize cache.
    """
    global _cache_hits, _cache_misses

    if not label:
        return ""

    # Check cache first (hot path)
    if label in _SANITIZE_CACHE:
        _cache_hits += 1
        return _SANITIZE_CACHE[label]

    _cache_misses += 1

    try:
        cleaned = _ANSI_PATTERN.sub('', label)
        result = ''.join(ch for ch in cleaned if ch.isprintable()).strip()
    except Exception:
        result = ''.join(ch for ch in label if ch.isprintable()).strip()

    # Simple size-limited cache (FIFO-like eviction)
    if len(_SANITIZE_CACHE) >= _SANITIZE_CACHE_MAX_SIZE:
        evict_count = max(1, _SANITIZE_CACHE_MAX_SIZE // 4)
        for _ in range(evict_count):
            try:
                _SANITIZE_CACHE.pop(next(iter(_SANITIZE_CACHE)))
            except Exception:
                break

    _SANITIZE_CACHE[label] = result
    return result


class BufferedWriter:
    """
    Thread-safe buffered output writer with event-driven flushing.
    OPTIMIZED: Uses threading.Event to wake flush thread only when needed,
    reducing idle CPU usage.
    """

    def __init__(self, flush_interval=0.1, max_buffer_size=100):
        self.buffer = deque(maxlen=max_buffer_size)
        self.lock = threading.Lock()
        self.flush_event = threading.Event()
        self.flush_interval = flush_interval
        self.last_flush = time.time()
        self._active = True
        self._flush_thread = None
        self._start_flush_thread()

    def write(self, line):
        """Add line to buffer and signal flush if buffer is full"""
        with self.lock:
            self.buffer.append(line)
            if len(self.buffer) >= self.buffer.maxlen * 0.8:
                self.flush_event.set()

    def _flush_now(self):
        """Atomically flush buffer to stdout"""
        with self.lock:
            if not self.buffer:
                return
            try:
                sys.stdout.write(''.join(self.buffer))
                sys.stdout.flush()
            except Exception:
                pass
            self.buffer.clear()
            self.last_flush = time.time()

    def _start_flush_thread(self):
        def flush_loop():
            while self._active:
                event_fired = self.flush_event.wait(timeout=self.flush_interval)
                self.flush_event.clear()
                self._flush_now()
                now = time.time()
                if not event_fired and (now - self.last_flush) > 2.0:
                    time.sleep(0.5)

        self._flush_thread = threading.Thread(target=flush_loop, daemon=True)
        self._flush_thread.start()

    def flush_immediate(self):
        """Force immediate flush (use for high-priority messages)"""
        self.flush_event.set()
        time.sleep(0.01)

    def stop(self):
        """Gracefully stop writer and flush remaining buffer"""
        self._active = False
        self.flush_event.set()
        if self._flush_thread and self._flush_thread.is_alive():
            self._flush_thread.join(timeout=0.2)
        self._flush_now()



writer = BufferedWriter(flush_interval=0.1, max_buffer_size=50)
atexit.register(writer.stop)

# ==========================================================================
# RAM-only cache persistence (C-1): validate tmpfs and use atomic writes
# ==========================================================================
import os

CACHE_MAX_AGE = 300  # seconds

def _validate_ram_storage(path: Path) -> bool:
    try:
        if not path.exists() or not os.access(path, os.W_OK):
            return False
        # Compare device id with /dev/shm when present
        if Path('/dev/shm').exists():
            return path.stat().st_dev == Path('/dev/shm').stat().st_dev
        return False
    except Exception:
        return False

def _get_ram_storage() -> Path:
    candidates = [Path('/dev/shm'), Path(f'/run/user/{os.getuid()}')]
    for p in candidates:
        try:
            if _validate_ram_storage(p):
                return p
        except Exception:
            continue
    raise RuntimeError('FATAL: No RAM-backed storage available. Ensure /dev/shm is mounted.')

try:
    RAM_STORAGE = _get_ram_storage()
    CACHE_FILE = RAM_STORAGE / f"qz_bt_cache_{os.getpid()}.json"
except RuntimeError as e:
    print(f"ERROR: {e}", file=sys.stderr)
    sys.exit(1)

def load_cache():
    if not CACHE_FILE.exists():
        return {}
    try:
        with CACHE_FILE.open('r') as f:
            data = json.load(f)
            now = time.time()
            return {mac: d for mac, d in data.items() if now - d.get('timestamp', 0) < CACHE_MAX_AGE}
    except Exception as e:
        print(f"WARNING: Cache load failed: {e}", file=sys.stderr)
        return {}

def save_cache(devices):
    temp_path = CACHE_FILE.with_suffix('.tmp')
    try:
        with temp_path.open('w') as f:
            json.dump(devices, f)
        temp_path.replace(CACHE_FILE)
    except Exception as e:
        print(f"ERROR: Cache write failed: {e}", file=sys.stderr)
        try:
            temp_path.unlink(missing_ok=True)
        except Exception:
            pass

# In-memory registry of recent discoveries (pre-populated from cache)
discovered_devices = load_cache()


def detection_callback(device, advertisement_data):
    mac = device.address
    rssi = getattr(advertisement_data, 'rssi', None)
    raw_name = advertisement_data.local_name or device.name
    # Sanitize label first and only publish devices with a validated name
    label = sanitize_label(raw_name)
    if not is_valid_name(label, mac):
        return
    # update in-memory registry for cache persistence and quick pre-population
    discovered_devices[mac] = {
        'rssi': rssi,
        'label': label,
        'timestamp': time.time()
    }

    # Emit sanitized line
    writer.write(f"{mac}|{rssi}|{label}\n")


def update_heartbeat(path: Path):
    """Update heartbeat with minimal disk I/O (30s intervals instead of 2s)"""
    global _last_disk_sync

    if not path:
        return

    now = time.time()
    # Only write to disk every HEARTBEAT_DISK_INTERVAL seconds
    if now - _last_disk_sync >= HEARTBEAT_DISK_INTERVAL:
        try:
            path.touch(exist_ok=True)
            _last_disk_sync = now
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
                lbl = sanitize_label(data.get('label')) if data.get('label') else ''
                writer.write(f"{mac}|{data.get('rssi')}|{lbl}\n")
            except Exception:
                pass

        # Signal initialization state
        writer.write("STATUS|0|INITIALIZING\n")
        writer.flush_immediate()
        writer.write("STATUS|0|CONNECTING\n")

        scanner = BleakScanner(detection_callback)
        await scanner.start()

        # Scanning is active
        writer.write("STATUS|0|CONNECTED\n")

        # Register shutdown handler for final heartbeat
        if HEARTBEAT_FILE:
            atexit.register(lambda p=HEARTBEAT_FILE: p.touch(exist_ok=True))

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
                # Report cache stats every 60s
                if int(now) % 60 == 0:
                    try:
                        # non-intrusive status to stderr
                        total = _cache_hits + _cache_misses
                        if total > 0:
                            hit_rate = (_cache_hits / total) * 100
                            print(f"STATUS|0|Cache: {_cache_hits}/{total} hits ({hit_rate:.1f}%)", file=sys.stderr, flush=True)
                    except Exception:
                        pass
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