#pragma once
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <ctime>
class __FlashStringHelper;
#define F(x) (reinterpret_cast<const __FlashStringHelper*>(x))
#define PROGMEM
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define LOW 0
#define HIGH 1
typedef uint8_t byte;
void delay(unsigned long);
unsigned long millis();
void pinMode(int,int);
int  digitalRead(int);
long random(long);
class String {
public:
  String(){} String(const char*){} String(const std::string&){}
  String(int){} String(unsigned int){} String(long){} String(unsigned long){}
  String(unsigned long long){} String(double){} String(float){}
  String(float,int){} String(double,int){} String(int,int){}
  String operator+(const String&) const {return String();}
  String operator+(const char*) const {return String();}
  String& operator+=(const String&){return *this;}
  String& operator+=(const char*){return *this;}
  String& operator+=(char){return *this;}
  float toFloat() const {return 0;}
  int   toInt()   const {return 0;}
  const char* c_str() const {return "";}
  unsigned length() const {return 0;}
};
inline String operator+(const char*, const String&){return String();}
class HardwareSerial {
public:
  void begin(unsigned long){}
  int  available(){return 0;}
  int  read(){return 0;}
  int  peek(){return 0;}
  String readStringUntil(char){return String();}
  int printf(const char* f, ...) __attribute__((format(printf,2,3)));
  void print(){} void println(){}
  void print(const char*){}            void println(const char*){}
  void print(const __FlashStringHelper*){} void println(const __FlashStringHelper*){}
  void print(char){}                   void println(char){}
  void print(int){}                    void println(int){}
  void print(unsigned int){}           void println(unsigned int){}
  void print(long){}                   void println(long){}
  void print(unsigned long){}          void println(unsigned long){}
  void print(double){}                 void println(double){}
  void print(double,int){}             void println(double,int){}
  void print(const String&){}          void println(const String&){}
};
extern HardwareSerial Serial;
void configTime(long,int,const char*,const char* =0,const char* =0);
inline int HardwareSerial::printf(const char*, ...) { return 0; }
