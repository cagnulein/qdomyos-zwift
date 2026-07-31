# Freebeat Fit Bike — Communication Protocol

Reverse-engineered from APK `freebeat_original.apk` (decompiled with apktool).
Key smali files: `UsbParseData.smali`, `UsbParseData$ParseAction.smali`, `SerialPortSDK.smali`, `DCUARTDriver.smali`.

## Physical Layer

| Parameter | Value |
|-----------|-------|
| Interface | Internal UART (`/dev/ttyS4` on the bike's Android tablet) |
| Baud rate | 9600 bps |
| Frame | 8N1 |
| Library | `libserial_port.so` (JNI wrapper of `android-serialport-api`) |

**Not USB serial** — the bike's Android tablet communicates with the hardware controller
over a native UART. QZ opens this port directly with `open()` + `termios`, preceded by
`chmod 777 /dev/ttyS4` to obtain rw access (same approach as `DCUARTDriver` in the original app).

---

## Command Packets (Host → Bike)

### 5-byte command

```
[ startCode | action | data | checksum | 0xA0 ]
checksum = (action + data) & 0xFF
```

### 4-byte LED heartbeat

```
[ 0xB5 | 0x08 | 0x08 | 0xA0 ]
```
Must be sent periodically to keep the bike alive.

### Command table

| Name              | startCode | action | data   | Notes                         |
|-------------------|-----------|--------|--------|-------------------------------|
| SET_RESISTANCE    | 0x25      | 0x03   | 1–100  | Set resistance level          |
| DAME_MIX          | 0x25      | 0x05   | 0x00   |                               |
| DAME_MAX          | 0x25      | 0x06   | 0x00   |                               |
| MACHINE_RESET     | 0x25      | 0x07   | 0x00   |                               |
| MACHINE_QUERY     | 0x25      | 0x08   | 0x00   | Request status packet         |
| MACHINE_STOP      | 0x25      | 0x09   | 0x00   |                               |
| CHECK_SERIAL_VER  | 0x25      | 0x2F   | 0x00   |                               |
| CHECK_LED_LIGHT   | 0xB5      | 0x08   | 0x08   | LED heartbeat (4-byte form)   |
| LED_LIGHT_CHANGE  | 0xB5      | 0x03   | value  |                               |

---

## Telemetry Packets (Bike → Host)

### Data packet — 13 bytes

```
byte[0]  = 0x55          sync byte
byte[1]  = type          0x15 | 0x03 | 0x25
byte[2]  = 0x00
byte[3]  = 0x00
byte[4]  = speed_lo      speed (km/h × 10) little-endian
byte[5]  = speed_hi
byte[6]  = 0x00
byte[7]  = resistance    1–100
byte[8]  = rpm_lo        RPM little-endian
byte[9]  = rpm_hi
byte[10] = 0x00
byte[11] = checksum      sum(bytes[1..10]) & 0xFF
byte[12] = 0x3F          verify byte
```

**Parsing:**
```
speed (km/h)  = ((byte[5] << 8) | byte[4]) * 0.1
resistance    = byte[7]          (1–100)
rpm           = (byte[9] << 8) | byte[8]
```

### LED status packet — 20 bytes

```
byte[0] = 0xE5           sync byte for LED packets
...                      (remaining bytes not decoded; ignore for telemetry)
```

---

## Power Calculation

The original app uses two coefficients depending on wheel circumference:

```
watt = rpm * coefficient * resistance / 100.0
```

| Variant      | Coefficient |
|--------------|-------------|
| Standard     | 4.56        |
| Alternative  | 7.39        |

QZ uses the standard coefficient (4.56).

---

## Initialization Sequence

1. Open `/dev/ttyS4` at 9600 baud 8N1
2. Send `MACHINE_QUERY` (`[0x25, 0x08, 0x00, 0x08, 0xA0]`)
3. Wait 500 ms
4. Enter polling loop (200 ms interval):
   - Send `SET_RESISTANCE` if pending
   - Send `MACHINE_QUERY`
   - Send LED heartbeat (`[0xB5, 0x08, 0x08, 0xA0]`)
   - Read and parse response
