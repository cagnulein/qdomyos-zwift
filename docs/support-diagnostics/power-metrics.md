# Power and training metrics

## Power differs from the machine or training app

### Provide first

- Machine model and QZ version.
- One simultaneous power reading from the machine and QZ.
- The current Watt Gain and Watt Offset values.
- The configured FTP if the issue concerns power zones.
- Which power source is selected in the training app.

### If still unclear

- Record a short QZ debug log while pedaling steadily.
- Note the approximate time and expected versus observed watts.

### Why it matters

- QZ can scale received power using gain and offset.
- FTP affects power-zone classification separately from watts.

## Power is implausibly high and cadence may also be wrong

### Provide first

- Exact bike/trainer model, advertised Bluetooth name, QZ platform and **installed build**.
- During a short, steady effort, give the **simultaneous** QZ cadence, QZ watts, and resistance readings. Compare cadence with the bike console or a known independent cadence sensor, if available.
- State whether the bike itself reports power or QZ estimates it; if unsure, say which power source QZ is configured to use. Include current **Watt Gain**, **Watt Offset**, and any selected cadence/power sensor.
- Describe whether the error is constant, appears only at certain resistance levels, or coincides with abrupt cadence jumps. An external power-meter comparison is useful but not required initially.

### If still unclear

- Record a short QZ debug log beginning **before Bluetooth connection** and covering a brief idle period, steady pedaling at a comfortable cadence, and one small resistance change. Give approximate timestamps and the corresponding console values.
- If the cadence or power spikes, include a screenshot or short recording showing both tiles at the same time; do not rely only on workout-average watts.
- If support provides a parser test build, confirm its installed version and repeat the **same short test** with a new log. Keep the old and new logs distinct.

### Why it matters

- Some bikes derive power from cadence and resistance rather than transmitting independently measured watts. A malformed or misclassified cadence notification can therefore inflate both metrics; adjusting Watt Gain would mask the symptom without identifying the input error.
- In one reviewed case, an abnormally high wattage report led to a debug-log comparison that identified non-cadence frames being treated as cadence. The parser was changed, but the subsequent test build had not yet been confirmed by the rider.
- Comparing instantaneous cadence, resistance, and watts distinguishes **source/packet decoding** from an ordinary power-scale or FTP-setting discrepancy. For external meter versus trainer ERG mismatches, use the [external power-match intake](external-sensors.md#external-power-match-correction-appears-ineffective-during-erg-workouts).

## Power alternates or drifts only when an external app controls ERG mode

### Provide first

- Identify the bike/ergometer, its **USB/serial or Bluetooth connection** to QZ, QZ build, receiving app/version, and which device runs each app.
- Test a short, comfortable steady effort **with QZ alone**, then repeat with the receiving app connected in **free ride** and in a modest **ERG target**. Report which mode first shows the problem; do not change several QZ settings between tests.
- At the same moment, compare **bike console watts**, **QZ's live Power tile**, and **the receiving app's displayed watts**, plus the ERG target. Record a few concrete readings, especially whether power briefly falls to a fixed low value and then recovers.
- Screenshot the receiving app's **Power Source** and **Controllable** selections. Confirm that QZ, rather than the physical machine, is the intended bridge.

### If still unclear

- Capture two short, clearly labeled **QZ debug logs**, one with QZ controlling the machine without external ERG and one with external ERG active. Start logging before connection, include a steady pedaling period and one modest target change, and **stop the QZ session to finalize the log**.
- Record the relevant **ERG filter up/down** and **power averaging** options, and whether any were changed between runs. Compare identical efforts before and after changing just one option; report both displayed power and how resistance actually feels.
- If QZ's live power itself alternates between a very low reading and the plausible console value, preserve the raw incoming readings in the log before treating the receiving app as the source of the error. If QZ is stable but the receiving app differs, inspect the virtual power stream and pairing roles instead.

### Why it matters

- The bike's reported power, QZ's processed power, the virtual power sent to a training app, and the app's ERG target are distinct signals. A mismatch visible already in QZ points upstream, whereas a mismatch only in the receiving app points downstream.
- In a USB ergometer investigation, QZ-only and MyWhoosh-controlled captures exposed a discrepancy that was initially attributed to outbound power but was subsequently traced to intermittent low power values coming from the machine. This illustrates why synchronized readings and paired A/B logs are more informative than changing ERG filters first.
- Averaging can conceal brief invalid readings without establishing their source; retain the original capture and avoid treating an unconfirmed filter adjustment as a resolved fix.

## ERG target cannot be reached even at the highest resistance

### Provide first

- Exact ergometer model, QZ build, the machine's maximum **physical console resistance level**, and the highest level QZ can actually command. Report the ERG target and the observed steady watts, cadence, and resistance at the same moment.
- Confirm whether the power ceiling is also present when the machine is operated **from its own console with QZ and other controlling apps disconnected**. In the console's manual-resistance mode (not constant-watt mode), use a comfortable high level and report console resistance, cadence, and watts after a brief steady effort; do not force an unsafe maximum.
- State whether the target was tested after an ERG calibration reset, and whether a **QZ-only free ride** with several resistance levels was performed before returning to ERG. Include the selected Power Sensor source and whether automatic target-power control was active during calibration.

### If still unclear

- Provide two separately labeled short QZ debug logs: a **free ride** with a few safe, manually selected resistance levels and steady cadence, followed by a **modest target-power workout** without clearing the calibration in between. Note resistance, cadence, and watts at each step, plus the QZ settings controlling the allowed resistance range.
- If QZ power drops to zero or shows implausible spikes at low resistance, record which level and cadence produce those readings. Keep the raw log rather than relying on the filtered Power tile.
- Compare the console-only physical effort with QZ commanding a similar resistance. A photo of the console showing resistance, cadence, and watts is helpful only if the machine can display those values while not connected to QZ.

### Why it matters

- A target above the machine's achievable power at the rider's cadence cannot be solved by expanding QZ's resistance limit alone. The independent console test distinguishes a genuine mechanical/physical ceiling from incorrect command scaling or telemetry.
- An empty learned ERG calibration table can prevent reliable target selection. A free-ride calibration test distinguishes missing learned points from an ERG control defect; do not infer that a reset alone fixes target tracking.
- Implausible or zero power at low resistance can mislead calibration and ERG feedback. Separating raw telemetry, resistance commands, and physical console output avoids treating a filtering artifact as proof of an achievable target.
