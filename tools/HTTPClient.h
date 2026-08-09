#pragma once
#include "Arduino.h"
#include "WiFiClientSecure.h"
class HTTPClient {
public:
  bool begin(WiFiClientSecure&,const String&){return true;}
  void addHeader(const char*,const char*){}
  int  PUT(const String&){return 200;}
  int  POST(const String&){return 200;}
  String getString(){return String();}
  void end(){} void setConnectTimeout(int){} void setTimeout(int){}
};
