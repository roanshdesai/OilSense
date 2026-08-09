#pragma once
#include "Arduino.h"
#define FDC1004_100HZ 1
#define FDC1004_200HZ 2
#define FDC1004_400HZ 3
class FDC1004 {
public:
  FDC1004(uint8_t rate=FDC1004_100HZ){}
  uint8_t configureMeasurementSingle(uint8_t,uint8_t,uint8_t){return 0;}
  uint8_t triggerSingleMeasurement(uint8_t,uint8_t){return 0;}
  uint8_t readMeasurement(uint8_t,uint16_t*){return 0;}
};
