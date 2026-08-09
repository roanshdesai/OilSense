#pragma once
#include "Arduino.h"
#define WIFI_STA 1
#define WL_CONNECTED 3
class IPAddress { public: String toString() const {return String();} };
class WiFiClass {
public:
  void mode(int){} void begin(const char*,const char*){}
  int status(){return WL_CONNECTED;}
  IPAddress localIP(){return IPAddress();}
};
extern WiFiClass WiFi;
