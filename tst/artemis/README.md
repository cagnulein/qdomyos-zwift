# QZ Android Artemis pilot tests

This directory contains Android end-to-end tests driven by [Google Artemis](https://github.com/google/artemis). They complement the deterministic C++/GoogleTest suite; they do not replace it.

## Why Artemis

QZ uses Qt Quick/QML, so Android's accessibility hierarchy can be incomplete or misleading. Existing Android automation therefore sometimes falls back to fixed coordinates. Artemis can combine accessibility data with coordinate and visual targeting, which makes it useful for regression tests across QZ's custom UI.

## CI design

QZ already builds the Android APK and runs it in `android-emulator-test` with `ReactiveCircus/android-emulator-runner`. Artemis should reuse that same emulator rather than creating a second Android job.

The pilot runs only on the existing API 35 matrix entry. This avoids running LLM-driven UI tests on all nine Android versions while still testing the exact APK produced by `android-build`.

Artemis itself runs locally on the GitHub-hosted runner with `artemis run --standalone`; no Artemis daemon or external device host is required. The Artemis checkout is pinned to commit `371aa6df56880643da57b30da936e9812fb0ec66` so upstream changes cannot silently change test behavior.

## Required GitHub secret

Artemis needs an LLM credential. Add this repository Actions secret:

```text
ARTEMIS_GEMINI_API_KEY
```

Its value is a Google AI Studio / Gemini API key. The workflow maps it to `GEMINI_API_KEY`, which Artemis accepts as its Google provider credential.

When the secret is unavailable, for example on an untrusted fork PR, `run_ci.sh` normally exits successfully with a notice instead of exposing credentials. Set `ARTEMIS_REQUIRED=true` for a trusted run when a missing credential must fail the job.

## Existing workflow integration

For the API 35 entry, the workflow prepares Artemis before `android-emulator-runner`:

```yaml
- name: Setup Python for Artemis
  if: matrix.api-level == 35
  uses: actions/setup-python@v5
  with:
    python-version: '3.12'

- name: Checkout pinned Artemis
  if: matrix.api-level == 35
  uses: actions/checkout@v4
  with:
    repository: google/artemis
    ref: 371aa6df56880643da57b30da936e9812fb0ec66
    path: .artemis

- name: Prepare Artemis
  if: matrix.api-level == 35
  run: |
    python -m pip install --upgrade uv
    cd .artemis
    uv sync --frozen
```

The existing emulator step exposes the secret:

```yaml
env:
  GEMINI_API_KEY: ${{ secrets.ARTEMIS_GEMINI_API_KEY }}
  ARTEMIS_REQUIRED: true
```

After QZ has been installed, permissions granted, and the app process verified, the same `script:` invokes:

```bash
if [ "${{ matrix.api-level }}" = "35" ]; then
  bash tst/artemis/run_ci.sh
fi
```

This means the lifecycle is:

`android-build -> APK artifact -> existing API 35 emulator -> QZ -> Artemis UI tests`

There is no second APK build and no second emulator for Artemis.

## Pilot coverage

1. **FTMS Bike search consistency**: reads the FTMS Bike value in normal Settings, searches for `ftms`, and requires the filtered result to show exactly the same value. A clean emulator will normally have `Disabled`; the exact saved-non-default-device variant still requires a discoverable BLE test device or a future deterministic Bluetooth fixture.
2. **Experimental setting persistence**: enables Android Notification and Fake Device, leaves Settings, reopens it, and verifies both values persisted.
3. **Settings navigation smoke test**: exercises Settings, search, Experimental Features, and the return to the main screen while checking for crashes, ANRs, blank pages, and unrecoverable navigation failures.

The first two tasks use Artemis `pro` with strict final/checkpoint verification; the broad smoke flow uses `flash` to keep the pilot reasonably fast.

## Local use

The same runner can be used on a local emulator or dedicated Android device. Checkout the pinned Artemis revision into `.artemis`, install it with Python 3.12 and `uv sync --frozen`, export a Gemini key, make sure `adb devices` shows the target, then run:

```bash
export GEMINI_API_KEY='...'
bash tst/artemis/run_ci.sh
```

Optional variables:

- `ARTEMIS_DIR`: Artemis checkout, defaults to `$GITHUB_WORKSPACE/.artemis`.
- `ARTEMIS_DEVICE_SERIAL`: target ADB serial; auto-detected when omitted.
- `ARTEMIS_TRACES_PATH`: output directory for Artemis traces.
- `ARTEMIS_REQUIRED`: set to `true` to fail instead of skip when no LLM credential is present.
- `QZ_ANDROID_PACKAGE`: defaults to `org.cagnulen.qdomyoszwift`.

## Safety

The persistence test changes Android Notification and Fake Device. Run it only on an emulator or dedicated QZ test profile, not on a personal production configuration.
