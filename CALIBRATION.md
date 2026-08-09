# Calibration log

Every fit belongs to one probe configuration in one session. **They do not
transfer.** Air has permittivity 1.000 every single time, so any change in the
air reading between sessions is the probe changing, not the medium.

Append a new block after every re-fit. Never edit an old one.

---

## Why this file exists

Air readings across five sessions:

```
6.394   6.625   6.816   7.440   7.514  pF
```

Spread: **1.12 pF**, on a medium whose permittivity is identical every time.

A full fresh-to-discard signal is 0.377 pF at K=0.686, or 0.144 pF at K=0.262.
So the session-to-session baseline drift is **3–8× larger than the entire
quantity being measured**. This is the central unsolved problem, and the reason
constants must be re-fitted rather than reused.

---

## Fitting procedure

```
C = P + K · εr

air   (εr = 1.000)      →  C_air  = P + K
fresh (εr ≈ 3.00, lit.) →  C_fresh = P + 3K

K = (C_fresh − C_air) / 2
P = C_air − K
```

Figure of merit = C_fresh / C_air. Ideal 3.00. Below 2.0 means parasitics
dominate.

Bench sketch: press `a`, then `1`, then `r`. It prints K, P, FoM and parasitic
share.

---

## Session A — original disclosure

| | |
|---|---|
| C_air | 14.4 pF |
| C_fresh | 36.0 pF |
| **K** | **10.8 pF/ε** |
| **P** | **3.6 pF** |
| Parasitic share | 25 % |
| FoM | 2.50 |

These are the numbers in the technical disclosure. **They are almost certainly
from a plate or concentric geometry, not two thin rods** — two M6 rods at 50 mm
cannot physically reach 10.8 (max ~1.8 even nearly touching). Treat as
aspirational, not reproducible with the current probe.

---

## Session E — 7 Aug, best capacitive run

Temperature 30.0 °C throughout. Air spread 0.058 pF across 8 runs — cleanest
stability achieved.

| Sample | C (pF) | σ | εr | Λ (raw 435/940) | 410/940 corrected |
|---|---|---|---|---|---|
| air | 6.394 | 0.021 | 1.000 | — | — |
| fresh | 7.766 | 0.072 | 3.000 | 0.8478 | 3.033 |
| slightly degraded | 7.491 | 0.068 | 2.599 | 0.8976 | 2.981 |
| amber | 7.841 | 0.068 | 3.109 | 0.5196 | 1.256 |

**K = 0.686 · P = 5.708 · parasitic 89 % · FoM 1.21**

**Key result:** both channels independently ranked *slightly degraded* as the
freshest of the three. Two unrelated measurements, same ordering — the sample
labels were wrong, or they were different base oils.

**Caveat found later:** the fresh→amber capacitance gap is only 0.075 pF against
0.144 pF of noise, so that pair was **not** statistically resolvable. Ordering
matched; magnitude did not.

Optical survival fresh→amber: 410 nm 32 %, 435 nm 45 %, 460 nm 65 %, 940 nm 78 %.
Smooth monotonic — genuine absorbance.

---

## Session F — 7 Aug, later

| Sample | C (pF) | σ | Λ |
|---|---|---|---|
| air | 6.625 | 0.021 | — |
| fresh | 7.149 | 0.072 | 1.3625 |
| amber | 6.992 | 0.068 | 1.0738 |

**K = 0.262 · P = 6.364 · parasitic 96 % · FoM 1.079**

**Capacitive channel inverted** — amber read 0.157 pF *below* fresh. That
discrepancy is 30 % of the whole air-to-oil gap, i.e. roughly a 30 % change in
immersed rod length between dips. At this cell constant the full fresh-to-discard
span is 0.144 pF against 0.072 pF of noise, so depth variation of a few
millimetres beats the signal.

Optical direction correct: Λ fell 1.3625 → 1.0738, a **21 % drop**. That 0.788
ratio is the reference used for setting OPT_DISCARD.

---

## Session G — 7 Aug 21:41, fresh oil only

| Λ | note |
|---|---|
| 1.009 | |
| 1.095 | |
| 0.848 | **probe accidentally raised** |

Excluding the raised reading: mean **1.052**, spread **8.2 %**.
Including it: mean 0.984, spread **25 %**.

**This accident is the most useful data in the log.** Raising the probe changed
the optical path length and moved the reading by 25 % — direct evidence that path
length, not the sensor, is the dominant optical error source. It also tells you
the fix precisely: a fixed gap to a reflector.

Capacitance in this session gave εr of 6.3–9.1 — **physically impossible** for an
edible oil (2.5–4.5), while σ was 0.001–0.013 pF. Quiet and wrong. This is the
case that motivated the permittivity plausibility bound.

**Constants currently in the field firmware:**

```
OPT_FRESH   = 1.052
OPT_DISCARD = 0.829     (= 1.052 × 0.788)
CELL_PF_PER_EPS = 0.262
PARASITIC_PF    = 6.364
REF_TEMP_C  = 30
SD_LIMIT_PF = 0.15
```

---

## Noise reference

| Quantity | Typical | Genuine disturbance |
|---|---|---|
| σ within one measurement (64 samples) | 0.02–0.08 pF | 0.12–0.37 |
| Run-to-run spread (8 runs) | 0.06–0.14 pF | — |
| Session-to-session air drift | — | **1.12 pF** |
| Optical dip-to-dip, consistent depth | 8.2 % | — |
| Optical dip-to-dip, varying depth | 25 % | — |

σ over 64 samples taken milliseconds apart is always flattering. **Run-to-run
spread is the honest error.** Session-to-session drift is the real limit.

---

## Constants that are NOT measured

Flag these clearly whenever quoting a TPC figure.

| Constant | Value | Basis |
|---|---|---|
| εr fresh | 3.00 | literature, not measured |
| εr discard | 3.55 | assumed |
| OPT_DISCARD | fresh × 0.788 | anchored to the amber sample, **not to a known TPC** |
| TPM-eq mapping | 8 + 17 × index | provisional; index 1.00 pinned to 25 % |

Stage-3 anchoring against lab-measured TPC replaces all four. Because raw data is
stored, every historical reading re-derives at that point.
