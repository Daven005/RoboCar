#pragma once

typedef enum { STOP,
               SLOW,
               CRUISE,
               FAST,
               REVERSE,
               SAME } Motion_t;
typedef enum { STRAIGHT,
               LEFT,
               RIGHT } Turn_t;
typedef enum { OFF,
               MANUAL,
               AUTO } Mode_t;