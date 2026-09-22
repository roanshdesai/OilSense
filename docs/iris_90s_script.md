# OilSense — 90-second presentation (IRIS National Fair)

Three parts: **40 s slides · 30 s live demo · 20 s slides.**

> Your brief said 40 + 30 + 30, which is 100 seconds. This is cut to 90 by
> compressing the closing to 20. If you really have 100, the closing is where
> the extra 10 seconds should go — ask and I'll restore it.

Word counts are per part, at 158 wpm. **Indented lines are stage directions** —
not spoken, not counted. Everything else is said out loud exactly as written.

---

## PART 1 — SLIDES · 0:00 → 0:40

**98 words ≈ 37 s of speech, plus ~2 s of slide advances.**

> Slide 1 up as you begin. Do not read it.

Every fried snack you've eaten was cooked in oil someone decided was good
enough. In India that decision has a legal number: **twenty-five percent total
polar compounds**. Past it, the oil is illegal to serve.

> Advance to slide 3.

The instrument that measures it costs **forty to fifty thousand rupees** —
FSSAI's own figure. I built one for **nine thousand two hundred and sixty-two**.

> Advance to slide 4.

But a cheap sensor that lies is worse than none. So I used two, through
unrelated physics. Polar compounds raise permittivity. They also absorb blue
light. Capacitance and optics — nothing in common.

> Advance to slide 5.

One measurement. Two ways to check it.

---

## PART 2 — LIVE DEMO · 0:40 → 1:10

**39 words ≈ 15 s of speech; the other 15 s is your hands moving.**

The silence *is* the demo. Do not fill it with talking.

> Step away from the screen. Pick up the probe. Both jars already open on the
> table, labelled, inside the judges' eyeline.

Fresh oil on the left. Oil from a week of frying on the right.

> Dip into the fresh jar. Hold still — do not stir. Three seconds.

Fresh. The screen shows the index.

> Lift. Wipe on the cloth. Dip into the used jar. Three seconds.

Same probe, degraded oil. The number climbs.

> Turn the device so the judges can read the screen. Hold it there.

And it is already uploaded — capacitance, temperature, all eighteen
wavelengths, stored raw.

---

## PART 3 — SLIDES · 1:10 → 1:30

**52 words ≈ 20 s of speech.**

> Put the probe down. Back to the screen, slide 10.

The optical channel resolves degradation today — twenty-one percent from fresh
to amber. The next electrode is designed: a concentric tube, **forty-eight
times** the cell constant.

> Advance to slide 11.

Then anchoring against lab-measured TPC. Because we store raw data, every past
reading becomes calibrated.

**That decision shouldn't be a guess. Now it doesn't have to be.**

---

# Delivery notes

**Pace.** 189 spoken words across 90 seconds is well under a rushed tempo —
because roughly 15 seconds of Part 2 is silent action. That silence is the
demo. Do not fill it with talking.

**Two lines carry the talk.** *"One measurement. Two ways to check it."* closes
Part 1, and it is on the screen behind you as you say it. *"That decision
shouldn't be a guess. Now it doesn't have to be."* ends the whole thing — it
answers the sentence you opened with, so slow down, drop your pitch, and stop
there. Do not add anything after it.

**Do not round the price.** "Nine thousand two hundred and sixty-two" sounds
like somebody who costed a build. "About nine thousand" sounds like a guess.

**The handoff into the demo is the risky moment.** Stop talking, step away,
pick up the probe. Let the silence sit for a second before your first demo
line. Rushing it makes the demo feel like an interruption rather than the
proof.

**End on forward motion.** Part 3 is two pieces of progress — one channel
resolving, the next electrode designed. Say the number, then stop. No "but
there's still work to do."

**If you run over**, cut "Past it, the oil is illegal to serve" from Part 1
(8 words, ≈3 s), then "Because we store raw data" from Part 3 (≈2 s). Never cut
the demo, and never cut the last line — buy the time from the slides.

---

# Demo rehearsal checklist

Run this the night before and again the morning of. A live demo that stalls
costs more than it gains.

| Check | Why |
|---|---|
| Both jars filled to the **same depth**, and the probe dipped to the same depth each time | Path length through the oil sets the optical reading. Varying it varies the number. |
| Probe **wipes clean** between jars — cloth pre-positioned, not hunted for | Carryover from the used jar biases the fresh reading, and hunting for a cloth eats your 30 seconds. |
| Re-run the bench calibration **in the room**, on the day | Air baseline shifts between sessions. Constants from last week give wrong numbers today. |
| Device **powered and warmed up** before you start, not switched on in front of judges | Boot, WiFi and first reading take time you do not have. |
| Know what you say **if the number does not move** | See below. |

**If the demo misbehaves**, say it plainly and move on: *"That's the
repeatability I'm fixing with the reflector gap — the trend across a full
frying run is in the data."* Then go to Part 3. Judges forgive a demo that
stumbles. They do not forgive pretending it didn't.

**Practical warning worth checking before you commit to a two-jar demo:** the
optical window currently sits well above the electrode tips, so one dip may not
wet both channels at once. If that is still true on the day, demo the **optical
channel only** and say so — it is the channel that resolves, and it is the
honest demo.

---

# Prep sheet — for you, not for the slides

Questions judges reliably ask. Answer at this level of confidence; none of this
belongs on screen.

| They ask | You say |
|---|---|
| "You said two channels — how many sensors is that?" | Three. Capacitance and optics are the two measurement channels; the third is a temperature sensor. Permittivity is temperature-dependent, so without it you cannot tell a hot reading from a degraded one. It compensates the capacitive channel rather than measuring degradation itself. |
| "Does it tell you to throw the oil out?" | Not yet, deliberately. The scale isn't anchored to lab-measured TPC, so a keep-or-discard instruction would claim more than the measurement supports. It shows the index and which channels produced it. Anchoring is the next milestone. |
| "Why a concentric tube?" | Capacitance scales with the *logarithm* of electrode spacing, so parallel rods have a hard ceiling — around 1.8 pF/ε even nearly touching. A concentric tube reaches 12.5, and the outer tube shields the inner electrode at the same time. One part, two problems. |
| "How do you know the optical signal is real?" | Dark-corrected survival rises monotonically with wavelength — 32% at 410 nm through 78% at 860 nm. That ordering is what absorbance does and what noise does not. And I verified those wavelengths against the sensor library's own channel map rather than assuming them. |
| "Why upload raw data instead of the result?" | Every derived number depends on calibration constants, and those change whenever the probe geometry does. Storing raw means correcting a constant re-derives the whole history instead of invalidating it. |
| "What makes it a product?" | Fixed geometry on both channels, then anchoring against lab-measured TPC. Because raw data is stored, every reading already taken becomes absolutely calibrated the moment that anchoring happens. |
| "How does it compare to the commercial unit?" | Portable TPC devices carry ±2–5% field uncertainty; a benchtop Testo 270 specs ±0.5%. The cross-check is what a single-channel unit at any price cannot do — it has no second measurement to confirm its own number against, and no way to tell you whether it is reading degradation, water, or residue. |

**If a judge asks directly about the capacitive channel's current performance:**
answer it straight — the cell is being rebuilt to the concentric geometry, the
calculation is done, and the optical channel carries the measurement meanwhile.
Do not volunteer it, do not dodge it if asked.

---

# Fallback: continuous 90 s, no demo

If the demo is cut for any reason, run Part 1 and Part 3 back to back and
insert this between them (18 words, ~7 s), which restores the water-and-residue
material the demo would otherwise have carried:

> Because the two channels differ, each catches what the other cannot. Water
> raises permittivity but is colourless. Burnt residue blocks light but barely
> shifts permittivity.
