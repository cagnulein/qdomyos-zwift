# ProForm / NordicTrack FitPro migration coverage

This document describes the state of the migration in `fix/proform-ifit-generic-codec`.
It intentionally does not claim hardware compatibility that has not been exercised.

## Runtime policy

1. A selected QZ model profile remains the primary path.
2. If no treadmill or bike model profile is enabled, the driver enters the generic FitPro fallback.
3. The fallback probes `GET_SUPPORTED_DEVICES (0x80)`, `GET_INFO (0x81)`, and
   `GET_SUPPORTED_COMMANDS (0x88)` on the `00001533` service.
4. It uses the APK-derived codec for speed, grade, resistance, required-start,
   workout-mode polling, FE/fragment/FF packetization, reassembly, checksum, and
   standard FitPro field decoding, but only after a valid discovery response has
   passed frame and checksum validation.
5. Legacy model-specific paths are not silently replaced. A device that does not
   answer with valid FitPro framing remains unsupported by the generic path and
   must retain its QZ profile or be reported as unsupported.

## Verified generic codec surface

| Area | Status | Evidence |
|---|---|---|
| Field IDs and fixed-point conversion | verified | C++ unit tests and APK-derived Python tests |
| `WRITE_READ_DATA (0x02)` | verified | exact frame fixtures |
| Checksum | verified | valid and corrupted-frame tests |
| BLE wrapper `02 04 02` | verified | exact frame fixtures |
| FE/fragment/FF packetizer | verified | single and multi-fragment fixtures |
| Reassembly and frame decoder | verified | round-trip C++ test |
| Discovery `0x80/0x81/0x88` | verified at frame level | exact APK-derived fixtures |
| Required start field `0x6c` | verified at frame level | exact capture fixture |
| Speed field `0x00` | verified against captures | 12 speed fixtures, zero mismatches |
| Grade field `0x01` | verified against captures | 5 grade fixtures, zero mismatches |
| Resistance field `0x02` | frame generation verified | no physical bike validation yet |
| Workout mode field `0x0c` | read-frame generation verified | no physical device validation yet |
| Generic RX telemetry | parser verified synthetically | no physical notification capture using this path yet |

## Profile inventory

### Treadmills

There are 59 selectable treadmill profile settings in the current driver. The
following profiles remain QZ-specific and are **not proven replaceable by the
generic codec** because their initialization, polling, lifecycle, telemetry, or
conversion behavior has not been shown byte-equivalent to the modern APK:

- `nordictrack10`
- `nordictrackt70`
- `nordictrack_t65s_treadmill`
- `nordictrack_treadmill_ultra_le`
- `nordictrack_treadmill_commercial_le`
- `proform_treadmill_carbon_tls`
- `nordictrack_s30_treadmill`
- `proform_treadmill_1800i`
- `proform_treadmill_se`
- `proform_treadmill_8_0`
- `proform_treadmill_9_0`
- `proform_cadence_lt`
- `norditrack_s25i_treadmill`
- `norditrack_s25_treadmill`
- `nordictrack_t65s_83_treadmill`
- `nordictrack_incline_trainer_x7i`
- `nordictrack_incline_trainer_x7i_ntl15010_0`
- `nordictrack_incline_trainer_x7i_netl18716_0`
- `proform_treadmill_z1300i`
- `proform_pro_1000_treadmill`
- `nordictrack_s20_treadmill`
- `nordictrack_s20i_treadmill`
- `proform_treadmill_l6_0s`
- `proform_8_5_treadmill`
- `proform_2000_treadmill`
- `proform_treadmill_sport_8_5`
- `proform_treadmill_505_cst`
- `proform_treadmill_705_cst`
- `proform_carbon_tl`
- `proform_proshox2`
- `proform_595i_proshox2`
- `proform_treadmill_8_7`
- `proform_treadmill_705_cst_V78_239`
- `nordictrack_treadmill_exp_5i`
- `proform_carbon_tl_PFTL59720`
- `proform_treadmill_sport_70`
- `proform_treadmill_575i`
- `proform_performance_300i`
- `proform_performance_400i`
- `proform_treadmill_c700`
- `proform_treadmill_c960i`
- `nordictrack_tseries5_treadmill`
- `proform_carbon_tl_PFTL59722c`
- `proform_treadmill_1500_pro`
- `proform_505_cst_80_44`
- `proform_trainer_8_0`
- `proform_trainer_8_0_pftl59721_int_0`
- `proform_trainer_8_0_pftl59721_0`
- `proform_treadmill_705_cst_V80_44`
- `nordictrack_t65s_treadmill_81_miles`
- `nordictrack_elite_800`
- `proform_treadmill_995i`
- `nordictrack_series_7`
- `proform_treadmill_sport_3_0`
- `proform_carbon_tlx_treadmill`
- `proform_carbon_tlx_v84_314_treadmill`
- `proform_carbon_tl_PFTL59723_6`
- `proform_treadmill_cst_505_pftl59420_0`
- `proform_treadmill_105_cst`

A prior static inventory of the treadmill source found 50 array-bearing branches,
42 distinct ordered schedules, and 8 duplicate schedule groups. Those schedules
are still legacy coverage, not generic-codec coverage.

### Bikes

There are 22 selectable bike profile settings. They remain model-specific until
resistance conversion and telemetry are validated on captures or hardware:

- `proform_studio`
- `proform_tdf_10`
- `nordictrack_GX4_5_bike`
- `nordictrack_gx_2_7`
- `proform_hybrid_trainer_PFEL03815`
- `proform_bike_sb`
- `proform_cycle_trainer_300_ci`
- `nordictrack_gx_4_5_pro`
- `proform_bike_225_csx`
- `proform_bike_325_csx`
- `proform_tour_de_france_clc`
- `proform_studio_NTEX71021`
- `freemotion_coachbike_b22_7`
- `proform_cycle_trainer_400`
- `proform_bike_PFEVEX71316_1`
- `nordictrack_gx_44_pro`
- `proform_bike_PFEVEX71316_0`
- `proform_xbike`
- `proform_225_csx_PFEX32925_INT_0`
- `proform_csx210`
- `nordictrack_vr21`
- `proform_bike_325_csx_PFEX439210INT_0`

## Settings decision

The profile settings must not be removed or commented yet. They still select
legacy initialization/telemetry semantics, and no hardware test has demonstrated
that a generic discovery response makes those semantics unnecessary.

The safe removal gate is:

- generic discovery response parsed and capability-checked;
- generic telemetry verified for speed, grade, resistance, cadence, and power;
- start/stop and polling verified on at least one treadmill and one bike;
- every legacy profile either mapped to the same fields or explicitly retained as
  a non-FitPro exception;
- fallback failure reported without sending unsupported control fields;
- capture regression suite passes for all distinct legacy schedules.

Until then, the implementation is a generic fallback and RX parser, not a claim
that every listed model speaks the modern FitPro dialect.
