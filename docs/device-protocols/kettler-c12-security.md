# Kettler C12 BLE authentication (QZ)

## Protocol investigation

The Kettler C12 BLE protocol was identified through reverse engineering of
Bluetooth traffic observed while controlling a **real Kettler C12 ergometer**.
The resulting implementation keeps device-specific authentication confined
to the C12 driver and does not alter unrelated bike protocols.

The C12 uses a 16-bit challenge and a 65,536-entry response lookup table.
The protocol is **table based**; QZ does not invent a cryptographic response.

### Initialization and commands

1. Discover the C12 service `638af000-7bde-3e25-ffc5-9de9b2a0197a`
   and enable notifications on its measurement characteristics.
2. Read `638a1104-7bde-3e25-ffc5-9de9b2a0197a` (ATT value handle `0x30`).
3. Require exactly two challenge bytes and interpret them as an unsigned
   **big-endian** index from `0x0000` to `0xffff`.
4. Select the indexed response from the bundled `SecureData.txt` resource.
   Each record is `HHHH:` followed by 32 hexadecimal digits (16 bytes).
5. Send the 16-byte response using `WriteWithResponse` on
   `638a1105-7bde-3e25-ffc5-9de9b2a0197a` (ATT value handle `0x32`).
6. Only after the matching write callback, send native resistance-level
   commands on `638a1001` (ATT `0x38`) and two-byte little-endian
   watt targets on `638a100e` (ATT `0x42`).

The write acknowledgment confirms transmission through BLE but does **not**
prove the bicycle has accepted the session: actual hardware load must be
tested separately.

## Bundled resource

The complete table is committed at
`src/devices/kettlerc12bike/SecureData.txt` and included for **all supported
platforms** by `src/kettlerc12_resources.qrc` (listed in
`src/qdomyos-zwift.pri`).

QZ reads it from `:/kettlerc12/SecureData.txt` automatically. No separate
user installation, configuration setting or internet access is required.
An invalid or missing table fails closed: metrics remain readable but QZ
will not send an unverified authentication response.

The driver validates the file before lookup with SHA-256:

```text
70d3eeec6c510e132383fa69b2466e28dec0c89065ab0481a33d191e6281b3a3
```

Sample verified protocol vector (used solely as a test, **not** a fallback):

```text
Challenge:  3E 7C
Index:      0x3E7C (15996 decimal)
Response:   4E 0E 07 9D E0 E7 BA 94 74 AA AB 50 3C 5F 4C C3
```

Challenge/response bytes and full table records are not logged at runtime.

## Tests

From the repository root:

```sh
g++ -std=c++11 -Wall -Wextra -Werror \
  tst/Devices/kettlerc12protocoltest.cpp -o /tmp/kettlerc12protocoltest
/tmp/kettlerc12protocoltest
python3 tools/validate_kettler_c12_security.py
```

The Python check verifies the complete file SHA-256, all 65,536 contiguous
indexes, their unique 16-byte responses, and the observed hardware vector.
The standalone C++ tests validate indexing, endianness, record parsing and
resistance/power encoders. Qt/Android/iOS builds are covered by the CI jobs;
hardware acceptance remains a separate physical-bike test.
