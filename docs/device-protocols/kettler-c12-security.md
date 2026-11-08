# Kettler C12 BLE authentication (QZ)

## Evidence

A MyHomeFIT 2.3.5 static analysis supplied for this PR identified the
following proprietary BLE challenge/response flow:

1. Read `638a1104-7bde-3e25-ffc5-9de9b2a0197a` (ATT value handle `0x30`).
2. Interpret the 1–4 challenge bytes as an **unsigned big-endian** integer.
   The security asset only covers values `0x0000`–`0xffff`.
3. Look up that index in `SecureData.txt`, containing 65,536 sequential
   records in `HHHH:<32 hex digits>` format.
4. Write the 16 decoded bytes to
   `638a1105-7bde-3e25-ffc5-9de9b2a0197a` (ATT value handle `0x32`).
5. Once the BLE write acknowledgment arrives, enable native level writes to
   `638a1001` and watt target writes to `638a100e`.

Known Kinomap/MyHomeFIT cross-check:

```text
Read challenge: 3E 7C (big-endian index 0x3E7C)
Table record:   3E7C:4E0E079DE0E7BA9474AAAB503C5F4CC3
Write response: 4E 0E 07 9D E0 E7 BA 94 74 AA AB 50 3C 5F 4C C3
```

The analysis confirms a table lookup, **not** a known cryptographic function
that can currently be recomputed from the challenge without the table.

## Providing the lookup asset

The attached analysis includes only the example record, not the original
65,536-record file. QZ therefore **does not bundle the full table in this PR**.

When you have obtained the exact `assets/SecureData.txt` from MyHomeFIT,
keep it external unless you have confirmed redistribution rights. QZ looks for:

1. Qt resource `:/kettlerc12/SecureData.txt` (only if a future build bundles it).
2. `QStandardPaths::AppDataLocation/kettlerc12/SecureData.txt`.
3. `QStandardPaths::DocumentsLocation/QZ/SecureData.txt`.
4. `<QZ executable directory>/SecureData.txt`.

On Android/iOS, storage access may be limited to app-specific directories;
a properly packaged resource is preferable once licensing has been checked.

**Required SHA-256:** `70d3eeec6c510e132383fa69b2466e28dec0c89065ab0481a33d191e6281b3a3`.

The implementation validates the entire file digest before decoding the
requested row. It rejects invalid indexes, malformed records, and mismatches;
it never sends a random or guessed response.

For the one captured `3E 7C` challenge only, QZ retains the observed
16-byte response as an explicit fallback. Other challenges cannot be
authenticated until the full verified table is available.

## Verification

Standalone protocol regression test from the repository root:

```sh
g++ -std=c++11 -Wall -Wextra -Werror \
  tst/Devices/kettlerc12protocoltest.cpp -o /tmp/kettlerc12protocoltest
/tmp/kettlerc12protocoltest
```

The tests cover endian conversions, out-of-range indexes, malformed rows,
case-insensitive hexadecimal data, CRLF, the captured vector and the
manual resistance/ERG encoders.

This is a protocol-level implementation, **not** confirmation that the real
C12 has accepted authentication and changed resistance; this requires a
new hardware test and QZ debug log.
