# Devices detail

## Echelon


## Sportstech

### ESX 500
#### HRM
The [cardio belt provided](https://www.sports-tech.uk.com/chest-strap-sportstech-uncoded) with the bike is also supported, the bike reads the value and forwards it in its bluetooth signal.
The cardio captors are reported to not be accurate.
#### Resistance 
The resistance is adjusted after a few seconds delay (time for the engine to adapt magnetic resistance).

## Stages

### SB20

The SB20 advertises as `Stages Bike <NNNN>` and is handled by the **`ftmsbike`** driver — no
dedicated device class. It has **no display of its own**, so initial pairing/setup is done in the
Stages Cycling app.

#### Power and cadence
Both come from the standard FTMS **Indoor Bike Data** characteristic (`0x2AD2`). The bike sets
flags `0x00c5` — instantaneous cadence, instantaneous power and average power.

#### Auto resistance (ERG)
Fully supported over standard FTMS, and reliable in regular use — including QZ-managed **Peloton
Power Zone** workouts, where QZ drives the target watts for the whole session.

QZ takes control and sets a target, and the bike honours it:

```
0x2AD9 <- 00        Request Control     -> 80 00 01        (success)
0x2AD9 <- 07        Start/Resume        -> 80 07 01        (success)
0x2AD9 <- 05 XX XX  Set Target Power    -> 80 05 01 XX XX  (success)
```

#### Rare issue: telemetry drops to zero
Uncommon — power and cadence are reliable in normal use. Documented because the symptom looks like
a QZ bug when it is not.

The bike can occasionally end up publishing a **stripped** Indoor Bike Data frame — flags `0x0011`, distance
only, no power or cadence — while the Cycling Speed and Cadence characteristic stops updating.
QZ then correctly shows 0 W and 0 rpm, because that is genuinely all the bike is sending.

A power-cycle of the bike restores the full `0x00c5` frame. The trigger is not confirmed; a
plausible cause is another consumer holding an FTMS **control** connection (for example a head unit
or watch paired to the bike *as a trainer* rather than as sensors). If QZ shows no power while you
are clearly pedalling, power-cycle the bike and check what else is connected to it.
