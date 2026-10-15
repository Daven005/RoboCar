#include "cmdArduino.h"

// command line message buffer and pointer
static uint8_t msg[2][MAX_MSG_SIZE];
static uint8_t *msg_ptr[2];

// linked list for command table
static cmd_t *cmd_tbl_list, *cmd_tbl;

// text strings for command prompt (stored in flash)
const char cmd_prompt[] PROGMEM = ">> ";
const char cmd_unrecog[] PROGMEM = "CMD: Command not recognized.";

Cmd cmd;

/**************************************************************************/
/*!
    Generate the main command prompt
*/
/**************************************************************************/
void Cmd::display() {
  char buf[50];

  strcpy_P(buf, cmd_prompt);
  _ser1->print(buf);
  if (_ser2 != NULL) _ser2->print(buf);
}

/**************************************************************************/
/*!
    Parse the command line. This function tokenizes the command input, then
    searches for the command table entry associated with the commmand. Once found,
    it will jump to the corresponding function.
*/
/**************************************************************************/
void Cmd::parse(char *cmd) {
  uint8_t argc = 0;
  char *argv[31];
  char buf[50];
  cmd_t *cmd_entry;

  fflush(stdout);

  // parse the command line statement and break it up into space-delimited
  // strings. the array of strings will be saved in the argv array.
  char *token = strtok(cmd, " ");
  while (token != NULL && argc < 30) {
    argv[argc++] = token;
    token = strtok(NULL, " ");
  }
  argv[argc] = NULL;

  if (argc == 0) {
    display();
    return;
  }

  // parse the command table for valid command. used argv[0] which is the
  // actual command name typed in at the prompt
  for (cmd_entry = cmd_tbl; cmd_entry != NULL; cmd_entry = cmd_entry->next) {
    if (!strcmp(argv[0], cmd_entry->cmd)) {
      cmd_entry->func(argc, argv);
      display();
      return;
    }
  }

  // command not recognized. print message and re-generate prompt.
  strcpy_P(buf, cmd_unrecog);
  _ser1->println(buf);
  if (_ser2 != NULL) _ser2->println(buf);

  display();
}

/**************************************************************************/
/*!
    This function processes the individual characters typed into the command
    prompt. It saves them off into the message buffer unless its a "backspace"
    or "enter" key. 
*/
/**************************************************************************/
void Cmd::handler(HardwareSerial *serial, uint8_t port) {
  char c = serial->read();
  switch (c) {
    case '\r':
    case '\n':
      if (msg[port][0] == 0) break; // msg is empty (allows for \r\n)
      // terminate the msg and reset the msg ptr. then send
      // it to the handler for processing.
      _ser1->println((char *)msg[port]);
      *msg_ptr[port] = '\0';
      _ser1->print("\n");
      if (_ser2 != NULL) _ser2->print("\n");
      parse((char *)msg[port]);
      memset(msg[port], 0, sizeof(msg[port]));
      msg_ptr[port] = msg[port];
      break;

    case '\b':
      // backspace
      _ser1->print(c);
      if (_ser2 != NULL) _ser2->print(c);
      if (msg_ptr[port] > msg[port]) {
        msg_ptr[port]--;
      }
      break;

    default:
      // normal character entered. add it to the buffer
      // _ser1->print(c);
      if (_ser2 != NULL) _ser2->print(c);
      *msg_ptr[port]++ = c;

      // msg too long, clear command and display warning
      if ((msg_ptr[port] - msg[port]) == (MAX_MSG_SIZE - 1)) {
        _ser1->println("Command too long. Pleaes reduce command size.");
        if (_ser2 != NULL) _ser2->println("Command too long. Pleaes reduce command size.");
        memset(msg[port], 0, sizeof(msg[port]));
        msg_ptr[port] = msg[port];
      }
      break;
  }
}

/**************************************************************************/
/*!
    This function should be set inside the main loop. It needs to be called
    constantly to check if there is any available input at the command prompt.
*/
/**************************************************************************/
void Cmd::poll() {
  while (_ser1->available()) {
    handler(_ser1, 0);
  }
  if (_ser2 != NULL && _ser2 != _ser1) {
    while (_ser2->available()) {
      handler(_ser2, 1);
    }
  }
}

/**************************************************************************/
/*!
    Initialize the command line interface. This sets the terminal speed and
    and initializes things. 
*/
/**************************************************************************/
void Cmd::begin(HardwareSerial *ser1 = NULL, HardwareSerial *ser2 = NULL) {
  msg_ptr[0] = msg[0];
  msg_ptr[1] = msg[1];
  memset(msg, 0, sizeof(msg));

  cmd_tbl_list = NULL;  // init the command table

  if (ser1 == NULL) {
    _ser1 = &Serial;
  } else {
    _ser1 = ser1;
  }
  if (ser2 == NULL) {
    _ser2 = &Serial1;
  } else {
    _ser2 = ser2;
  }
}

/**************************************************************************/
/*!
    Add a command to the command table. The commands should be added in
    at the setup() portion of the sketch. 
*/
/**************************************************************************/
void Cmd::add(const char *name, void (*func)(int argc, char **argv)) {
  // alloc memory for command struct
  cmd_tbl = (cmd_t *)malloc(sizeof(cmd_t));

  // alloc memory for command name
  char *cmd_name = (char *)malloc(strlen(name) + 1);

  // copy command name
  strcpy(cmd_name, name);

  // terminate the command name
  cmd_name[strlen(name)] = '\0';

  // fill out structure
  cmd_tbl->cmd = cmd_name;
  cmd_tbl->func = func;
  cmd_tbl->next = cmd_tbl_list;
  cmd_tbl_list = cmd_tbl;
}

/**************************************************************************/
/*!
    Convert a string to a number. The base must be specified, ie: "32" is a
    different value in base 10 (decimal) and base 16 (hexadecimal).
*/
/**************************************************************************/
uint32_t Cmd::conv(char *str, uint8_t base) {
  return strtol(str, NULL, base);
}
