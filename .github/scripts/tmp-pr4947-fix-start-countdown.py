from pathlib import Path
import json


def replace_once(path, old, new):
    p = Path(path)
    text = p.read_text()
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{path}: expected exactly one match, found {count}")
    p.write_text(text.replace(old, new, 1))


replace_once(
    "src/trainprogram.h",
    "    int32_t trainingProgramPowerOffset = 0;\n    int lastLapButtonToastStep = -1;\n",
    "    int32_t trainingProgramPowerOffset = 0;\n"
    "    int treadmillStartCountdownElapsed = 0;\n"
    "    bool treadmillStartCountdownCompensationApplied = false;\n"
    "    int lastLapButtonToastStep = -1;\n",
)

old_countdown = '''    const int treadmillStartCountdown =
        qMax(0, settings.value(QZSettings::treadmill_start_countdown,
                               QZSettings::default_treadmill_start_countdown).toInt());
    const bool treadmillStartupCountdownActive =
        bluetoothManager && bluetoothManager->device() &&
        bluetoothManager->device()->deviceType() == TREADMILL &&
        treadmillStartCountdown > 0 && ticks >= 0 && ticks < treadmillStartCountdown;

    if (treadmillStartupCountdownActive) {
        qDebug() << "trainprogram treadmill startup countdown" << ticks
                 << "of" << treadmillStartCountdown << "seconds";
    }
'''
new_countdown = '''    const int treadmillStartCountdown =
        qMax(0, settings.value(QZSettings::treadmill_start_countdown,
                               QZSettings::default_treadmill_start_countdown).toInt());
    const bool treadmillStartupCountdownActive =
        started && bluetoothManager && bluetoothManager->device() &&
        bluetoothManager->device()->deviceType() == TREADMILL && currentStep == 0 &&
        treadmillStartCountdown > 0 && treadmillStartCountdownElapsed < treadmillStartCountdown;

    if (treadmillStartupCountdownActive) {
        qDebug() << "trainprogram treadmill startup countdown window"
                 << treadmillStartCountdownElapsed << "of" << treadmillStartCountdown << "seconds";
    }
'''
replace_once("src/trainprogram.cpp", old_countdown, new_countdown)

replace_once(
    "src/trainprogram.cpp",
    '''    ticks++;
    qDebug() << QStringLiteral("trainprogram ticks") << ticks << QStringLiteral("currentTimerJitter") << currentTimerJitter;
''',
    '''    ticks++;
    if (treadmillStartupCountdownActive && treadmillStartCountdownElapsed < treadmillStartCountdown) {
        treadmillStartCountdownElapsed++;
    }
    qDebug() << QStringLiteral("trainprogram ticks") << ticks << QStringLiteral("currentTimerJitter") << currentTimerJitter;
''',
)

replace_once(
    "src/trainprogram.cpp",
    '''    if (ticks == 1 && currentStep == 0) {
        rows[currentStep].started = QDateTime::currentDateTime();
        currentStepDistance = 0;
        lastOdometer = odometerFromTheDevice;
        emit intervalTransitionApplied();
        if (bluetoothManager->device()->deviceType() == TREADMILL) {
''',
    '''    if (ticks == 1 && currentStep == 0) {
        rows[currentStep].started = QDateTime::currentDateTime();
        currentStepDistance = 0;
        lastOdometer = odometerFromTheDevice;
        emit intervalTransitionApplied();

        if (bluetoothManager->device()->deviceType() == TREADMILL && treadmillStartCountdown > 0 &&
            !treadmillStartCountdownCompensationApplied) {
            ticks += treadmillStartCountdown;
            treadmillStartCountdownCompensationApplied = true;
            qDebug() << "trainprogram treadmill startup countdown compensation"
                     << treadmillStartCountdown << "seconds, adjusted ticks" << ticks;
        }

        if (bluetoothManager->device()->deviceType() == TREADMILL) {
''',
)

replace_once(
    "src/trainprogram.cpp",
    '''void trainprogram::restart() {
    trainingProgramPowerOffset = 0;
''',
    '''void trainprogram::restart() {
    trainingProgramPowerOffset = 0;
    treadmillStartCountdownElapsed = 0;
    treadmillStartCountdownCompensationApplied = false;
''',
)

old_description = "Time in seconds between the Start command and the treadmill belt beginning to move. Training programs continue advancing during this countdown to stay synchronized. Set to 0 to disable. Default is 0."
new_description = "Seconds to compensate for a treadmill's startup countdown. QZ advances the training-program timeline by this amount once at the initial start and keeps the program clock running during the countdown. It is not applied to later treadmill restarts. Set to 0 to disable. Default is 0."
replace_once("src/settings.qml", old_description, new_description)

catalog_path = Path("src/settings-catalog.json")
catalog = json.loads(catalog_path.read_text())
matches = [s for s in catalog.get("settings", []) if s.get("key") == "treadmill_start_countdown"]
if len(matches) != 1:
    raise SystemExit(f"settings-catalog: expected one treadmill_start_countdown, found {len(matches)}")
matches[0]["description"] = new_description
catalog_path.write_text(json.dumps(catalog, indent=2, ensure_ascii=False) + "\n")
