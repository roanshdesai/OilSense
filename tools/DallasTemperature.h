#pragma once
#include "OneWire.h"
class DallasTemperature {
public:
  DallasTemperature(OneWire*){}
  void begin(){} void setResolution(uint8_t){}
  void requestTemperatures(){}
  float getTempCByIndex(uint8_t){return 25.0f;}
  uint8_t getDeviceCount(){return 1;}
};
