// Machine constants for the 3RRS ball balancer.
//
// The geometry block must match cad/parameters.md and
// tools/verify_kinematics.py. If you change a link length in Fusion, change
// it in all three places or the platform will not go where the solver thinks
// it does.

#pragma once

// A0 and friends come from the core. The .ino gets Arduino.h for free, but a
// .cpp does not, so include it here or every translation unit that pulls in
// this file fails on the analog pin names.
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Geometry, millimetres and degrees
// ---------------------------------------------------------------------------

constexpr float BASE_MOTOR_R     = 78.0f;   // centre to crank pivot axis
constexpr float PLATFORM_JOINT_R = 95.0f;   // platform centre to ball centre
constexpr float CRANK_LEN        = 55.0f;   // crank pivot to knee
constexpr float COUPLER_LEN      = 95.0f;   // knee to ball centre, link_2 + tie rod
constexpr float MOTOR_SHAFT_Z    = 35.0f;   // pivot height above base plate top

constexpr float CRANK_HOME_DEG   = 20.0f;   // crank angle with the platform level
constexpr float CRANK_MIN_DEG    = -20.0f;  // mechanical travel, do not exceed
constexpr float CRANK_MAX_DEG    = 60.0f;

// Platform height, level, every crank at CRANK_HOME_DEG. Derived, but kept
// here as a constant so the control loop does no square roots at startup.
constexpr float HOME_PLATFORM_Z  = 119.444f;

// Legs are 120 degrees apart. Leg A is on the +X axis.
constexpr float LEG_PHI_DEG[3] = {0.0f, 120.0f, 240.0f};

// ---------------------------------------------------------------------------
// Steppers
// ---------------------------------------------------------------------------

constexpr int   MICROSTEPS      = 16;       // MS1 and MS2 tied high on the TMC2208
constexpr int   FULL_STEPS_REV  = 200;      // 1.8 degree NEMA 17
constexpr long  STEPS_PER_REV   = (long)FULL_STEPS_REV * MICROSTEPS;  // 3200
constexpr float DEG_PER_STEP    = 360.0f / (float)STEPS_PER_REV;      // 0.1125

// Set true per motor if a leg drives the wrong way. Easier than unplugging
// and reversing a motor pair, and it is what Stepper_Test is for.
constexpr bool  MOTOR_INVERT[3] = {false, false, false};

// Speed and acceleration scale with how far the motor still has to go, which
// is what keeps the platform from chattering when it is nearly still.
constexpr float SPEED_PER_STEP  = 36.0f;    // steps/s of speed per step of error
constexpr float ACCEL_PER_STEP  = 320.0f;   // steps/s^2 per step of error
constexpr float SPEED_MIN       = 320.0f;   // steps/s
constexpr float SPEED_MAX       = 14000.0f;
constexpr float ACCEL_MIN       = 3000.0f;
constexpr float ACCEL_MAX       = 180000.0f;

// ---------------------------------------------------------------------------
// Pins. Teensy 4.1.
// ---------------------------------------------------------------------------

constexpr int PIN_ENABLE   = 0;   // ENA on all three drivers, active low

constexpr int PIN_STEP[3]  = {1, 3, 5};
constexpr int PIN_DIR[3]   = {2, 4, 6};

// 4 wire resistive panel. If X and Y come out swapped, swap XP/YP with
// XM/YM here rather than rewiring. docs/calibration.md walks through it.
constexpr int PIN_TOUCH_XP = A0;
constexpr int PIN_TOUCH_YP = A1;
constexpr int PIN_TOUCH_XM = A2;
constexpr int PIN_TOUCH_YM = A3;

constexpr int TOUCH_PLATE_OHMS = 300;  // X plate resistance, only affects pressure

// ---------------------------------------------------------------------------
// Touch panel mapping
// ---------------------------------------------------------------------------

// Raw ADC span of the panel. Measure yours with the calibration sketch and
// put the numbers here; the defaults are a starting point, not a spec.
constexpr int TOUCH_RAW_X_MIN = 120;
constexpr int TOUCH_RAW_X_MAX = 910;
constexpr int TOUCH_RAW_Y_MIN = 130;
constexpr int TOUCH_RAW_Y_MAX = 900;

// Active area of the 8.4 inch panel, millimetres. The ball position is
// reported in millimetres from the centre so the gains have physical units.
constexpr float TOUCH_ACTIVE_W = 170.4f;
constexpr float TOUCH_ACTIVE_H = 127.8f;

// Pressure below this is noise or a lifted ball.
constexpr int TOUCH_PRESSURE_MIN = 40;
constexpr int TOUCH_PRESSURE_MAX = 1000;

// Mechanical centre rarely lands on the electrical centre. Trim these until
// a balanced ball sits in the middle of the plate.
constexpr float TOUCH_OFFSET_X = 0.0f;   // mm
constexpr float TOUCH_OFFSET_Y = 0.0f;   // mm

// ---------------------------------------------------------------------------
// Control loop
// ---------------------------------------------------------------------------

constexpr unsigned long LOOP_INTERVAL_US = 20000UL;  // 20 ms, 50 Hz

// Gains act on millimetres of error and produce degrees of platform tilt.
// Start here, then follow docs/tuning.md. These have not been tuned on
// hardware; see docs/validation.md.
constexpr float KP = 0.085f;   // deg per mm
constexpr float KI = 0.0012f;  // deg per mm-second
constexpr float KD = 0.040f;   // deg per mm/s

constexpr float TILT_LIMIT_DEG  = 12.0f;   // clamp, well inside the 15 deg envelope
constexpr float INTEGRAL_LIMIT  = 8.0f;    // anti windup, degrees of contribution

// Derivative is taken on a filtered velocity. 1.0 is no filtering.
constexpr float D_FILTER = 0.35f;

// Ball considered lost after this long with no valid touch.
constexpr unsigned long BALL_LOST_MS = 400UL;

// ---------------------------------------------------------------------------
// Startup
// ---------------------------------------------------------------------------

// The machine has no encoders or endstops. You push the platform all the way
// down before powering on, which puts every crank at CRANK_MIN_DEG, and the
// firmware takes that as its zero. Same bargain the reference design makes.
constexpr float STARTUP_CRANK_DEG = CRANK_MIN_DEG;

constexpr float HOMING_SPEED      = 1800.0f;   // steps/s
constexpr float HOMING_ACCEL      = 9000.0f;   // steps/s^2
