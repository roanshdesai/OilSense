#pragma once
#include "Arduino.h"
class TwoWire {
public:
  void begin(int,int){} void setClock(unsigned long){}
  void beginTransmission(uint8_t){} uint8_t endTransmission(){return 0;}
};
extern TwoWire Wire;
