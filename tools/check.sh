#!/usr/bin/env bash
# Verify an Arduino sketch without an ESP32 attached.
#
# Two passes. The second is the one that matters: it reproduces the way
# arduino-builder hoists auto-generated prototypes to just before the first
# function definition, which is what breaks code whose structs come later
# ('Result' does not name a type). Plain g++ cannot catch that.
set -u
SKETCH="${1:?usage: ./check.sh path/to/sketch.ino}"
DIR="$(cd "$(dirname "$0")" && pwd)"
TMP="$(mktemp -d)"

preamble() {
  cat <<'P'
#include "Arduino.h"
#include "Wire.h"
#include "WiFi.h"
HardwareSerial Serial; TwoWire Wire; WiFiClass WiFi;
void delay(unsigned long){} unsigned long millis(){return 0;}
void pinMode(int,int){} int digitalRead(int){return 1;} long random(long){return 0;}
void configTime(long,int,const char*,const char*,const char*){}
P
}

FLAGS="-std=gnu++17 -fsyntax-only -I$DIR -Wall -Wextra -Wformat=2 -Wshadow -Wno-unused-parameter"

echo "== pass 1: plain compile =="
{ preamble; cat "$SKETCH"; } > "$TMP/a.cpp"
g++ $FLAGS "$TMP/a.cpp" 2>&1 | grep -vE '^\s*\||^\s*\^|^\s*~|Protocentral_FDC1004\.h|unused parameter .rate.' && echo "  (errors above)" || echo "  clean"

echo "== pass 2: with Arduino prototype hoisting =="
python3 "$DIR/arduino_sim.py" "$SKETCH" "$TMP/h.ino" > /dev/null
{ preamble; cat "$TMP/h.ino"; } > "$TMP/b.cpp"
g++ $FLAGS "$TMP/b.cpp" 2>&1 | grep -vE '^\s*\||^\s*\^|^\s*~|Protocentral_FDC1004\.h|unused parameter .rate.' && echo "  (errors above)" || echo "  clean"

rm -rf "$TMP"
