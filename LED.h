#pragma once
class LED {
public:
  LED() {}
  void begin(int pin) {
    pinMode(_pin = pin, OUTPUT);
    digitalWrite(_pin, 0);
    status = OFF;
  }
  void update() {
    switch (status) {
      case OFF:
      case ON: break;
      case BLINK_ON:
        if ((millis() - lastMS) > _onTime) {
          status = BLINK_OFF;
          digitalWrite(_pin, 0);
          lastMS = millis();
        }
      case BLINK_OFF:
        if ((millis() - lastMS) > _offTime) {
          if (_count == 0) {
            digitalWrite(_pin, 0);
            status = OFF;
          } else {
            if (_count > 0) _count--;
            status = BLINK_ON;
            digitalWrite(_pin, 1);
            lastMS = millis();
          }
        }
    }
  }
  void set(bool set) {
    digitalWrite(_pin, set);
    status = set ? ON : OFF;
  }
  void setON() {
    digitalWrite(_pin, 1);
    status = ON;
  }
  void setOFF() {
    digitalWrite(_pin, 0);
    status = OFF;
  }
  void blink(int onTime, int offTime) {
    _onTime = onTime;
    _offTime = offTime;
    _count = -1;
    status = BLINK_ON;
    digitalWrite(_pin, 1);
    startTime = lastMS = millis();
  }
  void flash(int onTime, int offTime, int count) {
    _onTime = onTime;
    _offTime = offTime;
    if (count >= 1) {
      _count = count - 1;
      status = BLINK_ON;
      digitalWrite(_pin, 1);
      startTime = lastMS = millis();
    }
  }
  long elapsed() {
    return (millis() - startTime);
  }
  int pin() {
    return _pin;
  }
private:
  int _pin;
  enum { OFF,
         ON,
         BLINK_ON,
         BLINK_OFF } status;
  int _onTime;
  int _offTime;
  int _count;
  unsigned long lastMS;
  long startTime;
};