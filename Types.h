#pragma once

typedef enum { STOP,
               SLOW,
               CRUISE,
               FAST,
               REVERSE,
               SAME } Motion_t;
const char *motion_str[] = { "STOP",
               "SLOW",
               "CRUISE",
               "FAST",
               "REVERSE",
               "SAME" };
const char *motionStr() { return motion_str[(int)motion]; }
typedef enum { STRAIGHT,
               LEFT,
               RIGHT } Turn_t;
typedef enum { OFF,
               MANUAL,
               AUTO } Mode_t;