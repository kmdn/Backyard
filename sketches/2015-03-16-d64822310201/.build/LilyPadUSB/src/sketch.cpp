#include <Arduino.h>
#include <OSSex.h>// Third motor only
int third(int seq);
int first(int seq);
int second(int seq);
int pulse(int seq);
int fadeCos(int seq);
int weird3(int seq);
int flicker(int seq);
int weird2(int seq);
int fadeOffset(int seq);
int pulse2(int seq);
void click();
void doubleClick();
void setup();
void loop();
#line 1 "src/sketch.ino"
//#include <OSSex.h>// Third motor only
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
  Toy.step[0] = 200;
  Toy.step[1] = 0;
  Toy.step[2] = 0;
  Toy.step[3] = 50;
  return 1;
}// Second motor only
int second(int seq) {
  Toy.step[0] = 0;
  Toy.step[1] = 100;
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
}int fadeCos(int seq) {
  Toy.step[0] = Toy.step[1] = Toy.step[2] = round(127 * cos((seq / (8*PI))-PI) + 127);
  Toy.step[3] = 50;
  return 1;
}int weird3(int seq) {
  Toy.step[2] = round(50*(cos(seq/(8*PI)+PI/2) + sin(seq/2))+100);
  Toy.step[3] = 30;
  return 1;
}
// Turn on all outputs slightly offset from each other.
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
}int weird2(int seq) {
  Toy.step[2] = round(127*cos(tan(tan(seq/(8*PI)))-PI/2)+127);
  Toy.step[3] = 30;
  return 1;
}
int fadeOffset(int seq) {
  // cos sequence takes 158 steps to run. start motor 1 a third of the way in (53 steps), start motor 2 2/3 (106 steps) of the way in.
  Toy.step[0] = round(127 * cos((seq / (8*PI))-PI) + 127);
  if (seq >= 58) {
    Toy.step[1] = round(127 * cos(((seq-58) / (8*PI))-PI) + 127);
  } else if (seq >= 106) {
    Toy.step[2] = round(127 * cos(((seq-106) / (8*PI))-PI) + 127);
  }
  Toy.step[3] = 50;
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
// Click handler. Currently moves to next pattern.
void click() {
  Toy.cyclePattern();
}// Double click handler
void doubleClick() {
  Toy.increasePower();
}

void setup() {
Toy.setID(BETA);
Toy.setPowerScale(0.2);Toy.setTimeScale(0.1);
Toy.addPattern(third);Toy.addPattern(first);Toy.addPattern(second);
Toy.addPattern(pulse);Toy.addPattern(fadeCos);Toy.addPattern(weird3);
Toy.addPattern(flicker);Toy.addPattern(weird2);
Toy.addPattern(fadeOffset);Toy.addPattern(pulse2);Toy.attachClick(click);Toy.attachDoubleClick(doubleClick);
}

void loop() {}