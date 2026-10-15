#pragma once

class Switch {
public:
  Switch(int pin) {
    _pin = pin;
    pinMode(_pin, INPUT_PULLUP);
    _state = readSW();
    _count = 0;
    _hasChanged = _toggle = false;
  }
  void update() {
    if ((millis() - _lastMS) > 2) {
      _lastMS = millis();
      if (readSW() != _state) {
        if (++_count > 2) {  // debounce
          _hasChanged = true;
          _state = readSW();
          if (!_state) _toggle = !_toggle;
          _count = 0;
        }
      }
    }
  }
  bool isON() {
    return _state;
  }
  bool isOFF() {
    return !_state;
  }
  bool hasChanged() {
    bool ret = _hasChanged;
    _hasChanged = false;
    return ret;
  }
  bool turnedON() {
    if (hasChanged()) {
      return isON();
    }
    return false;
  }
  bool turnedOFF() {
    if (hasChanged()) {
      return isOFF();
    }
    return false;
  }
  bool toggleIsON() {
    return _toggle;
  }
private:
  int _pin;
  bool _state;
  int _count;
  bool _hasChanged;
  bool _toggle;
  unsigned long _lastMS;
  bool inline readSW() {
    return !(bool)digitalRead(_pin);
  }
};