#pragma once
#include "LED.h"
class LEDS {
public:
  LEDS(int pins[], int count) {
    _count = count;
    leds = new LED[_count];
    for (int idx = 0; idx < _count; idx++) {
      leds[idx].begin(pins[idx]);
    }
  }
  void update() {
    for (int idx = 0; idx < _count; idx++) {
      leds[idx].update();
    }
  }
  void set(int pin, bool set) {
    LED *led = get(pin);
    if (led != NULL) led->set(set);    
  }
  void setON(int pin) {
    LED *led = get(pin);
    if (led != NULL) led->setON();
  }
  void setOFF(int pin) {
    LED *led = get(pin);
    if (led != NULL) led->setOFF();
  }
  void blink(int pin, int onTime, int offTime) {
    LED *led = get(pin);
    if (led != NULL) led->blink(onTime, offTime);
  }
  void flash(int pin, int onTime, int offTime, int count) {
    LED *led = get(pin);
    if (led != NULL) led->flash(onTime, offTime, count);
  }
  long elapsed(int pin) {
    LED *led = get(pin);
    if (led != NULL) return led->elapsed();
    return 0;
  }
private:
  int _count;
  LED *leds;
  LED *get(int pin) {
    for (int idx = 0; idx < _count; idx++) {
      if (leds[idx].pin() == pin) return &leds[idx];
    }
    return NULL;
  }
};