#!/usr/bin/env python3
"""Lightweight BLE provider for the dashboard.
Emits lines of the form: MAC|RSSI|LABEL to stdout for the Bash TUI to consume.
"""
import asyncio
import sys
from bleak import BleakScanner

# Minimal OUI hint database
OUI_DB = {
    "E0:E3:E1": "Proform 705 CST",
    "D0:C9:07": "Govee/iHome",
    "D0:26:A6": "Amazon",
    "10:4E:89": "Garmin Watch",
}

def is_valid_name(name, mac):
    if not name:
        return False
    if name.replace(':', '').replace('-', '').lower() in mac.lower().replace(':', ''):
        return False
    if any(x in name for x in ["RSSI", "Manufacturer", "Pairable"]):
        return False
    return True

def detection_callback(device, advertisement_data):
    mac = device.address
    rssi = getattr(advertisement_data, 'rssi', None)
    raw_name = advertisement_data.local_name or device.name

    label = "Searching..."
    if is_valid_name(raw_name, mac):
        label = raw_name
    elif mac[:8].upper() in OUI_DB:
        label = f"~{OUI_DB[mac[:8].upper()]}"

    sys.stdout.write(f"{mac}|{rssi}|{label}\n")
    sys.stdout.flush()

async def main():
    scanner = BleakScanner(detection_callback)
    await scanner.start()
    try:
        while True:
            await asyncio.sleep(10)
    finally:
        await scanner.stop()

if __name__ == '__main__':
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        sys.exit(0)
