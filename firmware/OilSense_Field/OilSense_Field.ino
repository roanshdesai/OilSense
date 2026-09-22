/* ============================================================================
   OilSense - FIELD firmware
   Switch on, dip the probe, read the screen. No laptop, no typing.

   This is a SEPARATE sketch from the bench one. The bench sketch exists to
   find the calibration constants with a laptop attached. This one bakes those
   constants in and just runs.

   >>> BEFORE FLASHING: paste your latest bench numbers into CALIBRATION below.
   >>> Run the bench sketch, press 'a' then '1' then 'r', and copy what it
   >>> prints. Constants from a different session will give wrong answers.

   Loop: measure -> verdict on the OLED -> upload raw to Firebase -> wait.

   LIBRARIES: Protocentral FDC1004, SparkFun AS7265X, OneWire,
              DallasTemperature, Adafruit SSD1306, Adafruit GFX
   ========================================================================== */

#include <Wire.h>
#include <Protocentral_FDC1004.h>
#include <SparkFun_AS7265X.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <math.h>
#include <time.h>

// ---------------------------------------------------------------- wiring
#define PIN_SDA        21
#define PIN_SCL        22
#define PIN_ONEWIRE    4          // KY-001 data, needs 4.7k to 3V3
#define OLED_ADDR      0x3C
#define FDC_CHANNEL    0          // CIN1
#define FDC_MEAS       0
#define N_SAMPLES      64         // determinations per reading
#define FDC_RATE       FDC1004_100HZ

// ---------------------------------------------------------------- network
#define WIFI_SSID      "YOUR_WIFI"
#define WIFI_PASSWORD  "YOUR_PASSWORD"
#define DB_HOST        "https://YOUR-PROJECT-default-rtdb.REGION.firebasedatabase.app"
#define DEVICE_ID      "probe-01"

// ---------------------------------------------------------------- behaviour
#define MEASURE_EVERY_MS  8000UL  // how often to take a reading
#define TH_ADVISORY       0.70f   // 20% TPM equivalent
#define TH_DISCARD        1.00f   // 25% TPM equivalent

/* ====================== CALIBRATION - EDIT THIS ==========================
   From the bench run of 7 Aug. Replace after any change to the probe.

   CAP_TRUSTED decides whether the capacitive channel gets a vote. Right now
   it is 0, because the measured cell constant is 0.262 pF/eps against a target
   near 10, which puts 96% of the reading in parasitic capacitance - the
   channel cannot resolve one oil from another. The verdict therefore comes
   from the optical channel alone and the screen says so.

   Set it to 1 only when a bench run shows the air-to-oil gap comfortably
   larger than the run-to-run scatter. Lying to this flag produces confident
   nonsense, which is worse than one honest channel.
   ======================================================================== */
#define CAP_TRUSTED    0

const float CELL_PF_PER_EPS = 0.262f;   // K
const float PARASITIC_PF    = 6.364f;   // P
const float TEMP_COEFF      = -0.0015f; // d(eps)/dT
const float REF_TEMP_C      = 30.0f;
const float T_OFFSET        = 0.0f;     // thermowell correction, from 'o'

const float EPS_FRESH       = 3.0000f;
const float EPS_DISCARD     = 3.5500f;  // assumed, needs a known-TPC sample
const float OPT_FRESH       = 1.052f;   // mean of consistent fresh dips, 7 Aug 21:41
const float OPT_DISCARD     = 0.829f;   // x0.788, the fresh-to-amber drop measured

const float TOLERANCE       = 0.15f;    // channel agreement window
/* Set from YOUR measured noise, not a guess. Normal readings on this probe
   run sd 0.02-0.08 pF; real disturbances (a knock, a bubble, a loose lead)
   showed up as 0.12-0.37. 0.15 sits between the two. If good readings start
   getting rejected as unstable, raise it - do not lower it to hide noise. */
const float SD_LIMIT_PF     = 0.15f;

/* Below this the probe is reading air, not oil: the midpoint between air
   (eps = 1) and fresh oil (eps = EPS_FRESH), in permittivity terms.

   DERIVED, not hardcoded, and that is the whole point. The old literal 6.89
   was correct for the 7 Aug session (air = P + K = 6.626 pF) but nothing tied
   it to P and K, so it went stale the moment the probe was re-mounted. Air has
   measured 6.394-7.514 pF across sessions (CALIBRATION.md); in the two sessions
   that read 7.440 and 7.514, a fixed 6.89 would have called AIR "in oil" and
   happily uploaded it. Writing it in terms of P and K means re-fitting the
   calibration re-fits this too. */
const float IN_OIL_MIN_PF   = PARASITIC_PF
                            + CELL_PF_PER_EPS * (1.0f + 0.5f * (EPS_FRESH - 1.0f));

/* Sigma only sees random noise. A stale cell constant gives a rock-steady
   reading that is still nonsense - one session showed sigma of 0.005 pF with a
   derived permittivity of 9.1. No edible oil is above about 4.5, so a value
   outside this range means the calibration is wrong however quiet it looks. */
const float EPS_MIN         = 2.0f;
const float EPS_MAX         = 5.0f;

// ---------------------------------------------------------------- objects
FDC1004           fdc;
AS7265X           spec;
OneWire           oneWire(PIN_ONEWIRE);
DallasTemperature ds(&oneWire);
Adafruit_SSD1306  oled(128, 64, &Wire, -1);

bool haveOled = false, haveSpec = false, haveTemp = false;
bool netUp = false, clockOK = false;
uint8_t capdac = 2;

#define FALLBACK_EPOCH_MS 1786000000000ULL

const uint16_t NM[18] = {410,435,460,485,510,535,560,585,610,
                         645,680,705,730,760,810,860,900,940};

/* SparkFun's channel LETTERS ARE NOT IN WAVELENGTH ORDER. Verified against the
   library's own Example1_BasicReadings, whose header line reads:

       "A,B,C,D,E,F,G,H,R,I,S,J,T,U,V,W,K,L"

   i.e.  A410 B435 C460 D485 E510 F535 G560 H585 R610
         I645 S680 J705 T730 U760 V810 W860 K900 L940

   Reading them in the natural order A..L then R..W and calling that 410->940
   mislabels every channel from 610 nm upward. SRC[i] is the position, inside a
   letter-ordered array, of the i-th ASCENDING wavelength in NM[] above. */
const uint8_t SRC[18] = { 0, 1, 2, 3, 4, 5, 6, 7,   // A B C D E F G H
                         12,                        // R -> 610
                          8,                        // I -> 645
                         13,                        // S -> 680
                          9,                        // J -> 705
                         14,15,16,17,               // T U V W -> 730..860
                         10,                        // K -> 900
                         11 };                      // L -> 940

const uint8_t CH_SHORT = 1;    // 435 nm

/* 860 nm, NOT 940. The old code used letter-array index 17, which is W = 860 nm
   while claiming it was 940 nm. OPT_FRESH and OPT_DISCARD below were fitted
   against that channel, so CH_NIR stays on 860 nm and the existing constants
   remain valid. To move to a true 940 nm reference set this to 17 and RE-FIT
   OPT_FRESH / OPT_DISCARD - they are not transferable between channels. */
const uint8_t CH_NIR   = 15;   // 860 nm

struct Reading {
  float tempC = NAN, capPf = 0, capSd = 0, capSpread = 0;
  float lit[18], dark[18];
  float lambda = 0;
  uint16_t worstRaw = 0;
  bool inOil = false;
};

struct Result {
  float eps = NAN, nCap = NAN, nOpt = NAN, idx = NAN, tpm = NAN, delta = NAN;
  const char *line1 = "--";
  const char *line2 = 0;
  const char *note  = "";
};

#define HIST 5
float lamHist[HIST]; uint8_t lamN = 0, lamPos = 0;

// median is used, not mean: one bad dip cannot drag it.
float lamMedian() {
  if (!lamN) return 0;
  float t[HIST];
  for (uint8_t i = 0; i < lamN; i++) t[i] = lamHist[i];
  for (uint8_t i = 1; i < lamN; i++) {
    float k = t[i]; int8_t j = i - 1;
    while (j >= 0 && t[j] > k) { t[j+1] = t[j]; j--; }
    t[j+1] = k;
  }
  return t[lamN / 2];
}


// ---- explicit prototypes: the IDE auto-generates these otherwise, and it
// ---- inserts them above the struct definitions, which will not compile.
bool  fdcOnce(float &pF, bool allowRange = true);
void  autoRange();
void  readCap(float &mean, float &sd, float &spread);
void  readSpectrum(Reading &r);
float readTemp();
float lamMedian();
Result judge(const Reading &r);
void  screen(const Reading &r, const Result &o);
void  banner(const char *l1, const char *l2);
void  upload(const Reading &r);

// ------------------------------------------------------------- capacitance
bool fdcOnce(float &pF, bool allowRange) {
  uint16_t v[2];
  fdc.configureMeasurementSingle(FDC_MEAS, FDC_CHANNEL, capdac);
  fdc.triggerSingleMeasurement(FDC_MEAS, FDC_RATE);
  delay(12);
  if (fdc.readMeasurement(FDC_MEAS, v)) return false;
  int16_t msb = (int16_t) v[0];
  pF = (((float) msb) * 0.457f + ((float) capdac) * 3028.0f) / 1000.0f;
  /* Only allowed to re-range OUTSIDE an acquisition. A capdac step is worth
     3.028 pF at the input; if it lands in the middle of the 64-sample burst,
     every sample after it sits on a different pedestal and the burst's sd and
     spread report that step rather than the noise. That fires the NOISY veto
     on a probe that was perfectly still. */
  if (allowRange) {
    if (msb > 16384 && capdac < 31) capdac++;
    else if (msb < -16384 && capdac > 0) capdac--;
  }
  return true;
}

void autoRange() {
  for (uint8_t i = 0; i < 12; i++) {
    float v; uint8_t before = capdac;
    if (!fdcOnce(v)) break;
    if (capdac == before) break;
    delay(4);
  }
}

void readCap(float &mean, float &sd, float &spread) {
  autoRange();
  float s[N_SAMPLES]; uint8_t n = 0;
  for (uint8_t i = 0; i < N_SAMPLES; i++) {
    float v; if (fdcOnce(v, false)) s[n++] = v; delay(2);   // capdac frozen
  }
  mean = sd = spread = 0;
  if (!n) return;
  float sum = 0, mn = s[0], mx = s[0];
  for (uint8_t i = 0; i < n; i++) { sum += s[i]; if (s[i] < mn) mn = s[i]; if (s[i] > mx) mx = s[i]; }
  mean = sum / n;
  spread = mx - mn;
  float sq = 0;
  for (uint8_t i = 0; i < n; i++) { float d = s[i] - mean; sq += d * d; }
  sd = (n > 1) ? sqrtf(sq / (n - 1)) : 0;
}

// ---------------------------------------------------------------- spectrum
void readSpectrum(Reading &r) {
  for (uint8_t i = 0; i < 18; i++) { r.lit[i] = 0; r.dark[i] = 0; }
  r.lambda = 0; r.worstRaw = 0;
  if (!haveSpec) return;

  spec.takeMeasurements();                     // bulb OFF - ambient
  const float d[18] = {
    spec.getCalibratedA(), spec.getCalibratedB(), spec.getCalibratedC(),
    spec.getCalibratedD(), spec.getCalibratedE(), spec.getCalibratedF(),
    spec.getCalibratedG(), spec.getCalibratedH(), spec.getCalibratedI(),
    spec.getCalibratedJ(), spec.getCalibratedK(), spec.getCalibratedL(),
    spec.getCalibratedR(), spec.getCalibratedS(), spec.getCalibratedT(),
    spec.getCalibratedU(), spec.getCalibratedV(), spec.getCalibratedW()};
  for (uint8_t i = 0; i < 18; i++) r.dark[i] = d[SRC[i]];   // letter -> ascending nm

  spec.takeMeasurementsWithBulb();             // bulb ON
  const float l[18] = {
    spec.getCalibratedA(), spec.getCalibratedB(), spec.getCalibratedC(),
    spec.getCalibratedD(), spec.getCalibratedE(), spec.getCalibratedF(),
    spec.getCalibratedG(), spec.getCalibratedH(), spec.getCalibratedI(),
    spec.getCalibratedJ(), spec.getCalibratedK(), spec.getCalibratedL(),
    spec.getCalibratedR(), spec.getCalibratedS(), spec.getCalibratedT(),
    spec.getCalibratedU(), spec.getCalibratedV(), spec.getCalibratedW()};
  for (uint8_t i = 0; i < 18; i++) r.lit[i] = l[SRC[i]];    // letter -> ascending nm

  const uint16_t raw[18] = {
    spec.getA(), spec.getB(), spec.getC(), spec.getD(), spec.getE(), spec.getF(),
    spec.getG(), spec.getH(), spec.getI(), spec.getJ(), spec.getK(), spec.getL(),
    spec.getR(), spec.getS(), spec.getT(), spec.getU(), spec.getV(), spec.getW()};
  for (uint8_t i = 0; i < 18; i++) if (raw[i] > r.worstRaw) r.worstRaw = raw[i];

  // Ambient leak can be most of a channel, so subtract dark before the ratio.
  float sc = r.lit[CH_SHORT] - r.dark[CH_SHORT];
  float nc = r.lit[CH_NIR]   - r.dark[CH_NIR];
  if (nc > 0) r.lambda = sc / nc;
}

// -------------------------------------------------------------- temperature
float readTemp() {
  if (!haveTemp) return NAN;
  ds.requestTemperatures();
  float t = ds.getTempCByIndex(0);
  /* 125 C is the DS18B20's own maximum, so the old >150 bound was unreachable
     and let out-of-range readings through. NOTE THE HARDWARE LIMIT: Indian
     frying runs 170-190 C, above what this sensor can survive or report, so
     temp_c is trustworthy on the bench and NOT in a live fryer. A K-type
     thermocouple is needed before any hot-oil measurement means anything. */
  if (t < -50 || t > 125 || t == 85.0f) return NAN;   // -127 absent, 85 default
  return t + T_OFFSET;
}

// ------------------------------------------------------------------- verdict
static float clamp135(float v) { return v < 0 ? 0 : (v > 1.35f ? 1.35f : v); }

Result judge(const Reading &r) {
  Result o;

  if (!r.inOil) { o.line1 = "DIP"; o.line2 = "PROBE"; o.note = "not in oil"; return o; }

  float tc = isnan(r.tempC) ? REF_TEMP_C : r.tempC;
  o.eps = (r.capPf - PARASITIC_PF) / CELL_PF_PER_EPS - TEMP_COEFF * (tc - REF_TEMP_C);

#if CAP_TRUSTED
  // only if the derived permittivity is physically possible for an edible oil
  if (o.eps > EPS_MIN && o.eps < EPS_MAX)
    o.nCap = clamp135((o.eps - EPS_FRESH) / (EPS_DISCARD - EPS_FRESH));
#endif

  // Logarithmic, because absorbance is what rises linearly with concentration.
  // Uses the median of the last HIST dips - a single odd dip should not decide.
  float lamUse = lamMedian();
  if (lamUse > 0 && OPT_FRESH > 0 && OPT_DISCARD > 0) {
    float span = logf(OPT_FRESH / OPT_DISCARD);
    if (fabsf(span) > 1e-6f) o.nOpt = clamp135(logf(OPT_FRESH / lamUse) / span);
  }

  const bool haveC = !isnan(o.nCap), haveO = !isnan(o.nOpt);

  // Dispersion is a hardware fault, not an oil condition - it overrides.
  if (r.capSd > SD_LIMIT_PF) { o.line1 = "NOISY"; o.note = "let it settle"; return o; }

  if (haveC && haveO) {
    o.delta = fabsf(o.nCap - o.nOpt);
    if (o.delta > TOLERANCE) {
      // BOTH channels are credible and they disagree - that is the one case
      // where HOLD is the right answer, and the direction names the fault.
      o.line1 = "CHECK";
      o.note  = (o.nCap > o.nOpt) ? "water?" : "solids?";
      return o;
    }
    o.idx  = 0.5f * (o.nCap + o.nOpt);
    o.note = "both channels agree";
  } else if (haveO) {
    o.idx  = o.nOpt;               // one credible channel decides alone
    o.note = "optical only";
  } else if (haveC) {
    o.idx  = o.nCap;
    o.note = "capacitive only";
  } else {
    o.line1 = "NO"; o.line2 = "READING"; o.note = "no valid channel";
    return o;
  }

  o.tpm = 8.0f + 17.0f * o.idx;

  /* No keep/discard call. The scale is not anchored against a known TPC
     reference, so a food-safety instruction would claim more than the
     measurement supports. Report the number and where it came from. */
  if (o.idx >= 1.34f) { o.line1 = "OFF"; o.line2 = "SCALE"; o.note = "reference is stale"; }
  else                { o.line1 = 0; }        // 0 means: show the index instead
  return o;
}

// --------------------------------------------------------------------- OLED
void screen(const Reading &r, const Result &o) {
  if (!haveOled) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  oled.setTextSize(1);
  oled.setCursor(0, 0); oled.print(F("OilSense"));
  oled.setCursor(84, 0);
  if (isnan(r.tempC)) oled.print(F("-- C"));
  else { oled.print(r.tempC, 0); oled.print(F(" C")); }
  oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  if (o.line1) {
    // a named state: not in oil, too noisy, off scale, channels disagree
    oled.setTextSize(2);
    if (o.line2) { oled.setCursor(0, 16); oled.print(o.line1);
                   oled.setCursor(0, 34); oled.print(o.line2); }
    else         { oled.setCursor(0, 24); oled.print(o.line1); }
    oled.setTextSize(1);
    oled.setCursor(0, 55); oled.print(o.note);
  } else {
    // a measurement: the number is the headline, no verdict word
    oled.setTextSize(2);
    oled.setCursor(0, 16); oled.print(o.idx, 2);
    oled.setTextSize(1);
    oled.setCursor(62, 17); oled.print(F("~"));
    oled.print(o.tpm, 1); oled.print(F("%"));
    oled.setCursor(62, 27); oled.print(F("TPM-eq"));

    oled.setCursor(0, 40); oled.print(F("L"));
    oled.print(lamMedian(), 3);
    oled.setCursor(66, 40); oled.print(F("e"));
    if (isnan(o.nCap)) oled.print(F("n/r")); else oled.print(o.eps, 2);

    oled.setCursor(0, 55); oled.print(o.note);
    oled.setCursor(104, 55); oled.print(F("n=")); oled.print(lamN);
  }
  oled.display();
}

void banner(const char *l1, const char *l2) {
  if (!haveOled) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(2); oled.setCursor(0, 12); oled.print(l1);
  oled.setTextSize(1); oled.setCursor(0, 42); oled.print(l2);
  oled.display();
}

// ----------------------------------------------------------------- upload
static String nowMs() {
  if (!clockOK && time(nullptr) > 100000) clockOK = true;
  unsigned long long ms = clockOK ? (unsigned long long)time(nullptr) * 1000ULL
                                  : FALLBACK_EPOCH_MS + (unsigned long long)millis();
  char b[24]; snprintf(b, sizeof(b), "%llu", ms);
  return String(b);
}

static String arr(const float *v) {
  String s = "[";
  for (uint8_t i = 0; i < 18; i++) { if (i) s += ','; s += String(v[i], 3); }
  s += ']';
  return s;
}

// Raw observables only. No index, no verdict - those depend on constants that
// will change, and raw means the history re-derives instead of going stale.
void upload(const Reading &r) {
  if (!netUp) return;
  String j = "{\"ts\":" + nowMs();
  if (!clockOK) j += ",\"ts_estimated\":true";
  j += ",\"temp_c\":"    + String(isnan(r.tempC) ? REF_TEMP_C : r.tempC, 1);
  j += ",\"cap_pf\":"    + String(r.capPf, 4);
  j += ",\"cap_sd_pf\":" + String(r.capSd, 4);
  j += ",\"cap_spread_pf\":" + String(r.capSpread, 4);
  j += ",\"in_oil\":"    + String(r.inOil ? "true" : "false");
  /* Records written before this fix stored the 18 channels in SparkFun's letter
     order while the schema claimed 410->940, so everything from index 8 up was
     mislabelled. New records are genuinely ascending. The marker is what lets
     the dashboard tell the two apart - without it the archive is unusable, and
     re-derivable raw data is the entire point of rule 2.1. Absent field on an
     old record means letter order. */
  j += ",\"ch_order\":\"asc_410_940\"";
  j += ",\"channels\":"      + arr(r.lit);
  j += ",\"channels_dark\":" + arr(r.dark);
  j += ",\"worst_raw\":" + String(r.worstRaw);
  j += "}";

  WiFiClientSecure client; client.setInsecure(); client.setTimeout(8000);
  HTTPClient http;
  http.setConnectTimeout(8000); http.setTimeout(8000);
  String url = String(DB_HOST) + "/devices/" + DEVICE_ID + "/readings.json";
  if (!http.begin(client, url)) return;
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(j);
  http.end();
  Serial.printf("upload %s (HTTP %d)\n", code == 200 ? "ok" : "FAILED", code);
}

// ------------------------------------------------------------------- setup
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("\n=== OilSense FIELD ==="));

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000);

  haveOled = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  banner("OilSense", "starting...");

  ds.begin(); ds.setResolution(12);
  haveTemp = (ds.getDeviceCount() > 0);

  haveSpec = spec.begin();
  if (haveSpec) {
    spec.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_WHITE);
    spec.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_IR);
    spec.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_UV);
    spec.setGain(AS7265X_GAIN_16X);
    spec.setIntegrationCycles(49);
    spec.disableIndicator();
  }
  Serial.printf("oled %d  temp %d  spec %d\n", haveOled, haveTemp, haveSpec);

  // WiFi last, and never blocking - a dead network must not stop measuring.
  banner("connecting", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (uint8_t i = 0; i < 24 && WiFi.status() != WL_CONNECTED; i++) delay(400);
  netUp = (WiFi.status() == WL_CONNECTED);
  if (netUp) {
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    for (uint8_t i = 0; i < 10 && time(nullptr) < 100000; i++) delay(300);
    clockOK = (time(nullptr) > 100000);
    banner("wifi ok", WiFi.localIP().toString().c_str());
  } else {
    banner("no wifi", "measuring anyway");
  }
  delay(1200);
  banner("ready", "dip the probe");
}

// -------------------------------------------------------------------- loop
void loop() {
  Reading r;
  r.tempC = readTemp();
  readCap(r.capPf, r.capSd, r.capSpread);
  r.inOil = (r.capPf > IN_OIL_MIN_PF);
  if (r.inOil) readSpectrum(r);        // skip the bulb when there is no sample

  if (r.inOil && r.lambda > 0) {          // feed the median only with real dips
    lamHist[lamPos] = r.lambda;
    lamPos = (lamPos + 1) % HIST;
    if (lamN < HIST) lamN++;
  }
  if (!r.inOil) { lamN = 0; lamPos = 0; }  // out of the oil - start the run over

  Result o = judge(r);
  screen(r, o);

  Serial.printf("C %.3f pF  sd %.4f  T %.1f  L %.4f (med %.4f n=%u)  eps %.2f  idx %.2f  [%s] %s%s\n",
                r.capPf, r.capSd, isnan(r.tempC) ? 0.0f : r.tempC, r.lambda,
                lamMedian(), lamN, o.eps, isnan(o.idx) ? 0.0f : o.idx, o.note,
                o.line1 ? o.line1 : "", o.line2 ? o.line2 : "");

  if (r.inOil) upload(r);              // nothing worth storing from air

  delay(MEASURE_EVERY_MS);
}
