# Kettler C12 BLE authentication (QZ)

## Evidence

A MyHomeFIT 2.3.5 static analysis supplied for this PR identified the
following proprietary BLE challenge/response flow:

1. Read `638a1104-7bde-3e25-ffc5-9de9b2a0197a` (ATT value handle `0x30`).
2. Require exactly **two** challenge bytes for the observed C12 protocol and
   interpret them as an **unsigned big-endian** 16-bit integer.
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

The complete 65,536-record table was supplied in the second analysis document
as an appendix. We have verified its extracted byte-for-byte content and
SHA-256 **locally**, but do **not** redistribute this third-party authentication
asset in the public source repository until distribution rights have been
explicitly clarified. The `.gitignore` rule protects any local `SecureData.txt`.

To produce a local test build, extract the raw table into a file named
`SecureData.txt` using **CRLF line endings including a final CRLF**, and place
it in one of the following locations that QZ searches:

1. Qt resource `:/kettlerc12/SecureData.txt` (only if a future build bundles it).
2. Android package asset `assets:/SecureData.txt` (when the file is packaged under `src/android/assets/`).
3. `QStandardPaths::AppDataLocation/kettlerc12/SecureData.txt`.
4. `QStandardPaths::DocumentsLocation/QZ/SecureData.txt`.
5. `<QZ executable directory>/SecureData.txt`.

On Android/iOS, storage access may be limited to app-specific directories;
a properly packaged resource is preferable once licensing has been checked.

**Required SHA-256:** `70d3eeec6c510e132383fa69b2466e28dec0c89065ab0481a33d191e6281b3a3`.

The implementation validates the entire file digest before decoding the
requested row. It rejects invalid indexes, malformed records, and mismatches;
it never sends a random or guessed response.

There is **no hardcoded challenge-response fallback**. If the full validated
asset is absent, the bike can still stream metrics, but QZ does not attempt
unverified authentication or send control commands. All 65,536 challenges can
be resolved when the verified asset is present.

## Verification

Standalone protocol regression test from the repository root:

```sh
g++ -std=c++11 -Wall -Wextra -Werror \
  tst/Devices/kettlerc12protocoltest.cpp -o /tmp/kettlerc12protocoltest
/tmp/kettlerc12protocoltest
```

The C++11 tests cover strict 2-byte challenge length, big-endian conversion
(including `0000` and `FFFF`), malformed rows, case-insensitive hexadecimal,
CRLF, the captured vector and manual resistance/ERG encoders.

Verify the **entire private asset** (all 65,536 indices, unique 16-byte
responses, SHA-256 and Kinomap test vector) with:

```sh
python3 tools/validate_kettler_c12_security.py /path/to/SecureData.txt
```

This verification was performed against the appended file content. On Qt
Android, placing `SecureData.txt` in `src/android/assets/` before a **local
build** packages it as an application asset, accessible via
`assets:/SecureData.txt`. Confirm actual Qt Android packaging in that build
rather than assuming the public CI APK already contains the table.

This is a protocol-level implementation, **not** confirmation that the real
C12 has accepted authentication and changed resistance; this requires a
new hardware test and QZ debug log.
