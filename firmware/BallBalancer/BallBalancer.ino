// Ball balancer, Teensy 4.1.
//
// A 1 inch steel ball sits on an 8.4 inch resistive touch panel. The panel
// reports where it is, two PID loops work out how the platform should tilt to
// bring it back to the setpoint, and the inverse kinematics turn that tilt
// into three crank angles. Three NEMA 17 steppers on TMC2208 drivers hold
// those angles.
//
// Before power on, push the platform all the way down until the cranks hit
// their lower stop and hold it flat. There are no encoders and no endstops,
// so that position is the only zero the firmware gets. It then lifts to the
// level pose on its own.
//
// Serial console at 115200. Type "help".
//
// Libraries: AccelStepper, Adafruit TouchScreen. Versions in
// firmware/README.md.

#include <AccelStepper.h>
#include <TouchScreen.h>

#include "Config.h"
#include "Kinematics.h"
#include "Patterns.h"
#include "Pid.h"

namespace {

constexpr float DEG2RAD = 3.14159265358979f / 180.0f;
constexpr float RAD2DEG = 180.0f / 3.14159265358979f;

AccelStepper motors[3] = {
  AccelStepper(AccelStepper::DRIVER, PIN_STEP[0], PIN_DIR[0]),
  AccelStepper(AccelStepper::DRIVER, PIN_STEP[1], PIN_DIR[1]),
  AccelStepper(AccelStepper::DRIVER, PIN_STEP[2], PIN_DIR[2]),
};

TouchScreen panel(PIN_TOUCH_XP, PIN_TOUCH_YP, PIN_TOUCH_XM, PIN_TOUCH_YM,
                  TOUCH_PLATE_OHMS);

Pid pidX(KP, KI, KD);
Pid pidY(KP, KI, KD);

Pattern activePattern = Pattern::Hold;
unsigned long patternStartMs = 0;

unsigned long lastControlUs = 0;
unsigned long lastGoodTouchMs = 0;
bool haveBall = false;
bool telemetry = false;

float lastBallX = 0.0f;
float lastBallY = 0.0f;
float lastTiltX = 0.0f;
float lastTiltY = 0.0f;

// Live copies of the centre trim, so it can be nudged over serial while a
// ball is balancing instead of reflashing between every guess. Config.h
// holds the value you settle on.
float touchOffsetX = TOUCH_OFFSET_X;
float touchOffsetY = TOUCH_OFFSET_Y;

// Reads the panel and converts to millimetres from the centre. Returns false
// when there is no ball, or the reading is too light to trust.
bool readBall(float &xMm, float &yMm) {
  TSPoint raw = panel.getPoint();

  // getPoint leaves the shared pins configured for reading. Put the two that
  // double as digital outputs back, or the next read drifts.
  pinMode(PIN_TOUCH_XM, OUTPUT);
  pinMode(PIN_TOUCH_YP, OUTPUT);

  if (raw.z < TOUCH_PRESSURE_MIN || raw.z > TOUCH_PRESSURE_MAX) {
    return false;
  }

  const float fx = (float)(raw.x - TOUCH_RAW_X_MIN) /
                   (float)(TOUCH_RAW_X_MAX - TOUCH_RAW_X_MIN);
  const float fy = (float)(raw.y - TOUCH_RAW_Y_MIN) /
                   (float)(TOUCH_RAW_Y_MAX - TOUCH_RAW_Y_MIN);

  // A reading well outside the calibrated span is a glitch, not a ball at
  // the edge. The panel does this when the ball is lifted off.
  if (fx < -0.15f || fx > 1.15f || fy < -0.15f || fy > 1.15f) {
    return false;
  }

  xMm = (fx - 0.5f) * TOUCH_ACTIVE_W + touchOffsetX;
  yMm = (fy - 0.5f) * TOUCH_ACTIVE_H + touchOffsetY;
  return true;
}

// Speed and acceleration scale with distance still to travel. Close to the
// target the motors creep, far away they move. Without this the platform
// buzzes constantly, because every 20 ms correction is only a few steps and
// a fixed high acceleration turns each one into a jolt.
void driveTo(AccelStepper &motor, long target) {
  const long remaining = labs(target - motor.currentPosition());

  float speed = SPEED_PER_STEP * (float)remaining;
  if (speed < SPEED_MIN) speed = SPEED_MIN;
  if (speed > SPEED_MAX) speed = SPEED_MAX;

  float accel = ACCEL_PER_STEP * (float)remaining;
  if (accel < ACCEL_MIN) accel = ACCEL_MIN;
  if (accel > ACCEL_MAX) accel = ACCEL_MAX;

  motor.setMaxSpeed(speed);
  motor.setAcceleration(accel);
  motor.moveTo(target);
}

void commandPose(const PlatformPose &pose) {
  const LegSolution solution = kinematics::solve(pose);
  if (!solution.valid) {
    return;  // refuse poses that would drive a crank past its stop
  }
  for (int leg = 0; leg < 3; leg++) {
    driveTo(motors[leg], kinematics::crankRadToSteps(leg, solution.crankRad[leg]));
  }
}

void goLevel() {
  PlatformPose pose;
  pose.heightMm = HOME_PLATFORM_Z;
  pose.rollRad = 0.0f;
  pose.pitchRad = 0.0f;
  commandPose(pose);
}

void printHelp() {
  Serial.println(F("commands"));
  Serial.println(F("  kp <v> / ki <v> / kd <v>   set a gain on both axes"));
  Serial.println(F("  offset <x> <y>             trim the panel centre, mm"));
  Serial.println(F("  pattern <name>             hold line circle ellipse"));
  Serial.println(F("                             triangle rectangle figure8"));
  Serial.println(F("  level                      go flat and clear the PIDs"));
  Serial.println(F("  telemetry                  toggle the data stream"));
  Serial.println(F("  status                     print the current state"));
}

void printStatus() {
  Serial.print(F("gains kp="));
  Serial.print(pidX.kp(), 4);
  Serial.print(F(" ki="));
  Serial.print(pidX.ki(), 4);
  Serial.print(F(" kd="));
  Serial.println(pidX.kd(), 4);

  Serial.print(F("pattern "));
  Serial.print(patterns::name(activePattern));
  Serial.print(F("   ball "));
  Serial.println(haveBall ? F("present") : F("absent"));

  Serial.print(F("ball x="));
  Serial.print(lastBallX, 1);
  Serial.print(F(" y="));
  Serial.print(lastBallY, 1);
  Serial.print(F(" mm   tilt x="));
  Serial.print(lastTiltX, 2);
  Serial.print(F(" y="));
  Serial.print(lastTiltY, 2);
  Serial.println(F(" deg"));

  Serial.print(F("steps "));
  for (int i = 0; i < 3; i++) {
    Serial.print(motors[i].currentPosition());
    Serial.print(i < 2 ? F(", ") : F("\n"));
  }
}

float argFloat(const String &line, int index) {
  int start = 0;
  for (int i = 0; i < index; i++) {
    start = line.indexOf(' ', start);
    if (start < 0) return 0.0f;
    start++;
  }
  return line.substring(start).toFloat();
}

void handleSerial() {
  if (!Serial.available()) {
    return;
  }

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) {
    return;
  }

  if (line.startsWith("kp ")) {
    const float v = argFloat(line, 1);
    pidX.setGains(v, pidX.ki(), pidX.kd());
    pidY.setGains(v, pidY.ki(), pidY.kd());
    printStatus();
  } else if (line.startsWith("ki ")) {
    const float v = argFloat(line, 1);
    pidX.setGains(pidX.kp(), v, pidX.kd());
    pidY.setGains(pidY.kp(), v, pidY.kd());
    pidX.reset();
    pidY.reset();
    printStatus();
  } else if (line.startsWith("kd ")) {
    const float v = argFloat(line, 1);
    pidX.setGains(pidX.kp(), pidX.ki(), v);
    pidY.setGains(pidY.kp(), pidY.ki(), v);
    printStatus();
  } else if (line.startsWith("offset ")) {
    touchOffsetX = argFloat(line, 1);
    touchOffsetY = argFloat(line, 2);
    Serial.print(F("offset x="));
    Serial.print(touchOffsetX, 2);
    Serial.print(F(" y="));
    Serial.print(touchOffsetY, 2);
    Serial.println(F(" mm, copy this into Config.h once you are happy"));
  } else if (line.startsWith("pattern ")) {
    Pattern chosen;
    String arg = line.substring(8);
    arg.trim();
    if (patterns::fromName(arg.c_str(), chosen)) {
      activePattern = chosen;
      patternStartMs = millis();
      Serial.print(F("pattern "));
      Serial.println(patterns::name(activePattern));
    } else {
      Serial.println(F("unknown pattern"));
    }
  } else if (line == "level") {
    activePattern = Pattern::Hold;
    pidX.reset();
    pidY.reset();
    goLevel();
    Serial.println(F("level"));
  } else if (line == "telemetry") {
    telemetry = !telemetry;
    Serial.println(telemetry ? F("telemetry on") : F("telemetry off"));
  } else if (line == "status") {
    printStatus();
  } else if (line == "help") {
    printHelp();
  } else {
    Serial.println(F("unknown, type help"));
  }
}

void control() {
  const unsigned long now = micros();
  const float dt = (float)(now - lastControlUs) * 1e-6f;
  lastControlUs = now;

  float ballX = 0.0f;
  float ballY = 0.0f;
  const bool gotBall = readBall(ballX, ballY);

  if (gotBall) {
    lastGoodTouchMs = millis();
    if (!haveBall) {
      // Fresh ball. Start the derivative and integral clean.
      pidX.reset();
      pidY.reset();
      haveBall = true;
    }
    lastBallX = ballX;
    lastBallY = ballY;
  } else if (haveBall && (millis() - lastGoodTouchMs) > BALL_LOST_MS) {
    haveBall = false;
    pidX.reset();
    pidY.reset();
    goLevel();
  }

  if (!haveBall) {
    return;
  }

  const float seconds = (float)(millis() - patternStartMs) * 1e-3f;
  const Setpoint target = patterns::evaluate(activePattern, seconds);

  // Error is where the ball should be minus where it is. Tilting the
  // platform down in the direction of that error is what rolls the ball
  // back, so the sign carries straight through to the tilt.
  const float errorX = target.x - ballX;
  const float errorY = target.y - ballY;

  lastTiltY = pidX.update(errorX, dt);   // tilt about Y moves the ball in X
  lastTiltX = -pidY.update(errorY, dt);  // tilt about X moves the ball in Y

  PlatformPose pose;
  pose.heightMm = HOME_PLATFORM_Z;
  pose.rollRad = lastTiltX * DEG2RAD;
  pose.pitchRad = lastTiltY * DEG2RAD;
  commandPose(pose);

  if (telemetry) {
    Serial.print(ballX, 1);
    Serial.print(',');
    Serial.print(ballY, 1);
    Serial.print(',');
    Serial.print(errorX, 1);
    Serial.print(',');
    Serial.print(errorY, 1);
    Serial.print(',');
    Serial.print(lastTiltX, 2);
    Serial.print(',');
    Serial.println(lastTiltY, 2);
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);

  pinMode(PIN_ENABLE, OUTPUT);
  digitalWrite(PIN_ENABLE, HIGH);  // TMC2208 enable is active low, so off

  for (int i = 0; i < 3; i++) {
    motors[i].setMaxSpeed(HOMING_SPEED);
    motors[i].setAcceleration(HOMING_ACCEL);
    // Whatever the cranks are resting on is the zero, which is why you push
    // the platform down before switching on.
    motors[i].setCurrentPosition(0);
  }

  delay(300);
  digitalWrite(PIN_ENABLE, LOW);  // drivers live

  // Lift from the bottom stop to the level pose, slowly, before the control
  // loop takes over.
  goLevel();
  const unsigned long deadline = millis() + 6000UL;
  bool moving = true;
  while (moving && millis() < deadline) {
    moving = false;
    for (int i = 0; i < 3; i++) {
      if (motors[i].distanceToGo() != 0) {
        moving = true;
      }
      motors[i].run();
    }
  }

  lastControlUs = micros();
  patternStartMs = millis();

  Serial.println(F("ball balancer ready"));
  printHelp();
}

void loop() {
  // AccelStepper generates steps from run(), so it has to be called as often
  // as possible. The control maths only needs 50 Hz.
  for (int i = 0; i < 3; i++) {
    motors[i].run();
  }

  if ((micros() - lastControlUs) >= LOOP_INTERVAL_US) {
    control();
  }

  handleSerial();
}
