#!/usr/bin/env python3
"""Host-side regression tests for the ProForm Wi-Fi / ANT+ FE-C bridge.

These tests intentionally need no hardware and no third-party Python packages.
They validate the wire contract between the ESP32 and XIAO and the critical ERG
math copied from QZ's ProForm Wi-Fi behavior.
"""

import math
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
ESP32 = ROOT / "esp32" / "ProFormWifiANTFEC_ESP32.ino"
XIAO = ROOT / "xiao_nrf52840" / "src" / "main.c"


def qz_corrected_power(raw_watts: float, gain: float) -> float:
    return raw_watts * gain if raw_watts > 0 else 0.0


def qz_erg_bike_target(requested_watts: float, gain: float) -> float:
    if gain <= 0:
        raise ValueError("gain must be > 0")
    return requested_watts / gain


def parse_metrics_line(line: str):
    parts = line.strip().split(",")
    if len(parts) != 6 or parts[0] != "M":
        raise ValueError("invalid metrics line")
    power, cadence, speed, distance, elapsed = map(int, parts[1:])
    return {
        "power_w": power,
        "cadence_rpm": cadence,
        "speed_mm_s": speed,
        "distance_m": distance,
        "elapsed_quarter_s": elapsed,
    }


def parse_target_line(line: str) -> float:
    parts = line.strip().split(",")
    if len(parts) != 2 or parts[0] != "T4":
        raise ValueError("invalid target line")
    quarter_watts = int(parts[1])
    if not 0 <= quarter_watts <= 16000:
        raise ValueError("FE-C target out of range")
    return quarter_watts / 4.0


def decode_fec_page49(payload: bytes) -> float:
    if len(payload) != 8 or payload[0] != 49:
        raise ValueError("not FE-C page 49")
    quarter_watts = payload[6] | (payload[7] << 8)
    if quarter_watts > 16000:
        raise ValueError("FE-C target out of range")
    return quarter_watts / 4.0


class ErgMathTests(unittest.TestCase):
    def test_250w_with_default_gain(self):
        bike = qz_erg_bike_target(250.0, 0.85)
        self.assertAlmostEqual(bike, 294.117647, places=5)
        self.assertAlmostEqual(qz_corrected_power(bike, 0.85), 250.0, places=5)

    def test_round_trip_multiple_targets(self):
        for target in (50, 100, 150, 200, 250, 300, 400, 800):
            with self.subTest(target=target):
                raw = qz_erg_bike_target(target, 0.85)
                corrected = qz_corrected_power(raw, 0.85)
                self.assertTrue(math.isclose(corrected, target, rel_tol=1e-9))

    def test_zero_power_stays_zero(self):
        self.assertEqual(qz_corrected_power(0, 0.85), 0.0)


class UartProtocolTests(unittest.TestCase):
    def test_metrics_example(self):
        parsed = parse_metrics_line("M,248,91,8930,5214,77\n")
        self.assertEqual(
            parsed,
            {
                "power_w": 248,
                "cadence_rpm": 91,
                "speed_mm_s": 8930,
                "distance_m": 5214,
                "elapsed_quarter_s": 77,
            },
        )

    def test_target_250w(self):
        self.assertEqual(parse_target_line("T4,1000\n"), 250.0)

    def test_target_max_4000w(self):
        self.assertEqual(parse_target_line("T4,16000"), 4000.0)

    def test_target_above_fec_range_rejected(self):
        with self.assertRaises(ValueError):
            parse_target_line("T4,16001")


class FecPageTests(unittest.TestCase):
    def test_page49_250w_little_endian(self):
        payload = bytes([49, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE8, 0x03])
        self.assertEqual(decode_fec_page49(payload), 250.0)

    def test_wrong_page_rejected(self):
        with self.assertRaises(ValueError):
            decode_fec_page49(bytes([25, 0, 0, 0, 0, 0, 0, 0]))


class FirmwareContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.esp = ESP32.read_text(encoding="utf-8")
        cls.xiao = XIAO.read_text(encoding="utf-8")

    def test_esp32_keeps_qz_websocket_contract(self):
        self.assertIn('PROFORM_WS_PATH[] = "/control"', self.esp)
        self.assertIn('WORKOUT_TYPE_ERG[] = "WATTS_GOAL"', self.esp)
        self.assertIn('sendProFormSet("Target Watts"', self.esp)

    def test_default_watt_gain_is_085(self):
        self.assertIn("WATT_GAIN = 0.85f", self.esp)

    def test_xiao_advertises_fec_device_type(self):
        self.assertIn("FEC_DEVICE_TYPE = 0x11", self.xiao)

    def test_xiao_advertises_target_power_only(self):
        self.assertIn("FEC_CAP_TARGET_POWER = 0x02", self.xiao)
        self.assertIn("page[7] = FEC_CAP_TARGET_POWER", self.xiao)

    def test_uart_pin_contract_is_documented_in_source(self):
        self.assertIn("UART_RX_PIN = 16", self.esp)
        self.assertIn("UART_TX_PIN = 17", self.esp)


if __name__ == "__main__":
    unittest.main()
