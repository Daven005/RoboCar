#pragma once

typedef struct {
  int freq;
  int duration;
} Tone_t;
class Tones {
public:
  Tones(int pin) {
    currentList = NULL;
    _pin = pin;
  };
  void play(int id, Tone_t *toneList, bool repeat) {
    currentList = toneList;
    rpt = repeat;
    idx = 0;
    playID = id;
    tone(_pin, currentList[idx].freq, currentList[idx].duration);
    lastMS = millis();
  }
  int playing() {
    return playID;
  }
  void update() {
    long diff;
    if (currentList == NULL) return;
    if ((diff = millis() - lastMS) > currentList[idx].duration) {
      if (currentList[idx + 1].duration == 0) {
        if (!rpt) {
          stop();
          return;
        }
        idx = 0;
      } else idx++;
      tone(_pin, currentList[idx].freq, currentList[idx].duration);
      String s;
      // Serial.println(s+currentList[idx].freq +"/"+ currentList[idx].duration+"  "+diff+" "+millis()+"-"+lastMS);
      lastMS = millis();
    }
  }
  void stop() {
    noTone(_pin);
    currentList = NULL;
    playID = 0;
  }
private:
  Tone_t *currentList;
  int idx;
  unsigned long lastMS;
  bool rpt;
  int _pin;
  int playID;
};