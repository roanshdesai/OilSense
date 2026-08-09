# OilSense — project context

Read this first. It exists so a new session does not re-derive decisions that are
already made, or re-introduce bugs that are already fixed.

**What it is:** a low-cost frying-oil degradation meter. Two physically unrelated
sensors measure the same chemistry, so they can cross-check each other. Built by
Roansh Desai, Class 11, Mumbai. Target ₹9,262 against ₹40,000–50,000 for a
commercial tester (that price is FSSAI's own published figure).

---

## 1. THE ONE IDEA

Frying degrades oil into **polar compounds**. India's legal limit is **25 % total
polar compounds (TPC)**, binding on all food businesses since 1 July 2018.

Polar compounds have two unrelated physical consequences:

| Channel | Physics | What it sees |
|---|---|---|
| **Capacitive** | polar molecules align in an E-field, raising permittivity | εr rises linearly with concentration |
| **Optical** | conjugated double bonds absorb blue, ignore NIR | transmittance falls exponentially |

Because the mechanisms are unrelated, **agreement validates a reading and
disagreement exposes a fault** — and the *direction* of disagreement names it:

- capacitive ahead of optical → **water** (εr 80 vs oil's 3, but colourless)
- optical ahead of capacitive → **particulate or window fouling** (blocks light,
  not dissolved, so permittivity barely moves)

That asymmetry is the invention. Everything else is implementation.

---

## 2. HARD-WON RULES — DO NOT UNDO THESE

### 2.1 Upload raw observables only. Never derived values.

The device sends capacitance, temperature and 18 raw channel values. It does
**not** send permittivity, Λ, the index or the verdict.

Every derived quantity depends on calibration constants, and those constants
change every time the probe geometry changes (see §4 — they have changed five
times). If conclusions were uploaded, correcting a constant would invalidate
every historical reading. Storing raw means the whole history re-derives.

### 2.2 The optical indicator must be normalised in LOG space.

```
n_opt = ln(OPT_FRESH / Λ) / ln(OPT_FRESH / OPT_DISCARD)
```

Permittivity is linear in concentration; transmittance is exponential
(Beer-Lambert). Normalising Λ linearly makes the two channels bow apart in
mid-life as a pure artefact of the arithmetic.

Measured: linear normalisation gave a worst-case channel separation of **0.130**
against a fault tolerance of 0.15 — consuming almost the entire margin. Log
normalisation dropped it to **0.066**.

### 2.3 A channel only votes if it is BOTH quiet and physically possible.

σ alone is not sufficient. One session produced σ of 0.005 pF — beautifully
quiet — with a derived permittivity of **9.1**. No edible oil exceeds ~4.5. The
error was systematic drift, which σ cannot see.

So the credibility test is:
- span to resolve > 5 × measured noise, **and**
- derived εr within 2.0–5.0

Factor of 5, not 2: two only distinguishes the endpoints, but the 20 % advisory
threshold sits between them and must be told apart from both.

### 2.4 Disagreement only counts when BOTH channels are credible.

If one channel is excluded there is nothing to disagree with. Forcing a fault
there hides the working channel — this was a real bug that pinned the dashboard
on HOLD permanently.

### 2.5 Subtract the dark reading before computing Λ.

The spectrum is read twice, bulb off then bulb on. Ambient leak has measured
**67–87 %** of some channels. Λ is a ratio, so a leak biases it silently.

### 2.6 No keep/discard verdict on screen.

The scale is not anchored to a lab-measured TPC reference (spec §5.6 stage 3 is
incomplete). A food-safety instruction would claim more than the measurement
supports. Show the index and which channels produced it.

---

## 3. HARDWARE

| Part | Interface | Address | ₹ |
|---|---|---|---|
| ESP32 DevKit | — | — | 385 |
| ProtoCentral FDC1004 | I2C | 0x50 | 1,495 |
| SparkFun AS7265x Triad | I2C | 0x49 | 6,660 |
| SSD1306 OLED 128×64 | I2C | 0x3C | 250 |
| DS18B20 / KY-001 | 1-Wire GPIO4 | — | 48 |
| SS 304 rods ×2 + thermowell rod | — | — | 120 |
| PETG housing, battery, switch | — | — | 304 |
| **Total** | | | **9,262** |

**Wiring:** SDA GPIO21, SCL GPIO22 for all three I2C devices. DS18B20 on GPIO4
with a **4.7 kΩ pull-up to 3V3 — not optional**, 1-Wire is open-drain and the
sensor can only pull low.

**Probe:** rod → CIN1 (sense), rod → GND (return), third rod is a thermowell for
the DS18B20 and **must be tied to GND** — floating metal in the field region
drifts.

**Note:** the ProtoCentral FDC1004 breakout has an onboard 3.3 V regulator, so
5 V on Vcc is safe and may be preferable to avoid LDO dropout. The bare IC is
3.6 V max; the breakout is not.

---

## 4. CALIBRATION — SESSION-SPECIFIC, SEE CALIBRATION.md

**The single most important fact in this project:** the probe geometry is not
repeatable between sessions, so calibration constants do not survive.

Air has permittivity 1.000 every time. Measured air readings across sessions:

```
6.394   6.625   6.816   7.440   7.514  pF     → spread 1.12 pF
```

A full fresh-to-discard signal at K=0.686 is **0.377 pF**. The baseline drift is
**3× larger than the entire signal**. At K=0.262 it is 8× larger.

Always re-fit air + fresh in the current setup before trusting anything.

---

## 5. KNOWN LIMITATION — STATE IT HONESTLY

The capacitive channel measures correctly but **does not resolve** at the current
electrode geometry.

| | |
|---|---|
| Measured cell constant | 0.262 pF/ε |
| Needed | ~10 |
| Parasitic share | 96 % |

This is not a wiring fault. Two M6 rods at 50 mm immersion top out near
**1.8 pF/ε** even nearly touching, because capacitance depends on the *logarithm*
of the spacing.

**Calculated fixes:**

| Geometry | K (pF/ε) |
|---|---|
| Two rods (current) | 0.26 |
| Plates 40×40 mm, 2 mm gap | 7.1 |
| **8 mm rod inside 10 mm tube, 50 mm** | **12.5** |
| Tube 10 mm inside 12 mm, 50 mm | 15.3 |

Concentric tube is preferred: hits the target *and* the outer tube shields the
inner electrode, fixing the parasitic problem simultaneously.

**Optical channel works.** Fresh → amber, dark-corrected survival:
410 nm 32 %, 435 nm 45 %, 460 nm 65 %, 940 nm 78 %. Smooth monotonic rise with
wavelength — that ordering is the proof it is real absorbance, not noise.

Its limitation is repeatability: **8.2 %** dip-to-dip at consistent depth,
**25 %** if the probe height varies. Against a fresh→amber signal of 21 %. Needs
a fixed optical path length (defined gap to a reflector).

**Both channels have the same root cause: unfixed geometry.**

---

## 6. FIREBASE

RTDB, plain HTTPS REST, no API key, no SDK. Rules must be open
(`{"rules":{".read":true,".write":true}}`) — lock them down after demos.

```
https://YOUR-PROJECT-default-rtdb.REGION.firebasedatabase.app
```

**Record schema** — `POST /devices/{id}/readings.json`:

```json
{
  "ts": 1786100000000,
  "ts_estimated": true,
  "temp_c": 30.0,
  "cap_pf": 7.766,
  "cap_sd_pf": 0.072,
  "cap_spread_pf": 0.126,
  "in_oil": true,
  "channels":      [18 floats, lit, 410→940 nm],
  "channels_dark": [18 floats, bulb off],
  "worst_raw": 800
}
```

Named bench slots go to `PUT /devices/{id}/bench/{air|s1_fresh|s2_slight|s3_amber}`.
Fitted constants to `PUT /devices/{id}/calibration`.

Institutional WiFi commonly blocks NTP (UDP 123). When it does, timestamps fall
back to a fixed base + `millis()` and the record is flagged `ts_estimated` —
capture order is preserved, which is what the dashboard needs.

Dashboard streams via **EventSource** (RTDB REST supports SSE natively), falls
back to polling, falls back to demo data.

---

## 7. BUGS ALREADY FOUND — DO NOT REINTRODUCE

| # | Bug | Fix |
|---|---|---|
| 1 | `AS7265x_led_white` | It is `AS7265x_LED_WHITE` — lowercase x, uppercase LED_. But `AS7265X_GAIN_16X` and `AS7265X_LED_CURRENT_LIMIT_12_5MA` use uppercase X. SparkFun is inconsistent. |
| 2 | capdac re-range at ±28000 | ProtoCentral reference uses **±16384** (0x4000). Wider bounds park the ADC at 75 % of full scale. |
| 3 | 400 Hz sample rate | 100 Hz is quieter; σ improved ~4× |
| 4 | `setIntegrationCycles(255)` | Channels return exactly 0.00 — library times out. Use ≤100. 16×/49 is the proven-good config. |
| 5 | Λ from raw lit values | Must dark-subtract; leak was 67–87 % |
| 6 | `SD_LIMIT_PF = 0.05` | Real noise is 0.068–0.072; genuine disturbances are 0.12–0.37. Use **0.15**. |
| 7 | `'Result' does not name a type` | **Arduino hoists auto-prototypes before the first function definition.** All structs must precede every function. Add explicit prototypes. |
| 8 | Linear optical normalisation | Use log — see §2.2 |
| 9 | σ-only credibility test | Add the permittivity plausibility bound — see §2.3 |
| 10 | AS7262 + AS7263 as a cheaper pair | **Both are I2C 0x49** — they collide. Also neither has 435 nor 940 nm. Not a viable substitution. |

---

## 8. VERIFYING FIRMWARE WITHOUT AN ESP32

`tools/` contains stub headers for all six libraries plus `arduino_sim.py`, which
reproduces Arduino's prototype hoisting.

```bash
cd tools
./check.sh ../firmware/OilSense_Field/OilSense_Field.ino
```

Compiles with `-Wall -Wextra -Wformat=2 -Wshadow`, both with and without
prototype hoisting. **Plain g++ alone will not catch bug #7** — the hoisting
simulation is what catches it. Verified: it reproduces the exact error on the
broken ordering and passes on the fixed one.

`Serial.printf` format strings are checked because the stub declares
`__attribute__((format(printf,2,3)))`.

---

## 9. WHAT REMAINS

1. **Concentric-tube electrode** — calculated at 12.5 pF/ε, fixes both the cell
   constant and the parasitic problem
2. **Fixed optical path length** — a defined gap to a reflector, to get
   repeatability below the 21 % signal
3. **Stage-3 anchoring** — samples of known TPC from a reference lab. Because
   raw data is stored, every historical reading becomes absolutely calibrated the
   moment this happens.
4. **Move the sensor window down the housing** — it currently sits ~99 mm above
   the rod tips, so a single dip cannot wet both channels

Contact made with **iNNovaHealth Foodlabs** (Greece, EU-funded). They run HPLC
and lipid-profile analysis — the reference method needed for item 3.

---

## 10. VERIFIED EXTERNAL FACTS

All checked against primary sources; safe to cite.

- **25 % TPC**, binding on all FBOs since **1 July 2018** (FSSAI)
- FSSAI's own site: handheld dielectric testers **₹40,000–50,000**
- Only FBOs using **>50 L/day** must monitor and use registered collectors
- FSSAI links polar compounds to *hypertension, atherosclerosis, Alzheimer's,
  liver disease*
- FSSAI advises reuse **a maximum of three times** (not four)
- Indian frying runs **170–190 °C** (FSSAI FoodSmart)
- Fry-life to 25 % TPC: soybean **132**, palm olein **138**, palm shortenings
  **143–146** fry-cycles — Wassell et al., *Int. J. Food Sci. Technol.* 56(8):3903,
  2021, DOI 10.1111/ijfs.15007. ~24.3 cycles/day, so **5–6 days**. Food was
  breaded chicken, not fries. Included 4 filtrations and 3 top-ups, and the load
  was halved after cycle 76 — these are best-case numbers.
- Maharashtra FDA, July 2026: **24 operations 14–16 July, ₹1.90 crore seized**;
  **five Mumbai clubs** suspended after a single-day drive on 27 July (CCI, RK
  Juhu Gymkhana, Aparna Juhu Gymkhana, MIG Cricket Club, Willingdon); a sixth
  given a stop-business order. **All violations were hygiene, not oil quality.**
- India discards **3 Mt** used cooking oil/year; **1.1 bn L** biodiesel potential;
  **105 M L** procured in 2019–20 (~10 %). Caveat: MoPNG says that biodiesel was
  *primarily imported palm stearin*, not UCO.
- Japan: world's longest life expectancy since 1985. Japanese-style diet
  adherence → CVD mortality RR **0.83**, stroke **0.80**. Literature notes the
  diet "does not use oil and fat". Do not claim oil alone explains it.
- Portable TPC devices carry **±2–5 %** field uncertainty (2026 Indian study);
  Testo 270 lab spec is ±0.5 %.

**Terminology:** FSSAI says *total polar compounds (TPC)*; instrument makers say
*total polar materials (TPM)*. Same measurement. Prefer TPC — it is the
regulatory term — and define it once.
