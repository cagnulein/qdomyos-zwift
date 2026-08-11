#!/usr/bin/env python3
"""
qz_rtss_bridge - le os dados do QZ (tablet) e publica no OSD do RivaTuner
Statistics Server, para aparecer dentro do frame do Rouvy.

    Gear: 12/24
    ERG: OFF
    Resistance: 18/32

Uso (no PC com o Rouvy + RTSS):
    pip install websocket-client
    python qz_rtss_bridge.py --host 192.168.1.50:8080

Requisitos no QZ (tablet):
    Settings > Template Settings > user_QZWS = Enabled

A porta padrao 6666 funciona aqui. So o overlay em HTML precisa de outra
porta (8080), porque navegador recusa a 6666 com ERR_UNSAFE_PORT.
"""

import argparse
import ctypes
import json
import os
import struct
import sys
import threading
import time

# ---------------------------------------------------------------------------
# RTSS shared memory
# ---------------------------------------------------------------------------

RTSS_MAP_NAME = "RTSSSharedMemoryV2"

# 'RTSS' como multi-char constant do MSVC vs. a ordem que ele cai na memoria.
# Aceitamos as duas leituras para nao depender de endianness da constante.
RTSS_SIGNATURES = (0x52545353, 0x53535452)

# RTSS_SHARED_MEMORY header: 9 x DWORD
#   dwSignature, dwVersion, dwAppEntrySize, dwAppArrOffset, dwAppArrSize,
#   dwOSDEntrySize, dwOSDArrOffset, dwOSDArrSize, dwOSDFrame
_HEADER = struct.Struct("<9I")
_OSD_FRAME_OFFSET = 8 * 4

# RTSS_SHARED_MEMORY_OSD_ENTRY
_OSD_TEXT_LEN = 256       # szOSD
_OSD_OWNER_OFFSET = 256   # szOSDOwner
_OSD_OWNER_LEN = 256
_OSD_EX_OFFSET = 512      # szOSDEx (shared memory v2.7+)
_OSD_EX_LEN = 4096


class RtssError(Exception):
    pass


class RtssOsd:
    """Publica texto em um slot de OSD proprio do RTSS.

    O layout nao e' hardcoded: offsets e tamanho de entrada sao lidos do
    cabecalho, entao versoes diferentes do RTSS continuam funcionando.
    """

    def __init__(self, owner="QZ", buffer=None):
        self.owner = owner
        self._buf = buffer          # injetavel para teste
        self._k32 = None
        self._handle = None
        self._view = None
        self._slot_offset = None

        if self._buf is None:
            self._open_windows_mapping()
        self._read_header()

    # -- abertura ----------------------------------------------------------

    def _open_windows_mapping(self):
        """Abre o mapping nomeado do RTSS via Win32.

        Nao usamos mmap.mmap(-1, 0, tagname=...): com fileno -1 o CPython
        quer um tamanho explicito e acaba criando um mapping novo em vez de
        anexar no que o RTSS ja publicou.
        """
        if os.name != "nt":
            raise RtssError("RTSS so existe no Windows")

        FILE_MAP_ALL_ACCESS = 0x000F001F

        k32 = ctypes.WinDLL("kernel32", use_last_error=True)
        k32.OpenFileMappingW.restype = ctypes.c_void_p
        k32.OpenFileMappingW.argtypes = (ctypes.c_uint32, ctypes.c_int,
                                         ctypes.c_wchar_p)
        k32.MapViewOfFile.restype = ctypes.c_void_p
        k32.MapViewOfFile.argtypes = (ctypes.c_void_p, ctypes.c_uint32,
                                      ctypes.c_uint32, ctypes.c_uint32,
                                      ctypes.c_size_t)
        k32.UnmapViewOfFile.argtypes = (ctypes.c_void_p,)
        k32.CloseHandle.argtypes = (ctypes.c_void_p,)

        handle = k32.OpenFileMappingW(FILE_MAP_ALL_ACCESS, False, RTSS_MAP_NAME)
        if not handle:
            raise RtssError(
                "nao achei o shared memory do RTSS - o RivaTuner Statistics "
                "Server esta rodando?")

        # tamanho 0 = mapeia a secao inteira
        view = k32.MapViewOfFile(ctypes.c_void_p(handle), FILE_MAP_ALL_ACCESS,
                                 0, 0, 0)
        if not view:
            err = ctypes.get_last_error()
            k32.CloseHandle(ctypes.c_void_p(handle))
            raise RtssError(f"MapViewOfFile falhou (erro {err}) - tente rodar "
                            "como administrador, igual ao RTSS")

        self._k32, self._handle, self._view = k32, handle, view

        # le o cabecalho para saber ate onde vai o array de OSD, e so entao
        # expoe um buffer do tamanho certo
        head = _HEADER.unpack_from(ctypes.string_at(view, _HEADER.size), 0)
        osd_entry_size, osd_arr_offset, osd_arr_size = head[5], head[6], head[7]
        needed = osd_arr_offset + osd_entry_size * osd_arr_size
        if not 0 < needed <= 64 * 1024 * 1024:
            self._close_mapping()
            raise RtssError(f"cabecalho do RTSS com tamanhos improvaveis: {needed}")

        self._buf = (ctypes.c_char * needed).from_address(view)

    def _close_mapping(self):
        if self._view:
            self._k32.UnmapViewOfFile(ctypes.c_void_p(self._view))
            self._view = None
        if self._handle:
            self._k32.CloseHandle(ctypes.c_void_p(self._handle))
            self._handle = None

    def _read_header(self):
        head = _HEADER.unpack_from(self._buf, 0)
        (signature, version, _app_entry_size, _app_off, _app_size,
         osd_entry_size, osd_arr_offset, osd_arr_size, _frame) = head

        if signature not in RTSS_SIGNATURES:
            raise RtssError(f"assinatura inesperada no shared memory: {signature:#x}")
        if version < 0x00020000:
            raise RtssError(f"shared memory v{version >> 16}.{version & 0xFFFF} "
                            "e' antiga demais (precisa >= 2.0)")

        self.version = version
        self.osd_entry_size = osd_entry_size
        self.osd_arr_offset = osd_arr_offset
        self.osd_arr_size = osd_arr_size
        # szOSDEx so existe a partir da v2.7 / entradas grandes o bastante
        self.has_osd_ex = osd_entry_size >= _OSD_EX_OFFSET + _OSD_EX_LEN

    # -- slot --------------------------------------------------------------

    def _entry_offset(self, index):
        return self.osd_arr_offset + index * self.osd_entry_size

    def _owner_at(self, index):
        base = self._entry_offset(index) + _OSD_OWNER_OFFSET
        raw = bytes(self._buf[base:base + _OSD_OWNER_LEN])
        return raw.split(b"\0", 1)[0].decode("ascii", "replace")

    def _claim_slot(self):
        """Reusa o slot com o nosso nome; senao pega o primeiro livre."""
        free = None
        for i in range(self.osd_arr_size):
            owner = self._owner_at(i)
            if owner == self.owner:
                return i
            if not owner and free is None:
                free = i
        if free is None:
            raise RtssError("todos os slots de OSD do RTSS estao ocupados")

        base = self._entry_offset(free) + _OSD_OWNER_OFFSET
        name = self.owner.encode("ascii", "replace")[:_OSD_OWNER_LEN - 1]
        self._buf[base:base + _OSD_OWNER_LEN] = name.ljust(_OSD_OWNER_LEN, b"\0")
        return free

    # -- escrita -----------------------------------------------------------

    def update(self, text):
        if self._slot_offset is None:
            self._slot_offset = self._entry_offset(self._claim_slot())

        payload = text.encode("ascii", "replace")

        base = self._slot_offset
        self._buf[base:base + _OSD_TEXT_LEN] = \
            payload[:_OSD_TEXT_LEN - 1].ljust(_OSD_TEXT_LEN, b"\0")

        if self.has_osd_ex:
            ex = base + _OSD_EX_OFFSET
            self._buf[ex:ex + _OSD_EX_LEN] = \
                payload[:_OSD_EX_LEN - 1].ljust(_OSD_EX_LEN, b"\0")

        # avisa o RTSS que o conteudo mudou
        frame = struct.unpack_from("<I", self._buf, _OSD_FRAME_OFFSET)[0]
        struct.pack_into("<I", self._buf, _OSD_FRAME_OFFSET,
                         (frame + 1) & 0xFFFFFFFF)

    def release(self):
        """Libera o slot para o RTSS parar de desenhar nosso texto."""
        if self._slot_offset is None:
            return
        base = self._slot_offset
        self._buf[base:base + _OSD_TEXT_LEN] = b"\0" * _OSD_TEXT_LEN
        if self.has_osd_ex:
            ex = base + _OSD_EX_OFFSET
            self._buf[ex:ex + _OSD_EX_LEN] = b"\0" * _OSD_EX_LEN
        owner = base + _OSD_OWNER_OFFSET
        self._buf[owner:owner + _OSD_OWNER_LEN] = b"\0" * _OSD_OWNER_LEN
        frame = struct.unpack_from("<I", self._buf, _OSD_FRAME_OFFSET)[0]
        struct.pack_into("<I", self._buf, _OSD_FRAME_OFFSET,
                         (frame + 1) & 0xFFFFFFFF)
        self._slot_offset = None
        self._close_mapping()


class ConsoleOsd:
    """Fallback quando o RTSS nao esta disponivel (teste/diagnostico)."""

    def __init__(self, reason=""):
        if reason:
            print(f"[osd] RTSS indisponivel ({reason}) - saida no console",
                  file=sys.stderr)

    def update(self, text):
        print("\n".join("  " + ln for ln in text.splitlines()), flush=True)

    def release(self):
        pass


# ---------------------------------------------------------------------------
# Estado vindo do QZ
# ---------------------------------------------------------------------------

class QzState:
    """Guarda o ultimo valor conhecido de cada campo, com trava."""

    def __init__(self, max_gear, max_resistance):
        self.lock = threading.Lock()
        self.max_gear = max_gear
        self.max_resistance = max_resistance
        self.gear = None
        self.resistance = None
        self.erg = None
        self.connected = False

    def apply_workout(self, content):
        with self.lock:
            if "gears" in content:
                self.gear = content["gears"]
            if "resistance" in content:
                self.resistance = content["resistance"]

    def apply_settings(self, content):
        with self.lock:
            if "zwift_erg" in content:
                v = content["zwift_erg"]
                # QSettings pode devolver bool ou a string "true"/"false"
                if isinstance(v, str):
                    v = v.strip().lower() in ("true", "1")
                self.erg = bool(v) if v is not None else None
            # o QZ so limita a marcha em 1..24 nesses modos
            for key in ("gears_zwift_ratio", "gears_custom_table_enabled"):
                v = content.get(key)
                if isinstance(v, str):
                    v = v.strip().lower() in ("true", "1")
                if v:
                    self.max_gear = 24

    def render(self):
        with self.lock:
            if not self.connected:
                return "QZ: offline"

            gear = fmt_number(self.gear)
            res = fmt_number(self.resistance)
            erg = "--" if self.erg is None else ("ON" if self.erg else "OFF")

            return (f"Gear: {gear}/{self.max_gear}\n"
                    f"ERG: {erg}\n"
                    f"Resistance: {res}/{self.max_resistance}")


def fmt_number(v):
    """Marcha pode ser fracionaria quando gears_gain < 1."""
    if v is None:
        return "--"
    try:
        n = float(v)
    except (TypeError, ValueError):
        return "--"
    return str(int(round(n))) if abs(n - round(n)) < 0.05 else f"{n:.1f}"


# ---------------------------------------------------------------------------
# Cliente do WebSocket do QZ
# ---------------------------------------------------------------------------

# O QZ nao publica zwift_erg no payload periodico; tem que ser pedido.
SETTINGS_REQUEST = json.dumps({
    "msg": "getsettings",
    "content": {"keys": ["zwift_erg", "gears_zwift_ratio",
                         "gears_custom_table_enabled"]},
})


def run_client(url, state, settings_period, stop_event):
    import websocket  # websocket-client

    def on_open(ws):
        state.connected = True
        print(f"[qz] conectado em {url}", flush=True)
        ws.send(SETTINGS_REQUEST)

        def poll():
            while not stop_event.is_set() and state.connected:
                time.sleep(settings_period)
                try:
                    ws.send(SETTINGS_REQUEST)
                except Exception:
                    return
        threading.Thread(target=poll, daemon=True).start()

    def on_message(ws, raw):
        try:
            msg = json.loads(raw)
        except (ValueError, TypeError):
            return
        kind = msg.get("msg")
        content = msg.get("content")
        if not isinstance(content, dict):
            return
        if kind == "workout":
            state.apply_workout(content)
        elif kind == "R_getsettings":
            state.apply_settings(content)

    def on_close(ws, *_):
        state.connected = False

    def on_error(ws, err):
        state.connected = False

    while not stop_event.is_set():
        ws = websocket.WebSocketApp(url, on_open=on_open, on_message=on_message,
                                    on_close=on_close, on_error=on_error)
        try:
            ws.run_forever(ping_interval=20, ping_timeout=10)
        except Exception as exc:
            print(f"[qz] erro: {exc}", file=sys.stderr, flush=True)
        state.connected = False
        if stop_event.wait(2.0):
            break
        print("[qz] reconectando...", flush=True)


# ---------------------------------------------------------------------------

def make_osd(owner, force_console):
    if force_console:
        return ConsoleOsd("--console")
    try:
        return RtssOsd(owner=owner)
    except RtssError as exc:
        return ConsoleOsd(str(exc))
    except OSError as exc:
        return ConsoleOsd(str(exc))


def main(argv=None):
    ap = argparse.ArgumentParser(description="Ponte QZ -> OSD do RTSS")
    ap.add_argument("--host", required=True,
                    help="IP:porta do QZ Web Server no tablet, ex 192.168.1.50:8080")
    ap.add_argument("--max-gear", type=int, default=24)
    ap.add_argument("--max-resistance", type=int, default=32)
    ap.add_argument("--owner", default="QZ", help="nome do slot no RTSS")
    ap.add_argument("--refresh", type=float, default=0.5,
                    help="segundos entre atualizacoes do OSD")
    ap.add_argument("--settings-period", type=float, default=2.0,
                    help="segundos entre consultas do estado do ERG")
    ap.add_argument("--console", action="store_true",
                    help="imprime no console em vez do RTSS")
    ap.add_argument("--stop-on-stdin-eof", action="store_true",
                    help="encerra quando a stdin fechar (usado pelo launcher, "
                         "para desligar limpo e liberar o slot do RTSS)")
    args = ap.parse_args(argv)

    url = f"ws://{args.host}/qzrtss-ws"
    state = QzState(args.max_gear, args.max_resistance)
    osd = make_osd(args.owner, args.console)
    stop_event = threading.Event()

    client = threading.Thread(target=run_client,
                              args=(url, state, args.settings_period, stop_event),
                              daemon=True)
    client.start()

    if args.stop_on_stdin_eof:
        def watch_stdin():
            try:
                for _ in sys.stdin:
                    pass
            except Exception:
                pass
            print("[osd] stdin fechou, encerrando", flush=True)
            stop_event.set()
        threading.Thread(target=watch_stdin, daemon=True).start()

    last = None
    try:
        while not stop_event.is_set():
            text = state.render()
            if text != last:
                osd.update(text)
                last = text
            stop_event.wait(args.refresh)
    except KeyboardInterrupt:
        pass
    finally:
        stop_event.set()
        osd.release()
        print("\n[osd] slot liberado", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
