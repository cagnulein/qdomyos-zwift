#!/usr/bin/env python3
"""
paddle_logger.py - Logger BLE/FTMS para caracterizar o comportamento dos paddles.

Objetivo (etapa 1 do plano de fork do QZ):
  descobrir, com timestamp, como a bike reporta mudanca de nivel de resistencia,
  quantos niveis cada toque de paddle produz, e qual a latencia da notificacao.

Uso:
    pip install bleak
    python paddle_logger.py --scan                 # lista dispositivos
    python paddle_logger.py --name MEGA            # conecta por trecho do nome
    python paddle_logger.py --addr AA:BB:CC:DD:EE:FF

Gera paddle_log.txt no diretorio atual, alem de imprimir na tela.

IMPORTANTE: a bike aceita UMA conexao por vez.
Feche QZ / Kinomap / MyWhoosh / BikeControl antes de rodar.
"""

import argparse
import asyncio
import time
from datetime import datetime

from bleak import BleakClient, BleakScanner

# --- UUIDs FTMS ---
SVC_FTMS = "00001826-0000-1000-8000-00805f9b34fb"
CHR_INDOOR_BIKE_DATA = "00002ad2-0000-1000-8000-00805f9b34fb"
CHR_MACHINE_STATUS = "00002ada-0000-1000-8000-00805f9b34fb"
CHR_MACHINE_FEATURE = "00002acc-0000-1000-8000-00805f9b34fb"
CHR_CONTROL_POINT = "00002ad9-0000-1000-8000-00805f9b34fb"

T0 = time.monotonic()
LOGFILE = None


def ts() -> str:
    return f"{time.monotonic() - T0:9.3f}"


def emit(line: str) -> None:
    print(line)
    if LOGFILE:
        LOGFILE.write(line + "\n")
        LOGFILE.flush()


# ---------------------------------------------------------------- parsers

def parse_indoor_bike_data(data: bytes) -> dict:
    """Decodifica 0x2AD2. Campos sao condicionais ao bitfield de flags."""
    if len(data) < 2:
        return {}
    flags = int.from_bytes(data[0:2], "little")
    i = 2
    out = {}

    def take(n: int, signed: bool = False):
        nonlocal i
        if i + n > len(data):
            raise IndexError("truncado")
        v = int.from_bytes(data[i:i + n], "little", signed=signed)
        i += n
        return v

    try:
        # bit 0 tem logica INVERTIDA: 0 = velocidade instantanea presente
        if not (flags & 0x0001):
            out["speed"] = take(2) / 100.0
        if flags & 0x0002:
            out["avg_speed"] = take(2) / 100.0
        if flags & 0x0004:
            out["cadence"] = take(2) / 2.0
        if flags & 0x0008:
            out["avg_cadence"] = take(2) / 2.0
        if flags & 0x0010:
            out["distance"] = take(3)
        if flags & 0x0020:
            out["resistance"] = take(2, signed=True)
        if flags & 0x0040:
            out["power"] = take(2, signed=True)
        if flags & 0x0080:
            out["avg_power"] = take(2, signed=True)
        if flags & 0x0100:
            out["energy_total"] = take(2)
            out["energy_hour"] = take(2)
            out["energy_min"] = take(1)
        if flags & 0x0200:
            out["hr"] = take(1)
        if flags & 0x0400:
            out["met"] = take(1)
        if flags & 0x0800:
            out["elapsed"] = take(2)
        if flags & 0x1000:
            out["remaining"] = take(2)
    except IndexError:
        out["_truncado"] = True

    out["_flags"] = f"0x{flags:04X}"
    return out


STATUS_OPCODES = {
    0x01: "Reset",
    0x02: "Parado/pausado pelo usuario",
    0x03: "Parado por safety key",
    0x04: "Iniciado ou retomado",
    0x05: "Target Speed alterado",
    0x06: "Target Incline alterado",
    0x07: "RESISTANCE LEVEL ALTERADO",
    0x08: "Target Power alterado",
    0x09: "Target HR alterado",
    0x12: "Indoor Bike Simulation Params alterado",
    0xFF: "Control Point alterado",
}


def parse_machine_status(data: bytes) -> str:
    if not data:
        return "vazio"
    op = data[0]
    name = STATUS_OPCODES.get(op, f"opcode desconhecido 0x{op:02X}")
    payload = data[1:]
    extra = ""
    if op == 0x07 and payload:
        # normalmente uint8 ou sint16, dependendo do firmware
        if len(payload) == 1:
            extra = f" -> nivel={payload[0]}"
        else:
            extra = (f" -> nivel_u8={payload[0]} "
                     f"nivel_s16={int.from_bytes(payload[0:2], 'little', signed=True)}")
    return f"{name}{extra}"


# ---------------------------------------------------------------- estado

class Tracker:
    """Detecta mudancas de nivel e mede o intervalo entre elas."""

    def __init__(self):
        self.last_level = None
        self.last_change_t = None

    def feed(self, level: int, origem: str) -> None:
        if level is None:
            return
        if self.last_level is None:
            self.last_level = level
            emit(f"[{ts()}] BASELINE nivel={level} (via {origem})")
            return
        if level != self.last_level:
            delta = level - self.last_level
            now = time.monotonic()
            gap = ""
            if self.last_change_t is not None:
                gap = f" | {(now - self.last_change_t) * 1000:.0f} ms desde a anterior"
            emit(f"[{ts()}] >>> MUDANCA {self.last_level} -> {level} "
                 f"(delta={delta:+d}) via {origem}{gap}")
            self.last_level = level
            self.last_change_t = now


# ---------------------------------------------------------------- main

async def run(address: str) -> None:
    tracker = Tracker()

    emit(f"# sessao iniciada {datetime.now().isoformat(timespec='seconds')}")
    emit(f"# alvo: {address}")

    async with BleakClient(address) as client:
        emit(f"[{ts()}] conectado")

        # Inventario de caracteristicas
        emit("\n--- caracteristicas do servico FTMS ---")
        for svc in client.services:
            if svc.uuid.lower() != SVC_FTMS:
                continue
            for ch in svc.characteristics:
                emit(f"  {ch.uuid}  props={','.join(ch.properties)}")
        emit("---\n")

        # Le a feature uma vez, pra deixar registrado no log
        try:
            feat = await client.read_gatt_char(CHR_MACHINE_FEATURE)
            emit(f"[{ts()}] 0x2ACC raw = {feat.hex('-')}")
            machine = int.from_bytes(feat[0:4], "little")
            target = int.from_bytes(feat[4:8], "little")
            emit(f"[{ts()}] machine_features=0x{machine:08X} "
                 f"target_features=0x{target:08X}")
            emit(f"[{ts()}] bit13 (Indoor Bike Simulation) = "
                 f"{'SIM' if target & (1 << 13) else 'NAO'}")
            emit(f"[{ts()}] bit12 (Power Target)           = "
                 f"{'SIM' if target & (1 << 12) else 'NAO'}")
            emit(f"[{ts()}] bit11 (Resistance Target)      = "
                 f"{'SIM' if target & (1 << 11) else 'NAO'}")
        except Exception as e:
            emit(f"[{ts()}] falha lendo 0x2ACC: {e}")

        def on_ibd(_, data: bytearray):
            d = parse_indoor_bike_data(bytes(data))
            raw = bytes(data).hex("-")
            campos = " ".join(f"{k}={v}" for k, v in d.items()
                              if not k.startswith("_"))
            emit(f"[{ts()}] IBD  {raw}  | {campos}")
            tracker.feed(d.get("resistance"), "0x2AD2")

        def on_status(_, data: bytearray):
            raw = bytes(data).hex("-")
            emit(f"[{ts()}] STAT {raw}  | {parse_machine_status(bytes(data))}")
            b = bytes(data)
            if b and b[0] == 0x07 and len(b) >= 2:
                tracker.feed(b[1], "0x2ADA")

        started = []
        try:
            await client.start_notify(CHR_INDOOR_BIKE_DATA, on_ibd)
            started.append(CHR_INDOOR_BIKE_DATA)
            emit(f"[{ts()}] notify ON: 0x2AD2 (Indoor Bike Data)")
        except Exception as e:
            emit(f"[{ts()}] 0x2AD2 indisponivel: {e}")

        try:
            await client.start_notify(CHR_MACHINE_STATUS, on_status)
            started.append(CHR_MACHINE_STATUS)
            emit(f"[{ts()}] notify ON: 0x2ADA (Machine Status)")
        except Exception as e:
            emit(f"[{ts()}] 0x2ADA indisponivel: {e}")

        if not started:
            emit("nenhuma notificacao disponivel - abortando")
            return

        emit("\n=== PEDALE E APERTE OS PADDLES. Ctrl+C para encerrar. ===")
        emit("=== sugestao: 1 toque, espere 3s, 1 toque, espere 3s,")
        emit("===           depois 3 toques rapidos seguidos.\n")

        try:
            while client.is_connected:
                await asyncio.sleep(1)
        except asyncio.CancelledError:
            pass
        finally:
            for u in started:
                try:
                    await client.stop_notify(u)
                except Exception:
                    pass
            emit(f"[{ts()}] encerrado")


async def scan() -> None:
    print("escaneando 8s...")
    devs = await BleakScanner.discover(timeout=8.0, return_adv=True)
    for d, adv in devs.values():
        print(f"  {d.address}  rssi={adv.rssi:>4}  {d.name or '(sem nome)'}")


async def find_by_name(fragment: str) -> str:
    print(f"procurando por '{fragment}'...")
    devs = await BleakScanner.discover(timeout=8.0, return_adv=True)
    frag = fragment.lower()
    for d, _adv in devs.values():
        if d.name and frag in d.name.lower():
            print(f"encontrado: {d.name} @ {d.address}")
            return d.address
    raise SystemExit("nao encontrado - rode com --scan para ver os nomes")


def main() -> None:
    global LOGFILE
    ap = argparse.ArgumentParser()
    ap.add_argument("--scan", action="store_true", help="apenas lista dispositivos")
    ap.add_argument("--addr", help="endereco MAC / UUID do dispositivo")
    ap.add_argument("--name", help="trecho do nome do dispositivo")
    ap.add_argument("--out", default="paddle_log.txt", help="arquivo de log")
    args = ap.parse_args()

    if args.scan:
        asyncio.run(scan())
        return

    LOGFILE = open(args.out, "a", encoding="utf-8")

    if args.addr:
        addr = args.addr
    elif args.name:
        addr = asyncio.run(find_by_name(args.name))
    else:
        raise SystemExit("use --scan, --addr ou --name")

    try:
        asyncio.run(run(addr))
    except KeyboardInterrupt:
        emit("\ninterrompido pelo usuario")


if __name__ == "__main__":
    main()
