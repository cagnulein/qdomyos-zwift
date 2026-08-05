#!/usr/bin/env python3
"""Wired Xbox controller -> QZ virtual gears, over QZ's template WebSocket.

Use this instead of the AutoHotkey script when you want QZ to own the gearing
(custom gear tables, gear->resistance mapping, gear tile on the QZ UI). QZ
relays every gear change to MyWhoosh over OpenBikeControl when the MyWhoosh
Link is enabled, so MyWhoosh still shifts and no companion app is involved.

Requirements on the QZ side:
  * Settings -> Template Settings: enable the web server (default port 6666).
  * Settings -> OpenBikeControl: "Enable OpenBikeControl" (this is what talks
    to MyWhoosh); enable "override local gears" if you want the shift to go
    only to MyWhoosh.
  * A workout must be running with auto resistance on, otherwise QZ ignores
    gear commands.

Only the Python standard library is used. Controller reading is XInput, so
Windows only; the WebSocket half runs anywhere, which is handy for testing:

    python qz_gear_bridge.py --host 192.168.1.50 --send up
"""

from __future__ import annotations

import argparse
import base64
import ctypes
import hashlib
import json
import os
import socket
import struct
import sys
import time

# XInput button masks.
BUTTONS = {
    "dpad_up": 0x0001,
    "dpad_down": 0x0002,
    "dpad_left": 0x0004,
    "dpad_right": 0x0008,
    "start": 0x0010,
    "back": 0x0020,
    "lthumb": 0x0040,
    "rthumb": 0x0080,
    "lb": 0x0100,
    "rb": 0x0200,
    "a": 0x1000,
    "b": 0x2000,
    "x": 0x4000,
    "y": 0x8000,
}


# --------------------------------------------------------------------------
# Minimal WebSocket client (RFC 6455, text frames only, no extensions).
# --------------------------------------------------------------------------
class WebSocketClient:
    def __init__(self, host: str, port: int, path: str = "/", timeout: float = 5.0):
        self.host = host
        self.port = port
        self.path = path
        self.timeout = timeout
        self.sock: socket.socket | None = None

    def connect(self) -> None:
        key = base64.b64encode(os.urandom(16)).decode()
        sock = socket.create_connection((self.host, self.port), self.timeout)
        sock.settimeout(self.timeout)
        request = (
            f"GET {self.path} HTTP/1.1\r\n"
            f"Host: {self.host}:{self.port}\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            f"Sec-WebSocket-Key: {key}\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "\r\n"
        )
        sock.sendall(request.encode())

        response = b""
        while b"\r\n\r\n" not in response:
            chunk = sock.recv(4096)
            if not chunk:
                raise ConnectionError("server closed the connection during handshake")
            response += chunk

        head = response.split(b"\r\n\r\n", 1)[0].decode("latin-1")
        status = head.split("\r\n", 1)[0]
        if "101" not in status:
            raise ConnectionError(f"handshake refused: {status}")

        expected = base64.b64encode(
            hashlib.sha1((key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode()).digest()
        ).decode()
        for line in head.split("\r\n")[1:]:
            name, _, value = line.partition(":")
            if name.strip().lower() == "sec-websocket-accept":
                if value.strip() != expected:
                    raise ConnectionError("bad Sec-WebSocket-Accept")
                break
        else:
            raise ConnectionError("missing Sec-WebSocket-Accept")

        self.sock = sock

    def send_text(self, text: str) -> None:
        if self.sock is None:
            raise ConnectionError("not connected")
        payload = text.encode()
        header = bytearray([0x81])  # FIN + text opcode
        length = len(payload)
        if length < 126:
            header.append(0x80 | length)
        elif length < (1 << 16):
            header.append(0x80 | 126)
            header += struct.pack(">H", length)
        else:
            header.append(0x80 | 127)
            header += struct.pack(">Q", length)
        mask = os.urandom(4)
        header += mask
        masked = bytes(b ^ mask[i % 4] for i, b in enumerate(payload))
        self.sock.sendall(bytes(header) + masked)

    def close(self) -> None:
        if self.sock is not None:
            try:
                self.sock.sendall(b"\x88\x80" + os.urandom(4))  # close frame
            except OSError:
                pass
            self.sock.close()
            self.sock = None


class QZLink:
    """Keeps one WebSocket to QZ alive, reconnecting on demand."""

    def __init__(self, host: str, port: int):
        self.host = host
        self.port = port
        self.client: WebSocketClient | None = None

    def send(self, msg: str) -> bool:
        for attempt in (1, 2):
            if self.client is None:
                try:
                    client = WebSocketClient(self.host, self.port)
                    client.connect()
                    self.client = client
                    print(f"connected to QZ at ws://{self.host}:{self.port}/", flush=True)
                except OSError as exc:
                    print(f"connect failed: {exc}", file=sys.stderr, flush=True)
                    return False
            try:
                self.client.send_text(json.dumps({"msg": msg}))
                return True
            except OSError as exc:
                print(f"send failed ({exc}), reconnecting", file=sys.stderr, flush=True)
                self.client.close()
                self.client = None
                if attempt == 2:
                    return False
        return False


# --------------------------------------------------------------------------
# XInput
# --------------------------------------------------------------------------
class XInputGamepad(ctypes.Structure):
    _fields_ = [
        ("wButtons", ctypes.c_ushort),
        ("bLeftTrigger", ctypes.c_ubyte),
        ("bRightTrigger", ctypes.c_ubyte),
        ("sThumbLX", ctypes.c_short),
        ("sThumbLY", ctypes.c_short),
        ("sThumbRX", ctypes.c_short),
        ("sThumbRY", ctypes.c_short),
    ]


class XInputState(ctypes.Structure):
    _fields_ = [("dwPacketNumber", ctypes.c_ulong), ("Gamepad", XInputGamepad)]


def load_xinput():
    for name in ("xinput1_4", "xinput1_3", "xinput9_1_0"):
        try:
            return ctypes.windll.LoadLibrary(name)
        except OSError:
            continue
    raise SystemExit("XInput not available: plug in the controller (Windows only).")


def read_buttons(xinput, pad_index: int) -> int | None:
    state = XInputState()
    if xinput.XInputGetState(pad_index, ctypes.byref(state)) != 0:
        return None  # ERROR_DEVICE_NOT_CONNECTED
    return state.Gamepad.wButtons


# --------------------------------------------------------------------------
def run_loop(args) -> int:
    xinput = load_xinput()
    link = QZLink(args.host, args.port)
    up_mask = BUTTONS[args.up_button]
    down_mask = BUTTONS[args.down_button]

    pad = -1
    previous = 0
    next_repeat = {up_mask: 0.0, down_mask: 0.0}

    print(
        f"shifting with {args.up_button.upper()} (up) / {args.down_button.upper()} (down), "
        f"Ctrl+C to stop",
        flush=True,
    )
    while True:
        buttons = read_buttons(xinput, pad) if pad >= 0 else None
        if buttons is None:
            pad = -1
            for candidate in range(4):
                if read_buttons(xinput, candidate) is not None:
                    pad = candidate
                    break
            buttons = read_buttons(xinput, pad) if pad >= 0 else None
        if buttons is None:
            previous = 0
            time.sleep(0.5)
            continue

        now = time.monotonic()
        for mask, msg in ((up_mask, "gears_plus"), (down_mask, "gears_minus")):
            pressed = bool(buttons & mask)
            was_pressed = bool(previous & mask)
            if pressed and not was_pressed:
                link.send(msg)
                next_repeat[mask] = now + args.repeat_delay
            elif pressed and args.repeat_delay > 0 and now >= next_repeat[mask]:
                link.send(msg)
                next_repeat[mask] = now + args.repeat_interval

        previous = buttons
        time.sleep(args.poll_interval)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--host", default="127.0.0.1", help="host running QZ (default: %(default)s)")
    parser.add_argument("--port", type=int, default=6666, help="QZ template web server port")
    parser.add_argument("--up-button", default="rb", choices=sorted(BUTTONS), help="shift up button")
    parser.add_argument("--down-button", default="lb", choices=sorted(BUTTONS), help="shift down button")
    parser.add_argument("--poll-interval", type=float, default=0.015, help="controller poll seconds")
    parser.add_argument(
        "--repeat-delay",
        type=float,
        default=0.4,
        help="seconds a button must be held before repeating (0 disables repeat)",
    )
    parser.add_argument("--repeat-interval", type=float, default=0.18, help="seconds between repeats")
    parser.add_argument(
        "--send",
        choices=("up", "down"),
        help="send a single shift and exit, without touching the controller (for testing)",
    )
    args = parser.parse_args()

    if args.send:
        link = QZLink(args.host, args.port)
        ok = link.send("gears_plus" if args.send == "up" else "gears_minus")
        time.sleep(0.2)  # let QZ read the frame before the socket goes away
        if link.client:
            link.client.close()
        return 0 if ok else 1

    try:
        return run_loop(args)
    except KeyboardInterrupt:
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
