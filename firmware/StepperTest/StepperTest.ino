// Direction and wiring check. Run this before BallBalancer.ino.
//
// Each motor lifts its crank a little, pauses, and lowers it back. All three
// should rise together. If one drops while the others rise, that leg's coil
// pair is reversed: either swap one coil pair at the driver, or flip that
// leg's entry in MOTOR_INVERT in Config.h and reflash. The second way is
// easier and does exactly the same thing.
//
// Give yourself room. The platform can be off for this, and it is easier to
// watch the cranks with it off.

#include <AccelStepper.h>

#include "Config.h"

namespace {

AccelStepper motors[3] = {
  AccelStepper(AccelStepper::DRIVER, PIN_STEP[0], PIN_DIR[0]),
  AccelStepper(AccelStepper::DRIVER, PIN_STEP[1], PIN_DIR[1]),
  AccelStepper(AccelStepper::DRIVER, PIN_STEP[2], PIN_DIR[2]),
};

// 10 degrees of crank, far short of either stop.
const long TEST_STEPS = (long)(10.0f / DEG_PER_STEP);

void moveAll(long target) {
  for (int i = 0; i < 3; i++) {
    long signedTarget = MOTOR_INVERT[i] ? -target : target;
    motors[i].moveTo(signedTarget);
  }
  while (motors[0].distanceToGo() || motors[1].distanceToGo() ||
         motors[2].distanceToGo()) {
    for (int i = 0; i < 3; i++) {
      motors[i].run();
    }
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(PIN_ENABLE, OUTPUT);
  digitalWrite(PIN_ENABLE, HIGH);

  for (int i = 0; i < 3; i++) {
    motors[i].setMaxSpeed(1200.0f);
    motors[i].setAcceleration(4000.0f);
    motors[i].setCurrentPosition(0);
  }

  delay(300);
  digitalWrite(PIN_ENABLE, LOW);

  Serial.println(F("stepper test"));
  Serial.print(F("moving each crank "));
  Serial.print(TEST_STEPS);
  Serial.println(F(" microsteps up, then back"));
  Serial.println(F("all three cranks should rise together"));
}

void loop() {
  Serial.println(F("up"));
  moveAll(TEST_STEPS);
  delay(1500);

  Serial.println(F("down"));
  moveAll(0);
  delay(1500);
}
