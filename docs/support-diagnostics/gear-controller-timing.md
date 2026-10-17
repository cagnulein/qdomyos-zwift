# Virtual gearing and controller timing diagnostics

## Shifting stops after an initial working period

### Provide first

- Controller model and QZ version.
- Whether the Gears tile continues to change after button presses stop having an effect.
- Time from connection until the behavior changes.

### If still unclear

- Record a short QZ debug log with early and late button presses.
- Compare QZ-only operation with the external training app closed.
- Record which app pairs to the controller.

### Why it matters

- Distinguishes missing controller events from gear commands that do not affect the trainer.
- A time-dependent failure may require different evidence than a permanently missing control.

## Gear number increases but resistance levels off or becomes too strong

### Provide first

- Trainer model, QZ version, and the first gear at which physical resistance stops changing or becomes excessive.
- Does the Gears tile continue to change? Compare cadence and watts at a similar pedaling rate across two nearby gears.
- Report Gears Gain and whether Custom Gear Table or Zwift Gear Ratio is enabled.

### If still unclear

- With the training app closed, test a few adjacent gears individually and capture a short QZ log. Stop if the load becomes excessive.
- Share relevant trainer-specific gearing settings and a QZ dashboard screenshot.

### Why it matters

- Gear values, trainer resistance, and reported power are separate measurements.
- Local Gears Gain is not the step-size control in custom-table or Zwift-ratio modes; knowing the mode avoids an ineffective setting change.
