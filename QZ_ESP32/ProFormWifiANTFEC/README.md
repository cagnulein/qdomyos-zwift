# ProForm Wi-Fi -> BLE + ANT+ FE-C bridge

This prototype turns the two-board combination below into a standalone bridge for a
ProForm/iFit Wi-Fi bike:

- **AYWHP ESP32 DevKitC / ESP-WROOM-32**
  - Wi-Fi client for the ProForm bike
  - WebSocket implementation derived from QZ's `proformwifibike` behavior
  - BLE Cycling Power peripheral
  - ERG target translation
- **Seeed Studio XIAO nRF52840**
  - ANT+ FE-C trainer peripheral
  - receives FE-C Target Power commands from a Garmin
  - sends target-power commands to the ESP32 over UART
  - publishes bike power, cadence, speed and distance over FE-C

The first implementation deliberately focuses on **ERG / FE-C Target Power**.
It does **not** advertise Basic Resistance or Simulation Mode.

## Data flow

```text
                         Wi-Fi
                  ws://BIKE_IP/control
                           |
                           v
+-----------+       +-----------------+       UART       +------------------+
| ProForm   |<----->| ESP32 DevKitC   |<--------------->| XIAO nRF52840    |
| Wi-Fi bike|       | Wi-Fi + BLE     |  115200 8N1    | ANT+ FE-C        |
+-----------+       +-----------------+                 +------------------+
                           |                                      ^
                           | BLE Cycling Power                    |
                           v                                      | ANT+ FE-C
                    BLE applications                         Garmin / watch
```

ERG control travels in the opposite direction:

```text
Garmin FE-C Target Power (page 49)
        |
        v
XIAO nRF52840
        |
        | T4,<target in 0.25 W units>
        v
ESP32
        |
        | requested watts / WATT_GAIN
        v
ProForm WebSocket:
  Workout Type = WATTS_GOAL
  Target Watts = compensated target
```

## Relation to QZ

The ESP32 behavior follows the existing QZ implementation:

- [`proformwifibike.cpp`](../../src/devices/proformwifibike/proformwifibike.cpp)
  connects to `ws://<bike-ip>/control`.
- ERG mode selects `Workout Type = WATTS_GOAL`.
- ERG power uses the `Target Watts` key.
- QZ compensates an ERG request by dividing by `watt_gain`.
- [`metric.cpp`](../../src/metric.cpp) applies `watt_gain` to the power reported by
  the bike.

For the default `WATT_GAIN = 0.85`:

```text
Garmin asks for 250 W
        |
        +--> ESP32 sends 250 / 0.85 = 294.12 W to the ProForm
        |
ProForm reports ~294 W raw
        |
        +--> ESP32 reports 294 * 0.85 ~= 250 W to ANT+ and BLE
```

This mirrors the two-way gain handling in QZ instead of only scaling the ERG
command.

## ESP32 configuration

Edit the constants near the top of:

`esp32/ProFormWifiANTFEC_ESP32.ino`

```cpp
static constexpr char WIFI_SSID[] = "YOUR_ESSID";
static constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
static constexpr char PROFORM_BIKE_IP[] = "192.168.1.100";
static constexpr float WATT_GAIN = 0.85f;
```

The ProForm bike and ESP32 must be reachable on the same IP network.

### ESP32 Arduino dependencies

Use an ESP32 Arduino core suitable for the ESP32 DevKitC / ESP-WROOM-32 and
install:

- **arduinoWebSockets** by Links2004
- **ArduinoJson 7.x**

BLE support comes from the ESP32 Arduino core.

The BLE peripheral advertises as:

`QZ-ProForm-Bridge`

and exposes the standard Cycling Power service (0x1818), including instantaneous
power and crank-revolution data.

## XIAO nRF52840 ANT firmware

The XIAO application is under:

`xiao_nrf52840/`

It uses **ANT for nRF Connect SDK**, maintained by Garmin/ANT. The nRF52840 is a
supported target in current ANT for nRF Connect SDK releases.

### ANT licensing

The source in this folder does not contain the ANT+ network key. It calls
`ant_plus_key_set()` from the ANT SDK dependency.

`prj.conf` currently contains:

```text
CONFIG_ANT_EVALUATION_KEY=y
```

That setting is for prototype/evaluation use. Before distributing this as part
of a commercial or other revenue-generating product, use the ANT licensing path
required by Garmin/ANT and replace the evaluation configuration as appropriate.

References:

- https://www.thisisant.com/developer/ant/nrf-connect-sdk
- https://www.thisisant.com/developer/ant/licensing
- https://www.thisisant.com/APIassets/ANTnRFConnectDoc/

### Build

Install a compatible Nordic nRF Connect SDK and enable its matching ANT add-on.
From the nRF Connect SDK workspace:

```sh
west config manifest.group-filter +ant
west update
```

Then build this application:

```sh
west build -p always -b xiao_ble/nrf52840 \
  /path/to/qdomyos-zwift/QZ_ESP32/ProFormWifiANTFEC/xiao_nrf52840
```

Use a matching NCS / ANT add-on release pair. ANT add-on v1.3.0 is documented as
production-ready for nRF52840 and targets nRF Connect SDK v2.7.0; newer matched
pairs may also be used according to the ANT compatibility documentation.

### Flashing the XIAO

The Zephyr `xiao_ble/nrf52840` target supports the XIAO's UF2 bootloader.

1. Connect the XIAO over USB.
2. Double-tap reset to enter the bootloader.
3. A drive named `XIAO BLE` appears.
4. Copy `build/zephyr/zephyr.uf2` to that drive.

An SWD debugger can also be used.

## Wiring

The UART is 3.3 V logic on both boards, so no level shifter is required.

| ESP32 DevKitC | XIAO nRF52840 | Purpose |
| --- | --- | --- |
| GPIO17 / TX2 | D7 / RX / P1.12 | ESP32 -> XIAO |
| GPIO16 / RX2 | D6 / TX / P1.11 | XIAO -> ESP32 |
| GND | GND | Common ground |

The XIAO's standard Zephyr board definition already maps UART0 to D6/TX
(P1.11) and D7/RX (P1.12).

### Power options

**Recommended while developing:** power each board from its own USB connection
and connect GND + the two UART wires above.

**Single-supply installation:** use one regulated 5 V source and feed both the
ESP32 DevKitC 5 V/VIN input and the XIAO 5 V/VBUS pin, plus common GND.

Do not connect two independent USB 5 V sources at the same time while also tying
their VBUS/5 V rails together. Avoid powering the XIAO through its 3V3 pin; the
documented XIAO 3V3 pin is an output from its onboard regulator.

## UART protocol

The protocol is intentionally human-readable for bring-up.

### ESP32 -> XIAO

Sent every 250 ms:

```text
M,<power_w>,<cadence_rpm>,<speed_mm_s>,<distance_m>,<elapsed_quarter_s>
```

Example:

```text
M,248,91,8930,5214,77
```

### XIAO -> ESP32

When an ANT+ controller sends FE-C Target Power page 49:

```text
T4,<target_power_quarter_watts>
```

Example for 250 W:

```text
T4,1000
```

The ESP32 converts 1000 / 4 to 250 W and applies the QZ ERG compensation before
sending the ProForm command.

## FE-C implementation

Current ANT+ side:

- FE-C device type 0x11
- ANT+ RF frequency 2457 MHz
- 4 Hz channel period
- General FE Data page 16
- Specific Trainer Data page 25
- Target Power page 49 receive/response
- FE Capabilities page 54
- Common Page Request page 70
- Command Status page 71
- Manufacturer Information page 80
- Product Information page 81
- target-power capability advertised
- Basic Resistance, Wind Resistance and Track/Simulation control are currently
  not advertised

The FE-C device number is currently a code constant:

```c
static const uint16_t FEC_DEVICE_NUMBER = 12345;
```

Change it if multiple bridges will be used in the same area.

## Garmin test

1. Start the ProForm bike and make sure its Wi-Fi control endpoint is reachable.
2. Power the ESP32 and verify it connects to the configured SSID.
3. Power the XIAO and wait for the FE-C channel to start.
4. On the Garmin, add/search for an **Indoor Trainer / ANT+ FE-C** sensor.
5. Select the trainer with device number 12345.
6. Start an indoor cycling activity or a structured workout that controls
   trainer target power.
7. Change the ERG target.
8. On the ESP32 USB serial monitor, verify a sequence similar to:

```text
[ERG] Garmin 250.00 W -> ProForm 294.12 W (gain 0.850)
[ProForm] TX {"type":"set","values":{"Workout Type":"WATTS_GOAL"}}
[ProForm] TX {"type":"set","values":{"Target Watts":"294.12"}}
```

The Garmin should then receive the corrected current power through FE-C page 25.

## Known limitations / next validation points

This is a hardware prototype and has not yet been validated on the target
ProForm + ESP32 DevKitC + XIAO + Garmin combination.

The first hardware test should specifically verify:

1. Garmin discovery and stable FE-C pairing.
2. Page 49 Target Power reception during a Garmin structured workout.
3. ProForm acceptance of `WATTS_GOAL` and compensated `Target Watts`.
4. Correct feedback scaling at `WATT_GAIN = 0.85`.
5. FE-C page scheduling and page-70/page-71 behavior expected by the Garmin.
6. UART stability while both radios are active.
7. Recovery after ProForm Wi-Fi/WebSocket reconnection.

Simulation mode can be added later by implementing FE-C page 51 and mapping its
grade request to the appropriate ProForm incline/resistance behavior.
