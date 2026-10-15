#pragma once
#include "Types.h"

void startMotion(Motion_t m, Turn_t t, unsigned int duration);

typedef struct {
  Motion_t m;
  Turn_t t;
  int duration;
} Prog_t;
Prog_t prog[] = {
  { FAST, STRAIGHT, 5000 },
  { SLOW, LEFT, 2000 },
  { FAST, STRAIGHT, 5000 },
  { SLOW, RIGHT, 2000 }
};
#define progLen (sizeof(prog) / sizeof(Prog_t))

class Auto {
public:
  void check() {
    startMotion(prog[idx].m, prog[idx].t, prog[idx].duration);
    if (++idx >= progLen) idx = 0;
  }
  void start() {
    startMotion(prog[0].m, prog[0].t, prog[0].duration);
    if (progLen > 1) idx = 1;
  }

private:
  int idx = 0;
};