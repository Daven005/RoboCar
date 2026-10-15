#pragma once

typedef enum { NEAR,
               CLOSE,
               FAR,
               BEYOND } Distance_t;
static uint8_t distanceMap[] = { 5, 20, 50 };
static const Distance_t distanceValues[] = { NEAR, CLOSE, FAR, BEYOND };
static const String distanceStrings[] = { "NEAR", "CLOSE", "FAR", "BEYOND" };
class Sonar {
public:

  Sonar(int trig, int echo, long repeatTime) {
    pinMode(_trig = trig, OUTPUT);
    pinMode(_echo = echo, INPUT_PULLUP);
    digitalWrite(_trig, LOW);
    _repeatTime = repeatTime;
    _lastMS = millis();
    _lastDistance = 0;
  }
  bool update() {
    if ((millis() - _lastMS) > _repeatTime) {
      digitalWrite(_trig, HIGH);
      delayMicroseconds(5);
      digitalWrite(_trig, LOW);
      unsigned long duration = pulseIn(_echo, HIGH, 30000UL);
      // The speed of sound is 340 m/s or 29 microseconds per centimeter.
      _lastDistance = duration == 0 ? 0xFFFFUL : duration / 29 / 2;
      _lastMS = millis();
      return true;
    }
    return false;
  }
  Distance_t distance() {
    for (int idx = 0; idx < sizeof(distanceMap); idx++) {
      if (_lastDistance < distanceMap[idx]) {
        return distanceValues[idx];
      }
    }
    return distanceValues[sizeof(distanceMap)];
  }
  String distanceStr() {
    return distanceStrings[(int) distance()];
  }
  void setMap(uint8_t distance[]) {
    for (int idx=0; idx< sizeof(distanceMap); idx++) {
      distanceMap[idx] = distance[idx];
    }
  }
private:
  int _trig, _echo;
  unsigned long _repeatTime, _lastMS, _lastDistance;
};