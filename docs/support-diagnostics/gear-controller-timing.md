# Controller timing diagnostics

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
