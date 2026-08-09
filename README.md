# OilSense

Low-cost dual-modality frying oil degradation meter. Two physically unrelated
sensors measure the same chemistry so they can cross-check each other.

**New to this repo? Read `CLAUDE.md` first.** It contains the design decisions
and the bugs already fixed, so you do not repeat either.

---

## Layout

```
CLAUDE.md              project context - read first
CALIBRATION.md         session-by-session fits, append only
firmware/
  OilSense_Bench/      laptop-driven, finds calibration constants
  OilSense_Field/      standalone: switch on, dip, read
dashboard/index.html   live view, open in a browser, no build step
tools/                 stub headers + verification script
docs/                  technical disclosure, poster, spec
data/                  bench run captures
```

## Verifying firmware without hardware

```bash
./tools/check.sh firmware/OilSense_Field/OilSense_Field.ino
```

Two passes. The second reproduces Arduino's prototype hoisting, which is the
only way to catch `'Result' does not name a type` — plain g++ misses it.

## Arduino libraries

Protocentral FDC1004 · SparkFun AS7265X · OneWire · DallasTemperature ·
Adafruit SSD1306 · Adafruit GFX

## Before flashing

1. Fill in `WIFI_SSID` and `WIFI_PASSWORD`
2. Open the database rules: `{"rules":{".read":true,".write":true}}` — and close
   them again afterwards
3. **Check the constants in the calibration block against `CALIBRATION.md`.**
   They are session-specific. Constants from a different probe setup give wrong
   answers.

## Field sketch

Switch on, dip, read. No commands. Measures every 8 s, shows the index on the
OLED, uploads raw values.

## Bench sketch

Serial at 115200.

```
a       air reference
1 2 3   fresh / slightly degraded / amber
4       re-measure fresh (carry-over control)
r       report + fitted constants
k v     anchor the scale, then the verdict table
t       test the Firebase link
o       set the thermowell temperature offset
u c w   upload all / upload constants / wifi status
```

Order: `t` first, then `o`, then `a 1 2 3`, then `r`, then `k v`.

## Dashboard

Open `dashboard/index.html` in a browser. Reads over plain HTTPS, no API key.
The calibration panel recomputes the entire history when you change a constant —
that is the point of storing raw data.

---

## Status

Optical channel resolves degradation reliably. Capacitive channel measures
correctly but does not resolve at the current electrode geometry (cell constant
0.262 pF/ε against ~10 needed, 96 % parasitic). Fix is calculated: concentric
tube at 12.5. See `CLAUDE.md` §5.

The firmware detects a channel below resolution and excludes it rather than
averaging in a number it cannot stand behind.
