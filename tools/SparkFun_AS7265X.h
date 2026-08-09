#pragma once
#include "Arduino.h"
#define AS7265X_GAIN_1X 0
#define AS7265X_GAIN_37X 1
#define AS7265X_GAIN_16X 2
#define AS7265X_GAIN_64X 3
#define AS7265X_LED_CURRENT_LIMIT_12_5MA 0
#define AS7265x_LED_WHITE 0
#define AS7265x_LED_IR 1
#define AS7265x_LED_UV 2
class AS7265X {
public:
  bool begin(){return true;}
  void takeMeasurements(){} void takeMeasurementsWithBulb(){}
  void setGain(uint8_t){} void setIntegrationCycles(uint8_t){}
  void setBulbCurrent(uint8_t,uint8_t){} void disableIndicator(){}
  float getCalibratedA(){return 0;} float getCalibratedB(){return 0;}
  float getCalibratedC(){return 0;} float getCalibratedD(){return 0;}
  float getCalibratedE(){return 0;} float getCalibratedF(){return 0;}
  float getCalibratedG(){return 0;} float getCalibratedH(){return 0;}
  float getCalibratedI(){return 0;} float getCalibratedJ(){return 0;}
  float getCalibratedK(){return 0;} float getCalibratedL(){return 0;}
  float getCalibratedR(){return 0;} float getCalibratedS(){return 0;}
  float getCalibratedT(){return 0;} float getCalibratedU(){return 0;}
  float getCalibratedV(){return 0;} float getCalibratedW(){return 0;}
  uint16_t getA(){return 0;} uint16_t getB(){return 0;} uint16_t getC(){return 0;}
  uint16_t getD(){return 0;} uint16_t getE(){return 0;} uint16_t getF(){return 0;}
  uint16_t getG(){return 0;} uint16_t getH(){return 0;} uint16_t getI(){return 0;}
  uint16_t getJ(){return 0;} uint16_t getK(){return 0;} uint16_t getL(){return 0;}
  uint16_t getR(){return 0;} uint16_t getS(){return 0;} uint16_t getT(){return 0;}
  uint16_t getU(){return 0;} uint16_t getV(){return 0;} uint16_t getW(){return 0;}
};
