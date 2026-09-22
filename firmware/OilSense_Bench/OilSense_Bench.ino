 /* ============================================================================
   OilSense - THREE-SAMPLE VALIDATION TEST
   FDC1004 + AS7265x only. No DS18B20, no OLED, no WiFi. Serial 115200.

   PURPOSE - this is not just a calibration run. It tests the central premise
   of the whole project: that the capacitive and optical channels are measuring
   the same underlying chemistry.

   Permittivity should rise LINEARLY with polar compound concentration.
   Absorbance, ln(L_fresh/L), should ALSO rise linearly with it.
   Therefore the ratio  ln(L_fresh/L) / (eps - eps_fresh)  should come out
   ROUGHLY CONSTANT across your three samples. If it does, the two channels
   are proportional and cross-validation is sound. If it doesn't, either
   something is wrong with the measurement or the physics differs from theory.
   Either result is worth knowing.

   ALL THREE SAMPLES MUST BE AT THE SAME TEMPERATURE. There is no temperature
   sensor in this test, so the only way the comparison is valid is if the
   samples have equilibrated in the same room. Log the temperature by hand.

   COMMANDS
     a   AIR reference      (probe clean, dry, in free air)
     1   sample 1  FRESH
     2   sample 2  SLIGHTLY DEGRADED
     3   sample 3  AMBER
     4   re-measure FRESH   (carry-over / drift control - do this last)
     r   full comparison report and premise test
     s   spectrum only, dark + lit, leak and saturation check
     g   change gain / integration  (set once, BEFORE sample 1)
     e   set assumed permittivity of fresh oil (default 3.00)
     h   help
   ========================================================================== */

#include <Wire.h>
#include <Protocentral_FDC1004.h>
#include <SparkFun_AS7265X.h>
#include <math.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/* KY-001 is a DS18B20 on a breakout. Most boards already carry the 4.7k
   pull-up - measure S to VCC, about 4.7k means it is fitted. If it reads
   open, add 4.7k from PIN_ONEWIRE to 3V3 or you will only ever get -127.

   Mounted on a thermowell rod, the sensor sits at the dry end and loses heat
   to air along the rod, so it reads BETWEEN oil and room temperature and lags.
   Compare against a real thermometer once and set T_OFFSET with 'o'. */
#define PIN_ONEWIRE 4

// SSD1306 shares the I2C bus at 0x3C. That is now THREE boards each carrying
// their own pull-ups - if the bus starts dropping devices, cut the pull-up
// jumpers on two of the three and keep one.
#define OLED_ADDR 0x3C

/* ============================ FIREBASE CONFIG ============================
   No API key. No Firebase library. Plain HTTPS straight at the database REST
   endpoint. Only two things to fill in: the WiFi lines below.

   YOU MUST OPEN THE DATABASE RULES FIRST.
   Firebase console -> Build -> Realtime Database -> Rules, paste this, Publish:

       { "rules": { ".read": true, ".write": true } }

   Be aware of what that means: anyone who knows the URL can read and write
   your data. That is fine for a demo. LOCK IT DOWN AFTERWARDS by setting both
   back to false, or the database stays wide open.
   ======================================================================== */
// WIFI_SSID, WIFI_PASSWORD and DB_HOST live in secrets.h, which is gitignored.
// Copy secrets.h.example to secrets.h and fill it in before flashing.
#include "secrets.h"
#define DEVICE_ID      "probe-01"
#define AUTO_UPLOAD    1        // push each sample as it is captured

#define PIN_SDA      21
#define PIN_SCL      22
#define FDC_CHANNEL  0
#define FDC_MEAS     0
#define N_SAMPLES    64
#define FDC_RATE     FDC1004_100HZ
#define N_RUNS       8              // repeats averaged per sample

float EPS_FRESH_ASSUMED = 3.00f;

FDC1004 fdc;
AS7265X spec;

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
bool haveOled = false;

OneWire           oneWire(PIN_ONEWIRE);
DallasTemperature ds(&oneWire);
bool  haveTemp = false;
float T_OFFSET = 0.0f;      // thermowell correction, set with 'o'

bool netUp   = false;    // WiFi joined
bool clockOK = false;    // real epoch time available

// Institutional networks often block NTP (UDP 123). If that happens we still
// need records to sort in capture order, so fall back to a fixed base plus
// millis() and flag the record as having an estimated timestamp.
#define FALLBACK_EPOCH_MS 1786000000000ULL
uint8_t capdac = 2;
int16_t lastMsb = 0;
bool    haveSpec = false;

const uint16_t NM[18] = {410,435,460,485,510,535,560,585,610,
                         645,680,705,730,760,810,860,900,940};

/* SparkFun's channel LETTERS ARE NOT IN WAVELENGTH ORDER. Verified against the
   library's own Example1_BasicReadings header line:

       "A,B,C,D,E,F,G,H,R,I,S,J,T,U,V,W,K,L"

   i.e.  A410 B435 C460 D485 E510 F535 G560 H585 R610
         I645 S680 J705 T730 U760 V810 W860 K900 L940

   Reading A..L then R..W and printing it against NM[] above mislabelled every
   channel from 610 nm up - including the printSpectrum() table that produced
   the survival figures in CLAUDE.md. SRC[i] is the position, inside a
   letter-ordered array, of the i-th ASCENDING wavelength. */
const uint8_t SRC[18] = { 0, 1, 2, 3, 4, 5, 6, 7,   // A B C D E F G H
                         12,                        // R -> 610
                          8,                        // I -> 645
                         13,                        // S -> 680
                          9,                        // J -> 705
                         14,15,16,17,               // T U V W -> 730..860
                         10,                        // K -> 900
                         11 };                      // L -> 940

const uint8_t CH_SHORT = 1;    // 435 nm

/* 860 nm, NOT 940. Index 17 of the letter-ordered array is W = 860 nm, so every
   lambda fitted so far has been 435/860 under a 940 label. Staying on 860 keeps
   the existing OPT_FRESH / OPT_DISCARD valid. Set to 17 for a true 940 nm
   reference and re-fit - the constants do not transfer between channels. */
const uint8_t CH_NIR   = 15;   // 860 nm

struct Sample {
  bool  have = false;
  char  name[26];
  float cMean = 0, cSd = 0, cSpread = 0;
  float lit[18], dark[18];
  float lambda = 0;        // raw 435/860, as originally tested (was mislabelled 940)
  float lambdaDark = 0;    // (435-dark)/(860-dark) - the more correct one
  float tempC = 0;
  uint16_t worstRaw = 0;
};
Sample air, s1, s2, s3, s4;

// forward declarations - capture() uploads, but the upload code lives further
// down. Arduino's auto-prototype generation is unreliable with reference args.
void netStart();
void setAnchor();
float readTemp();
void oledMsg(const char *l1, const char *l2);
void oledProgress(const char *label, uint8_t run, uint8_t total, float pF);
void oledSample(Sample &sm, bool optical);
void oledVerdict();
void oledBig();
void oledAuto(Sample &sm, bool optical);
void testUpload();
void verdictTable();
bool uploadSample(Sample &sm, bool optical);
void uploadAll();
void uploadFit();
bool fdcOnce(float &pF, bool allowRange = true);

// ------------------------------------------------------------- capacitance
bool fdcOnce(float &pF, bool allowRange) {
  uint16_t v[2];
  fdc.configureMeasurementSingle(FDC_MEAS, FDC_CHANNEL, capdac);
  fdc.triggerSingleMeasurement(FDC_MEAS, FDC_RATE);
  delay(12);
  if (fdc.readMeasurement(FDC_MEAS, v)) return false;
  int16_t msb = (int16_t) v[0];
  pF = (((float) msb) * 0.457f + ((float) capdac) * 3028.0f) / 1000.0f;
  /* Only re-range OUTSIDE an acquisition - a capdac step is 3.028 pF at the
     input, and one landing mid-burst makes sd/spread report the step instead
     of the noise. */
  if (allowRange) {
    if (msb > 16384 && capdac < 31) capdac++;
    else if (msb < -16384 && capdac > 0) capdac--;
  }
  lastMsb = msb;
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

void capStat(float &mean, float &sd, float &spread) {
  autoRange();
  float s[N_SAMPLES]; uint8_t n = 0;
  for (uint8_t i = 0; i < N_SAMPLES; i++) {
    float v; if (fdcOnce(v, false)) s[n++] = v; delay(2);   // capdac frozen
  }
  mean = sd = spread = 0;
  if (!n) return;
  float sum = 0, mn = s[0], mx = s[0];
  for (uint8_t i = 0; i < n; i++) { sum += s[i]; if (s[i]<mn) mn=s[i]; if (s[i]>mx) mx=s[i]; }
  mean = sum / n;
  float sq = 0;
  for (uint8_t i = 0; i < n; i++) { float d = s[i]-mean; sq += d*d; }
  sd = (n > 1) ? sqrtf(sq/(n-1)) : 0;
  spread = mx - mn;
}

// ---------------------------------------------------------------- spectrum
uint16_t readSpectrum(float lit[18], float dark[18]) {
  for (uint8_t i = 0; i < 18; i++) { lit[i] = dark[i] = 0; }
  if (!haveSpec) return 0;

  spec.takeMeasurements();                       // bulb OFF
  const float d[18] = {
    spec.getCalibratedA(), spec.getCalibratedB(), spec.getCalibratedC(),
    spec.getCalibratedD(), spec.getCalibratedE(), spec.getCalibratedF(),
    spec.getCalibratedG(), spec.getCalibratedH(), spec.getCalibratedI(),
    spec.getCalibratedJ(), spec.getCalibratedK(), spec.getCalibratedL(),
    spec.getCalibratedR(), spec.getCalibratedS(), spec.getCalibratedT(),
    spec.getCalibratedU(), spec.getCalibratedV(), spec.getCalibratedW()};
  for (uint8_t i = 0; i < 18; i++) dark[i] = d[SRC[i]];   // letter -> ascending nm

  spec.takeMeasurementsWithBulb();               // bulb ON
  const float l[18] = {
    spec.getCalibratedA(), spec.getCalibratedB(), spec.getCalibratedC(),
    spec.getCalibratedD(), spec.getCalibratedE(), spec.getCalibratedF(),
    spec.getCalibratedG(), spec.getCalibratedH(), spec.getCalibratedI(),
    spec.getCalibratedJ(), spec.getCalibratedK(), spec.getCalibratedL(),
    spec.getCalibratedR(), spec.getCalibratedS(), spec.getCalibratedT(),
    spec.getCalibratedU(), spec.getCalibratedV(), spec.getCalibratedW()};
  for (uint8_t i = 0; i < 18; i++) lit[i] = l[SRC[i]];    // letter -> ascending nm

  const uint16_t raw[18] = {
    spec.getA(), spec.getB(), spec.getC(), spec.getD(), spec.getE(), spec.getF(),
    spec.getG(), spec.getH(), spec.getI(), spec.getJ(), spec.getK(), spec.getL(),
    spec.getR(), spec.getS(), spec.getT(), spec.getU(), spec.getV(), spec.getW()};
  uint16_t worst = 0;
  for (uint8_t i = 0; i < 18; i++) if (raw[i] > worst) worst = raw[i];
  return worst;
}

void printSpectrum(float lit[18], float dark[18], uint16_t worstRaw) {
  Serial.println(F("   nm      lit       dark    dark/lit"));
  float leak = 0;
  for (uint8_t i = 0; i < 18; i++) {
    float f = (lit[i] > 0) ? dark[i]/lit[i] : 0;
    if (f > leak) leak = f;
    Serial.printf("  %3u  %9.2f  %9.2f   %5.1f%%\n", NM[i], lit[i], dark[i], f*100);
  }
  float lam = (lit[CH_NIR] > 0) ? lit[CH_SHORT]/lit[CH_NIR] : 0;
  float sC = lit[CH_SHORT] - dark[CH_SHORT];
  float nC = lit[CH_NIR]   - dark[CH_NIR];
  float lamD = (nC > 0) ? sC/nC : 0;
  Serial.printf("  lambda 435/860 = %.4f   (dark-corrected %.4f)\n", lam, lamD);
  float b410 = lit[0]-dark[0];
  Serial.printf("  410/860 dark-corrected = %.4f  <-- must FALL as oil degrades\n",
                (nC > 0) ? b410/nC : 0);
  Serial.printf("  worst raw %u%s\n", worstRaw,
                worstRaw > 64000 ? "  *** SATURATED - lower gain (g)" : "");
  Serial.printf("  worst leak %.1f%%%s\n", leak*100,
                leak > 0.05f ? "  *** >5% - shield from room light" : "");
  if (worstRaw < 3000) Serial.println(F("  ! very low signal - raise gain (g) BEFORE sample 1"));
}

// ------------------------------------------------------------------ capture
float askFloat(const char *prompt) {
  Serial.print(prompt);
  while (!Serial.available()) delay(20);
  String s = Serial.readStringUntil('\n');
  return s.toFloat();
}

void capture(Sample &sm, const char *label, bool optical) {
  strncpy(sm.name, label, sizeof(sm.name)-1);
  sm.name[sizeof(sm.name)-1] = 0;

  Serial.printf("\n=== %s ===\n", label);
  if (optical) {
    Serial.println(F("Probe wiped clean and immersed to the depth stop?"));
    Serial.println(F("Same container, same fill depth as the other samples."));
  }
  Serial.print(F("Press Enter when ready: "));
  while (!Serial.available()) delay(20);
  Serial.readStringUntil('\n');

  float m = 0, sdLast = 0, spLast = 0;
  for (uint8_t i = 0; i < N_RUNS; i++) {
    float mn, sd, sp;
    capStat(mn, sd, sp);
    Serial.printf("  run %u: %8.3f pF  sd %.4f  spread %.4f  capdac %u  msb %6d\n",
                  i+1, mn, sd, sp, capdac, lastMsb);
    oledProgress(label, i+1, N_RUNS, mn);
    m += mn; sdLast = sd; spLast = sp;
  }
  sm.cMean = m / N_RUNS;
  sm.cSd = sdLast;
  sm.cSpread = spLast;

  if (optical) {
    sm.worstRaw = readSpectrum(sm.lit, sm.dark);
    sm.lambda = (sm.lit[CH_NIR] > 0) ? sm.lit[CH_SHORT]/sm.lit[CH_NIR] : 0;
    float sC2 = sm.lit[CH_SHORT] - sm.dark[CH_SHORT];
    float nC2 = sm.lit[CH_NIR]   - sm.dark[CH_NIR];
    sm.lambdaDark = (nC2 > 0) ? sC2/nC2 : 0;
    printSpectrum(sm.lit, sm.dark, sm.worstRaw);
  }
  float t = readTemp();
  if (isnan(t)) {
    Serial.println(F("  no sensor - entering temperature by hand"));
    sm.tempC = askFloat("Sample temperature in C: ");
  } else {
    sm.tempC = t;
    Serial.printf("  temperature %.2f C from sensor", t);
    if (T_OFFSET != 0.0f) Serial.printf(" (offset %+.2f)", T_OFFSET);
    Serial.println();
  }
  sm.have = true;
  Serial.printf("  stored: C %.3f pF   lambda %.4f   T %.1f C\n",
                sm.cMean, sm.lambda, sm.tempC);
  oledAuto(sm, optical);
#if AUTO_UPLOAD
  uploadSample(sm, optical);
#endif
}



/* ============================================================================
   TEMPERATURE
   Two readings a second apart. If they still differ the thermowell has not
   settled, and a temperature drifting mid-measurement smears the capacitance
   average - so it says so rather than quietly using a moving number.
   ========================================================================== */
float readTemp() {
  if (!haveTemp) return NAN;

  ds.requestTemperatures();
  float a = ds.getTempCByIndex(0);
  delay(1000);
  ds.requestTemperatures();
  float b = ds.getTempCByIndex(0);

  // -127 = no device on the bus, 85 = powered up but never converted
  if (b < -50 || b > 150 || b == 85.0f) {
    Serial.println(F("  bad temperature reading (check the 4.7k pull-up)"));
    return NAN;
  }
  if (fabsf(b - a) > 0.3f)
    Serial.printf("  ! still settling (%.2f -> %.2f C) - wait and re-measure\n", a, b);
  return b + T_OFFSET;
}

void setTempOffset() {
  if (!haveTemp) { Serial.println(F("no sensor")); return; }
  ds.requestTemperatures();
  float raw = ds.getTempCByIndex(0);
  Serial.printf("\nsensor reads %.2f C\n", raw);
  float real = askFloat("what does a real thermometer in the oil say? ");
  T_OFFSET = real - raw;
  Serial.printf("T_OFFSET = %+.2f C  (applied to every reading from now on)\n", T_OFFSET);
  Serial.println(F("a thermowell normally reads LOW, so expect a positive offset."));
}

/* ============================================================================
   FIREBASE over plain HTTPS REST  -  raw observables only.

   Capacitance, temperature and the 18 channel values go up exactly as
   measured. Permittivity, lambda, the index and the verdict are NOT uploaded,
   because all of them depend on calibration constants that will change when
   the electrode geometry is fixed. Raw means the whole history re-derives
   later instead of becoming worthless. Do not "simplify" this.

   POST  /path.json  -> push, Firebase invents the key   (time-ordered stream)
   PUT   /path.json  -> set, overwrites that exact key   (named bench slots)
   ========================================================================== */

void netStart() {
  Serial.printf("WiFi \"%s\" ", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (uint8_t i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) {
    delay(400); Serial.print('.');
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(" FAILED - uploads off, bench test still works normally");
    netUp = false;
    oledMsg("wifi failed", "uploads off");
    return;
  }
  Serial.printf(" connected, %s\n", WiFi.localIP().toString().c_str());

  configTime(0, 0, "pool.ntp.org", "time.google.com");
  Serial.print("clock ");
  for (uint8_t i = 0; i < 20 && time(nullptr) < 100000; i++) { delay(400); Serial.print('.'); }
  clockOK = (time(nullptr) > 100000);
  if (clockOK) Serial.println(" synced");
  else {
    Serial.println(" NOT synced");
    Serial.println("  network is probably blocking NTP (UDP 123).");
    Serial.println("  falling back to boot-relative timestamps - records still");
    Serial.println("  sort in capture order and are flagged ts_estimated.");
  }

  netUp = true;
  Serial.println("Firebase: REST mode, no key. Rules must be open.");
  oledMsg("wifi ok", WiFi.localIP().toString().c_str());
}

// one HTTPS request. put=true overwrites a key, put=false pushes a new one.
static bool rtdb(const String &path, const String &body, bool put) {
  if (!netUp) { Serial.println("  [no wifi - not uploaded]"); return false; }

  WiFiClientSecure client;
  client.setInsecure();                 // no cert pinning; fine for this
  client.setTimeout(8000);

  HTTPClient http;
  String url = String(DB_HOST) + path + ".json";
  http.setConnectTimeout(8000);
  http.setTimeout(8000);
  if (!http.begin(client, url)) { Serial.println("  [http begin failed]"); return false; }
  http.addHeader("Content-Type", "application/json");

  int code = put ? http.PUT(body) : http.POST(body);
  String resp = http.getString();
  http.end();

  if (code == 200) return true;
  Serial.printf("  upload failed, HTTP %d: %s\n", code, resp.c_str());
  if (code == 401 || code == 403)
    Serial.println("  -> database rules are closed. set .read/.write true and Publish.");
  return false;
}

static String jsonArr(const float *v, uint8_t n) {
  String s = "[";
  for (uint8_t i = 0; i < n; i++) { if (i) s += ','; s += String(v[i], 3); }
  s += ']';
  return s;
}

static String nowMs() {
  if (!clockOK && time(nullptr) > 100000) {      // NTP may arrive late
    clockOK = true;
    Serial.println("  [clock synced late - timestamps now real]");
  }
  unsigned long long ms = clockOK
      ? (unsigned long long)time(nullptr) * 1000ULL
      : FALLBACK_EPOCH_MS + (unsigned long long)millis();
  char b[24];
  snprintf(b, sizeof(b), "%llu", ms);
  return String(b);
}

static String sampleJson(Sample &sm, bool optical) {
  String j = "{";
  j += "\"ts\":" + nowMs();
  if (!clockOK) j += ",\"ts_estimated\":true";
  j += ",\"label\":\"" + String(sm.name) + "\"";
  j += ",\"temp_c\":"        + String(sm.tempC, 1);
  j += ",\"cap_pf\":"        + String(sm.cMean, 4);
  j += ",\"cap_sd_pf\":"     + String(sm.cSd, 4);
  j += ",\"cap_spread_pf\":" + String(sm.cSpread, 4);
  j += ",\"runs\":"          + String(N_RUNS);
  j += ",\"gain_note\":\"16x, 49 cycles\"";
  if (optical) {
    j += ",\"channels\":"      + jsonArr(sm.lit, 18);
    j += ",\"channels_dark\":" + jsonArr(sm.dark, 18);
    j += ",\"worst_raw\":"     + String(sm.worstRaw);
  }
  j += "}";
  return j;
}

static const char *slotKey(Sample &sm) {
  if (&sm == &air) return "air";
  if (&sm == &s1)  return "s1_fresh";
  if (&sm == &s2)  return "s2_slight";
  if (&sm == &s3)  return "s3_amber";
  return "s4_recheck";
}

bool uploadSample(Sample &sm, bool optical) {
  if (!sm.have) return false;
  String body = sampleJson(sm, optical);

  String rp = String("/devices/") + DEVICE_ID + "/readings";
  bool a = rtdb(rp, body, false);                    // POST, time-ordered

  String bp = String("/devices/") + DEVICE_ID + "/bench/" + slotKey(sm);
  bool b = rtdb(bp, body, true);                     // PUT, named slot

  if (a && b) Serial.printf("  uploaded -> /devices/%s/bench/%s\n", DEVICE_ID, slotKey(sm));
  return a && b;
}

// Writes one throwaway record so you can prove the link works without
// capturing a sample. Look for it in the console under _test.
void testUpload() {
  if (!netUp) { Serial.println("no wifi - cannot test"); return; }
  String j = "{\"ts\":" + nowMs() + ",\"msg\":\"hello from esp32\",\"clock_ok\":";
  j += clockOK ? "true}" : "false}";
  String path = String("/devices/") + DEVICE_ID + "/_test";
  Serial.printf("PUT %s%s.json\n", DB_HOST, path.c_str());
  if (rtdb(path, j, true)) {
    Serial.println("SUCCESS - refresh the Firebase console, you should see _test");
    Serial.printf("  under devices > %s > _test\n", DEVICE_ID);
  } else {
    Serial.println("FAILED - most likely the database rules are still closed.");
    Serial.println("  console > Realtime Database > Rules:");
    Serial.println("  { \"rules\": { \".read\": true, \".write\": true } }   then Publish");
  }
}

void uploadAll() {
  if (!netUp) { Serial.println("no wifi - nothing uploaded"); return; }
  Serial.println("\nuploading everything captured...");
  Sample *arr[5] = {&air, &s1, &s2, &s3, &s4};
  bool opt[5]    = {false, true, true, true, true};
  uint8_t n = 0;
  for (uint8_t i = 0; i < 5; i++)
    if (arr[i]->have && uploadSample(*arr[i], opt[i])) n++;
  Serial.printf("%u record(s) uploaded\n", n);
}

void uploadFit() {
  if (!netUp) { Serial.println("no wifi"); return; }
  if (!air.have || !s1.have) { Serial.println("need AIR and FRESH first"); return; }
  float K = (s1.cMean - air.cMean) / (EPS_FRESH_ASSUMED - 1.0f);
  float P = air.cMean - K;

  String j = "{";
  j += "\"ts\":" + nowMs();
  j += ",\"cell_pf_per_eps\":" + String(K, 4);
  j += ",\"parasitic_pf\":"    + String(P, 4);
  j += ",\"opt_fresh_raw\":"   + String(s1.lambda, 4);
  j += ",\"opt_fresh_dark\":"  + String(s1.lambdaDark, 4);
  j += ",\"eps_fresh_assumed\":" + String(EPS_FRESH_ASSUMED, 2);
  j += ",\"c_air_pf\":"   + String(air.cMean, 4);
  j += ",\"c_fresh_pf\":" + String(s1.cMean, 4);
  j += ",\"temp_c\":"     + String(s1.tempC, 1);
  j += ",\"provisional\":true}";

  String cp = String("/devices/") + DEVICE_ID + "/calibration";
  if (rtdb(cp, j, true))
    Serial.printf("calibration uploaded: K=%.3f  P=%.3f  OPT_FRESH=%.4f\n", K, P, s1.lambda);
}


float TH_ADVISORY = 0.70f;    // 20% TPM equivalent
float TH_DISCARD  = 1.00f;    // 25% TPM equivalent
bool  anchored    = false;

static float noiseFloor() {
  float n = 0;
  Sample *a[5] = {&air,&s1,&s2,&s3,&s4};
  for (uint8_t i = 0; i < 5; i++) if (a[i]->have && a[i]->cSpread > n) n = a[i]->cSpread;
  return (n > 0) ? n : 0.05f;
}

/* ============================================================================
   OLED  -  128x64 SSD1306 at 0x3C

   Size 1 text is 6x8 px, so 21 characters across and 8 lines.
   Size 2 is 12x16, so 10 characters across.
   ========================================================================== */

void oledMsg(const char *l1, const char *l2) {
  if (!haveOled) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 22); oled.print(l1);
  if (l2) { oled.setCursor(0, 36); oled.print(l2); }
  oled.display();
}

// shown live while the 8 runs are being taken
void oledProgress(const char *label, uint8_t run, uint8_t total, float pF) {
  if (!haveOled) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);  oled.print(F("measuring"));
  oled.setCursor(84, 0); oled.print(run); oled.print('/'); oled.print(total);
  oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  oled.setCursor(0, 16); oled.print(label);
  oled.setTextSize(2);
  oled.setCursor(0, 30); oled.print(pF, 3);
  oled.setTextSize(1);
  oled.setCursor(78, 38); oled.print(F("pF"));
  // progress bar
  oled.drawRect(0, 54, 128, 8, SSD1306_WHITE);
  oled.fillRect(1, 55, (uint8_t)(126.0f * run / total), 6, SSD1306_WHITE);
  oled.display();
}

// what a single captured sample looks like
void oledSample(Sample &sm, bool optical) {
  if (!haveOled) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  oled.setTextSize(1);
  oled.setCursor(0, 0);  oled.print(F("OilSense"));
  oled.setCursor(80, 0); oled.print(sm.tempC, 1); oled.print(F(" C"));
  oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  oled.setCursor(0, 14); oled.print(sm.name);

  oled.setCursor(0, 28); oled.print(F("C  "));
  oled.print(sm.cMean, 3); oled.print(F(" pF"));

  oled.setCursor(0, 38); oled.print(F("sd "));
  oled.print(sm.cSd, 4);
  if (sm.cSd > 0.05f) { oled.setCursor(78, 38); oled.print(F("UNSTBL")); }

  if (optical) {
    oled.setCursor(0, 50); oled.print(F("L  "));
    oled.print(sm.lambda, 4);
    if (sm.worstRaw < 500) { oled.setCursor(78, 50); oled.print(F("LOWSIG")); }
  }
  oled.display();
}

/* The demo screen. Only shows a channel that is allowed to vote - the same
   resolution test the serial verdict uses. A channel below resolution prints
   n/r rather than a made-up number. */
void oledVerdict() {
  if (!haveOled) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);

  if (!anchored || !s1.have) {
    oled.setCursor(0, 20); oled.print(F("not anchored"));
    oled.setCursor(0, 34); oled.print(F("press k then v"));
    oled.display();
    return;
  }

  Sample *sm = s3.have ? &s3 : (s2.have ? &s2 : &s1);
  const float nf    = noiseFloor();
  const float cSpan = s3.cMean - s1.cMean;
  const bool  capOk = fabsf(cSpan) > 2.0f * nf;
  const float lSpan = (s1.lambda > 0 && s3.lambda > 0) ? logf(s1.lambda / s3.lambda) : 0;
  const bool  optOk = fabsf(lSpan) > 0.05f;

  float nOpt = (optOk && sm->lambda > 0) ? logf(s1.lambda / sm->lambda) / lSpan : NAN;
  float nCap = capOk ? (sm->cMean - s1.cMean) / cSpan : NAN;
  float idx  = !isnan(nOpt) ? nOpt : nCap;

  oled.setCursor(0, 0);  oled.print(F("OilSense"));
  oled.setCursor(80, 0); oled.print(sm->tempC, 1); oled.print(F(" C"));
  oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  oled.setTextSize(2);
  oled.setCursor(0, 15);
  if (isnan(idx)) oled.print(F("--")); else oled.print(idx, 2);
  oled.setTextSize(1);
  if (!isnan(idx)) { oled.setCursor(56, 22); oled.print(8.0f + 17.0f * idx, 1); oled.print(F("% TPM")); }

  oled.setCursor(0, 36);
  oled.print(F("L")); if (isnan(nOpt)) oled.print(F(" n/r")); else oled.print(nOpt, 2);
  oled.setCursor(56, 36);
  oled.print(F("e")); if (isnan(nCap)) oled.print(F(" n/r")); else oled.print(nCap, 2);

  oled.setCursor(0, 48);
  if (isnan(idx))           oled.print(F("NO VALID CHANNEL"));
  else if (idx >= 1.00f)    oled.print(F("DISCARD NOW"));
  else if (idx >= 0.70f)    oled.print(F("CHANGE SOON"));
  else                      oled.print(F("SAFE TO FRY"));

  oled.setCursor(0, 57);
  oled.print(F("provisional scale"));
  oled.display();
}


/* The showcase screen. One question answered in the biggest type that fits:
   is this oil still usable. Numbers are supporting evidence, small.
   128x64 with size 2 text is 10 characters across, so long verdicts split. */
void oledBig() {
  if (!haveOled) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  if (!anchored || !s1.have) {
    oled.setTextSize(1);
    oled.setCursor(0, 24); oled.print(F("not anchored yet"));
    oled.setCursor(0, 38); oled.print(F("press k, then v"));
    oled.display();
    return;
  }

  Sample *sm = s3.have ? &s3 : (s2.have ? &s2 : &s1);
  const float nf    = noiseFloor();
  const float cSpan = s3.cMean - s1.cMean;
  const bool  capOk = fabsf(cSpan) > 2.0f * nf;
  const float lSpan = (s1.lambda > 0 && s3.lambda > 0) ? logf(s1.lambda / s3.lambda) : 0;
  const bool  optOk = fabsf(lSpan) > 0.05f;

  float nOpt = (optOk && sm->lambda > 0) ? logf(s1.lambda / sm->lambda) / lSpan : NAN;
  float nCap = capOk ? (sm->cMean - s1.cMean) / cSpan : NAN;
  float idx  = !isnan(nOpt) ? nOpt : nCap;

  // header
  oled.setTextSize(1);
  oled.setCursor(0, 0);  oled.print(F("OilSense"));
  oled.setCursor(86, 0); oled.print(sm->tempC, 0); oled.print(F(" C"));
  oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // verdict, as large as it will go
  const char *l1 = "--", *l2 = 0;
  if (isnan(idx))            { l1 = "NO";      l2 = "READING"; }
  else if (idx >= TH_DISCARD){ l1 = "DISCARD"; }
  else if (idx >= TH_ADVISORY){ l1 = "CHANGE"; l2 = "SOON"; }
  else                       { l1 = "KEEP";    l2 = "FRYING"; }

  oled.setTextSize(2);
  if (l2) { oled.setCursor(0, 15); oled.print(l1); oled.setCursor(0, 33); oled.print(l2); }
  else    { oled.setCursor(0, 22); oled.print(l1); }

  // supporting numbers, small
  oled.setTextSize(1);
  oled.setCursor(0, 54);
  if (!isnan(idx)) {
    oled.print(idx, 2);
    oled.setCursor(34, 54);
    oled.print(8.0f + 17.0f * idx, 1); oled.print(F("%"));
  }

  // trust: only meaningful when both channels are allowed to speak
  oled.setCursor(78, 54);
  if (isnan(nOpt) || isnan(nCap)) oled.print(F("1 chan"));
  else if (fabsf(nOpt - nCap) <= 0.15f) oled.print(F("agreed"));
  else oled.print(nCap > nOpt ? F("water?") : F("solids"));
  oled.display();
}

/* Before anchoring you are calibrating, so show the bench numbers.
   After anchoring you are demonstrating, so show the verdict. */
void oledAuto(Sample &sm, bool optical) {
  if (anchored) oledBig(); else oledSample(sm, optical);
}

/* ============================================================================
   VERDICT
   Anchored to YOUR OWN samples: fresh = 0.00, amber = 1.00. That is an
   assumption, not a measurement - it presumes your darkest sample sits at the
   25% TPC discard limit. Absolute anchoring needs a reference instrument
   (spec 5.6 stage 3). Every verdict prints PROVISIONAL.

   Normalising between two measured points makes the cell constant and the
   parasitic offset cancel exactly:
       n = (eps - eps_f)/(eps_d - eps_f) = (C - C_f)/(C_d - C_f)
   so the verdict does not depend on K or P at all.

   A channel only votes if its span beats its own noise. If it cannot resolve
   a difference it prints n/r instead of inventing a number.
   ========================================================================== */

void setAnchor() {
  if (!s1.have || !s3.have) {
    Serial.println(F("Need sample 1 (fresh) and sample 3 (amber) first."));
    return;
  }
  anchored = true;
  Serial.println(F("\nScale anchored: fresh = 0.00, amber = 1.00"));
  Serial.printf("  capacitance  %.3f -> %.3f pF   (span %.3f)\n",
                s1.cMean, s3.cMean, s3.cMean - s1.cMean);
  Serial.printf("  lambda       %.4f -> %.4f\n", s1.lambda, s3.lambda);
  Serial.println(F("  this ASSUMES amber is at the discard limit. say so out loud."));
}

void verdictTable() {
  if (!anchored) { Serial.println(F("Press 'k' first to anchor the scale.")); return; }

  const float nf    = noiseFloor();
  const float cSpan = s3.cMean - s1.cMean;
  const bool  capUsable = fabsf(cSpan) > 2.0f * nf;
  const float lSpan = (s1.lambda > 0 && s3.lambda > 0) ? logf(s1.lambda/s3.lambda) : 0;
  const bool  optUsable = fabsf(lSpan) > 0.05f;

  Serial.println(F("\n\n===================== VERDICT ====================="));
  Serial.println(F("  *** PROVISIONAL - anchored to your own samples,"));
  Serial.println(F("      not to a measured TPC value. ***\n"));
  Serial.println(F("  sample                  optical  capacitive   VERDICT"));

  Sample *a[4] = {&s1,&s2,&s3,&s4};
  for (uint8_t i = 0; i < 4; i++) {
    Sample *sm = a[i];
    if (!sm->have) continue;

    float nOpt = NAN;
    if (optUsable && sm->lambda > 0) nOpt = logf(s1.lambda / sm->lambda) / lSpan;

    float nCap = NAN;
    bool  thisRes = false;
    if (capUsable) {
      nCap = (sm->cMean - s1.cMean) / cSpan;
      thisRes = (sm == &s1) || (fabsf(sm->cMean - s1.cMean) > 2.0f * nf);
    }

    float idx = NAN;
    if (!isnan(nOpt) && !isnan(nCap) && thisRes) idx = 0.5f*(nOpt+nCap);
    else if (!isnan(nOpt))                       idx = nOpt;
    else if (!isnan(nCap) && thisRes)            idx = nCap;

    const char *v = "no valid channel";
    if (!isnan(idx)) {
      if      (idx >= TH_DISCARD)  v = "DISCARD NOW";
      else if (idx >= TH_ADVISORY) v = "CHANGE SOON";
      else                         v = "SAFE TO KEEP FRYING";
    }

    char oc[12], cc[12];
    if (isnan(nOpt)) snprintf(oc, sizeof(oc), "   --");
    else             snprintf(oc, sizeof(oc), "%+6.2f", nOpt);
    if (isnan(nCap) || !thisRes) snprintf(cc, sizeof(cc), "  n/r");
    else                         snprintf(cc, sizeof(cc), "%+6.2f", nCap);

    Serial.printf("  %-22s %6s     %5s     %s\n", sm->name, oc, cc, v);
  }

  Serial.println();
  Serial.printf("  capacitive span %.3f pF against noise %.3f pF\n", cSpan, nf);
  if (!capUsable) {
    Serial.println(F("  -> capacitive channel CANNOT resolve this. marked n/r."));
    Serial.println(F("     verdict is optical only. fix is electrode geometry:"));
    Serial.println(F("     plates or a concentric tube. K needs roughly 15x."));
  } else {
    Serial.println(F("  -> both channels voting. cross-validation active."));
  }
  if (!optUsable) Serial.println(F("  -> optical span too small to use either."));

  if (capUsable && optUsable && s2.have) {
    float nO = logf(s1.lambda/s2.lambda)/lSpan;
    float nC = (s2.cMean - s1.cMean)/cSpan;
    if (fabsf(s2.cMean - s1.cMean) > 2.0f*nf) {
      float d = fabsf(nO - nC);
      Serial.printf("\n  cross-validation on sample 2: delta %.3f (tolerance 0.15)\n", d);
      if (d <= 0.15f) Serial.println(F("  channels agree - reading trustworthy"));
      else if (nC > nO) Serial.println(F("  permittivity leads -> MOISTURE suspected"));
      else              Serial.println(F("  optics leads -> PARTICULATE or FOULING suspected"));
    }
  }
  Serial.println(F("\n  verdicts are NOT uploaded. only raw values go to Firebase, so the"));
  Serial.println(F("  whole history re-derives once you anchor against real TPC."));
  Serial.println(F("==================================================\n"));
}

// ------------------------------------------------------------------- report
void report() {
  Serial.println(F("\n\n################ COMPARISON REPORT ################\n"));

  if (!air.have || !s1.have) {
    Serial.println(F("Need at least AIR (a) and FRESH (1)."));
    return;
  }

  // stage-1 fit from air and fresh oil
  float K = (s1.cMean - air.cMean) / (EPS_FRESH_ASSUMED - 1.0f);
  float P = air.cMean - K;
  Serial.println(F("--- PROBE CALIBRATION (from air + fresh) ---"));
  Serial.printf("  C_air            %8.3f pF  (sd %.4f)\n", air.cMean, air.cSd);
  Serial.printf("  C_fresh          %8.3f pF  (sd %.4f)\n", s1.cMean, s1.cSd);
  Serial.printf("  eps_fresh assumed%8.3f\n", EPS_FRESH_ASSUMED);
  Serial.printf("  CELL_PF_PER_EPS  %8.3f\n", K);
  Serial.printf("  PARASITIC_PF     %8.3f\n", P);
  Serial.printf("  parasitic share  %8.1f %% of air reading\n",
                air.cMean != 0 ? P/air.cMean*100 : 0);
  Serial.printf("  figure of merit  %8.3f  (ideal %.2f)\n",
                air.cMean != 0 ? s1.cMean/air.cMean : 0, EPS_FRESH_ASSUMED);
  if (K < 1) Serial.println(F("  !! K below 1 pF/eps - the oil is barely affecting the reading."));
  if (P < 0) Serial.println(F("  !! negative offset - assumed eps wrong, or a bad reading."));

  // per-sample table
  Serial.println(F("\n--- SAMPLES ---"));
  Serial.println(F("  sample                    T     C(pF)     sd     eps      lambda   absorb"));
  Sample *arr[4] = {&s1, &s2, &s3, &s4};
  float eps[4], absorb[4];
  for (uint8_t i = 0; i < 4; i++) {
    Sample *s = arr[i];
    if (!s->have) { eps[i] = absorb[i] = NAN; continue; }
    eps[i] = (s->cMean - P) / K;
    absorb[i] = (s->lambda > 0 && s1.lambda > 0)
                ? logf(s1.lambda / s->lambda) : NAN;
    Serial.printf("  %-22s %5.1f %9.3f %7.4f %7.3f %9.4f %8.4f\n",
                  s->name, s->tempC, s->cMean, s->cSd, eps[i], s->lambda, absorb[i]);
  }

  // temperature consistency
  Serial.println(F("\n--- TEMPERATURE CHECK ---"));
  float tmin = 1e9, tmax = -1e9;
  for (uint8_t i = 0; i < 4; i++) if (arr[i]->have) {
    if (arr[i]->tempC < tmin) tmin = arr[i]->tempC;
    if (arr[i]->tempC > tmax) tmax = arr[i]->tempC;
  }
  Serial.printf("  spread across samples: %.1f C\n", tmax - tmin);
  if (tmax - tmin > 2.0f) {
    float err = 0.0015f * (tmax - tmin);
    Serial.printf("  ! that is %.4f eps of uncompensated error (%.1f%% of a 0.55 span)\n",
                  err, err/0.55f*100);
    Serial.println(F("  ! let the samples equilibrate and repeat."));
  } else Serial.println(F("  + tight enough to compare directly."));

  // monotonicity
  Serial.println(F("\n--- DO BOTH CHANNELS RANK THE SAMPLES THE SAME WAY? ---"));
  if (s2.have && s3.have) {
    bool capOk = (eps[0] < eps[1]) && (eps[1] < eps[2]);
    bool optOk = (s1.lambda > s2.lambda) && (s2.lambda > s3.lambda);
    Serial.printf("  capacitive: eps %.3f -> %.3f -> %.3f   %s\n",
                  eps[0], eps[1], eps[2], capOk ? "RISING, correct" : "*** NOT MONOTONIC");
    Serial.printf("  optical:  lambda %.4f -> %.4f -> %.4f  %s\n",
                  s1.lambda, s2.lambda, s3.lambda, optOk ? "FALLING, correct" : "*** NOT MONOTONIC");
    if (capOk && optOk)
      Serial.println(F("  + both channels agree on the ordering. This is the key result."));
    else
      Serial.println(F("  * disagreement means carry-over, changed geometry, or too small a step."));
  } else Serial.println(F("  need samples 1, 2 and 3."));

  // THE PREMISE TEST
  Serial.println(F("\n--- PREMISE TEST: are the two channels proportional? ---"));
  Serial.println(F("  absorbance should be a fixed multiple of the permittivity rise."));
  if (s2.have && s3.have) {
    float dEps2 = eps[1] - eps[0], dEps3 = eps[2] - eps[0];
    float r2 = (fabsf(dEps2) > 1e-4f) ? absorb[1]/dEps2 : NAN;
    float r3 = (fabsf(dEps3) > 1e-4f) ? absorb[2]/dEps3 : NAN;
    Serial.printf("  sample 2:  d_eps %+.4f   absorbance %+.4f   ratio %8.3f\n", dEps2, absorb[1], r2);
    Serial.printf("  sample 3:  d_eps %+.4f   absorbance %+.4f   ratio %8.3f\n", dEps3, absorb[2], r3);
    if (!isnan(r2) && !isnan(r3) && r2 != 0) {
      float dev = fabsf(r3 - r2) / fabsf(r2) * 100;
      Serial.printf("  ratios differ by %.1f%%\n", dev);
      if (dev < 25)      Serial.println(F("  ++ PROPORTIONAL. Cross-validation is on solid ground."));
      else if (dev < 60) Serial.println(F("  +  roughly proportional. Plausible with only 3 points."));
      else               Serial.println(F("  *  not proportional. Suspect carry-over, window fouling,"
                                          "\n     or degradation steps too uneven to compare."));
      Serial.printf("\n  scaling factor absorbance per unit eps: %.3f\n", (r2+r3)/2);
      Serial.println(F("  Use it to set OPT_DISCARD once you know EPS_DISCARD:"));
      Serial.printf("    OPT_DISCARD = OPT_FRESH / exp(%.3f x (EPS_DISCARD - %.2f))\n",
                    (r2+r3)/2, EPS_FRESH_ASSUMED);
    }
  } else Serial.println(F("  need samples 1, 2 and 3."));

  // carry-over control
  if (s4.have) {
    Serial.println(F("\n--- CARRY-OVER CONTROL (fresh, re-measured last) ---"));
    float dC = s4.cMean - s1.cMean;
    float dL = (s1.lambda > 0) ? (s4.lambda - s1.lambda) : 0;
    Serial.printf("  C drift      %+.3f pF  (%.2f%% of the fresh->amber step)\n", dC,
                  (s3.have && fabsf(s3.cMean - s1.cMean) > 1e-3f)
                    ? fabsf(dC)/fabsf(s3.cMean - s1.cMean)*100 : 0);
    Serial.printf("  lambda drift %+.4f\n", dL);
    if (fabsf(dC) < 0.1f) Serial.println(F("  + clean. Wiping between samples is working."));
    else Serial.println(F("  ! fresh no longer reads the same. Residue on the electrodes,"
                          "\n    or the immersion depth is not repeating."));
  } else Serial.println(F("\n  (run '4' to re-measure fresh as a carry-over control)"));

  // constants to copy out
  Serial.println(F("\n--- PASTE INTO THE FIRMWARE AND DASHBOARD ---"));
  Serial.printf("  CELL_PF_PER_EPS = %.3f\n", K);
  Serial.printf("  PARASITIC_PF    = %.3f\n", P);
  Serial.printf("  OPT_FRESH       = %.4f\n", s1.lambda);
  Serial.println(F("  EPS_DISCARD / OPT_DISCARD still need a sample of known TPC."));
  Serial.println(F("\n###################################################\n"));
#if AUTO_UPLOAD
  uploadFit();
#endif
  if (s1.have && s3.have)
    Serial.println(F("  press 'k' to anchor, then 'v' for the verdict."));
}

void help() {
  Serial.println(F(
    "\n===== OilSense three-sample validation =====\n"
    " g  gain / integration  (SET FIRST, then leave alone)\n"
    " a  AIR reference\n"
    " 1  sample 1  FRESH\n"
    " 2  sample 2  SLIGHTLY DEGRADED\n"
    " 3  sample 3  AMBER\n"
    " 4  re-measure FRESH  (carry-over control, do last)\n"
    " r  comparison report + premise test\n"
    " s  spectrum only\n"
    " e  set assumed fresh permittivity\n"
    " T  read temperature now     o  set thermowell offset\n"
    " t  TEST the firebase link (do this first)\n"
    " k  anchor the scale (fresh=0, amber=1)\n"
    " v  VERDICT - safe / change soon / discard\n"
    " d  OLED: bench numbers      D  OLED: big verdict\n"
    " u  upload everything captured\n"
    " c  upload the fitted constants\n"
    " w  wifi / firebase status\n"
    " h  help\n"
    "Measure in order 1,2,3 - least degraded first - and wipe between.\n"
    "============================================\n"));
}

void setGain() {
  Serial.println(F("\nGain: 0=1x  1=3.7x  2=16x  3=64x"));
  Serial.print(F("choice: "));
  while (!Serial.available()) delay(20);
  int g = Serial.readStringUntil('\n').toInt();
  if (haveSpec) {
    if (g == 0) spec.setGain(AS7265X_GAIN_1X);
    else if (g == 1) spec.setGain(AS7265X_GAIN_37X);
    else if (g == 3) spec.setGain(AS7265X_GAIN_64X);
    else spec.setGain(AS7265X_GAIN_16X);
  }
  int c = (int) askFloat("integration cycles 1-255 (49 default): ");
  if (haveSpec && c >= 1 && c <= 255) spec.setIntegrationCycles((uint8_t) c);
  Serial.println(F("set. Do NOT change this again mid-run - it breaks comparability."));
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("\n\n=== OilSense THREE-SAMPLE VALIDATION ==="));

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000);

  haveOled = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  if (haveOled) {
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(2);
    oled.setCursor(0, 12); oled.print(F("OilSense"));
    oled.setTextSize(1);
    oled.setCursor(0, 38); oled.print(F("starting..."));
    oled.display();
  } else {
    Serial.println(F("OLED not found - try 0x3D, or check the pull-ups"));
  }

  ds.begin();
  ds.setResolution(12);                       // 0.0625 C, 750 ms per conversion
  haveTemp = (ds.getDeviceCount() > 0);
  Serial.println(haveTemp ? F("DS18B20 found") :
                            F("DS18B20 NOT found - check the 4.7k pull-up on the data pin"));

  bool sawFdc = false, sawSpec = false, sawOled = false;
  Serial.println(F("I2C scan:"));
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  0x%02X", a);
      if (a == 0x50) { Serial.print(F("  FDC1004")); sawFdc = true; }
      if (a == 0x49) { Serial.print(F("  AS7265x")); sawSpec = true; }
      if (a == 0x3C || a == 0x3D) { Serial.print(F("  SSD1306")); sawOled = true; }
      Serial.println();
    }
  }
  if (!sawFdc)  Serial.println(F("  ! FDC1004 missing"));
  if (!sawSpec) Serial.println(F("  ! AS7265x missing"));
  if (!sawOled) Serial.println(F("  ! SSD1306 missing"));

  haveSpec = spec.begin();
  if (haveSpec) {
    spec.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_WHITE);
    spec.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_IR);
    spec.setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_UV);
    spec.setGain(AS7265X_GAIN_16X);
    spec.setIntegrationCycles(49);
    spec.disableIndicator();
    Serial.println(F("AS7265x ready (16x, 49 cycles, 12.5 mA)"));
  }

  netStart();

  Serial.println(F("\nNo temperature sensor. All samples must sit at the SAME"));
  Serial.println(F("temperature or the comparison is invalid. Log it by hand.\n"));
  oledMsg("ready", "a 1 2 3  then k v");
  help();
}

void loop() {
  if (!Serial.available()) { delay(30); return; }
  char c = Serial.read();
  while (Serial.available() && (Serial.peek()=='\n' || Serial.peek()=='\r')) Serial.read();

  switch (c) {
    case 'a': capture(air, "AIR", false); break;
    case '1': capture(s1, "1 FRESH", true); break;
    case '2': capture(s2, "2 SLIGHTLY DEGRADED", true); break;
    case '3': capture(s3, "3 AMBER", true); break;
    case '4': capture(s4, "4 FRESH re-check", true); break;
    case 'r': report(); break;
    case 's': { float l[18], d[18]; uint16_t w = readSpectrum(l, d); printSpectrum(l, d, w); break; }
    case 'g': setGain(); break;
    case 'e':
      EPS_FRESH_ASSUMED = askFloat("assumed fresh permittivity: ");
      if (EPS_FRESH_ASSUMED < 1.1f) EPS_FRESH_ASSUMED = 3.00f;
      Serial.printf("%.3f\n", EPS_FRESH_ASSUMED);
      break;
    case 'd':
      if (s1.have) { oledSample(s3.have ? s3 : s1, true); Serial.println(F("bench screen")); }
      break;
    case 'D': oledBig(); Serial.println(F("verdict screen")); break;
    case 'o': setTempOffset(); break;
    case 'T':
      { float t = readTemp();
        if (isnan(t)) Serial.println(F("no reading"));
        else Serial.printf("temperature %.2f C (offset %+.2f)\n", t, T_OFFSET); }
      break;
    case 't': testUpload(); break;
    case 'k': setAnchor(); oledBig(); break;
    case 'v': verdictTable(); oledBig(); break;
    case 'u': uploadAll(); break;
    case 'c': uploadFit(); break;
    case 'w':
      Serial.printf("wifi %s   ip %s\n  clock %s\n  host %s\n",
                    netUp ? "up" : "DOWN",
                    netUp ? WiFi.localIP().toString().c_str() : "-",
                    clockOK ? "synced (real timestamps)" : "NOT synced (estimated timestamps)",
                    DB_HOST);
      break;
    case 'h': help(); break;
    default: break;
  }
}
