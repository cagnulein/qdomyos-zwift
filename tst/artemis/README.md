# QZ Android Artemis pilot tests

This directory contains opt-in Android end-to-end tests driven by [Google Artemis](https://github.com/google/artemis). They complement the deterministic C++/GoogleTest suite; they do not replace it and are not a required GitHub Actions check.

## Why Artemis

QZ uses Qt Quick/QML, so Android's accessibility hierarchy can be incomplete or misleading. Existing Android automation therefore sometimes falls back to fixed coordinates. Artemis can combine accessibility data with coordinate and visual targeting, which makes it useful for regression tests across QZ's custom UI.

## Requirements

- Python 3.10+
- an Android emulator or dedicated test device with QZ installed
- USB debugging or an emulator visible to the Artemis host
- a running Artemis daemon
- `artemis-client`

Install the pinned client version used by this pilot:

```bash
python3 -m pip install -r tst/artemis/requirements.txt
```

The pin is intentional so an upstream Artemis change cannot silently alter QZ test behavior. Update it explicitly when validating a newer Artemis revision.

Start Artemis from its own checkout and connect the target device as described by the Artemis project. By default these tests connect to `http://127.0.0.1:8000`.

## Important: use a test device/profile

The persistence test enables **Android Notification** and **Fake Device**. Run these tests only on an emulator or dedicated QZ test device/profile, not on a personal production configuration.

The FTMS search regression test expects **FTMS Bike** to have a non-default value. Before running the suite, set FTMS Bike to a test device in QZ and export the exact displayed value:

```bash
export QZ_FTMS_BIKE_EXPECTED='My Test Bike'
```

Using a real/non-default value is intentional: if FTMS Bike is left at `Disabled`, the regression where the search result incorrectly falls back to `Disabled` cannot be detected.

## Run

```bash
export ARTEMIS_BASE_URL=http://127.0.0.1:8000
export ARTEMIS_DEVICE_SERIAL=emulator-5554   # optional when only one device is connected
export QZ_FTMS_BIKE_EXPECTED='My Test Bike'

python3 tst/artemis/test_qz_android.py -v
```

Optional variables:

- `ARTEMIS_PROFILE`: `flash` by default; use `pro` for deeper exploration.
- `ARTEMIS_TIMEOUT`: per-test timeout in seconds, default `900`.
- `ARTEMIS_TOKEN`: optional token if the daemon is behind an authenticated proxy.
- `QZ_ANDROID_PACKAGE`: defaults to `org.cagnulen.qdomyoszwift`.

## Pilot coverage

1. **FTMS Bike search consistency**: verifies that a non-default FTMS Bike value is identical in the normal Settings view and in search results. This protects against the regression where search showed `Disabled` even though a device was saved.
2. **Experimental setting persistence**: enables Android Notification and Fake Device, leaves Settings, reopens it, and verifies both values persisted.
3. **Settings navigation smoke test**: exercises Settings, search, Experimental Features, and the return to the main screen while checking for crashes, ANRs, blank pages, and unrecoverable navigation failures.

## CI strategy

Keep these tests opt-in while evaluating reliability. If the pilot proves stable, the recommended next step is a nightly Artemis job on a dedicated emulator/device, with a much smaller smoke test considered later for pull requests.
