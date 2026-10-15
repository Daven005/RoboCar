#pragma once

#include <avr/pgmspace.h>
#include <Arduino.h>

#define MAX_MSG_SIZE    60
#include <stdint.h>

// command line structure
typedef struct _cmd_t
{
    char *cmd;
    void (*func)(int argc, char **argv);
    struct _cmd_t *next;
} cmd_t;

class Cmd
{
public:
    Cmd(){}
    void begin(HardwareSerial *ser1 = NULL, HardwareSerial *ser2 = NULL);
    void poll();
    void add(const char *name, void (*func)(int argc, char **argv));
    uint32_t conv(char *str, uint8_t base=10);

private:
    void display();
    void parse(char *cmd);
    void handler(HardwareSerial *serial, uint8_t port);
    HardwareSerial *_ser1; 
    HardwareSerial *_ser2; 
};

extern Cmd cmd;
