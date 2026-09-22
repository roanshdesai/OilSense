# Bench run captures

Paste raw serial output here, one file per session, named by date and time.
Keep them even when the run went badly - the failed sessions are what revealed
the 1.12 pF baseline drift and the raised-probe path-length effect.

Record alongside each run:
- probe geometry (rod spacing, immersed length, clamp position)
- container used and fill depth
- room temperature
- anything that changed since the last session

---

## Files here

| File | Rows | Notes |
|---|---|---|
| `oilsense_probe-01_2026-08-07.csv` | 34 | dashboard export, probe-01 |
| `oilsense_probe-01_2026-08-09.csv` | 99 | dashboard export, probe-01 |

Both are dashboard exports, so they carry derived columns (`permittivity`,
`optical_ratio`, `index`, `tpm_equiv`) alongside the raw observables. The
derived values were computed with the calibration constants in force at the
time; re-derive from `cap_pf` and the channel columns rather than trusting them.

## ⚠ The channel columns are mislabelled

Both files predate the AS7265x channel-order fix (CLAUDE.md §7 #11). The header
reads `ch410 … ch940` ascending, but the device uploaded the channels in
SparkFun's **letter order**, so every column from `ch610` on holds a different
wavelength than its name says. Columns `ch410` through `ch585` are correct.

| Column header | Actually holds |
|---|---|
| `ch610` | 645 nm |
| `ch645` | 705 nm |
| `ch680` | 900 nm |
| `ch705` | 940 nm |
| `ch730` | 610 nm |
| `ch760` | 680 nm |
| `ch810` | 730 nm |
| `ch860` | 760 nm |
| `ch900` | 810 nm |
| `ch940` | 860 nm |

So the `optical_ratio` column in these files is **435 / 860 nm**, not 435 / 940
as the old code claimed. That is the same channel the current firmware uses
(`CH_NIR = 15`), so the ratio itself is consistent with present-day readings —
only the label was ever wrong.

To read these correctly, apply the inverse of the `SRC[]` permutation in
`firmware/*/*.ino`, or just use the mapping above. Records uploaded after the
fix carry `"ch_order": "asc_410_940"` and need no remapping; the dashboard
handles both in `normalise()`.
