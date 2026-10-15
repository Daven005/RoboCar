#include "Types.h"
#include "LEDS.h"
#include "Sonar.h"
#include "Tones.h"
#include "Auto.h"
#include "AFMotor.h"
#include "Switch.h"
#include "cmdArduino.h"
#include <EEPROM.h>

AF_DCMotor rightWheel(1, MOTOR12_2KHZ);
AF_DCMotor leftWheel(2, MOTOR12_2KHZ);

#define onboardLED 13
#define redLEDpin 23
#define yellowLEDpin 25
#define green2LEDpin 27
#define blueLEDpin 29
#define leftEchoPin 33
#define leftTrigPin 35
#define rightEchoPin 37
#define rightTrigPin 39
#define switchPin 31
#define speakerPin 45

Sonar leftSonar(leftTrigPin, leftEchoPin, 200);
Sonar rightSonar(rightTrigPin, rightEchoPin, 200);
Switch sw1(switchPin);
LEDS *leds;
Auto autoRun;
int pins[] = { onboardLED, redLEDpin, yellowLEDpin, blueLEDpin, green2LEDpin };

Mode_t mode = OFF;
bool toneON = false;

Motion_t motion = STOP;
Turn_t turn = STRAIGHT;

uint8_t speedValues[] = { 0, 110, 180, 255 };

typedef struct {
  uint8_t leftRun;
  uint8_t leftSpeed;
  uint8_t rightRun;
  uint8_t rightSpeed;
  int yellowLED;  // Time ON out of 1000
  int green1LED;  // Time ON out of 1000
} Instruction_t;

Tones tones(speakerPin);

Instruction_t instructions[5][3]{
  // [Motion][Turn]
  //        STRAIGHT       LEFT            RIGHT
  { { RELEASE, STOP, RELEASE, STOP, 0, 0 },
    { RELEASE, STOP, RELEASE, STOP, 0, 0 },
    { RELEASE, STOP, RELEASE, STOP, 0, 0 } },  // STOP

  { { FORWARD, SLOW, FORWARD, SLOW, 100, 100 },
    { BACKWARD, 0, FORWARD, SLOW, 0, 100 },
    { FORWARD, SLOW, BACKWARD, 0, 100, 0 } },  // SLOW

  { { FORWARD, CRUISE, FORWARD, CRUISE, 300, 300 },
    { FORWARD, SLOW, FORWARD, CRUISE, 100, 300 },
    { FORWARD, CRUISE, FORWARD, SLOW, 300, 100 } },  // CRUISE

  { { FORWARD, FAST, FORWARD, FAST, 500, 500 },
    { FORWARD, CRUISE, FORWARD, FAST, 0, 0 },
    { FORWARD, FAST, FORWARD, CRUISE, 0, 0 } },  // FAST

  { { BACKWARD, SLOW, BACKWARD, SLOW, 0, 0 },
    { BACKWARD, SLOW, RELEASE, 0, 900, 0 },
    { RELEASE, 0, BACKWARD, SLOW, 0, 900 } },  // REVERSE
};
unsigned long motionStarted;
unsigned int motionDuration;
bool motionInProgress = false;
bool motorsRunning = false;
uint16_t battVolts;
const char *version = "Robo Car v0.0.6";

void printBoth(String &s) {
  Serial.print(s);
  Serial1.print(s);
}

void printBothln(String &s) {
  Serial.println(s);
  Serial1.println(s);
}

void printBoth(char *s) {
  Serial.print(s);
  Serial1.print(s);
}

void printBothln() {
  Serial.println();
  Serial1.println();
}

void printBothln(char *s) {
  Serial.println(s);
  Serial1.println(s);
}

void printBoth(int i) {
  Serial.print(i);
  Serial1.print(i);
}

void readConfig() {
  uint8_t values[10];
  for (int idx = 0; idx < sizeof(values); idx++) {
    values[idx] = EEPROM[idx];
  }
  if (values[0] != 255) {  // Implies not yet written
    speedValues[SLOW] = values[0];
    speedValues[CRUISE] = values[1];
    speedValues[FAST] = values[2];
    leftSonar.setMap(&values[3]);
    rightSonar.setMap(&values[3]);
  }
  Serial.print("Config: ");
  for (int idx = 0; idx < sizeof(values); idx++) {
    printBoth(values[idx]);
    printBoth(" ");
  }
  printBothln();
}

void flash(int argCnt, char **args) {
  if (argCnt < 4) {
    printBothln("Usage: flash <onTime> <offTime> <count>");
    return;
  }
  int c;
  leds->flash(blueLEDpin, cmd.conv(args[1], 10), cmd.conv(args[2], 10), c = cmd.conv(args[3], 10));
}

void emergencyStop() {
  motionInProgress = false;
  motion = STOP;
  turn = STRAIGHT;
  if (mode == AUTO) mode = MANUAL;
  leftWheel.run(4);
  leftWheel.setSpeed(0);
  rightWheel.run(4);
  rightWheel.setSpeed(0);
  motorsRunning = false;
  tone(3000, 1000);
}

void setMotors(int leftRun, int leftSpeed, int rightRun, int rightSpeed) {
  leftWheel.run(leftRun);
  leftWheel.setSpeed(leftSpeed);
  rightWheel.run(rightRun);
  rightWheel.setSpeed(rightSpeed);
  motorsRunning = ((leftRun != RELEASE) || (rightRun != RELEASE));
}

void testMotors(int argCnt, char **args) {
  if (argCnt < 5) {
    printBothln("Usage: motors <leftRun> <leftSpeed> <rightRun> <rightSpeed>");
    return;
  }
  if (mode == MANUAL) {
    setMotors(cmd.conv(args[1], 10), cmd.conv(args[2], 10), cmd.conv(args[3], 10), cmd.conv(args[4], 10));
  } else {
    printBothln("Mode not MANUAL");
    emergencyStop();
  }
}

void setMode(int argCnt, char **args) {
  if (argCnt < 2) {
    printBothln("Usage: mode off|manual|auto");
    return;
  }
  if (!strcmp(args[1], "off")) {
    if (mode != OFF && motorsRunning) emergencyStop();
    mode = OFF;
    motion = STOP;
  } else if (!strcmp(args[1], "manual")) {
    if (mode == AUTO && motorsRunning) emergencyStop();
    mode = MANUAL;
  } else if (!strcmp(args[1], "auto")) {
    mode = AUTO;
    autoRun.start();
  }
}

void setSound(int argCnt, char **args) {
  if (argCnt < 2) {
    printBothln("Usage: sound on|off");
    return;
  }
  if (!strcmp(args[1], "on")) toneON = true;
  else if (!strcmp(args[1], "off")) toneON = false;
}

void setRun(int argCnt, char **args) {
  if (argCnt < 2) {
    printBothln("Usage: run stop|slow|cruise|fast|reverse|straight|left|right|slower|faster");
    return;
  }
  if (!strcmp(args[1], "stop")) motion = STOP;
  else if (!strcmp(args[1], "slow")) motion = SLOW;
  else if (!strcmp(args[1], "cruise")) motion = CRUISE;
  else if (!strcmp(args[1], "fast")) motion = FAST;
  else if (!strcmp(args[1], "reverse")) {
    motion = REVERSE;
    turn = STRAIGHT;
  } else if (!strcmp(args[1], "straight")) turn = STRAIGHT;
  else if (!strcmp(args[1], "left")) {
    const Turn_t newTurn[] = { LEFT, LEFT, STRAIGHT };
    turn = newTurn[(int)turn];
  } else if (!strcmp(args[1], "right")) {
    const Turn_t newTurn[] = { RIGHT, STRAIGHT, RIGHT };
    turn = newTurn[(int)turn];
  } else if (!strcmp(args[1], "faster")) {
    const Motion_t newMotion[] = { SLOW, CRUISE, FAST, FAST, STOP };
    motion = newMotion[(int)motion];
  } else if (!strcmp(args[1], "slower")) {
    const Motion_t newMotion[] = { REVERSE, STOP, SLOW, CRUISE, STOP };
    motion = newMotion[(int)motion];
  } else {
    printBoth("Bad param: ");
    printBothln(args[1]);
  }
}

void printStatus(int argCnt, char **args) {
  String s = "mode1 ";
  printBoth(s + "Mode: " + mode + " Motion: " + motion + " Turn: " + turn);
  printBoth(s + " Distance: " + (int)leftSonar.distance() + "/" + (int)rightSonar.distance());
  printBoth(s + " Motors: ");
  if (motorsRunning) printBoth("running ");
  else printBoth("stopped ");
  printBothln(s + battVolts + "mV");
}

void continuousStatus() {
  static unsigned long lastMS = millis();

  if (millis() > (lastMS + 5000)) {
    printStatus(0, NULL);
    lastMS = millis();
  }
}

void setConfig(int argCnt, char **args) {
  if (argCnt > 1) {
    for (int idx = 1; idx < argCnt; idx++) {
      EEPROM.update(idx - 1, (uint8_t)cmd.conv(args[idx], 10));
    }
  }
  readConfig();
}

void help(int argCnt, char **args) {
  Serial1.print(F("Commands:\n  help\n  status\n"));
  Serial1.print(F("  mode off|manual|auto\n"));
  Serial1.print(F("  run stop|slow|cruise|fast|reverse|straight|left|right|slower|faster\n"));
  Serial1.print(F("  motors <leftRun> <leftSpeed> <rightRun> <rightSpeed>\n"));
  Serial1.print(F("  sound on|off\n"));
  Serial1.print(F("  config <slow> <cruise> <fast> <near> <close> <far>\n"));
  Serial1.print(F("  flash <onTime> <offTime> <count>\n"));
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  printBothln(version);
  readConfig();
  leds = new LEDS(pins, 5);
  leds->blink(onboardLED, 100, 1000);
  Serial1.setTimeout(500);
  cmd.begin(&Serial, &Serial1);
  cmd.add("flash", flash);
  cmd.add("mode", setMode);
  cmd.add("run", setRun);
  cmd.add("motors", testMotors);
  cmd.add("sound", setSound);
  cmd.add("status", printStatus);
  cmd.add("config", setConfig);
  cmd.add("help", help);
}

void startMotion(Motion_t m, Turn_t t, unsigned int duration) {
  String s;
  if (m != SAME) motion = m;
  turn = t;
  printBothln(s + "Manual motion " + motion + "/" + turn + " started");
  motionStarted = millis();
  motionDuration = duration;
  motionInProgress = true;
}

bool checkMotion() {
  String s;
  if (!motionInProgress) return false;
  if ((millis() - motionStarted) > motionDuration) {
    switch (mode) {
      case OFF: return false;
      case MANUAL:
        if (motion != STOP) {
          printBothln(s + "Manual motion " + motion + "/" + turn + " complete");
          motion = STOP;
          turn = STRAIGHT;
        }
        break;
      case AUTO:
        printBothln(s + "Auto motion " + motion + " complete");
        break;
    }
    motionInProgress = false;
    return true;
  }
  return false;
}

void checkDistance() {
  // Tone list IDs
  const int HIT = 0;
  const int EMERGENCY = 1;
  const int RUNNING = 2;

  Distance_t leftDistance, rightDistance;
  static Distance_t oldLeftDistance, oldRightDistance;
  static int reverseAwayCount = 0;
  String s;

  leftSonar.update();
  rightSonar.update();

  s = "";
  leftDistance = leftSonar.distance();
  if (oldLeftDistance != leftDistance) {
    oldLeftDistance = leftDistance;
    // printBothln(s = " Left->"+leftSonar.distanceStr());
  }
  rightDistance = rightSonar.distance();
  if (oldRightDistance != rightDistance) {
    oldRightDistance = rightDistance;
    // printBothln(s = " Right->"+rightSonar.distanceStr());
  }
  Distance_t closest = min(leftDistance, rightDistance);
  if (toneON && sw1.toggleIsON()) {
    switch (closest) {
      case NEAR:
        static Tone_t hit[] = { { 250, 400 }, { 0, 0 } };
        if (tones.playing() != HIT) {
          tones.play(HIT, hit, true);
        }
        break;
      case CLOSE:
        if (motion != STOP) {
          static Tone_t emergency[] = { { 2000, 700 }, { 500, 700 }, { 0, 0 } };
          if (tones.playing() != EMERGENCY) {
            tones.play(EMERGENCY, emergency, true);
          }
        }
        break;
      default:
        static Tone_t running[] = { { 3000, 100 }, { 2900, 100 }, { 2800, 100 }, { 2700, 100 }, { 2600, 100 }, { 2500, 100 }, { 0, 0 } };
        if (tones.playing() != RUNNING) {
          tones.play(RUNNING, running, true);
          break;
        }
    }
  } else {
    tones.stop();
  }
  if (mode != OFF && closest == NEAR && motion != REVERSE && motion != STOP) {
    if (++reverseAwayCount < 3) {
      Turn_t turnDirection = STRAIGHT;  //Default
      if (leftSonar.distance() == NEAR) {
        if (rightSonar.distance() != NEAR) turnDirection = RIGHT;
      } else {
        if (rightSonar.distance() == NEAR) turnDirection = LEFT;
      }
      printBothln(s + "Reverse away " + reverseAwayCount);
      startMotion(REVERSE, turnDirection, 2000);
    } else {
      motion = STOP;
      reverseAwayCount = 0;
    }
  }
}

void checkMotors() {
  static Motion_t lastMotion;
  static Turn_t lastTurn;
  String s;
  checkMotion();
  if (motion == lastMotion and turn == lastTurn) return;  // No change
  lastMotion = motion;
  lastTurn = turn;
  Instruction_t instr = instructions[motion][turn];
  leds->blink(yellowLEDpin, instr.yellowLED, 1000);
  leds->blink(blueLEDpin, instr.green1LED, 1000);
  printBothln(s + "Left: " + instr.leftRun + "/" + speedValues[instr.leftSpeed] + " Right: " + instr.rightRun + "/" + speedValues[instr.rightSpeed]);
  setMotors(instr.leftRun, speedValues[instr.leftSpeed], instr.rightRun, speedValues[instr.rightSpeed]);
}

void checkMode() {
  switch (mode) {
    case OFF: return;
    case MANUAL:
      checkMotors();
      break;
    case AUTO:
      if (checkMotion()) autoRun.check();
      checkMotors();
      break;
  }
}

void loop() {
  cmd.poll();
  leds->update();
  tones.update();
  sw1.update();
  checkMode();
  checkDistance();
  if (sw1.toggleIsON()) { continuousStatus(); }
  leds->set(yellowLEDpin, sw1.toggleIsON());
  battVolts = 1525L * analogRead(A0) / 100;
}