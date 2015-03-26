#include <OSSex.h>
int fade_cos(int seq) {
  Toy.step[0] = Toy.step[1] = Toy.step[2] = round(127 * cos((seq / (8*PI))-PI) + 127);
  Toy.step[3] = 50;
  return 1;
}// Turn on all outputs slightly offset from each other.
int flicker(int seq) {
  // set all motors initally to -1, ie "leave it alone"
  Toy.step[0] = Toy.step[1] = Toy.step[2] = -1;

  if (seq > 2) {
    Toy.step[3] = 200;
  } else {
    Toy.step[3] = 20;
  }

  seq %= 3;
  Toy.step[seq] = 80;

  return 1;
}
// Second motor only
int second(int seq) {
  Toy.step[0] = 0;
  Toy.step[1] = 100;
  Toy.step[2] = 0;
  Toy.step[3] = 50;
  return 1;
}
// Third motor only
int third(int seq) {
  Toy.step[0] = 0;
  Toy.step[1] = 0;
  Toy.step[2] = 100;
  Toy.step[3] = 50;
  return 1;
}
// First motor only
// Why have a 50ms timing on the step (Toy.step[3]) ? This lets you adjust the power of the pattern,
// so that instead of running [100, 0, 0, 50] the whole time, it might become [120, 0, 0, 50] after a button click
int first(int seq) {
  Toy.step[0] = 100;
  Toy.step[1] = 0;
  Toy.step[2] = 0;
  Toy.step[3] = 50;
  return 1;
}
// Randomly blip an output on for a short burst.
int pulse(int seq) {
  if (seq % 2) {
    Toy.step[0] = Toy.step[1] = Toy.step[2] = 0;
  } else {
    Toy.step[random(0,3)] = 144;
  }

  Toy.step[3] = 70;
  return 1;
}
// Opposite of pulse() -- turn on all outputs, randomly blip one off
int pulse2(int seq) {
  if (seq % 2) {
    Toy.step[0] = Toy.step[1] = Toy.step[2] = 100;
  } else {
    Toy.step[random(0,3)] = 0;
  }

  Toy.step[3] = 100;
  return 1;
}
int sharp_ramp(int seq) {
  // neat exponential sequence inspired by github/jgeisler0303
  const uint8_t fadeTable[32] = {0, 1, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 9, 10, 12, 15, 17, 21, 25, 30, 36, 43, 51, 61, 73, 87, 104, 125, 149, 178, 213, 255};
  seq %= 32;
  Toy.step[0] = Toy.step[1] = Toy.step[2] = fadeTable[seq];
  Toy.step[3] = 12 ;
}
int weird2(int seq) {
  Toy.step[2] = round(127*cos(tan(tan(seq/(8*PI)))-PI/2)+127);
  Toy.step[3] = 30;
  return 1;
}
int weird3(int seq) {
  Toy.step[2] = round(50*(cos(seq/(8*PI)+PI/2) + sin(seq/2))+100);
  Toy.step[3] = 30;
  return 1;
}
// Click handler. Currently moves to next pattern.
void click() {
  Toy.cyclePattern();;
}
// Double click handler
void doubleClick() {
  Toy.increasePower();;
}
// Cycle through all the outputs, turn the LED on and leave it on to show that we're on
void startupSequence() {
  int outs = Toy.device.outCount;
  int outInterval = 100 / outs;

  Toy.setLED(0,128);

  for (int i = 0; i < outs; i++) {
    Toy.setOutput(i, 100+outInterval*i);
    delay(100);
    Toy.setOutput(i, 0);
    delay(50);
  }
  for (int i = 0; i < 3; i++) {
    Toy.setLED(0,0);
    delay(100);
    Toy.setLED(0,128);
    delay(100);
  }
}

void setup() {

Toy.addPattern(fade_cos);
Toy.addPattern(flicker);
Toy.addPattern(second);
Toy.addPattern(third);
Toy.addPattern(first);
Toy.addPattern(pulse);
Toy.addPattern(pulse2);
Toy.addPattern(sharp_ramp);Toy.addPattern(weird2);
Toy.addPattern(weird3);
Toy.setID(ALPHA);

Toy.attachClick(click);
Toy.attachDoubleClick(doubleClick);
// Start the Serial console
Serial.begin(9600);
// Blip all the motors and flash the LED to show that everything is working and the device is on.
startupSequence();
}

void loop() {
// Serial console. Read a character in to command[1], and a value in to val
  char command[1];
  byte val;

  if (Serial.available() > 0) {
    Serial.readBytes(command,1);
    if (command[0] == 'l') { // Set LED power
      val = Serial.parseInt();
      Toy.setLED(0,val);
      Serial.println(val);
    } else if (command[0] == '0' || command[0] == '1' || command[0] == '2') { // Set power of individual motor
      val = Serial.parseInt();
      Toy.setOutput(command[0], val);
      Serial.println(val);
    } else if (command[0] == '-') { // Catch '-1', set power of all motors
      int out = Serial.parseInt();
      out *= -1;
      val = Serial.parseInt();
      Toy.setOutput(out,val);
    } else if (command[0] == '{') {
      int motors[3];
      motors[0] = Serial.parseInt();
      motors[1] = Serial.parseInt();
      motors[2] = Serial.parseInt();
      for (int i = 0; i < 3; i++) {
        if (motors[i] >= 0) {
          Toy.setOutput(i, motors[i]);
        }
      }
    } else if (command[0] == 'p') {
      Serial.println(Toy.decreasePower());
    } else if (command[0] == 'P') {
      Serial.println(Toy.increasePower());
    } else if (command[0] == 't') { // Decrease pattern time, as in everything goes faster
      Serial.println(Toy.decreaseTime());
    } else if (command[0] == 'T') {
      Serial.println(Toy.increaseTime());
    } else if (command[0] == 'r') { // Run a specific pattern, r,0; r,1; etc.
      val = Serial.parseInt();
      Toy.runPattern(val);
      Serial.println(Toy.getPattern());
    } else if (command[0] == 'g') {  // Get number of currently running pattern
      Serial.println(Toy.getPattern());
    } else if (command[0] == 's') {
      Toy.stop();
    } else if (command[0] == 'c') {
      Toy.cyclePattern();
      Serial.println(Toy.getPattern());
    } else if (command[0] == 'i') { // Read input 0 or 1 and print it to serial port
      int in = Serial.parseInt();
      in %= Toy.device.inCount;
      Serial.println(Toy.getInput(in));
    }
  }
}