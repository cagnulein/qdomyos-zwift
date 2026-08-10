# Automatic ERG mode — parked, unproven

`src/ergautomode.h` decides when ERG should be engaged, from what the training app
asks for over the control point. It is committed with its tests and **wired into
nothing**. This note exists so that whoever picks it up knows what was actually
established, what was only assumed, and what would have to happen before it could
be trusted on a bike.

Status as of 2026-08-10: **not validated against a ride.** Do not treat the
confidence of the comments in the header as evidence.

## The idea

MyWhoosh alternates two FTMS control point opcodes, and the alternation lines up
with the workout blocks rather than with anything about the rider:

```
19:35:07  indoor bike simulation parameters (0x11)   free ride
19:35:45  erg mode (0x05), then every ~5 s           workout block
19:37:10  indoor bike simulation parameters (0x11)   free ride again
19:37:50  erg mode (0x05)                            next block
```

So the app already says which mode it wants; nothing has to be inferred from
cadence or power. Three conditions decide the state, and all three are needed:

1. **Which opcode came last.** A simulation write means the app is driving slope.
2. **Whether the target is non-zero.** 673 of the power requests in that ride were
   for 0 W, which is "no target", not "hold zero watts". Opcode alone would latch
   ERG on through every coast.
3. **Whether it is still fresh.** Targets arrive every ~5 s during a block. If they
   stop, the block is over or the app is gone, and holding the last resistance
   forever is the worst of the three outcomes.

The class takes `now` from the caller and owns no Qt objects or clock, so every
branch is reachable from a test.

## What is actually evidence, and what is not

**Evidence.** The opcode sequence, the ~5 s cadence, the 673 zero-watt requests and
the three simulation writes inside 200 ms at 19:37:10 all come from one real
capture: MyWhoosh writing into QZ's virtual bike, on 2026-08-07.

**Not evidence.** Everything else, including:

- `kStaleMs = 15000` and `kMinHoldMs = 3000` are chosen to be "three missed updates"
  and "longer than the observed burst". Neither was tuned against a ride, and no
  rider has reported how either feels.
- One app, one session, one workout structure. Rouvy and Zwift were never captured.
  MyWhoosh's own behaviour across workout types is unsampled.
- The claim that a zero-watt request means "no target" is an inference from its
  frequency, not from anything the protocol says. A workout with a genuine 0 W
  interval would be indistinguishable, and this class would drop ERG through it.

## Tests

`tst/Erg/TestErgAutoMode.{h,cpp}` — 11 cases, now wired into
`tst/qdomyos-zwift-tests.pro`, including a replay of the transition sequence above.

They pin the behaviour the class was written to have. They cannot tell you the
behaviour is *right*, because the same single capture produced both the code and
the cases. Read a green run as "unchanged", not as "correct".

**The tests do not run in this fork's CI.** They are built and executed by
`linux-x86-build` (`cd tst; ./qdomyos-zwift-tests`), which is gated off here — see
[BUILDING-ON-WINDOWS.md](BUILDING-ON-WINDOWS.md). Re-enable that job, or run them
locally, before believing anything about their state.

## Picking it up

Nothing consumes this class. To finish it:

1. **Feed it.** The control point writes arrive in the virtual bike's FTMS handler;
   `onTargetPower()` and `onSimulation()` need calling from wherever 0x05 and 0x11
   are decoded. `ergShouldBeOn()` must also be polled, not only called on writes -
   staleness is a function of time passing, so nothing else notices targets
   stopping.
2. **Decide what it drives.** It currently reports intent only. Whether that becomes
   `erg_mode`, a resistance path, or only a tile is unresolved.
3. **Give it a tile and a setting.** There is no UI and no way to turn it off, which
   makes it untestable by anyone but the person who built it. Note `src/CLAUDE.md`
   for the rules about adding a setting - `allsettingscount` and the catalog count
   both have to move.
4. **Capture a second ride before trusting the constants**, ideally on a different
   app, and specifically look for a workout with a real zero-watt interval.

## Provenance

Written alongside the ERG work that lived on `claude/frente-a-atraso-ima-cpxlhj`
(tip `59562d93`) and `resistance-target-fix` (tip `774347c5`). Both branches were
deleted on 2026-08-10; their objects survive in the local reflog for about 90 days,
so `git branch <name> <sha>` still recovers them until then. If the constants here
ever need their original reasoning, that is where the rest of it is.

One change was made when committing: `#include <cstdint>` was added. The header uses
`uint16_t` but included only `<QtGlobal>`, which does not pull it in - it was
compiling by luck, and would have broken the test build sooner or later.
