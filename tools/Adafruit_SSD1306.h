#pragma once
#include "Arduino.h"
#include "Wire.h"
#define SSD1306_WHITE 1
#define SSD1306_BLACK 0
#define SSD1306_SWITCHCAPVCC 2
class Adafruit_SSD1306 {
public:
  Adafruit_SSD1306(int,int,TwoWire*,int){}
  bool begin(uint8_t,uint8_t){return true;}
  void clearDisplay(){} void display(){}
  void setTextColor(uint16_t){} void setTextSize(uint8_t){}
  void setCursor(int16_t,int16_t){}
  void drawLine(int16_t,int16_t,int16_t,int16_t,uint16_t){}
  void drawRect(int16_t,int16_t,int16_t,int16_t,uint16_t){}
  void fillRect(int16_t,int16_t,int16_t,int16_t,uint16_t){}
  void print(const char*){}  void println(const char*){}
  void print(const __FlashStringHelper*){} void println(const __FlashStringHelper*){}
  void print(char){} void print(int){} void print(unsigned int){}
  void print(long){} void print(unsigned long){}
  void print(double){} void print(double,int){}
  void print(const String&){}
};
