A) What the sketch does
RoboCar.ino controls a two-wheel robot with two ultrasonic sensors, LEDs, a speaker, a switch, and a serial command interface. It starts in OFF mode.

Manual mode: run commands select a motion and steering direction. The sketch maps those choices to motor directions and speeds, and uses the yellow and blue LEDs to indicate the selected motion. A run motion continues until another command changes it; the serial interface does not offer timed manual runs.
Auto mode: Repeats a fixed sequence from Auto.h: fast straight for 5 seconds, slow left for 2 seconds, fast straight for 5 seconds, then slow right for 2 seconds.
Obstacle response: The sonars are sampled about every 200 ms. Distances are categorized using thresholds from Sonar.h—defaults are 5, 20, and 50 cm. In either MANUAL or AUTO, if the closest reading is NEAR while moving forward, the sketch attempts a two-second reverse with a turn toward the clearer side. After three such escape attempts, it sets motion to STOP.
Switch, sound, and status: The switch in Switch.h is active-low and toggles an internal state. When that state is on, the yellow LED is lit and status is printed every five seconds. Audible sonar cues additionally require sound on; sound is off by default. Obstacle response does not depend on sound being enabled.
Battery: Each loop reads A0 and reports a calculated millivolt value. The conversion assumes the board’s voltage-divider scaling is appropriate for 1525L * analogRead(A0) / 100.
B) Command interface
The sketch listens on both Serial and Serial1 at 115200 baud. Commands are case-sensitive and split on spaces; enter one command per line. Available commands are registered in the sketch and implemented by the parser in cmdArduino.cpp.

Command	Effect
help	Prints the command list. The help implementation writes to Serial1 specifically.
status	Prints mode, motion, turn, sonar categories, motor state, and battery reading.
mode off|manual|auto	Selects operating mode. auto starts the sequence immediately.
run stop|slow|cruise|fast|reverse|straight|left|right|slower|faster	Changes motion or steering. left/right adjust the current turn; slower/faster step through motion levels.
motors <leftRun> <leftSpeed> <rightRun> <rightSpeed>	Sets raw motor direction/speed values, but only in MANUAL mode.
sound on|off	Enables or disables audible sonar cues.
config <slow> <cruise> <fast> <near> <close> <far>	Writes configuration bytes to EEPROM, then reloads and displays configuration.
flash <onTime> <offTime> <count>	Flashes the blue LED.
For example, mode manual followed by run slow starts slow forward motion. run left changes steering; it does not mean “turn left once and stop.” motors accepts raw numbers without range checking, so it is a lower-level control than run.

A couple of interface details: the parser accepts at most 30 tokens and clears an overlong command; the command buffer is 60 bytes. It tokenizes on spaces only. Some output is sent to both ports, but help is only sent to Serial1.

C) EEPROM usage
At startup, readConfig() reads 10 bytes from EEPROM. The sketch uses the first six as follows:

EEPROM address	Meaning	Default when EEPROM is blank
0	Slow motor speed	110
1	Cruise motor speed	180
2	Fast motor speed	255
3	NEAR distance threshold (cm)	5
4	CLOSE distance threshold (cm)	20
5	FAR distance threshold (cm)	50
6–9	Read and displayed, but not used by this sketch	—
If EEPROM address 0 is 255, the sketch treats the configuration as unwritten and keeps the defaults. Otherwise it loads addresses 0–2 as speed settings and 3–5 as sonar thresholds. The defaults are defined across RoboCar.ino and Sonar.h.

